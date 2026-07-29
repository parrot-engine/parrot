#include "parrot/video/video.h"
#include "parrot/core/math.h"
#include "parrot/core/util.h"
#include "src/video/platform.h"
#include "src/video/video_backend.h"
#include "stb_ds.h"
#include <complex.h>
#include <stdlib.h>
#include <string.h>

typedef struct ParrotVideoObject ParrotVideoObject;

typedef struct {
    ParrotVideoWindow *window;

    int width;
    int height;

    bool close_requested;
} ParrotVideoObjectWindow;

typedef struct {
    ParrotVideoBackendViewportHandle viewport;

    int width;
    int height;
} ParrotVideoObjectViewport;

typedef struct {
    bool use_clear_color;
    ParrotVec3 clear_color;
} ParrotVideoObjectCamera;

typedef struct {
    ParrotReal width;
    ParrotReal height;
} ParrotVideoObjectRect;

typedef struct {
    ParrotVideoObjectHandle key;
} ParrotVideoObjectChild;

struct ParrotVideoObject {
    ParrotVideoObjectHandle parent;
    ParrotVideoObjectChild *shm_children;

    ParrotMat matrix;

    bool visible;
    ParrotVec4 tint;

    ParrotVideoObjectWindow *window;
    ParrotVideoObjectViewport *viewport;

    ParrotVideoObjectCamera *camera;

    ParrotVideoObjectRect *rect;
};

typedef struct ParrotVideoObjectPointer {
    uint32_t key;
    ParrotVideoObject *value;
} ParrotVideoObjectPointer;

typedef struct {
    ParrotVideoObject *root_object;
    ParrotVideoObjectHandle root_object_handle;

    ParrotVideoObjectPointer *hm_pointers;
    uint32_t next_pointer_id;
} ParrotVideo;

static ParrotVideo *self = NULL;

void ParrotVideo_init(void) {
    PARROT_FAIL_COND(ParrotVideo_is_initialized());

    self = malloc(sizeof(ParrotVideo));
    memset(self, 0, sizeof(ParrotVideo));

    ParrotVideoBackend_init();

    ParrotVideo_create_object();
}

void ParrotVideo_shutdown(void) {
    PARROT_FAIL_COND(!ParrotVideo_is_initialized());

    ParrotVideo_delete_object(ParrotVideo_get_root());

    hmfree(self->hm_pointers);

    ParrotVideoBackend_shutdown();

    free(self);
    self = NULL;
}

bool ParrotVideo_is_initialized(void) {
    return self;
}

ParrotVideoObjectHandle ParrotVideo_get_root(void) {
    return self->root_object_handle;
}

ParrotVideoObjectHandle ParrotVideo_create_object(void) {
    PARROT_FAIL_COND(!ParrotVideo_is_initialized());

    ParrotVideoObject *object = malloc(sizeof(ParrotVideoObject));
    memset(object, 0, sizeof(ParrotVideoObject));

    object->matrix = ParrotMat_identity();

    object->visible = true;
    object->tint = ParrotVec4_n(1);

    uint32_t index = self->next_pointer_id++;
    hmput(self->hm_pointers, index, object);

    ParrotVideoObjectHandle handle = (ParrotVideoObjectHandle){
        .index = index,
    };

    object->parent = self->root_object_handle;
    if (self->root_object) {
        hmputs(self->root_object->shm_children, (ParrotVideoObjectChild){handle});
    }

    if (!self->root_object) {
        self->root_object = object;
        self->root_object_handle = handle;
    }

    return handle;
}

void ParrotVideo_delete_object(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));

    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);

    while (hmlen(object->shm_children) > 0) {
        ParrotVideo_delete_object(object->shm_children[0].key);
    }

    if (ParrotVideo_object_has_window(handle)) {
        ParrotVideo_object_remove_window(handle);
    }

    if (ParrotVideo_object_has_viewport(handle)) {
        ParrotVideo_object_remove_viewport(handle);
    }

    if (ParrotVideo_object_has_camera(handle)) {
        ParrotVideo_object_remove_camera(handle);
    }

    ParrotVideoObject *parent_object = hmget(self->hm_pointers, object->parent);
    hmdel(parent_object->shm_children, handle.index);

    hmdel(self->hm_pointers, handle.index);

    free(object);
}

bool ParrotVideo_does_object_exist(ParrotVideoObjectHandle handle) {
    return hmgeti(self->hm_pointers, handle.index) >= 0;
}

