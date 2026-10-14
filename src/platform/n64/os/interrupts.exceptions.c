#include "parrot/core/math.h"
#include "parrot/platform/n64/os.h"
#include "src/platform/n64/os/os.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define INSTRUCTIONS_AROUND 2

typedef enum {
    ExceptionReadStage_NONE = 0,
    ExceptionReadStage_HANDLING,
    ExceptionReadStage_READING,
    ExceptionReadStage_FAIL,
} ExceptionReadStage;

static bool handle_page_exception(ParrotN64OSInterruptFrame *frame) {
    if (frame->badvaddr < 0x1000) {
        printf("Parrot: Refusing to resolve page exceptions with NULL pages\n");
        return false;
    }

    uint32_t virtual_page = frame->badvaddr / 4096;
    uint32_t level1_index = (virtual_page >> 10) & 0x3FF;
    uint32_t level2_index = virtual_page & 0x3FF;

    if (_Parrot_n64_os_page_table_handlers[level1_index]) {
        ParrotOSPageFaultHandler handler = _Parrot_n64_os_page_table_handlers[level1_index][level2_index];
        if (handler) {
            handler((void *)(uintptr_t)frame->badvaddr);
        }
    }

    if (!_Parrot_n64_os_page_table[level1_index]) {
        return false;
    }

    ParrotOSPageTableFlags flags =
        ParrotOSPageTableEntry_UNPACK_FLAGS(_Parrot_n64_os_page_table[level1_index][level2_index]);

    if (!(flags & ParrotOSPageTableFlags_VALID)) {
        return false;
    }

    uint32_t tlb_flags = (1 << 1); // Valid

    if (!(flags & ParrotOSPageTableFlags_UNCACHED)) {
        tlb_flags |= (1 << 3); // Cached
    }

    if (flags & ParrotOSPageTableFlags_WRITE) {
        tlb_flags |= (1 << 2); // Dirty
    }

    uint32_t physical_page = _Parrot_n64_os_page_table[level1_index][level2_index] & 0xFFFFF;

    uint32_t entry_hi, entry_lo0, entry_lo1;
    uint32_t virtual_page2 = virtual_page >> 1;

    entry_hi = (virtual_page2 << 13);

    if ((virtual_page & 1) == 0) {
        entry_lo0 = (physical_page << 6) | tlb_flags;
        entry_lo1 = 0;
    } else {
        entry_lo0 = 0;
        entry_lo1 = (physical_page << 6) | tlb_flags;
    }

    __asm__ volatile("mtc0 %0, $10\n" // EntryHi
                     "mtc0 %1, $2\n"  // EntryLo0
                     "mtc0 %2, $3\n"  // EntryLo1
                     "mtc0 %3, $5\n"  // PageMask

                     "tlbwr\n"
                     "nop\n"
                     :
                     : "r"(entry_hi), "r"(entry_lo0), "r"(entry_lo1), "r"(0));

    return true;
}

static void try_print_translated_address(uint32_t address) {
    if (address >= 0x80000000) {
        return;
    }

    void *translated = Parrot_n64_os_get_physical_address((void *)(uintptr_t)address);

    if (translated) {
        printf(" (Physical: 0x%08X)", translated);
    }
}

