#include "parrot/platform/n64/os.h"
#include "parrot/core/os.h"
#include "parrot/core/scheduler.h"
#include "parrot/core/util.h"
#include "parrot/platform/n64/util.h"
#include "src/platform/n64/os/boot_data.h"
#include "src/platform/n64/os/libc/stdlib.malloc.h"
#include "src/platform/n64/os/os.h"
#include "src/platform/n64/os/rom.h"
#include <stdio.h>
#include <string.h>

extern uint8_t _gp;
extern uint8_t Parrot_n64_os_free_mem_begin;

extern uint8_t Parrot_n64_os_interrupt_handler_entry;
extern uint32_t Parrot_n64_os_interrupt_handler_entry_size;

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

int main(int argc, char *argv[]);

#define STACK_SIZE (8192)

static uint64_t Parrot_os_get_performance_counter_impl(void) {
    uint32_t count;
    uint32_t upper;

    do {
        upper = _Parrot_n64_os_upper_count;
        __asm__ volatile("mfc0 %0, $9\n" : "=r"(count));
    } while (upper != _Parrot_n64_os_upper_count);

    return ((uint64_t)upper << 32) | count;
}

static uint64_t Parrot_os_get_performance_frequency_impl(void) {
    return 46875000;
}

static void Parrot_os_sleep_impl(double seconds) {
    if (seconds == 0) {
        if (_Parrot_n64_os_interrupt_count > 0) {
            return;
        }

        __asm__ volatile("li $v0, %0\n"
                         "syscall\n"
                         :
                         : "i"(PARROT_N64_OS_SYS_YIELD)
                         : "v0");
        return;
    }

    uint64_t begin = Parrot_os_get_performance_counter();
    while (Parrot_os_get_performance_counter() - begin < seconds * Parrot_os_get_performance_frequency()) {
        Parrot_os_yield();
    }
}

struct ParrotOSThreadState {
    ParrotN64OSInterruptFrame frame;
    ParrotN64OSInterruptFrame return_frame;
};

void _ParrotOSThreadState_new_impl_return(void);

static ParrotOSThreadState *
ParrotOSThreadState_new_impl(void (*func)(void *ctx), void *ctx, void *stack, size_t stack_size) {
    PARROT_FAIL_NULL(func);
    PARROT_FAIL_NULL(ctx);
    PARROT_FAIL_NULL(stack);

    ParrotOSThreadState *self = PARROT_ALLOC(ParrotOSThreadState);

    __asm__ volatile("li $v0, %0\n"
                     "move $v1, %1\n"
                     "syscall\n"
                     :
                     : "i"(PARROT_N64_OS_SYS_SAVE_FRAME), "r"(&self->frame)
                     : "v0", "v1");

    self->frame.pc = (uintptr_t)func;

    for (size_t i = 0; i < 31; i++) {
        self->frame.regs[i] = 0;
    }

    self->frame.regs[3] = /* $a0 */ (uintptr_t)ctx;
    self->frame.regs[27] = /* $gp */ (uintptr_t)&_gp;
    self->frame.regs[28] = /* $sp */ ((uint64_t)0xffffffff << 32) | ((uintptr_t)stack + stack_size);

    self->frame.regs[15] = /* $s0 */ ((uint64_t)0xffffffff << 32) | (uintptr_t)&self->return_frame;

    *(uintptr_t *)(uintptr_t)self->frame.regs[28] = (uintptr_t)&self->return_frame;
    self->frame.regs[30] =
        /* $ra */ ((uint64_t)0xffffffff << 32) | (uintptr_t)_ParrotOSThreadState_new_impl_return;

    self->frame.reg_hi = 0;
    self->frame.reg_lo = 0;

    return self;
}

static void ParrotOSThreadState_delete_impl(ParrotOSThreadState *self) {
    PARROT_FAIL_NULL(self);

    free(self);
}

void _ParrotOSThreadState_restore_impl_swap(ParrotN64OSInterruptFrame *load, ParrotN64OSInterruptFrame *save);

static void ParrotOSThreadState_restore_impl(ParrotOSThreadState *self, ParrotOSThreadState **out_current) {
    PARROT_FAIL_NULL(self);
    PARROT_FAIL_NULL(out_current);

    if (out_current) {
        if (!*out_current) {
            *out_current = malloc(sizeof(ParrotOSThreadState));
        }
        **out_current = (ParrotOSThreadState){0};
    }

    volatile bool called = false;

    __asm__ volatile("li $v0, %0\n"
                     "move $v1, %1\n"
                     "syscall\n"
                     :
                     : "i"(PARROT_N64_OS_SYS_SAVE_FRAME), "r"(&self->return_frame)
                     : "v0", "v1");

    if (!called) {
        called = true;
        _ParrotOSThreadState_restore_impl_swap(&self->frame, out_current ? &(*out_current)->frame : &self->return_frame);
    }
}

