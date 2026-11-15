#include "parrot/platform/n64/os.h"
#include "src/platform/n64/os/boot_data.h"
#include "src/platform/n64/os/os.h"
#include <stdint.h>

extern uint8_t Parrot_n64_os_free_mem_begin;

static uint8_t page_bitmap[256] = {0};

static void mark_page_used(uintptr_t addr) {
    uint32_t page = (addr - 0x80000000) / 4096;
    int index = page / 8;
    int bit = page % 8;
    page_bitmap[index] |= 1 << bit;
}

static void mark_page_free(uintptr_t addr) {
    uint32_t page = (addr - 0x80000000) / 4096;
    int index = page / 8;
    int bit = page % 8;
    page_bitmap[index] &= ~(1 << bit);
}

void _Parrot_n64_os_map_page(uint32_t virtual_page, uint32_t physical_page, ParrotOSPageTableFlags flags) {
    uint32_t level1_index = (virtual_page >> 10) & 0x3FF;
    if (!_Parrot_n64_os_page_table[level1_index]) {
        _Parrot_n64_os_page_table[level1_index] = Parrot_n64_os_alloc_continous_pages(1);
        memset(_Parrot_n64_os_page_table[level1_index], 0, 4096);
    }

    _Parrot_n64_os_page_table[level1_index][virtual_page & 0x3FF] =
        ParrotOSPageTableEntry_PACK(physical_page, flags | ParrotOSPageTableFlags_VALID);
}

void _Parrot_n64_os_map_page_handler(uint32_t virtual_page, ParrotOSPageFaultHandler handler) {
    uint32_t level1_index = (virtual_page >> 10) & 0x3FF;
    if (!_Parrot_n64_os_page_table_handlers[level1_index]) {
        _Parrot_n64_os_page_table_handlers[level1_index] = Parrot_n64_os_alloc_continous_pages(1);
        memset(_Parrot_n64_os_page_table_handlers[level1_index], 0, 4096);
    }

    _Parrot_n64_os_page_table_handlers[level1_index][virtual_page & 0x3FF] = handler;
}

void *Parrot_n64_os_alloc_pages(size_t count, ParrotOSPageTableFlags flags) {
    (void)flags;
    // TODO: Virtual memory
    return Parrot_n64_os_alloc_continous_pages(count);
}

void *Parrot_n64_os_alloc_continous_pages(size_t count) {
    PARROT_FAIL_COND(count == 0);

    const uint32_t total_pages = sizeof(page_bitmap) * 8;

    uint32_t run_start = 0;
    uint32_t run_length = 0;

    for (uint32_t page = 0; page < total_pages; page++) {
        int index = (int)(page / 8);
        int bit = (int)(page % 8);

        if (!(page_bitmap[index] & (1 << bit))) {
            if (run_length == 0) {
                run_start = page;
            }

            if (++run_length == count) {
                for (uint32_t i = run_start; i < run_start + count; i++) {
                    mark_page_used(0x80000000 + i * 4096);
                }

                return (void *)(uintptr_t)(0x80000000 + run_start * 4096);
            }
        } else {
            run_length = 0;
        }
    }

    return NULL;
}

void Parrot_n64_os_free_page(void *addr) {
    mark_page_free((uintptr_t)addr);
}

void *Parrot_n64_os_get_physical_address(void *address) {
    if ((uintptr_t)address >= 0x80000000) {
        return address;
    }

    size_t page = (uintptr_t)address / 4096;
    size_t level1_index = (((uintptr_t)address / 4096) >> 10) & 0x3FF;

    if (_Parrot_n64_os_page_table_handlers && _Parrot_n64_os_page_table_handlers[level1_index]) {
        ParrotOSPageFaultHandler handler = _Parrot_n64_os_page_table_handlers[level1_index][page & 0x3FF];
        if (handler) {
            handler((void *)(uintptr_t)address);
        }
    }

    return Parrot_n64_os_get_physical_address_raw(address);
}

void *Parrot_n64_os_get_physical_uncached_address(void *address) {
    if ((uintptr_t)address >= 0xA0000000) {
        return address;
    }

    return (uint8_t *)Parrot_n64_os_get_physical_address(address) + 0x20000000;
}

void *Parrot_n64_os_get_physical_address_raw(void *address) {
    if ((uintptr_t)address >= 0x80000000) {
        return address;
    }

    size_t page = (uintptr_t)address / 4096;
    size_t page_offset = (uintptr_t)address % 4096;

    size_t level1_index = (page >> 10) & 0x3FF;

    if (!_Parrot_n64_os_page_table || !_Parrot_n64_os_page_table[level1_index]) {
        return NULL;
    }

    ParrotOSPageTableEntry entry = _Parrot_n64_os_page_table[level1_index][page & 0x3FF];
    if (!(ParrotOSPageTableEntry_UNPACK_FLAGS(entry) & ParrotOSPageTableFlags_VALID)) {
        return NULL;
    }

    return (void *)((uintptr_t)(entry & 0xFFFFF) * 4096 + page_offset + 0x80000000);
}

void *Parrot_n64_os_get_physical_uncached_address_raw(void *address) {
    if ((uintptr_t)address >= 0xA0000000) {
        return address;
    }

    return (uint8_t *)Parrot_n64_os_get_physical_address_raw(address) + 0x20000000;
}

void _Parrot_n64_os_init_paging(void) {
    memset(page_bitmap, 0xFF, sizeof(page_bitmap));

    for (uintptr_t addr = 0x400; addr < LibdragonBootData_avaliable_memory_bytes; addr += 4096) {
        mark_page_free((uintptr_t)&Parrot_n64_os_free_mem_begin + addr);
    }

    _Parrot_n64_os_page_table = Parrot_n64_os_alloc_continous_pages(1);
    memset(_Parrot_n64_os_page_table, 0, 4096);
    _Parrot_n64_os_page_table_handlers = Parrot_n64_os_alloc_continous_pages(1);
    memset(_Parrot_n64_os_page_table_handlers, 0, 4096);
}

ParrotOSPageTableEntry **_Parrot_n64_os_page_table = NULL;
ParrotOSPageFaultHandler **_Parrot_n64_os_page_table_handlers = NULL;
