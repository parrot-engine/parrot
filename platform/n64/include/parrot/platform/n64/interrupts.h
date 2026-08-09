#ifndef PARROT_PLATFORM_N64_INCLUDE_PARROT_PLATFORM_N64_INTERRUPTS_H_
#define PARROT_PLATFORM_N64_INCLUDE_PARROT_PLATFORM_N64_INTERRUPTS_H_

#include "parrot/config.h"
#include "parrot/core/api.h"
#include "parrot/platform/n64/rdp.h"
#include <stdint.h>

PARROT_PACK_BEGIN()
typedef struct {
    uint32_t sp;
    uint32_t at;
    uint32_t v0, v1;
    uint32_t a0, a1, a2, a3;
    uint32_t t0, t1, t2, t3, t4, t5, t6, t7;
    uint32_t s0, s1, s2, s3, s4, s5, s6, s7;
    uint32_t t8, t9;
    uint32_t gp, fp, ra;
    uint32_t hi, lo;
    uint32_t pc;
    uint32_t cause;
} ParrotInterruptFrame;
PARROT_PACK_END()

typedef void (*ParrotInterruptHandler)(ParrotInterruptFrame *frame);
typedef void (*ParrotExceptionHandler)(ParrotInterruptFrame *frame, uint32_t exception);

PARROT_API extern ParrotInterruptHandler Parrot_os_interrupt_handler_func;
PARROT_API extern ParrotExceptionHandler Parrot_os_exception_handler_func;

PARROT_API void Parrot_os_default_interrupt_handler(ParrotInterruptFrame *frame);
PARROT_API void Parrot_os_default_exception_handler(ParrotInterruptFrame *frame, uint32_t exception);

#endif // PARROT_PLATFORM_N64_INCLUDE_PARROT_PLATFORM_N64_INTERRUPTS_H_