bool _Parrot_n64_os_exception_handler(ParrotN64OSInterruptFrame *frame, uint32_t exception) {
    static volatile ExceptionReadStage read_stage = ExceptionReadStage_NONE;
    static volatile uint32_t read_stage_exception = 0;

    switch (read_stage) {
    case ExceptionReadStage_READING: {
        frame->pc += 4;
        read_stage = ExceptionReadStage_FAIL;
        read_stage_exception = exception;
        return false;
    }
    case ExceptionReadStage_HANDLING: {
        for (;;)
            ;
    }
    default:
        break;
    }

    switch (exception) {
    case 0x02:
    case 0x03: {
        // TLB Exception
        if (handle_page_exception(frame)) {
            return true;
        }
    } break;
    case 0x08:
        // Syscall
        frame->pc += 4;

        switch (frame->regs[1]) {
        case PARROT_N64_OS_SYS_YIELD:
            break;
        case PARROT_N64_OS_SYS_SAVE_FRAME:
            *(ParrotN64OSInterruptFrame *)(uintptr_t)frame->regs[2] = *frame;
            break;
        case PARROT_N64_OS_SYS_LOAD_FRAME:
            *frame = *(ParrotN64OSInterruptFrame *)(uintptr_t)frame->regs[2];
            break;
        default:
            PARROT_FAIL_FMT("Bad syscall %d", (int)frame->regs[1]);
            break;
        }

        return true;
    case 0x09:
        // Breakpoint
        return false;
    default:
        break;
    }

    read_stage = ExceptionReadStage_HANDLING;

    static const char *messages[] = {
        "Interrupt",
        "TLB Modification (Write to read only address)",
        "TLB Miss (Load)",
        "TLB Miss (Store)",
        "Address Error (Load)",
        "Address Error (Store)",
        "Bus Error (Fetch)",
        "Bus Error (Load/Store)",
        "Syscall (BUG)",
        "Breakpoint (BUG)",
        "Reserved Instruction",
        "Coprocessor Unusable",
        "Arithmetic Overflow",
        "Trap",
        "Unknown (0x0E)",
        "Floating Point",
        "Unknown (0x10)",
        "Unknown (0x11)",
        "Unknown (0x12)",
        "Unknown (0x13)",
        "Unknown (0x14)",
        "Unknown (0x15)",
        "Unknown (0x16)",
        "Watch Address",
        "Unknown (0x18)",
        "Unknown (0x19)",
        "Unknown (0x1A)",
        "Unknown (0x1B)",
        "Unknown (0x1C)",
        "Unknown (0x1D)",
        "Unknown (0x1E)",
        "Unknown (0x1F)",
    };

    printf("\n");
    printf("Parrot Exception Handler\n");
    printf("##########################################################################################\n");
    printf("Exception: %s\n", messages[exception]);

    if (_Parrot_n64_os_interrupt_count - 1 > 0) {
        printf("HINT: This exception was raised while handling an interrupt\n");
    }

    if (!_Parrot_n64_os_scheduler_task) {
        printf("HINT: This exception was raised while not running any task\n");
    }

    if (frame->badvaddr < 0x1000) {
        printf("HINT: This exception likely came from %s\n",
               frame->pc != frame->badvaddr ? "dereferencing a NULL pointer" : "calling a NULL function pointer");
    }

    printf("\n");

    {
        printf("PC: 0x%08x", frame->pc);
        try_print_translated_address(frame->pc);
        putchar('\n');
    }
    printf("Cause: 0x%08x\n", frame->cause);
    {
        printf("BadVAddr: 0x%08x", frame->badvaddr);
        try_print_translated_address(frame->badvaddr);
        putchar('\n');
    }
    printf("\n");

    const char *register_names[] = {
        "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "s0",
        "s1", "s2", "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra",
    };

    for (size_t i = 0; i < 31; i += 2) {
        for (size_t j = i; j < PARROT_MIN(i + 2, 31); j++) {
            printf("$%s: 0x%08x%08x ", register_names[j], frame->regs[j] >> 32, frame->regs[j] & 0xFFFFFFFF);
        }
        printf("\n");
    }
    printf("\n");

    volatile uint32_t instruction_stream[INSTRUCTIONS_AROUND * 2 + 1];
    uint32_t instruction_stream_exception[INSTRUCTIONS_AROUND * 2 + 1] = {0};

    uint32_t pc_start = frame->pc - INSTRUCTIONS_AROUND * 4;
    volatile uint32_t *instructions_start = (volatile uint32_t *)(uintptr_t)pc_start;
    for (size_t i = 0; i < INSTRUCTIONS_AROUND * 2 + 1; i++) {
        read_stage = ExceptionReadStage_READING;
        read_stage_exception = 0;
        {
            instruction_stream[i] = instructions_start[i];
            instruction_stream_exception[i] = read_stage_exception;
        }
        read_stage = ExceptionReadStage_HANDLING;
    }

    printf("Instruction Stream:\n");
    printf("\n");
    for (size_t i = 0; i < INSTRUCTIONS_AROUND * 2 + 1; i++) {
        char highlight = ' ';

        if (i == INSTRUCTIONS_AROUND) {
            highlight = '>';
        }

        printf(" %c 0x%08x: 0x", highlight, (uint32_t)(pc_start + i * sizeof(uint32_t)));
        if (instruction_stream_exception[i] == 0) {
            printf("%08X", instruction_stream[i]);
        } else {
            printf("???????? (Got %s)", messages[instruction_stream_exception[i]]);
        }
        printf("\n");
    }

    printf("\n");
    printf("##########################################################################################\n");

    for (;;)
        ;
}
