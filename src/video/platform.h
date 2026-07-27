#ifndef __SRC_PARROT_SRC_VIDEO_PLATFORM_H_
#define __SRC_PARROT_SRC_VIDEO_PLATFORM_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct ParrotVideoWindow ParrotVideoWindow;
typedef struct ParrotVideoGLContext ParrotVideoGLContext;

ParrotVideoWindow *ParrotVideoWindow_new(int width, int height);
void ParrotVideoWindow_delete(ParrotVideoWindow *self);

void ParrotVideoWindow_poll_events(ParrotVideoWindow *self);

/// Clears on read
bool ParrotVideoWindow_should_close(ParrotVideoWindow *self);

void ParrotVideoWindow_set_title(ParrotVideoWindow *self, const char *title);
void ParrotVideoWindow_set_size(ParrotVideoWindow *self, int width, int height);
int ParrotVideoWindow_get_width(ParrotVideoWindow *self);
int ParrotVideoWindow_get_height(ParrotVideoWindow *self);

void ParrotVideoWindow_draw(ParrotVideoWindow *self, uint32_t *brga, int width, int height);

//

ParrotVideoGLContext *ParrotVideoGLContext_new(void);
void ParrotVideoGLContext_delete(ParrotVideoGLContext *self);

void ParrotVideoGLContext_make_current(ParrotVideoGLContext *self);

#endif // __SRC_PARROT_SRC_VIDEO_PLATFORM_H_