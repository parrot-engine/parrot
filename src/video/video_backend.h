#ifndef __SRC_PARROT_SRC_VIDEO_VIDEO_BACKEND_H_
#define __SRC_PARROT_SRC_VIDEO_VIDEO_BACKEND_H_

#include "parrot/core/math.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    ParrotVec3 position;
    ParrotVec3 normal;
    ParrotVec2 uv;
} ParrotVideoBackendVertex;

typedef struct {
    uint32_t index;
} ParrotVideoBackendViewportHandle;

void ParrotVideoBackend_init(void);
void ParrotVideoBackend_shutdown(void);
bool ParrotVideoBackend_is_initialized(void);

ParrotVideoBackendViewportHandle ParrotVideoBackend_create_viewport(int width, int height);
void ParrotVideoBackend_delete_viewport(ParrotVideoBackendViewportHandle handle);
void ParrotVideoBackend_read_viewport(ParrotVideoBackendViewportHandle handle, uint32_t *bgra);
void ParrotVideoBackend_clear_viewport(ParrotVideoBackendViewportHandle handle, ParrotVec3 clear_color);
void ParrotVideoBackend_draw_viewport_vertices(ParrotVideoBackendViewportHandle handle,
                                               ParrotVec4 color,
                                               ParrotMat matrix,
                                               const ParrotVideoBackendVertex *vertices,
                                               size_t count);

#endif // __SRC_PARROT_SRC_VIDEO_VIDEO_BACKEND_H_