void _Parrot_n64_os_main_return(void);

void Parrot_n64_os_main(void) {
    memcpy((void *)0x80000000, &Parrot_n64_os_interrupt_handler_entry, Parrot_n64_os_interrupt_handler_entry_size);
    Parrot_n64_writeback_invalidiate_data_cache((void *)0x80000000, Parrot_n64_os_interrupt_handler_entry_size);
    Parrot_n64_invalidiate_instruction_cache((void *)0x80000000, Parrot_n64_os_interrupt_handler_entry_size);

    memcpy((void *)0x80000180, &Parrot_n64_os_interrupt_handler_entry, Parrot_n64_os_interrupt_handler_entry_size);
    Parrot_n64_writeback_invalidiate_data_cache((void *)0x80000180, Parrot_n64_os_interrupt_handler_entry_size);
    Parrot_n64_invalidiate_instruction_cache((void *)0x80000180, Parrot_n64_os_interrupt_handler_entry_size);

    memset(page_bitmap, 0xFF, sizeof(page_bitmap));

    for (uintptr_t addr = 0x400; addr < LibdragonBootData_avaliable_memory_bytes; addr += 4096) {
        mark_page_free((uintptr_t)&Parrot_n64_os_free_mem_begin + addr);
    }

    _Parrot_n64_os_page_table = Parrot_n64_os_alloc_continous_pages(1);
    memset(_Parrot_n64_os_page_table, 0, 4096);
    _Parrot_n64_os_page_table_handlers = Parrot_n64_os_alloc_continous_pages(1);
    memset(_Parrot_n64_os_page_table_handlers, 0, 4096);

    if (!Parrot_os_get_performance_counter) {
        Parrot_os_get_performance_counter = Parrot_os_get_performance_counter_impl;
        Parrot_os_get_performance_frequency = Parrot_os_get_performance_frequency_impl;
    }

    if (!Parrot_os_sleep) {
        Parrot_os_sleep = Parrot_os_sleep_impl;
    }

    if (!ParrotOSThreadState_new) {
        ParrotOSThreadState_new = ParrotOSThreadState_new_impl;
    }

    if (!ParrotOSThreadState_delete) {
        ParrotOSThreadState_delete = ParrotOSThreadState_delete_impl;
    }

    if (!ParrotOSThreadState_restore) {
        ParrotOSThreadState_restore = ParrotOSThreadState_restore_impl;
    }

    _Parrot_n64_os_heap_init();

    _Parrot_n64_os_parse_rom();

    ParrotScheduler *scheduler = ParrotScheduler_new(ParrotN64OSTask);

    if ((uintptr_t)main >= 0x80000000) {
        printf("Parrot: main() is in kseg0-1 and will not be able to use demand paging\n");
    }

    ParrotN64OSTask *main_task = ParrotScheduler_create_task(scheduler, ParrotN64OSTask);
    ParrotScheduler_set_task_priority(main_task, 1);

    main_task->pc = (uintptr_t)main;

    static char *argv[] = {
        "/app",
        NULL,
    };

    main_task->regs[3] = /* $a0 */ 1;
    main_task->regs[4] = /* $a1 */ ((uint64_t)0xffffffff << 32) | (uintptr_t)argv;

    main_task->regs[27] = /* $gp */ (uintptr_t)&_gp;
    main_task->regs[28] = /* $sp */ ((uint64_t)0xffffffff << 32) | ((uintptr_t)malloc(STACK_SIZE) + STACK_SIZE);

    main_task->regs[30] =
        /* $ra */ ((uint64_t)0xffffffff << 32) | (uintptr_t)_Parrot_n64_os_main_return;

    _Parrot_n64_os_scheduler = scheduler;

    __asm__ volatile("li $t0, 1\n"
                     "mtc0 $t0, $11\n"
                     "mfc0 $t0, $12\n"
                     "ori $t0, $t0, 0x8001\n"
                     "mtc0 $t0, $12\n"
                     "mtc0 $zero, $9\n"
                     :
                     :
                     : "t0");

    for (;;)
        ;
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

ParrotOSPageTableEntry **_Parrot_n64_os_page_table = NULL;
ParrotOSPageFaultHandler **_Parrot_n64_os_page_table_handlers = NULL;

ParrotScheduler *_Parrot_n64_os_scheduler = NULL;
ParrotN64OSTask *_Parrot_n64_os_scheduler_task = NULL;

uint32_t _Parrot_n64_os_interrupt_count = 0;
uint32_t _Parrot_n64_os_upper_count = 0;