void ParrotVideo_set_object_parent(ParrotVideoObjectHandle handle, ParrotVideoObjectHandle parent) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(parent));

    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);

    ParrotVideoObject *new_parent = hmget(self->hm_pointers, parent);
    ParrotVideoObject *old_parent = hmget(self->hm_pointers, object->parent);

    PARROT_FAIL_NULL(new_parent);
    PARROT_FAIL_NULL(old_parent);

    hmdel(old_parent->shm_children, handle);
    hmputs(new_parent->shm_children, (ParrotVideoObjectChild){handle});
    object->parent = parent;
}

void ParrotVideo_set_object_visible(ParrotVideoObjectHandle handle, bool visible) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));

    hmget(self->hm_pointers, handle.index)->visible = visible;
}

void ParrotVideo_set_object_tint(ParrotVideoObjectHandle handle, ParrotVec4 tint) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));

    hmget(self->hm_pointers, handle.index)->tint = tint;
}

void ParrotVideo_set_object_matrix(ParrotVideoObjectHandle handle, ParrotMat matrix) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));

    hmget(self->hm_pointers, handle.index)->matrix = matrix;
}

void ParrotVideo_object_add_window(ParrotVideoObjectHandle handle, int width, int height) {
    PARROT_FAIL_COND(ParrotVideo_object_has_window(handle));

    PARROT_FAIL_COND(width == 0);
    PARROT_FAIL_COND(height == 0);

    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);
    object->window = malloc(sizeof(ParrotVideoObjectWindow));
    memset(object->window, 0, sizeof(ParrotVideoObjectWindow));

    object->window->window = ParrotVideoWindow_new(width, height);
}

void ParrotVideo_object_remove_window(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_window(handle));

    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);

    ParrotVideoWindow_delete(object->window->window);

    free(object->window);
    object->window = NULL;
}

bool ParrotVideo_object_has_window(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));

    return hmget(self->hm_pointers, handle.index)->window;
}

bool ParrotVideo_object_is_window_close_requested(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_window(handle));

    return ParrotVideoWindow_should_close(hmget(self->hm_pointers, handle.index)->window->window);
}

void ParrotVideo_object_set_window_title(ParrotVideoObjectHandle handle, const char *title) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_window(handle));

    ParrotVideoWindow_set_title(hmget(self->hm_pointers, handle.index)->window->window, title);
}

void ParrotVideo_object_set_window_size(ParrotVideoObjectHandle handle, int width, int height) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_window(handle));

    PARROT_FAIL_COND(width == 0);
    PARROT_FAIL_COND(height == 0);

    ParrotVideoWindow_set_size(hmget(self->hm_pointers, handle.index)->window->window, width, height);
}

int ParrotVideo_object_get_window_width(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_window(handle));
    return ParrotVideoWindow_get_width(hmget(self->hm_pointers, handle.index)->window->window);
}

int ParrotVideo_object_get_window_height(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_window(handle));
    return ParrotVideoWindow_get_height(hmget(self->hm_pointers, handle.index)->window->window);
}

void ParrotVideo_object_add_viewport(ParrotVideoObjectHandle handle, int width, int height) {
    PARROT_FAIL_COND(ParrotVideo_object_has_viewport(handle));

    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);
    object->viewport = malloc(sizeof(ParrotVideoObjectViewport));
    memset(object->viewport, 0, sizeof(ParrotVideoObjectViewport));

    PARROT_FAIL_COND(width == 0);
    PARROT_FAIL_COND(height == 0);

    object->viewport->viewport = ParrotVideoBackend_create_viewport(width, height);
    object->viewport->width = width;
    object->viewport->height = height;
}

void ParrotVideo_object_remove_viewport(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_viewport(handle));

    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);

    ParrotVideoBackend_delete_viewport(object->viewport->viewport);

    free(object->viewport);
    object->viewport = NULL;
}

bool ParrotVideo_object_has_viewport(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));

    return hmget(self->hm_pointers, handle.index)->viewport;
}

void ParrotVideo_object_set_viewport_size(ParrotVideoObjectHandle handle, int width, int height) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_viewport(handle));

    PARROT_FAIL_COND(width == 0);
    PARROT_FAIL_COND(height == 0);
}

void ParrotVideo_object_add_camera(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(ParrotVideo_object_has_camera(handle));

    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);
    object->camera = malloc(sizeof(ParrotVideoObjectCamera));
    memset(object->camera, 0, sizeof(ParrotVideoObjectCamera));

    object->camera->use_clear_color = true;
}

void ParrotVideo_object_remove_camera(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_camera(handle));

    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);

    free(object->camera);
    object->camera = NULL;
}

bool ParrotVideo_object_has_camera(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));

    return hmget(self->hm_pointers, handle.index)->camera;
}

void ParrotVideo_object_set_camera_clear_color(ParrotVideoObjectHandle handle, ParrotVec3 color) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_camera(handle));

    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);

    object->camera->clear_color = color;
    object->camera->use_clear_color = true;
}

