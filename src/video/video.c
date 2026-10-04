#include "parrot/video/video.h"
#include "parrot/core/array.h"
#include "parrot/core/scope.h"
#include "parrot/drivers/video_driver.h"
#include "parrot/drivers/window_driver.h"
#include "parrot/video/font.h"
#include "stb_image.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct ParrotVideoObject ParrotVideoObject;

typedef struct {
    ParrotScope *scope;

    ParrotWindowDriverWindow *window;

    int width;
    int height;

    bool close_requested;
} ParrotVideoObjectWindow;

typedef struct {
    ParrotScope *scope;

    ParrotVideoDriverViewport *viewport;

    int width;
    int height;

    bool physical_keys[ParrotWindowDriverEventKey_COUNT];
    bool logical_keys[ParrotWindowDriverEventKey_COUNT];

    bool mouse_buttons[ParrotWindowDriverEventMouseButton_COUNT];
    ParrotVec2 mouse_position;
} ParrotVideoObjectViewport;

typedef struct {
    ParrotScope *scope;

    bool corner_aligned;

    bool use_clear_color;
    ParrotColor clear_color;
} ParrotVideoObjectCamera;

typedef struct {
    ParrotScope *scope;

    ParrotReal width;
    ParrotReal height;

    ParrotVideoTexture *texture;
    int texture_region_x;
    int texture_region_y;
    int texture_region_width;
    int texture_region_height;
    bool texture_nearest_filter;
} ParrotVideoObjectRect;

typedef struct {
    ParrotScope *scope;

    ParrotVideoFont *font;
    float size;
    const char *text;
} ParrotVideoObjectText;

typedef struct {
    ParrotScope *scope;

    ParrotScope *func_scope;
    ParrotVideoCustomDrawFunc func;
} ParrotVideoObjectCustomDraw;

typedef struct {
    ParrotScope *scope;

    ParrotScope *title_scope;
    char *title;

    int width;
    int height;

    bool close_requested;
} ParrotVideoObjectUIWindow;

typedef struct {
    ParrotVideoObjectHandle key;
} ParrotVideoObjectChild;

struct ParrotVideoObject {
    ParrotScope *scope;

    ParrotVideoObjectHandle parent;
    ParrotVideoObjectChild *shm_children;

    ParrotMat matrix;

    bool visible;
    ParrotColor tint;

    ParrotVideoObjectEvent event_queue[256];
    uint8_t event_queue_read;
    uint8_t event_queue_write;

    ParrotVideoObjectWindow *window;
    ParrotVideoObjectViewport *viewport;

    ParrotVideoObjectCamera *camera;

    ParrotVideoObjectRect *rect;

    ParrotVideoObjectCustomDraw *custom_draw;

    ParrotVideoObjectUIWindow *ui_window;
};

typedef struct ParrotVideoObjectPointer {
    uint32_t key;
    ParrotVideoObject *value;
} ParrotVideoObjectPointer;

struct ParrotVideoTexture {
    ParrotScope *scope;

    int width;
    int height;
    uint32_t *rgba8888;
};

typedef struct {
    ParrotScope *scope;

    ParrotWindowDriver *window_driver;
    ParrotVideoDriver *video_driver;

    ParrotVideoObjectPointer *hm_pointers;
    uint32_t next_pointer_id;

    ParrotVideoFont *default_font;

    ParrotVideoObject *root_object;
    ParrotVideoObjectHandle root_object_handle;
} ParrotVideo;

static ParrotVideo *self = NULL;

void ParrotVideo_init(ParrotWindowDriver *window_driver, ParrotVideoDriver *video_driver) {
    PARROT_FAIL_COND(ParrotVideo_is_initialized());

    self = PARROT_ALLOC(ParrotVideo);

    self->scope = ParrotScope_new(NULL);
    ParrotScope_push_free(self->scope, self);

    self->window_driver = window_driver;
    self->video_driver = video_driver;

    ParrotScope_push_arrfree(self->scope, self->hm_pointers);

    self->default_font = ParrotVideoFont_new_default(ParrotVideoFontDefaultStyle_MODERN);
    ParrotScope_push(self->scope, ParrotVideoFont_vdelete, self->default_font);

    ParrotVideo_create_object();
}

