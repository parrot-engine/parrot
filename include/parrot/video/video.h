#ifndef __SRC_PARROT_INCLUDE_PARROT_VIDEO_VIDEO_H_
#define __SRC_PARROT_INCLUDE_PARROT_VIDEO_VIDEO_H_

#include "parrot/core/api.h"
#include "parrot/core/math.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint32_t index;
} ParrotVideoObjectHandle;

PARROT_API void ParrotVideo_init(void);
PARROT_API void ParrotVideo_shutdown(void);
PARROT_API bool ParrotVideo_is_initialized(void);

PARROT_API ParrotVideoObjectHandle ParrotVideo_get_root(void);

PARROT_API ParrotVideoObjectHandle ParrotVideo_create_object(void);
PARROT_API void ParrotVideo_delete_object(ParrotVideoObjectHandle handle);
PARROT_API bool ParrotVideo_does_object_exist(ParrotVideoObjectHandle handle);
PARROT_API void ParrotVideo_set_object_parent(ParrotVideoObjectHandle handle, ParrotVideoObjectHandle parent);

PARROT_API void ParrotVideo_set_object_visible(ParrotVideoObjectHandle handle, bool visible);
PARROT_API void ParrotVideo_set_object_tint(ParrotVideoObjectHandle handle, ParrotVec4 tint);

PARROT_API void ParrotVideo_set_object_matrix(ParrotVideoObjectHandle handle, ParrotMat matrix);

PARROT_API void ParrotVideo_object_add_window(ParrotVideoObjectHandle handle, int width, int height);
PARROT_API void ParrotVideo_object_remove_window(ParrotVideoObjectHandle handle);
PARROT_API bool ParrotVideo_object_has_window(ParrotVideoObjectHandle handle);
PARROT_API bool ParrotVideo_object_is_window_close_requested(ParrotVideoObjectHandle handle);
PARROT_API void ParrotVideo_object_set_window_title(ParrotVideoObjectHandle handle, const char *title);
PARROT_API void ParrotVideo_object_set_window_size(ParrotVideoObjectHandle handle, int width, int height);
PARROT_API int ParrotVideo_object_get_window_width(ParrotVideoObjectHandle handle);
PARROT_API int ParrotVideo_object_get_window_height(ParrotVideoObjectHandle handle);

PARROT_API void ParrotVideo_object_add_viewport(ParrotVideoObjectHandle handle, int width, int height);
PARROT_API void ParrotVideo_object_remove_viewport(ParrotVideoObjectHandle handle);
PARROT_API bool ParrotVideo_object_has_viewport(ParrotVideoObjectHandle handle);
PARROT_API void ParrotVideo_object_set_viewport_size(ParrotVideoObjectHandle handle, int width, int height);

PARROT_API void ParrotVideo_object_add_camera(ParrotVideoObjectHandle handle);
PARROT_API void ParrotVideo_object_remove_camera(ParrotVideoObjectHandle handle);
PARROT_API bool ParrotVideo_object_has_camera(ParrotVideoObjectHandle handle);
PARROT_API void ParrotVideo_object_set_camera_clear_color(ParrotVideoObjectHandle handle, ParrotVec3 color);
PARROT_API void ParrotVideo_object_clear_camera_clear_color(ParrotVideoObjectHandle handle);

PARROT_API void ParrotVideo_object_add_rect(ParrotVideoObjectHandle handle);
PARROT_API void ParrotVideo_object_remove_rect(ParrotVideoObjectHandle handle);
PARROT_API bool ParrotVideo_object_has_rect(ParrotVideoObjectHandle handle);
PARROT_API void ParrotVideo_object_set_rect_size(ParrotVideoObjectHandle handle, ParrotReal width, ParrotReal height);

PARROT_API void ParrotVideo_render(void);

#endif // __SRC_PARROT_INCLUDE_PARROT_VIDEO_VIDEO_H_