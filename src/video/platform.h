#ifndef __SRC_PARROT_SRC_VIDEO_PLATFORM_H_
#define __SRC_PARROT_SRC_VIDEO_PLATFORM_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct ParrotVideoWindow ParrotVideoWindow;

ParrotVideoWindow *ParrotVideoWindow_new(int width, int height);
void ParrotVideoWindow_delete(ParrotVideoWindow *self);

void ParrotVideoWindow_poll_events(ParrotVideoWindow *self);

/// Clears on read
bool ParrotVideoWindow_should_close(ParrotVideoWindow *self);

void ParrotVideoWindow_set_title(ParrotVideoWindow *self, const char *title);
void ParrotVideoWindow_set_size(ParrotVideoWindow *self, int width, int height);
int ParrotVideoWindow_get_width(ParrotVideoWindow *self);
int ParrotVideoWindow_get_height(ParrotVideoWindow *self);

/// Guarnteed to update at least every recall
void ParrotVideoWindow_set_image(
    ParrotVideoWindow *self,
    /* NULL = no image. Needs to live until replaced or window destroyed */ const uint32_t *rgbx8888);

#endif // __SRC_PARROT_SRC_VIDEO_PLATFORM_H_
