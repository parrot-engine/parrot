#include "src/platform/n64/os/rom.h"
#include "parrot/core/binary.h"
#include "parrot/core/math.h"
#include "parrot/core/util.h"
#include "parrot/platform/n64/os.h"
#include "parrot/platform/n64/util.h"
#include "src/platform/n64/os/os.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

static bool rom_read(ParrotScope *scope, size_t offset, uint8_t *out) {
    (void)scope;

    // For some reason on the N64, you can't read from ROM in 8-bit chunks

    uint32_t shift = (3 - (offset % sizeof(uint32_t))) * 8;
    uint32_t data = *(volatile uint32_t *)(0xB0000000 + PARROT_ALIGN_DOWN(offset, sizeof(uint32_t)));
    *out = (data >> shift) & 0xFF;
    return true;
}

static bool find_elf(ParrotBuffer *buffer) {
    size_t old_position = ParrotBuffer_rtell(buffer);
    {
        for (uint32_t i = 0; i < 0xFC00000; i += 0x100) {
            ParrotBuffer_rseek(buffer, i);

            uint32_t magic = 0;
            ParrotBuffer_read32(buffer, ParrotBufferEndian_BIG, &magic);

            if (magic == 0x7F454C46) {
                ParrotBuffer_rseek(buffer, i);
                return true;
            }
        }
    }
    ParrotBuffer_rseek(buffer, old_position);

    return false;
}

typedef struct {
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
} ProgramHeader;

static ProgramHeader parse_program_header(ParrotBuffer *buffer) {
    ProgramHeader header = {0};
    ParrotBuffer_read32(buffer, ParrotBufferEndian_BIG, &header.p_type);
    ParrotBuffer_read32(buffer, ParrotBufferEndian_BIG, &header.p_offset);
    ParrotBuffer_read32(buffer, ParrotBufferEndian_BIG, &header.p_vaddr);
    ParrotBuffer_read32(buffer, ParrotBufferEndian_BIG, &header.p_paddr);
    ParrotBuffer_read32(buffer, ParrotBufferEndian_BIG, &header.p_filesz);
    ParrotBuffer_read32(buffer, ParrotBufferEndian_BIG, &header.p_memsz);
    ParrotBuffer_read32(buffer, ParrotBufferEndian_BIG, &header.p_flags);
    ParrotBuffer_read32(buffer, ParrotBufferEndian_BIG, &header.p_align);
    return header;
}

static size_t rom_elf_offset = 0;
static uint32_t e_phoff = 0;
static uint16_t e_phnum = 0;

#define PI_DRAM_ADDR (*(volatile uint32_t *)0xA4600000)
#define PI_CART_ADDR (*(volatile uint32_t *)0xA4600004)
#define PI_RD_LEN (*(volatile uint32_t *)0xA4600008)
#define PI_WR_LEN (*(volatile uint32_t *)0xA460000C)
#define PI_STATUS (*(volatile uint32_t *)0xA4600010)