void ParrotVideo_shutdown(void) {
    PARROT_FAIL_COND(!ParrotVideo_is_initialized());

    ParrotScope_delete(self->scope);
    self = NULL;
}

void ParrotVideo_vshutdown(void *unused) {
    (void)unused;
    ParrotVideo_shutdown();
}

bool ParrotVideo_is_initialized(void) {
    return self;
}

ParrotVideoTexture *ParrotVideo_create_texture(const uint8_t *data, size_t size) {
    PARROT_FAIL_NULL(data);

    ParrotVideoTexture *texture = PARROT_ALLOC(ParrotVideoTexture);

    texture->scope = ParrotScope_new(self->scope);
    ParrotScope_push_free(texture->scope, texture);

    int channels = 0;
    texture->rgba8888 = (uint32_t *)stbi_load_from_memory(data, size, &texture->width, &texture->height, &channels, 4);
    if (!texture->rgba8888) {
        ParrotScope_delete(texture->scope);
        return NULL;
    }

    ParrotScope_push(texture->scope, stbi_image_free, texture->rgba8888);

    return texture;
}

ParrotVideoTexture *ParrotVideo_create_texture_raw(int width, int height, const uint32_t *rgba8888) {
    PARROT_FAIL_NULL(rgba8888);

    ParrotVideoTexture *texture = PARROT_ALLOC(ParrotVideoTexture);

    texture->scope = ParrotScope_new(self->scope);
    ParrotScope_push_free(texture->scope, texture);

    texture->width = width;
    texture->height = height;

    texture->rgba8888 = calloc(width * height, sizeof(*rgba8888));
    ParrotScope_push_free(texture->scope, texture->rgba8888);

    memcpy(texture->rgba8888, rgba8888, width * height * sizeof(rgba8888));

    return texture;
}

void ParrotVideo_delete_texture(ParrotVideoTexture *texture) {
    PARROT_FAIL_NULL(texture);

    ParrotScope_delete(texture->scope);
}

ParrotVideoObjectHandle ParrotVideo_get_root(void) {
    return self->root_object_handle;
}

ParrotVideoObjectHandle ParrotVideo_create_object(void) {
    PARROT_FAIL_COND(!ParrotVideo_is_initialized());

    ParrotVideoObject *object = PARROT_ALLOC(ParrotVideoObject);

    object->scope = ParrotScope_new(self->scope);
    ParrotScope_push_free(object->scope, object);

    object->matrix = ParrotMat_identity();

    object->visible = true;
    object->tint = ParrotColor_WHITE;

    uint32_t index = self->next_pointer_id++;
    ParrotArray_put(self->hm_pointers, ((ParrotVideoObjectPointer){index, object}));

    ParrotVideoObjectHandle handle = (ParrotVideoObjectHandle){
        .index = index,
    };

    if (self->root_object) {
        object->parent = self->root_object_handle;
        ParrotArray_put(self->root_object->shm_children, (ParrotVideoObjectChild){handle});
    }
    ParrotScope_push_arrfree(object->scope, object->shm_children);

    if (!self->root_object) {
        self->root_object = object;
        self->root_object_handle = handle;
    }

    return handle;
}

void ParrotVideo_delete_object(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));

    ParrotVideoObject *object = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value;

    while (ParrotArray_size(object->shm_children) > 0) {
        ParrotVideo_delete_object(object->shm_children[0].key);
    }

    ParrotVideoObject *parent_object =
        ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, object->parent))->value;
    ParrotArray_delk(parent_object->shm_children, handle.index);

    ParrotArray_delk(self->hm_pointers, handle.index);
    ParrotScope_delete(object->scope);
}

bool ParrotVideo_does_object_exist(ParrotVideoObjectHandle handle) {
    return ParrotArray_find(self->hm_pointers, handle.index) >= 0;
}

