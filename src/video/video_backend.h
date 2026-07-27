#ifndef __SRC_PARROT_SRC_VIDEO_VIDEO_BACKEND_H_
#define __SRC_PARROT_SRC_VIDEO_VIDEO_BACKEND_H_

#include "src/video/platform.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint32_t index;
} ParrotVideoBackendViewportHandle;

void ParrotVideoBackend_init(void);
void ParrotVideoBackend_shutdown(void);
bool ParrotVideoBackend_is_initialized(void);

ParrotVideoBackendViewportHandle ParrotVideoBackend_create_viewport(int width, int height);
void ParrotVideoBackend_delete_viewport(ParrotVideoBackendViewportHandle handle);
void ParrotVideoBackend_read_viewport(ParrotVideoBackendViewportHandle handle, uint32_t *bgra);

#endif // __SRC_PARROT_SRC_VIDEO_VIDEO_BACKEND_H_