void ParrotVideo_object_clear_camera_clear_color(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_camera(handle));

    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);

    object->camera->use_clear_color = false;
}

static bool ParrotVideo_find_camera(ParrotVideoObjectHandle handle,
                                    ParrotVideoObjectHandle *out_handle,
                                    ParrotVideoObjectCamera **out_camera) {
    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);

    if (!object->visible) {
        return false;
    }

    if (object->camera) {
        *out_handle = handle;
        *out_camera = object->camera;
        return true;
    }

    for (size_t i = 0; i < hmlen(object->shm_children); i++) {
        if (ParrotVideo_find_camera(object->shm_children[i].key, out_handle, out_camera)) {
            return true;
        }
    }
    return false;
}

static void ParrotVideo_render_object(ParrotVideoObjectHandle handle,
                                      ParrotVideoBackendViewportHandle *viewport,
                                      ParrotMat projection_view_matrix,
                                      ParrotVec4 tint) {
    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);

    tint = ParrotVec4_mul(tint, object->tint);

    if (ParrotVideo_object_has_window(handle)) {
        ParrotVideoWindow_poll_events(object->window->window);
    }

    if (ParrotVideo_object_has_viewport(handle)) {
        viewport = &object->viewport->viewport;
        ParrotVideoObjectHandle camera_handle;
        ParrotVideoObjectCamera *camera;

        if (ParrotVideo_find_camera(handle, &camera_handle, &camera)) {
            ParrotVideoObject *camera_object = hmget(self->hm_pointers, camera_handle.index);

            if (camera->use_clear_color) {
                ParrotVideoBackend_clear_viewport(object->viewport->viewport, camera->clear_color);
            }

            ParrotMat projection_matrix =
                ParrotMat_ortho(0, object->viewport->width, 0, object->viewport->height, 0, 1000000);

            projection_view_matrix = ParrotMat_mul(projection_matrix, ParrotMat_inverse(camera_object->matrix));
        }
    }

    if (object->visible) {
        for (size_t i = 0; i < hmlen(object->shm_children); i++) {
            ParrotVideo_render_object(object->shm_children[i].key, viewport, projection_view_matrix, tint);
        }
    }

    if (ParrotVideo_object_has_viewport(handle) && ParrotVideo_object_has_window(handle)) {
        uint32_t *bgra = calloc(object->viewport->width * object->viewport->height, sizeof(uint32_t));
        ParrotVideoBackend_read_viewport(object->viewport->viewport, bgra);
        ParrotVideoWindow_draw(object->window->window, bgra, object->viewport->width, object->viewport->height);
        free(bgra);
    }

    PARROT_RET_COND(!viewport);
    PARROT_RET_COND(!object->visible);

    ParrotMat matrix = ParrotMat_mul(projection_view_matrix, object->matrix);

    if (ParrotVideo_object_has_rect(handle)) {
        ParrotReal width = object->rect->width;
        ParrotReal height = object->rect->height;

        ParrotVideoBackendVertex vertices[] = {
            (ParrotVideoBackendVertex){.position = (ParrotVec3){0, 0, 0}},
            (ParrotVideoBackendVertex){.position = (ParrotVec3){width, 0, 0}},
            (ParrotVideoBackendVertex){.position = (ParrotVec3){0, height, 0}},

            (ParrotVideoBackendVertex){.position = (ParrotVec3){width, height, 0}},
            (ParrotVideoBackendVertex){.position = (ParrotVec3){0, height, 0}},
            (ParrotVideoBackendVertex){.position = (ParrotVec3){width, 0, 0}},
        };

        ParrotVideoBackend_draw_viewport_vertices(*viewport, tint, matrix, vertices, 6);
    }
}

void ParrotVideo_object_add_rect(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(ParrotVideo_object_has_rect(handle));

    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);

    object->rect = malloc(sizeof(ParrotVideoObjectRect));
    memset(object->rect, 0, sizeof(ParrotVideoObjectRect));
}

void ParrotVideo_object_remove_rect(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_rect(handle));

    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);

    free(object->rect);
    object->rect = NULL;
}

bool ParrotVideo_object_has_rect(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));

    return hmget(self->hm_pointers, handle.index)->rect;
}

void ParrotVideo_object_set_rect_size(ParrotVideoObjectHandle handle, ParrotReal width, ParrotReal height) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_rect(handle));

    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);

    object->rect->width = width;
    object->rect->height = height;
}

void ParrotVideo_render(void) {
    PARROT_FAIL_COND(!ParrotVideo_is_initialized());

    ParrotVideo_render_object(self->root_object_handle, NULL, ParrotMat_identity(), ParrotVec4_n(1));
}