#include "parrot/video/video.h"
#include "parrot/core/math.h"
#include "parrot/core/util.h"
#include "src/video/platform.h"
#include "src/video/video_backend.h"
#include "stb_ds.h"
#include <stdlib.h>
#include <string.h>

typedef struct ParrotVideoObject ParrotVideoObject;

typedef struct {
    ParrotVideoBackendViewportHandle viewport;

    int width;
    int height;
} ParrotVideoObjectViewport;

typedef struct {
    ParrotVideoWindow *window;

    int width;
    int height;

    bool close_requested;
} ParrotVideoObjectWindow;

typedef struct {
    ParrotVideoObject *key;
} ParrotVideoObjectChild;

struct ParrotVideoObject {
    ParrotVideoObject *parent;
    ParrotVideoObjectChild *shm_children;

    ParrotVec3f position;
    ParrotVec3f rotation;
    ParrotVec3f scale;

    ParrotVideoObjectWindow *window;
    ParrotVideoObjectViewport *viewport;
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

    object->parent = self->root_object;
    object->scale = ParrotVec3f_n(1);

    uint32_t index = self->next_pointer_id++;
    hmput(self->hm_pointers, index, object);

    ParrotVideoObjectHandle handle = (ParrotVideoObjectHandle){
        .index = index,
    };

    if (!self->root_object) {
        self->root_object = object;
        self->root_object_handle = handle;
    }

    return handle;
}

void ParrotVideo_delete_object(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));

    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);

    for (size_t i = 0; i < hmlen(object->shm_children); i++) {
        ParrotVideo_delete_object((ParrotVideoObjectHandle){
            .index = i,
        });
    }

    if (object->parent) {
        hmdel(object->parent->shm_children, object);
    }

    if (ParrotVideo_object_has_window(handle)) {
        ParrotVideo_object_remove_window(handle);
    }

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

    if (object->parent) {
        hmdel(object->parent->shm_children, object);
    }
}

void ParrotVideo_object_add_window(ParrotVideoObjectHandle handle, int width, int height) {
    PARROT_FAIL_COND(ParrotVideo_object_has_window(handle));

    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);
    object->window = malloc(sizeof(ParrotVideoObject));
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
    ParrotVideoWindow_set_size(hmget(self->hm_pointers, handle.index)->window->window, width, height);
}

int ParrotVideo_object_get_window_width(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_window(handle));
}

int ParrotVideo_object_get_window_height(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_window(handle));
}

void ParrotVideo_object_add_viewport(ParrotVideoObjectHandle handle, int width, int height) {
    PARROT_FAIL_COND(ParrotVideo_object_has_viewport(handle));

    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);
    object->viewport = malloc(sizeof(ParrotVideoObject));
    memset(object->viewport, 0, sizeof(ParrotVideoObjectViewport));

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
}

static void ParrotVideo_render_object(ParrotVideoObjectHandle handle) {
    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);

    if (ParrotVideo_object_has_window(handle)) {
        ParrotVideoWindow_poll_events(object->window->window);
    }

    if (ParrotVideo_object_has_viewport(handle)) {
        if (ParrotVideo_object_has_window(handle)) {
            uint32_t *bgra = calloc(object->viewport->width * object->viewport->height, sizeof(uint32_t));
            ParrotVideoBackend_read_viewport(object->viewport->viewport, bgra);
            ParrotVideoWindow_draw(object->window->window, bgra, object->viewport->width, object->viewport->height);
            free(bgra);
        }
    }
}

void ParrotVideo_render(void) {
    PARROT_FAIL_COND(!ParrotVideo_is_initialized());

    ParrotVideo_render_object(self->root_object_handle);
}