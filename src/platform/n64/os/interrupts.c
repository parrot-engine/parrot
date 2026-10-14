#include "parrot/core/scheduler.h"
#include "src/platform/n64/os/os.h"
#include <stdint.h>
#include <stdio.h>

bool _Parrot_n64_os_exception_handler(ParrotN64OSInterruptFrame *frame, uint32_t exception);

void _Parrot_n64_os_interrupt_handler(ParrotN64OSInterruptFrame *frame) {
    static uint32_t count_wrap_current = 0;

    uint32_t count;
    __asm__ volatile("mfc0 %0, $9\n" : "=r"(count));

    if (count_wrap_current != 0 && count_wrap_current > count) {
        _Parrot_n64_os_upper_count++;
        count_wrap_current = 0;
    }

    uint32_t exception = (frame->cause >> 2) & 0x1F;

    if (exception != 0) {
        if (!_Parrot_n64_os_exception_handler(frame, exception)) {
            return;
        }
    }

    if (_Parrot_n64_os_scheduler_task) {
        ParrotScheduler_end_task(_Parrot_n64_os_scheduler_task);

        ParrotN64OSTask *task = _Parrot_n64_os_scheduler_task;
        _Parrot_n64_os_scheduler_task = NULL;

        task->pc = frame->pc;
        for (size_t i = 0; i < 31; i++) {
            task->regs[i] = frame->regs[i];
        }
        task->reg_hi = frame->reg_hi;
        task->reg_lo = frame->reg_lo;
    }

    PARROT_RET_COND(!_Parrot_n64_os_scheduler);

    ParrotN64OSTask *task = ParrotScheduler_next(_Parrot_n64_os_scheduler, ParrotN64OSTask);
    if (task) {
        frame->pc = task->pc;
        for (size_t i = 0; i < 31; i++) {
            frame->regs[i] = task->regs[i];
        }
        frame->reg_hi = task->reg_hi;
        frame->reg_lo = task->reg_lo;

        ParrotScheduler_begin_task(task);
        _Parrot_n64_os_scheduler_task = task;
    }

    __asm__ volatile("mfc0 %0, $9\n" : "=r"(count));

    uint32_t compare = count + 10000;
    if (count > compare) {
        compare = 0;
        count_wrap_current = count;
    }

    __asm__ volatile("mtc0 %0, $11\n" ::"r"(compare));
}
