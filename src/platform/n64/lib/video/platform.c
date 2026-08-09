#include "src/video/platform.h"
#include "parrot/core/math.h"
#include "parrot/core/util.h"
#include "parrot/platform/n64/util.h"
#include "src/platform/n64/os/boot_data.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FRAMEBUFFERS 2

#define VI_CTRL (*(volatile uint32_t *)0xA4400000)
#define VI_ORIGIN (*(volatile uint32_t *)0xA4400004)
#define VI_WIDTH (*(volatile uint32_t *)0xA4400008)
#define VI_V_INTR (*(volatile uint32_t *)0xA440000C)
#define VI_V_CURRENT (*(volatile uint32_t *)0xA4400010)
#define VI_BURST (*(volatile uint32_t *)0xA4400014)
#define VI_V_SYNC (*(volatile uint32_t *)0xA4400018)
#define VI_H_SYNC (*(volatile uint32_t *)0xA440001C)
#define VI_H_SYNC_LEAP (*(volatile uint32_t *)0xA4400020)
#define VI_H_START (*(volatile uint32_t *)0xA4400024)
#define VI_V_START (*(volatile uint32_t *)0xA4400028)
#define VI_V_BURST (*(volatile uint32_t *)0xA440002C)
#define VI_X_SCALE (*(volatile uint32_t *)0xA4400030)
#define VI_Y_SCALE (*(volatile uint32_t *)0xA4400034)

struct ParrotVideoWindow {
    int width;

    uint32_t *framebuffers[MAX_FRAMEBUFFERS];
    uint64_t frame;
};

static int get_height(int width) {
    return width / (4 / 3);
}

static LibdragonBootDataTVType ParrotVideoWindow_tv_type = LibdragonBootDataTVType_NTSC;

ParrotVideoWindow *ParrotVideoWindow_new(int width, int height) {
    ParrotVideoWindow *self = malloc(sizeof(ParrotVideoWindow));
    PARROT_RET_COND_V(!self, NULL);
    memset(self, 0, sizeof(ParrotVideoWindow));

    ParrotVideoWindow_set_size(self, width, height);

    return self;
}

void ParrotVideoWindow_delete(ParrotVideoWindow *self) {
    PARROT_FAIL_NULL(self);

    for (size_t i = 0; i < MAX_FRAMEBUFFERS; i++) {
        free(self->framebuffers[i]);
    }

    free(self);
}

void ParrotVideoWindow_poll_events(ParrotVideoWindow *self) {
    PARROT_FAIL_NULL(self);
}

bool ParrotVideoWindow_should_close(ParrotVideoWindow *self) {
    PARROT_FAIL_NULL(self);
    return false;
}

void ParrotVideoWindow_set_title(ParrotVideoWindow *self, const char *title) {
    PARROT_FAIL_NULL(self);
    (void)title;
}

void ParrotVideoWindow_set_size(ParrotVideoWindow *self, int width, int height) {
    PARROT_FAIL_NULL(self);
    PARROT_RET_COND(width == self->width);

    self->width = PARROT_MIN(width, 640);
    (void)height;

    for (size_t i = 0; i < MAX_FRAMEBUFFERS; i++) {
        free(self->framebuffers[i]);
        self->framebuffers[i] = calloc(width * height, sizeof(uint32_t));
    }
}

int ParrotVideoWindow_get_width(ParrotVideoWindow *self) {
    PARROT_FAIL_NULL(self);
    return self->width;
}

int ParrotVideoWindow_get_height(ParrotVideoWindow *self) {
    PARROT_FAIL_NULL(self);
    return get_height(self->width);
}

void ParrotVideoWindow_draw(ParrotVideoWindow *self, uint32_t *rgbx8888, int width, int height) {
    PARROT_FAIL_NULL(self);
    PARROT_FAIL_COND(width != ParrotVideoWindow_get_width(self));

    uint32_t *framebuffer = self->framebuffers[self->frame % MAX_FRAMEBUFFERS];

    memcpy(framebuffer, rgbx8888, width * height * sizeof(uint32_t));
    Parrot_n64_writeback_invalidiate_data_cache(framebuffer, width * height * sizeof(uint32_t));

    if ((VI_CTRL & 0x3) != 0) {
        while ((VI_V_CURRENT & ~1) == 0)
            ;
        while ((VI_V_CURRENT & ~1) != 0)
            ;
    }

    PARROT_N64_IO_WRITE(VI_CTRL, 0);

    PARROT_N64_IO_WRITE(VI_WIDTH, width);
    PARROT_N64_IO_WRITE(VI_V_INTR, 0x3FF);
    PARROT_N64_IO_WRITE(VI_BURST, ParrotVideoWindow_tv_type == LibdragonBootDataTVType_NTSC ? 0x03E52239 : 0x0404233A);
    PARROT_N64_IO_WRITE(VI_V_SYNC, 0x20D);
    PARROT_N64_IO_WRITE(VI_H_SYNC, 0xC15);
    PARROT_N64_IO_WRITE(VI_H_SYNC_LEAP, 0x0C150C15);
    PARROT_N64_IO_WRITE(VI_H_START, 0x006C02EC);
    PARROT_N64_IO_WRITE(VI_V_START, 0x002301FD);
    PARROT_N64_IO_WRITE(VI_V_BURST, ParrotVideoWindow_tv_type == LibdragonBootDataTVType_NTSC ? 0x000E0204 : 0x0009026B);
    PARROT_N64_IO_WRITE(VI_X_SCALE, 0x200);
    PARROT_N64_IO_WRITE(VI_Y_SCALE, 0x400);
    PARROT_N64_IO_WRITE(VI_CTRL,
                        (/* Pixel Advance */ 3 << 12) | (/* AA = AA_ALWAYS  */ 0 << 8) | (/* TYPE = RGBX8888 */ 3 << 0));
    PARROT_N64_IO_WRITE(VI_ORIGIN, PARROT_N64_PHYSICAL_ADDRESS(framebuffer));

    self->frame++;
}

void ParrotVideoWindow_set_tv_type(LibdragonBootDataTVType type) {
    ParrotVideoWindow_tv_type = type;
}
