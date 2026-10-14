#ifndef PARROT_PARROT_SRC_PLATFORM_N64_OS_OS_H_
#define PARROT_PARROT_SRC_PLATFORM_N64_OS_OS_H_

#include "parrot/config.h"
#include "parrot/core/scheduler.h"
#include <stdint.h>

PARROT_PACK_BEGIN()
typedef struct {
    uint64_t regs[31];
    uint64_t reg_hi;
    uint64_t reg_lo;
    uint64_t badvaddr;
    uint32_t pc;
    uint32_t cause;
    uint32_t status;
} ParrotN64OSInterruptFrame;
PARROT_PACK_END()

typedef struct {
    uint64_t regs[31];
    uint64_t reg_hi;
    uint64_t reg_lo;
    uint64_t pc;
} ParrotN64OSTask;

enum {
    PARROT_N64_OS_SYS_YIELD = 0,

    PARROT_N64_OS_SYS_SAVE_FRAME,
    PARROT_N64_OS_SYS_LOAD_FRAME,
};

typedef uint32_t ParrotOSPageTableEntry;
typedef void (*ParrotOSPageFaultHandler)(void *address);

#define ParrotOSPageTableEntry_PACK(physical_page, flags)                                                               \
    ((ParrotOSPageTableEntry)(((physical_page) & 0xFFFFF) | ((flags) << 20)))
#define ParrotOSPageTableEntry_UNPACK_FLAGS(entry) ((ParrotOSPageTableFlags)((entry) >> 20))

extern ParrotOSPageTableEntry **_Parrot_n64_os_page_table;
extern ParrotOSPageFaultHandler **_Parrot_n64_os_page_table_handlers;

extern ParrotScheduler *_Parrot_n64_os_scheduler;
extern ParrotN64OSTask *_Parrot_n64_os_scheduler_task;

extern uint32_t _Parrot_n64_os_interrupt_count;
extern uint32_t _Parrot_n64_os_upper_count;

void _Parrot_n64_os_map_page(uint32_t virtual_page, uint32_t physical_page, ParrotOSPageTableEntry flags);
void _Parrot_n64_os_map_page_handler(uint32_t virtual_page, ParrotOSPageFaultHandler handler);

#endif // PARROT_PARROT_SRC_PLATFORM_N64_OS_OS_H_
