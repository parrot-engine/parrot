#include "parrot/core/scheduler.h"
#include "parrot/platform/n64/util.h"
#include "src/platform/n64/os/libc/stdlib.malloc.h"
#include "src/platform/n64/os/os.h"
#include "src/platform/n64/os/rom.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STACK_SIZE (8192)

extern uint8_t _gp;

extern uint8_t Parrot_n64_os_interrupt_handler_entry;
extern uint32_t Parrot_n64_os_interrupt_handler_entry_size;

void _Parrot_n64_os_init_impl(void);
void _Parrot_n64_os_init_paging(void);

void _Parrot_n64_os_main_return(void);

int main(int argc, char *argv[]);

void _Parrot_n64_os_main(void) {
    memcpy((void *)0x80000000, &Parrot_n64_os_interrupt_handler_entry, Parrot_n64_os_interrupt_handler_entry_size);
    Parrot_n64_writeback_invalidiate_data_cache((void *)0x80000000, Parrot_n64_os_interrupt_handler_entry_size);
    Parrot_n64_invalidiate_instruction_cache((void *)0x80000000, Parrot_n64_os_interrupt_handler_entry_size);

    memcpy((void *)0x80000180, &Parrot_n64_os_interrupt_handler_entry, Parrot_n64_os_interrupt_handler_entry_size);
    Parrot_n64_writeback_invalidiate_data_cache((void *)0x80000180, Parrot_n64_os_interrupt_handler_entry_size);
    Parrot_n64_invalidiate_instruction_cache((void *)0x80000180, Parrot_n64_os_interrupt_handler_entry_size);

    _Parrot_n64_os_init_paging();
    _Parrot_n64_os_init_impl();
    _Parrot_n64_os_init_heap();
    _Parrot_n64_os_parse_rom();

    ParrotScheduler *scheduler = ParrotScheduler_new(ParrotN64OSTask);

    if ((uintptr_t)main >= 0x80000000) {
        printf("Parrot: main() is in kseg0 or kseg1 and will not be able to use demand paging\n");
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
