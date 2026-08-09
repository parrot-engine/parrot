#include "parrot/config.h"
#include "parrot/platform/n64/util.h"
#include "src/platform/n64/lib/video/platform.h"
#include "src/platform/n64/os/boot_data.h"
#include "src/platform/n64/os/libc/stdlib.malloc.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern uint8_t Parrot_os_interrupt_handler_entry;
extern uint32_t Parrot_os_interrupt_handler_entry_size;

static void success_fb(void) {
    uint16_t *fb = (uint16_t *)0xA0100000;

    for (int i = 0; i < 320 * 240; i++) {
        fb[i] = 0xFFFF;
    }
}

static void success_message(void) {
    printf("Finished!\n");
}

int main(int argc, char *argv[]);

void Parrot_os(void) {
    memcpy((void *)0x80000180, &Parrot_os_interrupt_handler_entry, Parrot_os_interrupt_handler_entry_size);

    ParrotVideoWindow_set_tv_type(LibdragonBootData_tv_type);
    Parrot_os_heap_init(LibdragonBootData_avaliable_memory_bytes);

    char *argv[] = {
        "game",
    };

    int status = main(1, argv);
    exit(status);

    for (;;)
        ;
}
