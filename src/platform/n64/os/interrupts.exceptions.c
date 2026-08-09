#include "parrot/platform/n64/interrupts.h"
#include "parrot/core/util.h"
#include "parrot/platform/n64/util.h"
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

void Parrot_os_default_exception_handler(ParrotInterruptFrame *frame, uint32_t exception) {
    static volatile ExceptionReadStage read_stage = ExceptionReadStage_NONE;

    if (read_stage == ExceptionReadStage_READING) {
        frame->pc += 4;
        read_stage = ExceptionReadStage_FAIL;
        return;
    } else if (read_stage == ExceptionReadStage_HANDLING) {
        for (;;)
            ;
    }

    read_stage = ExceptionReadStage_HANDLING;

    static const char *messages[] = {
        "Interrupt",
        "TLB Modification",
        "TLB Miss (Load)",
        "TLB Miss (Store)",
        "Address Error (Load)",
        "Address Error (Store)",
        "Bus Error (Fetch)",
        "Bus Error (Load/Store)",
        "Syscall",
        "Breakpoint (Bug if crash reason)",
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
    printf("\n");

    uint32_t instruction_stream[INSTRUCTIONS_AROUND * 2 + 1];
    bool instruction_stream_valid[INSTRUCTIONS_AROUND * 2 + 1] = {false};

    uint32_t pc_start = frame->pc - INSTRUCTIONS_AROUND * 4;
    volatile uint32_t *instructions_start = (volatile uint32_t *)(uintptr_t)pc_start;
    for (size_t i = 0; i < INSTRUCTIONS_AROUND * 2 + 1; i++) {
        read_stage = ExceptionReadStage_READING;
        instruction_stream[i] = instructions_start[i];
        PARROT_N64_MSYNC_DEPEND("r"(instruction_stream[i]));
        instruction_stream_valid[i] = read_stage == ExceptionReadStage_READING;
        read_stage = ExceptionReadStage_HANDLING;
    }

    printf("Instruction Stream:\n");
    for (size_t i = 0; i < INSTRUCTIONS_AROUND * 2 + 1; i++) {
        char highlight = ' ';

        if (i == INSTRUCTIONS_AROUND) {
            highlight = '>';
        }

        printf(" %c 0x%08X: 0x", highlight, pc_start + i * sizeof(uint32_t));
        if (instruction_stream_valid[i]) {
            printf("%08X", instruction_stream[i]);
        } else {
            printf("????????");
        }
        printf("\n");
    }
    printf("\n");

    printf("$at: 0x%08X\n", frame->at);
    printf("\n");

    printf("$v0: 0x%08X $v1: 0x%08X\n", frame->v0, frame->v1);
    printf("\n");

    printf("$a0: 0x%08X $a1: 0x%08X $a2: 0x%08X $a3: 0x%08X\n", frame->a0, frame->a1, frame->a2, frame->a3);
    printf("\n");

    printf("$t0: 0x%08X $t1: 0x%08X $t2: 0x%08X $t3: 0x%08X\n", frame->t0, frame->t1, frame->t2, frame->t3);
    printf("$t4: 0x%08X $t5: 0x%08X $t6: 0x%08X $t7: 0x%08X\n", frame->t4, frame->t5, frame->t6, frame->t7);
    printf("$t8: 0x%08X $t9: 0x%08X\n", frame->t8, frame->t9);
    printf("\n");

    printf("$s0: 0x%08X $s1: 0x%08X $s2: 0x%08X $s3: 0x%08X\n", frame->s0, frame->s1, frame->s2, frame->s3);
    printf("$s4: 0x%08X $s5: 0x%08X $s6: 0x%08X $s7: 0x%08X\n", frame->s4, frame->s5, frame->s6, frame->s7);
    printf("\n");

    printf("$hi: 0x%08X $lo: 0x%08X\n", frame->hi, frame->lo);
    printf("\n");

    printf("$gp: 0x%08X\n", frame->gp);
    printf("$fp: 0x%08X\n", frame->fp);
    printf("\n");

    printf("$ra: 0x%08X\n", frame->ra);
    printf("$sp: 0x%08X\n", frame->sp);

    printf("\n");
    printf("##########################################################################################\n");

    for (;;)
        ;
}
