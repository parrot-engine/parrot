#include "parrot/platform/n64/interrupts.h"
#include <stdint.h>
#include <stdio.h>

void Parrot_os_default_interrupt_handler(ParrotInterruptFrame *frame) {
    (void)frame;
}

void Parrot_os_interrupt_handler(ParrotInterruptFrame *frame) {
    uint32_t exception = (frame->cause >> 2) & 0x1F;
    if (exception != 0 && exception != /* Breakpoint */ 0x09) {
        Parrot_os_exception_handler_func(frame, exception);
        return;
    }

    Parrot_os_interrupt_handler_func(frame);
}

ParrotInterruptHandler Parrot_os_interrupt_handler_func = Parrot_os_default_interrupt_handler;
ParrotExceptionHandler Parrot_os_exception_handler_func = Parrot_os_default_exception_handler;