void ParrotVideo_set_object_parent(ParrotVideoObjectHandle handle, ParrotVideoObjectHandle parent) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(parent));

    ParrotVideoObject *object = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value;
    PARROT_FAIL_NULL(object);

    ParrotVideoObject *new_parent = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, parent))->value;
    PARROT_FAIL_NULL(new_parent);

    ParrotVideoObject *old_parent =
        ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, object->parent))->value;
    PARROT_FAIL_NULL(old_parent);

    ParrotArray_delk(old_parent->shm_children, handle);
    ParrotArray_put(new_parent->shm_children, (ParrotVideoObjectChild){handle});
    object->parent = parent;
}

void ParrotVideo_set_object_visible(ParrotVideoObjectHandle handle, bool visible) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));

    ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value->visible = visible;
}

void ParrotVideo_set_object_tint(ParrotVideoObjectHandle handle, ParrotColor tint) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));

    ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value->tint = tint;
}

void ParrotVideo_set_object_matrix(ParrotVideoObjectHandle handle, ParrotMat matrix) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));

    ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value->matrix = matrix;
}

ParrotMat ParrotVideo_get_object_matrix(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));

    return ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value->matrix;
}

bool ParrotVideo_poll_object_events(ParrotVideoObjectHandle handle, ParrotVideoObjectEvent *out_event) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));

    ParrotVideoObject *object = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value;

    if (object->event_queue_read == object->event_queue_write) {
        return false;
    }

    *out_event = object->event_queue[object->event_queue_read++];
    return true;
}

void ParrotVideo_push_object_event(ParrotVideoObjectHandle handle, ParrotVideoObjectEvent event) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));

    ParrotVideoObject *object = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value;
    object->event_queue[object->event_queue_write++] = event;
}

void ParrotVideo_object_add_window(ParrotVideoObjectHandle handle, int width, int height) {
    PARROT_FAIL_COND(ParrotVideo_object_has_window(handle));

    PARROT_FAIL_COND(width == 0);
    PARROT_FAIL_COND(height == 0);

    ParrotVideoObject *object = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value;
    object->window = PARROT_ALLOC(ParrotVideoObjectWindow);

    object->window->scope = ParrotScope_new(object->scope);
    ParrotScope_push_free(object->window->scope, object->window);

    object->window->window = self->window_driver->create_window(self->window_driver, width, height);
    ParrotScope_push(object->window->scope, self->window_driver->vdelete_window, object->window->window);
}

void ParrotVideo_object_remove_window(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_window(handle));

    ParrotVideoObject *object = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value;

    ParrotScope_delete(object->window->scope);
    object->window = NULL;
}

bool ParrotVideo_object_has_window(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));

    return ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value->window;
}

bool ParrotVideo_object_is_window_close_requested(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_window(handle));

    ParrotVideoObject *object = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value;

    bool close_requested = object->window->close_requested;
    object->window->close_requested = false;
    return close_requested;
}

void ParrotVideo_object_set_window_title(ParrotVideoObjectHandle handle, const char *title) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_window(handle));

    self->window_driver->set_title(
        ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value->window->window, title);
}

void ParrotVideo_object_set_window_size(ParrotVideoObjectHandle handle, int width, int height) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_window(handle));

    PARROT_FAIL_COND(width == 0);
    PARROT_FAIL_COND(height == 0);

    self->window_driver->set_size(
        ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value->window->window,
        width,
        height);
}

int ParrotVideo_object_get_window_width(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_window(handle));
    return self->window_driver->get_width(
        ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value->window->window);
}

int ParrotVideo_object_get_window_height(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_window(handle));
    return self->window_driver->get_height(
        ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value->window->window);
}