static void handle_rom_page_fault(void *address) {
    PARROT_RET_COND(Parrot_n64_os_get_physical_address_raw(address));

    void *page_address = (void *)PARROT_ALIGN_DOWN((uintptr_t)address, 0x1000);

    ParrotScope *scope = ParrotScope_new(NULL);

    ParrotBuffer *buffer = ParrotBuffer_new(NULL, rom_read, NULL);
    ParrotScope_push(scope, ParrotBuffer_vdelete, buffer);

    ParrotBuffer_rseek(buffer, rom_elf_offset + e_phoff);
    for (size_t i = 0; i < e_phnum; i++) {
        ProgramHeader header = parse_program_header(buffer);

        if ((uintptr_t)address < header.p_paddr) {
            continue;
        }

        if ((uintptr_t)address >= header.p_paddr + header.p_memsz) {
            continue;
        }

        size_t elf_segment_offset = (uintptr_t)page_address - header.p_paddr;
        void *page = Parrot_n64_os_alloc_continous_pages(1);
        memset(page, 0, 4096);

        if (header.p_filesz > elf_segment_offset) {
            size_t remaining_size = PARROT_MIN(header.p_filesz - elf_segment_offset, 4096);

            ParrotBuffer_rseek(buffer, rom_elf_offset + header.p_offset + elf_segment_offset);
            if (ParrotBuffer_rtell(buffer) % 2 == 0) {
                while (PI_STATUS & /* DMA busy */ 1)
                    ;

                Parrot_n64_writeback_invalidiate_data_cache(page, 4096);

                PI_DRAM_ADDR = (uintptr_t)Parrot_n64_os_get_physical_uncached_address(page) & 0xFFFFFF;
                PI_CART_ADDR = 0x10000000 + ParrotBuffer_rtell(buffer);
                PI_WR_LEN = remaining_size - 1;

                while (PI_STATUS & /* DMA busy */ 1)
                    ;
            } else {
                ParrotBuffer_read(buffer, ParrotBufferEndian_HOST, page, remaining_size);
                Parrot_n64_writeback_invalidiate_data_cache(page, 4096);
            }
        }

        ParrotOSPageTableFlags flags = 0;

        if (header.p_flags & /* Write */ (1 << 1)) {
            flags |= ParrotOSPageTableFlags_WRITE;
        }

        if (header.p_flags & /* Exec */ (1 << 0)) {
            flags |= ParrotOSPageTableFlags_EXEC;
            Parrot_n64_invalidiate_instruction_cache(page, 4096);
        }

        _Parrot_n64_os_map_page((uintptr_t)page_address / 4096, ((uintptr_t)page - 0x80000000) / 4096, flags);
        break;
    }

    ParrotScope_delete(scope);
}

void _Parrot_n64_os_parse_rom(void) {
    ParrotScope *scope = ParrotScope_new(NULL);

    ParrotBuffer *buffer = ParrotBuffer_new(NULL, rom_read, NULL);
    ParrotScope_push(scope, ParrotBuffer_vdelete, buffer);

    PARROT_FAIL_COND_MSG(!find_elf(buffer), "Could not find ELF in ROM");

    rom_elf_offset = ParrotBuffer_rtell(buffer);

    unsigned char e_indent[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_shnum;
    uint16_t e_shentsize;
    uint16_t e_shstrndx;

    for (size_t i = 0; i < 16; i++) {
        uint8_t ascii;
        ParrotBuffer_read8(buffer, &ascii);
        e_indent[i] = Parrot_ascii_to_char(ascii);
    }

    ParrotBuffer_read16(buffer, ParrotBufferEndian_BIG, &e_type);
    ParrotBuffer_read16(buffer, ParrotBufferEndian_BIG, &e_machine);
    ParrotBuffer_read32(buffer, ParrotBufferEndian_BIG, &e_version);
    ParrotBuffer_read32(buffer, ParrotBufferEndian_BIG, &e_entry);
    ParrotBuffer_read32(buffer, ParrotBufferEndian_BIG, &e_phoff);
    ParrotBuffer_read32(buffer, ParrotBufferEndian_BIG, &e_shoff);
    ParrotBuffer_read32(buffer, ParrotBufferEndian_BIG, &e_flags);
    ParrotBuffer_read16(buffer, ParrotBufferEndian_BIG, &e_ehsize);
    ParrotBuffer_read16(buffer, ParrotBufferEndian_BIG, &e_phentsize);
    ParrotBuffer_read16(buffer, ParrotBufferEndian_BIG, &e_phnum);
    ParrotBuffer_read16(buffer, ParrotBufferEndian_BIG, &e_shentsize);
    ParrotBuffer_read16(buffer, ParrotBufferEndian_BIG, &e_shnum);
    ParrotBuffer_read16(buffer, ParrotBufferEndian_BIG, &e_shstrndx);

    (void)e_indent;

    ParrotBuffer_rseek(buffer, rom_elf_offset + e_phoff);
    for (size_t i = 0; i < e_phnum; i++) {
        ProgramHeader header = parse_program_header(buffer);

        if (header.p_paddr < 0x1000) {
            continue;
        }

        if (header.p_paddr % 0x1000 != 0) {
            continue;
        }

        if (header.p_paddr >= 0x80000000) {
            continue;
        }

        for (uint32_t address = header.p_paddr; address < header.p_paddr + header.p_memsz; address += 0x1000) {
            _Parrot_n64_os_map_page_handler(address / 4096, handle_rom_page_fault);
        }
    }

    (void)handle_rom_page_fault;
}
