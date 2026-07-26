#ifndef __SRC_PARROT_SRC_VIDEO_VIDEO_BACKEND_H_
#define __SRC_PARROT_SRC_VIDEO_VIDEO_BACKEND_H_

#include <stdint.h>

typedef struct {
    uint32_t handle;
} ParrotVideoBackendWindow;

typedef struct {
    uint32_t handle;
} ParrotVideoBackendViewport;

void ParrotVideoBackend_init(void);
void ParrotVideoBackend_shutdown(void);

ParrotVideoBackendViewport ParrotVideoBackend_create_viewport(void);
void ParrotVideoBackend_delete_viewport(ParrotVideoBackendViewport viewport);

ParrotVideoBackendWindow ParrotVideoBackend_create_window(void);
void ParrotVideoBackend_delete_window(ParrotVideoBackendWindow window);
void ParrotVideoBackend_set_title(ParrotVideoBackendWindow window, const char *title);
void ParrotVideoBackend_set_size(ParrotVideoBackendWindow window, int width, int height);
void ParrotVideoBackend_render(ParrotVideoBackendWindow window, ParrotVideoBackendViewport viewport);

#endif // __SRC_PARROT_SRC_VIDEO_VIDEO_BACKEND_H_