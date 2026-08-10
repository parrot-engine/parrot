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
    ParrotColor tint;
} ParrotVideoBackendVertex;

typedef struct {
    uint32_t index;
} ParrotVideoBackendViewportHandle;

void ParrotVideoBackend_init(void);
void ParrotVideoBackend_shutdown(void);
bool ParrotVideoBackend_is_initialized(void);

ParrotVideoBackendViewportHandle ParrotVideoBackend_create_viewport(int width, int height);
void ParrotVideoBackend_delete_viewport(ParrotVideoBackendViewportHandle handle);
/**
 * Return value is RGBA8888 and guarnteed to live at least as long as the viewport which updates every call to this
 * function.
 */
const uint32_t *ParrotVideoBackend_get_viewport_pixels(ParrotVideoBackendViewportHandle handle);
void ParrotVideoBackend_clear_viewport(ParrotVideoBackendViewportHandle handle, ParrotColor clear_color);
void ParrotVideoBackend_draw_viewport_vertices(ParrotVideoBackendViewportHandle handle,
                                               ParrotGMatSet matrix_set,
                                               const ParrotVideoBackendVertex *vertices,
                                               size_t count);

#endif // __SRC_PARROT_SRC_VIDEO_VIDEO_BACKEND_H_
