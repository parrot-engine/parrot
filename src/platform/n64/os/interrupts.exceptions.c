#include "parrot/core/math.h"
#include "parrot/core/util.h"
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

bool _Parrot_n64_os_exception_handler(ParrotN64OSInterruptFrame *frame, uint32_t exception) {
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

    _Parrot_os_crash_frame = frame;
    Parrot_crash_handler("%s", messages[exception]);

    for (;;)
        ;
}
