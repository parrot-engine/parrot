#include "parrot/core/math.h"
#include "parrot/core/scope.h"
#include "parrot/core/util.h"
#include "src/platform/n64/os/os.h"
#include <stdio.h>

static void Parrot_crash_handler_impl(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char *message = Parrot_vsprintf_malloc(fmt, args);
    va_end(args);

    ParrotN64OSInterruptFrame frame;

    if (!_Parrot_os_crash_frame) {
        __asm__ volatile("li $v0, %0\n"
                         "move $v1, %1\n"
                         "syscall\n"
                         :
                         : "i"(PARROT_N64_OS_SYS_SAVE_FRAME), "r"(&frame)
                         : "v0", "v1");
    } else {
        frame = *_Parrot_os_crash_frame;
    }

    uint32_t exception = (frame.cause >> 2) & 0x1F;

    fprintf(stderr, "\n");
    fprintf(stderr, "Parrot Crash Handler\n");
    fprintf(stderr, "##########################################################################################\n");
    fprintf(stderr, "%s\n", message);

    if (exception != 0) {
        fprintf(stderr, "HINT: This crash was likely caused by an exception\n");
        if (frame.badvaddr < 0x1000) {
            fprintf(stderr,
                    "HINT: This crash likely came from %s\n",
                    frame.pc != frame.badvaddr ? "dereferencing a NULL pointer" : "calling a NULL function pointer");
        }
    }

    if (!_Parrot_n64_os_scheduler_task) {
        fprintf(stderr, "HINT: This crash happened while not running any task\n");
    }

    fprintf(stderr, "\n");

    fprintf(stderr, "PC: 0x%08x\n", frame.pc);
    if (exception != 0) {
        fprintf(stderr, "Cause: 0x%08x\n", frame.cause);
        fprintf(stderr, "BadVAddr: 0x%08x\n", frame.badvaddr);
    }
    fprintf(stderr, "\n");

    const char *register_names[] = {
        "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "s0",
        "s1", "s2", "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra",
    };

    for (size_t i = 0; i < 31; i += 2) {
        for (size_t j = i; j < PARROT_MIN(i + 2, 31); j++) {
            fprintf(stderr, "$%s: 0x%08x%08x ", register_names[j], frame.regs[j] >> 32, frame.regs[j] & 0xFFFFFFFF);
        }
        fprintf(stderr, "\n");
    }

    fprintf(stderr, "\n");
    fprintf(stderr, "##########################################################################################\n");

    for (;;)
        ;
}

void _Parrot_n64_os_init_crash_impl(void) {
    Parrot_crash_handler = Parrot_crash_handler_impl;
}
