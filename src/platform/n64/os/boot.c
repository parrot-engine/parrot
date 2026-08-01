#include "platform/n64/stdlib/stdlib.h"
#include "src/platform/n64/os/boot_data.h"
#include <stdint.h>
#include <string.h>

#define VI_BASE 0xA4400000
#define VI_CONTROL (*(volatile uint32_t *)(VI_BASE + 0x00))
#define VI_ORIGIN (*(volatile uint32_t *)(VI_BASE + 0x04))
#define VI_WIDTH (*(volatile uint32_t *)(VI_BASE + 0x08))
#define VI_V_INTR (*(volatile uint32_t *)(VI_BASE + 0x0C))
#define VI_BURST (*(volatile uint32_t *)(VI_BASE + 0x14))
#define VI_V_SYNC (*(volatile uint32_t *)(VI_BASE + 0x18))
#define VI_H_SYNC (*(volatile uint32_t *)(VI_BASE + 0x1C))
#define VI_H_SYNC_LEAP (*(volatile uint32_t *)(VI_BASE + 0x20))
#define VI_H_START (*(volatile uint32_t *)(VI_BASE + 0x24))
#define VI_V_START (*(volatile uint32_t *)(VI_BASE + 0x28))
#define VI_V_BURST (*(volatile uint32_t *)(VI_BASE + 0x2C))
#define VI_X_SCALE (*(volatile uint32_t *)(VI_BASE + 0x30))
#define VI_Y_SCALE (*(volatile uint32_t *)(VI_BASE + 0x34))

int main(int argc, char *argv[]);

void Parrot_n64_main(void) {
    LibdragonBootData boot_data;
    memcpy(&boot_data, (void *)0xA4000000, sizeof(LibdragonBootData));

    Parrot_libc_stdlib_init(boot_data.random_seed, boot_data.avaliable_memory_bytes);

    uint16_t *fb = (uint16_t *)0xA0100000;

    VI_CONTROL = 0x00003202;
    VI_ORIGIN = (uint32_t)(uintptr_t)fb & 0x00FFFFFF;
    VI_WIDTH = 320;
    VI_V_INTR = 0x00000200;
    VI_BURST = 0x03E52239;
    VI_V_SYNC = 0x0000020D;
    VI_H_SYNC = 0x00000C15;
    VI_H_SYNC_LEAP = 0x0C150C15;
    VI_H_START = 0x006C02EC;
    VI_V_START = 0x002301FD;
    VI_V_BURST = 0x000E0204;
    VI_X_SCALE = 0x00000200;
    VI_Y_SCALE = 0x00000400;

    for (int i = 0; i < 320 * 240; i++) {
        fb[i] = 0xF800;
    }

    char *argv[] = {
        "game",
    };
    main(1, argv);

    for (int i = 0; i < 320 * 240; i++) {
        fb[i] = 0xFFFF;
    }
}