void ParrotVideo_object_add_viewport(ParrotVideoObjectHandle handle, int width, int height) {
    PARROT_FAIL_COND(ParrotVideo_object_has_viewport(handle));

    PARROT_FAIL_COND(width == 0);
    PARROT_FAIL_COND(height == 0);

    ParrotVideoObject *object = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value;
    object->viewport = PARROT_ALLOC(ParrotVideoObjectViewport);

    object->viewport->scope = ParrotScope_new(object->scope);
    ParrotScope_push_free(object->viewport->scope, object->viewport);

    object->viewport->viewport = self->video_driver->create_viewport(self->video_driver, width, height);
    ParrotScope_push(object->viewport->scope, self->video_driver->vdelete_viewport, object->viewport->viewport);

    object->viewport->width = width;
    object->viewport->height = height;
}

void ParrotVideo_object_remove_viewport(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_viewport(handle));

    ParrotVideoObject *object = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value;

    ParrotScope_delete(object->viewport->scope);
    object->viewport = NULL;
}

bool ParrotVideo_object_has_viewport(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));

    return ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value->viewport;
}

int ParrotVideo_object_get_viewport_width(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_viewport(handle));

    return ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value->viewport->width;
}

int ParrotVideo_object_get_viewport_height(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_viewport(handle));

    return ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value->viewport->height;
}

void ParrotVideo_object_set_viewport_size(ParrotVideoObjectHandle handle, int width, int height) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_viewport(handle));

    PARROT_FAIL_COND(width == 0);
    PARROT_FAIL_COND(height == 0);
}

bool ParrotVideo_object_is_viewport_key_down(ParrotVideoObjectHandle handle, ParrotWindowDriverEventKey key) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_viewport(handle));
    PARROT_FAIL_COND(key >= ParrotWindowDriverEventKey_COUNT);

    return ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))
        ->value->viewport->physical_keys[key];
}

bool ParrotVideo_object_is_viewport_logical_key_down(ParrotVideoObjectHandle handle, ParrotWindowDriverEventKey key) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_viewport(handle));
    PARROT_FAIL_COND(key >= ParrotWindowDriverEventKey_COUNT);

    return ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))
        ->value->viewport->logical_keys[key];
}

bool ParrotVideo_object_is_viewport_mouse_button_down(ParrotVideoObjectHandle handle,
                                                      ParrotWindowDriverEventMouseButton button) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_viewport(handle));
    PARROT_FAIL_COND(button >= ParrotWindowDriverEventMouseButton_COUNT);

    return ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))
        ->value->viewport->mouse_buttons[button];
}

ParrotVec2 ParrotVideo_object_get_viewport_mouse_position(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_viewport(handle));

    return ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))
        ->value->viewport->mouse_position;
}

void ParrotVideo_object_add_camera(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(ParrotVideo_object_has_camera(handle));

    ParrotVideoObject *object = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value;
    object->camera = PARROT_ALLOC(ParrotVideoObjectCamera);

    object->camera->scope = ParrotScope_new(object->scope);
    ParrotScope_push_free(object->camera->scope, object->camera);

    object->camera->use_clear_color = true;
}

void ParrotVideo_object_remove_camera(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_camera(handle));

    ParrotVideoObject *object = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value;

    ParrotScope_delete(object->camera->scope);
    object->camera = NULL;
}

bool ParrotVideo_object_has_camera(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));

    return ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value->camera;
}

void ParrotVideo_object_set_camera_corner_aligned(ParrotVideoObjectHandle handle, bool value) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_camera(handle));

    ParrotVideoObject *object = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value;

    object->camera->corner_aligned = value;
}

void ParrotVideo_object_set_camera_clear_color(ParrotVideoObjectHandle handle, ParrotColor color) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_camera(handle));

    ParrotVideoObject *object = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value;

    object->camera->clear_color = color;
    object->camera->use_clear_color = true;
}

void ParrotVideo_object_clear_camera_clear_color(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_camera(handle));

    ParrotVideoObject *object = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value;

    object->camera->use_clear_color = false;
}

