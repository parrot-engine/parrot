#include "parrot/platform/n64/os.h"
#include "parrot/core/os.h"
#include "parrot/core/scheduler.h"
#include "parrot/core/util.h"
#include "src/platform/n64/os/os.h"
#include <stdio.h>
#include <string.h>

extern uint8_t _gp;

void _Parrot_n64_os_init_crash_impl(void);

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

void _Parrot_n64_os_init_impl(void) {
    _Parrot_n64_os_init_crash_impl();

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
}

ParrotScheduler *_Parrot_n64_os_scheduler = NULL;
ParrotN64OSTask *_Parrot_n64_os_scheduler_task = NULL;

ParrotN64OSInterruptFrame *_Parrot_os_crash_frame = NULL;

uint32_t _Parrot_n64_os_interrupt_count = 0;
uint32_t _Parrot_n64_os_upper_count = 0;