void ParrotVideo_object_add_rect(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(ParrotVideo_object_has_rect(handle));

    ParrotVideoObject *object = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value;

    object->rect = PARROT_ALLOC(ParrotVideoObjectRect);

    object->rect->scope = ParrotScope_new(object->scope);
    ParrotScope_push_free(object->rect->scope, object->rect);
}

void ParrotVideo_object_remove_rect(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_rect(handle));

    ParrotVideoObject *object = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value;
    ParrotScope_delete(object->rect->scope);
    object->rect = NULL;
}

bool ParrotVideo_object_has_rect(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));

    return ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value->rect;
}

void ParrotVideo_object_set_rect_texture(ParrotVideoObjectHandle handle,
                                         ParrotVideoTexture *texture,
                                         bool nearest_filter) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_rect(handle));

    ParrotVideoObject *object = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value;

    object->rect->texture = texture;

    if (texture) {
        object->rect->texture_region_x = 0, object->rect->texture_region_y = 0;
        object->rect->texture_region_width = texture->width, object->rect->texture_region_width = texture->height;
        object->rect->texture_nearest_filter = nearest_filter;
    }
}

void ParrotVideo_object_set_rect_texture_region(ParrotVideoObjectHandle handle, int x, int y, int width, int height) {
    PARROT_FAIL_COND(width == 0);
    PARROT_FAIL_COND(height == 0);

    PARROT_FAIL_COND(!ParrotVideo_object_has_rect(handle));

    ParrotVideoObject *object = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value;

    PARROT_FAIL_NULL(object->rect->texture);

    object->rect->texture_region_x = x, object->rect->texture_region_y = y;
    object->rect->texture_region_width = width, object->rect->texture_region_height = height;
}

void ParrotVideo_object_set_rect_size(ParrotVideoObjectHandle handle, ParrotReal width, ParrotReal height) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_rect(handle));

    ParrotVideoObject *object = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value;

    object->rect->width = width;
    object->rect->height = height;
}

void ParrotVideo_object_set_custom_draw(ParrotVideoObjectHandle handle,
                                        ParrotScope *scope,
                                        ParrotVideoCustomDrawFunc func) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));

    PARROT_FAIL_NULL(func);

    ParrotVideoObject *object = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value;

    if (object->custom_draw) {
        ParrotScope_delete(object->custom_draw->scope);
    }

    object->custom_draw = PARROT_ALLOC(ParrotVideoObjectCustomDraw);

    object->custom_draw->scope = ParrotScope_new(object->scope);
    ParrotScope_push_free(object->custom_draw->scope, object->custom_draw);

    object->custom_draw->func_scope = scope;
    if (scope) {
        ParrotScope_set_parent(scope, object->custom_draw->scope);
    }

    object->custom_draw->func = func;
}

void ParrotVideo_object_clear_custom_draw(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));

    ParrotVideoObject *object = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value;

    if (object->custom_draw) {
        ParrotScope_delete(object->custom_draw->scope);
    }
    object->custom_draw = NULL;
}

void ParrotVideo_object_add_ui_window(ParrotVideoObjectHandle handle, int width, int height) {
    PARROT_FAIL_COND(ParrotVideo_object_has_ui_window(handle));

    ParrotVideoObject *object = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value;

    object->ui_window = PARROT_ALLOC(ParrotVideoObjectUIWindow);

    object->ui_window->scope = ParrotScope_new(object->scope);
    ParrotScope_push_free(object->ui_window->scope, object->ui_window);

    ParrotVideo_object_set_ui_window_title(handle, "");

    object->ui_window->width = width;
    object->ui_window->height = height;
}

void ParrotVideo_object_remove_ui_window(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_ui_window(handle));

    ParrotVideoObject *object = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value;

    ParrotScope_delete(object->ui_window->scope);
    object->ui_window->scope = NULL;
}

bool ParrotVideo_object_has_ui_window(ParrotVideoObjectHandle handle) {
    ParrotVideoObject *object = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value;
    return object->ui_window;
}

bool ParrotVideo_object_is_ui_window_close_requested(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_ui_window(handle));

    ParrotVideoObject *object = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value;

    bool value = object->ui_window->close_requested;
    object->ui_window->close_requested = false;
    return value;
}

void ParrotVideo_object_set_ui_window_title(ParrotVideoObjectHandle handle, const char *title) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_ui_window(handle));

    ParrotVideoObject *object = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value;

    if (object->ui_window->title_scope) {
        ParrotScope_delete(object->ui_window->title_scope);
    }

    object->ui_window->title_scope = ParrotScope_new(object->ui_window->scope);
    object->ui_window->title = strcpy(calloc(strlen(title) + 1, sizeof(char)), title);
    ParrotScope_push_free(object->ui_window->title_scope, object->ui_window->title);
}

void ParrotVideo_object_set_ui_window_size(ParrotVideoObjectHandle handle, int width, int height) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_ui_window(handle));

    ParrotVideoObject *object = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value;

    object->ui_window->width = width;
    object->ui_window->height = height;
}

int ParrotVideo_object_get_ui_window_width(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_ui_window(handle));

    ParrotVideoObject *object = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value;
    return object->ui_window->width;
}

int ParrotVideo_object_get_ui_window_height(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_ui_window(handle));

    ParrotVideoObject *object = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value;
    return object->ui_window->height;
}

static bool ParrotVideo_find_camera(ParrotVideoObjectHandle handle,
                                    ParrotVideoObjectHandle *out_handle,
                                    ParrotVideoObjectCamera **out_camera) {
    ParrotVideoObject *object = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value;

    if (!object->visible) {
        return false;
    }

    if (object->camera) {
        *out_handle = handle;
        *out_camera = object->camera;
        return true;
    }

    for (size_t i = 0; i < ParrotArray_size(object->shm_children); i++) {
        if (ParrotVideo_find_camera(object->shm_children[i].key, out_handle, out_camera)) {
            return true;
        }
    }
    return false;
}

static void render_object(ParrotVideoObjectHandle handle,
                          ParrotVideoDriverViewport *viewport,
                          ParrotMat view_matrix,
                          ParrotMat projection_matrix,
                          ParrotColor tint) {
    ParrotVideoObject *object = ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, handle.index))->value;

    tint = ParrotColor_mul(tint, object->tint);

    if (ParrotVideo_object_has_window(handle)) {
        ParrotWindowDriverEvent event = {0};
        while (self->window_driver->poll_events(object->window->window, &event)) {
            switch (event.type) {
            case ParrotWindowDriverEventType_QUIT: {
                object->window->close_requested = true;
            } break;
            case ParrotWindowDriverEventType_KEY: {
                if (!ParrotVideo_object_has_viewport(handle)) {
                    break;
                }

                object->viewport->physical_keys[event.data.key.physical_key] = event.data.key.key_down;
                object->viewport->logical_keys[event.data.key.logical_key] = event.data.key.key_down;
            } break;
            case ParrotWindowDriverEventType_MOUSE_BUTTON: {
                if (!ParrotVideo_object_has_viewport(handle)) {
                    break;
                }

                object->viewport->mouse_buttons[event.data.mouse_button.button] = event.data.mouse_button.button_down;
            } break;
            case ParrotWindowDriverEventType_MOUSE_MOTION: {
                if (!ParrotVideo_object_has_viewport(handle)) {
                    break;
                }

                object->viewport->mouse_position = event.data.mouse_motion.position;
            } break;
            default:
                break;
            }

            ParrotVideo_push_object_event(handle,
                                          (ParrotVideoObjectEvent){
                                              .type = ParrotVideoObjectEventType_WINDOW,
                                              .data.window = event,
                                          });
        }
    }

    if (ParrotVideo_object_has_viewport(handle)) {
        viewport = object->viewport->viewport;
        ParrotVideoObjectHandle camera_handle;
        ParrotVideoObjectCamera *camera;

        if (ParrotVideo_find_camera(handle, &camera_handle, &camera)) {
            ParrotVideoObject *camera_object =
                ((ParrotVideoObjectPointer *)ParrotArray_findp(self->hm_pointers, camera_handle.index))->value;

            if (camera->use_clear_color) {
                self->video_driver->clear_viewport(object->viewport->viewport, camera->clear_color);
            }

            view_matrix = ParrotMat_inverse(camera_object->matrix);
            projection_matrix =
                ParrotMat_ortho(!camera->corner_aligned ? -object->viewport->width / 2 : 0,
                                !camera->corner_aligned ? object->viewport->width / 2 : object->viewport->width,
                                !camera->corner_aligned ? -object->viewport->height / 2 : 0,
                                !camera->corner_aligned ? object->viewport->height / 2 : object->viewport->height,
                                -1000000,
                                1000000);
        }
    }

    if (object->visible) {
        for (size_t i = 0; i < ParrotArray_size(object->shm_children); i++) {
            render_object(object->shm_children[i].key, viewport, view_matrix, projection_matrix, tint);
        }
    }

    if (ParrotVideo_object_has_viewport(handle) && ParrotVideo_object_has_window(handle)) {
        self->window_driver->set_image_native(
            object->window->window,
            self->video_driver->get_viewport_pixels(
                object->viewport->viewport, self->window_driver->get_native_image_format(object->window->window)));
    }

    PARROT_RET_COND(!viewport);
    PARROT_RET_COND(!object->visible);

    if (ParrotVideo_object_has_rect(handle)) {
        ParrotReal width = object->rect->width;
        ParrotReal height = object->rect->height;

        float x0 = object->rect->texture ? object->rect->texture_region_x / (float)object->rect->texture->width : 0;
        float y0 = object->rect->texture ? object->rect->texture_region_y / (float)object->rect->texture->height : 0;
        float x1 =
            object->rect->texture ? x0 + object->rect->texture_region_width / (float)object->rect->texture->width : 0;
        float y1 =
            object->rect->texture ? y0 + object->rect->texture_region_height / (float)object->rect->texture->height : 0;

        ParrotVideoVertex vertices[] = {
            (ParrotVideoVertex){.position = (ParrotVec3){0, 0, 0}, .uv = {x0, y0}, .tint = tint},
            (ParrotVideoVertex){.position = (ParrotVec3){width, 0, 0}, .uv = {x1, y0}, .tint = tint},
            (ParrotVideoVertex){.position = (ParrotVec3){0, height, 0}, .uv = {x0, y1}, .tint = tint},

            (ParrotVideoVertex){.position = (ParrotVec3){width, height, 0}, .uv = {x1, y1}, .tint = tint},
            (ParrotVideoVertex){.position = (ParrotVec3){0, height, 0}, .uv = {x0, y1}, .tint = tint},
            (ParrotVideoVertex){.position = (ParrotVec3){width, 0, 0}, .uv = {x1, y0}, .tint = tint},
        };

        if (object->rect->texture) {
            self->video_driver->set_viewport_texture(viewport,
                                                     object->rect->texture->width,
                                                     object->rect->texture->height,
                                                     object->rect->texture->rgba8888,
                                                     object->rect->texture_nearest_filter);
        }
        self->video_driver->draw_viewport_vertices(viewport,
                                                   (ParrotGMatSet){
                                                       .model = object->matrix,
                                                       .view = view_matrix,
                                                       .projection = projection_matrix,
                                                   },
                                                   vertices,
                                                   6);
        self->video_driver->clear_viewport_texture(viewport);
    }

    if (object->custom_draw) {
        object->custom_draw->func(object->custom_draw->func_scope,
                                  self->video_driver,
                                  viewport,
                                  (ParrotGMatSet){
                                      .model = object->matrix,
                                      .view = view_matrix,
                                      .projection = projection_matrix,
                                  });
    }
}

void ParrotVideo_render(void) {
    PARROT_FAIL_COND(!ParrotVideo_is_initialized());

    render_object(self->root_object_handle, NULL, ParrotMat_identity(), ParrotMat_identity(), ParrotColor_WHITE);
}
