#include "parrot/video/video.h"
#include "parrot/core/math.h"
#include "parrot/core/scope.h"
#include "parrot/core/util.h"
#include "parrot/drivers/video_driver.h"
#include "parrot/drivers/window_driver.h"
#include "parrot/video/font.h"
#include "src/ds.h"
#include "stb_ds.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct ParrotVideoObject ParrotVideoObject;

typedef struct {
    ParrotWindowDriverWindow *window;

    int width;
    int height;

    bool close_requested;

    bool physical_keys[ParrotWindowDriverEventKey_COUNT];
    bool logical_keys[ParrotWindowDriverEventKey_COUNT];
} ParrotVideoObjectWindow;

typedef struct {
    ParrotVideoDriverViewport *viewport;

    int width;
    int height;
} ParrotVideoObjectViewport;

typedef struct {
    bool corner_aligned;

    bool use_clear_color;
    ParrotColor clear_color;
} ParrotVideoObjectCamera;

typedef struct {
    ParrotReal width;
    ParrotReal height;

    int texture_width;
    int texture_height;
    uint32_t *texture_rgba8888;

    int texture_region_x;
    int texture_region_y;
    int texture_region_width;
    int texture_region_height;
    bool texture_nearest_filter;
} ParrotVideoObjectRect;

typedef struct {
    ParrotVideoFont *font;
    float size;
    const char *text;
} ParrotVideoObjectText;

typedef struct {
    ParrotVideoObjectHandle key;
} ParrotVideoObjectChild;

struct ParrotVideoObject {
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
    ParrotVideoObjectText *text;
};

typedef struct ParrotVideoObjectPointer {
    uint32_t key;
    ParrotVideoObject *value;
} ParrotVideoObjectPointer;

typedef struct {
    ParrotScope *scope;

    ParrotVideoObjectPointer *hm_pointers;
    uint32_t next_pointer_id;

    ParrotVideoObject *root_object;
    ParrotVideoObjectHandle root_object_handle;
} ParrotVideo;

static ParrotVideo *self = NULL;

void ParrotVideo_init(void) {
    PARROT_FAIL_COND(ParrotVideo_is_initialized());

    self = malloc(sizeof(ParrotVideo));
    PARROT_FAIL_NULL(self);
    memset(self, 0, sizeof(ParrotVideo));

    self->scope = ParrotScope_new(NULL);

    ParrotScope_push(self->scope, Parrot_hmfree_scope_wrapper, Parrot_hmfree_scope_wrapper_PACK_CTX(self->hm_pointers));

    ParrotVideo_create_object();
}

void ParrotVideo_shutdown(void) {
    PARROT_FAIL_COND(!ParrotVideo_is_initialized());

    ParrotScope_delete(self->scope);
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
    object->tint = ParrotColor_WHITE;

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

void ParrotVideo_set_object_tint(ParrotVideoObjectHandle handle, ParrotColor tint) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));

    hmget(self->hm_pointers, handle.index)->tint = tint;
}

void ParrotVideo_set_object_matrix(ParrotVideoObjectHandle handle, ParrotMat matrix) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));

    hmget(self->hm_pointers, handle.index)->matrix = matrix;
}

ParrotMat ParrotVideo_get_object_matrix(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));

    return hmget(self->hm_pointers, handle.index)->matrix;
}

bool ParrotVideo_poll_object_events(ParrotVideoObjectHandle handle, ParrotVideoObjectEvent *out_event) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));

    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);

    if (object->event_queue_read == object->event_queue_write) {
        return false;
    }

    *out_event = object->event_queue[object->event_queue_read++];
    return true;
}

void ParrotVideo_push_object_event(ParrotVideoObjectHandle handle, ParrotVideoObjectEvent event) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));

    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);
    object->event_queue[object->event_queue_write++] = event;
}

void ParrotVideo_object_add_window(ParrotVideoObjectHandle handle, int width, int height) {
    PARROT_FAIL_COND(ParrotVideo_object_has_window(handle));

    PARROT_FAIL_COND(width == 0);
    PARROT_FAIL_COND(height == 0);

    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);
    object->window = malloc(sizeof(ParrotVideoObjectWindow));
    memset(object->window, 0, sizeof(ParrotVideoObjectWindow));

    object->window->window = Parrot_window_driver->create_window(width, height);
}

void ParrotVideo_object_remove_window(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_window(handle));

    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);

    Parrot_window_driver->delete_window(object->window->window);

    free(object->window);
    object->window = NULL;
}

bool ParrotVideo_object_has_window(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));

    return hmget(self->hm_pointers, handle.index)->window;
}

bool ParrotVideo_object_is_window_close_requested(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_window(handle));

    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);

    bool close_requested = object->window->close_requested;
    object->window->close_requested = false;
    return close_requested;
}

void ParrotVideo_object_set_window_title(ParrotVideoObjectHandle handle, const char *title) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_window(handle));

    Parrot_window_driver->set_title(hmget(self->hm_pointers, handle.index)->window->window, title);
}

void ParrotVideo_object_set_window_size(ParrotVideoObjectHandle handle, int width, int height) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_window(handle));

    PARROT_FAIL_COND(width == 0);
    PARROT_FAIL_COND(height == 0);

    Parrot_window_driver->set_size(hmget(self->hm_pointers, handle.index)->window->window, width, height);
}

int ParrotVideo_object_get_window_width(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_window(handle));
    return Parrot_window_driver->get_width(hmget(self->hm_pointers, handle.index)->window->window);
}

int ParrotVideo_object_get_window_height(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_window(handle));
    return Parrot_window_driver->get_height(hmget(self->hm_pointers, handle.index)->window->window);
}

bool ParrotVideo_object_is_window_key_down(ParrotVideoObjectHandle handle, ParrotWindowDriverEventKey key) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_window(handle));

    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);

    return hmget(self->hm_pointers, handle.index)->window->physical_keys[key];
}

bool ParrotVideo_object_is_window_logical_key_down(ParrotVideoObjectHandle handle, ParrotWindowDriverEventKey key) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_window(handle));

    return hmget(self->hm_pointers, handle.index)->window->logical_keys[key];
}

void ParrotVideo_object_add_viewport(ParrotVideoObjectHandle handle, int width, int height) {
    PARROT_FAIL_COND(ParrotVideo_object_has_viewport(handle));

    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);
    object->viewport = malloc(sizeof(ParrotVideoObjectViewport));
    memset(object->viewport, 0, sizeof(ParrotVideoObjectViewport));

    PARROT_FAIL_COND(width == 0);
    PARROT_FAIL_COND(height == 0);

    object->viewport->viewport = Parrot_video_driver->create_viewport(width, height);
    object->viewport->width = width;
    object->viewport->height = height;
}

void ParrotVideo_object_remove_viewport(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_viewport(handle));

    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);

    if (ParrotVideo_object_has_window(handle)) {
        Parrot_window_driver->set_image(object->window->window, NULL);
    }

    Parrot_video_driver->delete_viewport(object->viewport->viewport);

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

void ParrotVideo_object_set_camera_corner_aligned(ParrotVideoObjectHandle handle, bool value) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_camera(handle));

    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);

    object->camera->corner_aligned = value;
}

void ParrotVideo_object_set_camera_clear_color(ParrotVideoObjectHandle handle, ParrotColor color) {
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
                                      ParrotVideoDriverViewport *viewport,
                                      ParrotMat view_matrix,
                                      ParrotMat projection_matrix,
                                      ParrotColor tint) {
    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);

    tint = ParrotColor_mul(tint, object->tint);

    if (ParrotVideo_object_has_window(handle)) {
        ParrotWindowDriverEvent event = {0};
        while (Parrot_window_driver->poll_events(object->window->window, &event)) {
            switch (event.type) {
            case ParrotWindowDriverEventType_QUIT: {
                object->window->close_requested = true;
            } break;
            case ParrotWindowDriverEventType_KEY: {
                object->window->physical_keys[event.data.key.physical_key] = event.data.key.key_down;
                object->window->logical_keys[event.data.key.logical_key] = event.data.key.key_down;
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
            ParrotVideoObject *camera_object = hmget(self->hm_pointers, camera_handle.index);

            if (camera->use_clear_color) {
                Parrot_video_driver->clear_viewport(object->viewport->viewport, camera->clear_color);
            }

            view_matrix = ParrotMat_inverse(camera_object->matrix);
            projection_matrix =
                ParrotMat_ortho(!camera->corner_aligned ? -object->viewport->width / 2 : 0,
                                !camera->corner_aligned ? object->viewport->width / 2 : object->viewport->width,
                                !camera->corner_aligned ? -object->viewport->height / 2 : 0,
                                !camera->corner_aligned ? object->viewport->height / 2 : object->viewport->height,
                                0,
                                1000000);
        }
    }

    if (object->visible) {
        for (size_t i = 0; i < hmlen(object->shm_children); i++) {
            ParrotVideo_render_object(object->shm_children[i].key, viewport, view_matrix, projection_matrix, tint);
        }
    }

    if (ParrotVideo_object_has_viewport(handle) && ParrotVideo_object_has_window(handle)) {
        Parrot_window_driver->set_image(object->window->window,
                                        Parrot_video_driver->get_viewport_pixels(object->viewport->viewport));
    }

    PARROT_RET_COND(!viewport);
    PARROT_RET_COND(!object->visible);

    if (ParrotVideo_object_has_rect(handle)) {
        ParrotReal width = object->rect->width;
        ParrotReal height = object->rect->height;

        float x0 =
            object->rect->texture_rgba8888 ? object->rect->texture_region_x / (float)object->rect->texture_width : 0;
        float y0 =
            object->rect->texture_rgba8888 ? object->rect->texture_region_y / (float)object->rect->texture_height : 0;
        float x1 = object->rect->texture_rgba8888 ?
                       x0 + object->rect->texture_region_width / (float)object->rect->texture_width :
                       0;
        float y1 = object->rect->texture_rgba8888 ?
                       y0 + object->rect->texture_region_height / (float)object->rect->texture_height :
                       0;

        ParrotVideoVertex vertices[] = {
            (ParrotVideoVertex){.position = (ParrotVec3){0, 0, 0}, .uv = {x0, y0}, .tint = tint},
            (ParrotVideoVertex){.position = (ParrotVec3){width, 0, 0}, .uv = {x1, y0}, .tint = tint},
            (ParrotVideoVertex){.position = (ParrotVec3){0, height, 0}, .uv = {x0, y1}, .tint = tint},

            (ParrotVideoVertex){.position = (ParrotVec3){width, height, 0}, .uv = {x1, y1}, .tint = tint},
            (ParrotVideoVertex){.position = (ParrotVec3){0, height, 0}, .uv = {x0, y1}, .tint = tint},
            (ParrotVideoVertex){.position = (ParrotVec3){width, 0, 0}, .uv = {x1, y0}, .tint = tint},
        };

        if (object->rect->texture_rgba8888) {
            Parrot_video_driver->set_viewport_texture(viewport,
                                                      object->rect->texture_width,
                                                      object->rect->texture_height,
                                                      object->rect->texture_rgba8888,
                                                      object->rect->texture_nearest_filter);
        }
        Parrot_video_driver->draw_viewport_vertices(viewport,
                                                    (ParrotGMatSet){
                                                        .model = object->matrix,
                                                        .view = view_matrix,
                                                        .projection = projection_matrix,
                                                    },
                                                    vertices,
                                                    6);
        Parrot_video_driver->clear_viewport_texture(viewport);
    }

    if (object->text) {
        struct {
            char key;
            ParrotVideoFontChar value;
        } *hm_chars = NULL;

        struct {
            char key;
            ParrotVec2 *value;
        } *hm_arr_char_positions = NULL;

        float max_character_height = 0;

        ParrotVec2 position = ParrotVec2_n(0);
        for (size_t i = 0; i < strlen(object->text->text); i++) {
            char c = object->text->text[i];

            switch (c) {
            case '\n': {
                position.y += max_character_height;
                position.x = 0;

                max_character_height = 0;
                continue;
            }
            case '\t': {
                position.x += object->text->size * 5;
                continue;
            }
            default:
                break;
            }

            if (hmgeti(hm_chars, object->text->text[i]) < 0) {
                hmput(hm_chars, c, ParrotVideoFont_char(object->text->font, object->text->size, c));
                hmput(hm_arr_char_positions, c, NULL);
            }

            ParrotVideoFontChar character = hmget(hm_chars, c);
            max_character_height = PARROT_MAX(max_character_height, character.height);

            ParrotVec2 *arr_positions = hmget(hm_arr_char_positions, c);
            ParrotVec2 character_position = ParrotVec2_add(position, (ParrotVec2){character.x, character.y});
            arrpush(arr_positions, character_position);
            hmput(hm_arr_char_positions, c, arr_positions);

            position.x += character.advance;
        }

        for (size_t i = 0; i < shlen(hm_chars); i++) {
            ParrotVideoVertex *arr_vertices = NULL;

            ParrotVideoFontChar character = hm_chars[i].value;
            int w = character.width;
            int h = character.height;

            for (size_t j = 0; j < arrlen(hm_arr_char_positions[i].value); j++) {
                float x = hm_arr_char_positions[i].value[j].x;
                float y = hm_arr_char_positions[i].value[j].y;

                ParrotVideoVertex a = {(ParrotVec3){x, y, 0}, .uv = {0, 0}, .tint = tint};
                ParrotVideoVertex b = {(ParrotVec3){x + w, y, 0}, .uv = {1, 0}, .tint = tint};
                ParrotVideoVertex c = {(ParrotVec3){x, y + h, 0}, .uv = {0, 1}, .tint = tint};
                ParrotVideoVertex d = {(ParrotVec3){x + w, y + h, 0}, .uv = {1, 1}, .tint = tint};
                ParrotVideoVertex e = {(ParrotVec3){x, y + h, 0}, .uv = {0, 1}, .tint = tint};
                ParrotVideoVertex f = {(ParrotVec3){x + w, y, 0}, .uv = {1, 0}, .tint = tint};

                arrpush(arr_vertices, a);
                arrpush(arr_vertices, b);
                arrpush(arr_vertices, c);
                arrpush(arr_vertices, d);
                arrpush(arr_vertices, e);
                arrpush(arr_vertices, f);
            }

            uint32_t *rgba8888 = calloc(w * h, sizeof(uint32_t));
            {
                for (int y = 0; y < h; y++) {
                    for (int x = 0; x < w; x++) {
                        rgba8888[y * w + x] =
                            ParrotColor_to_rgba8888(ParrotColor_newa(255, 255, 255, character.bitmap[y * w + x]));
                    }
                }
                free(character.bitmap);

                Parrot_video_driver->set_viewport_texture(viewport, w, h, rgba8888, false);
                Parrot_video_driver->draw_viewport_vertices(viewport,
                                                            (ParrotGMatSet){
                                                                .model = object->matrix,
                                                                .view = view_matrix,
                                                                .projection = projection_matrix,
                                                            },
                                                            arr_vertices,
                                                            arrlen(arr_vertices));
                Parrot_video_driver->clear_viewport_texture(viewport);
            }
            free(rgba8888);

            arrfree(hm_arr_char_positions[i].value);
            arrfree(arr_vertices);
        }
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
    ParrotVideo_object_clear_rect_texture(handle);

    free(object->rect);
    object->rect = NULL;
}

bool ParrotVideo_object_has_rect(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));

    return hmget(self->hm_pointers, handle.index)->rect;
}

void ParrotVideo_object_set_rect_texture(
    ParrotVideoObjectHandle handle, int width, int height, const uint32_t *rgba8888, bool nearest_filter) {
    PARROT_FAIL_COND(width == 0);
    PARROT_FAIL_COND(height == 0);
    PARROT_FAIL_NULL(rgba8888);

    PARROT_FAIL_COND(!ParrotVideo_object_has_rect(handle));

    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);

    ParrotVideo_object_clear_rect_texture(handle);

    object->rect->texture_width = width, object->rect->texture_region_width = width;
    object->rect->texture_height = height, object->rect->texture_region_height = height;
    object->rect->texture_region_x = 0, object->rect->texture_region_y = 0;
    object->rect->texture_nearest_filter = nearest_filter;

    object->rect->texture_rgba8888 = calloc(width * height, sizeof(uint32_t));
    memcpy(object->rect->texture_rgba8888, rgba8888, width * height * sizeof(uint32_t));
}

void ParrotVideo_object_clear_rect_texture(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_rect(handle));

    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);

    free(object->rect->texture_rgba8888);
    object->rect->texture_rgba8888 = NULL;
}

void ParrotVideo_object_set_rect_texture_region(ParrotVideoObjectHandle handle, int x, int y, int width, int height) {
    PARROT_FAIL_COND(width == 0);
    PARROT_FAIL_COND(height == 0);

    PARROT_FAIL_COND(!ParrotVideo_object_has_rect(handle));

    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);

    PARROT_FAIL_NULL(object->rect->texture_rgba8888);

    object->rect->texture_region_x = x, object->rect->texture_region_y = y;
    object->rect->texture_region_width = width, object->rect->texture_region_height = height;
}

void ParrotVideo_object_set_rect_size(ParrotVideoObjectHandle handle, ParrotReal width, ParrotReal height) {
    PARROT_FAIL_COND(!ParrotVideo_object_has_rect(handle));

    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);

    object->rect->width = width;
    object->rect->height = height;
}

void ParrotVideo_object_set_text(ParrotVideoObjectHandle handle, ParrotVideoFont *font, float size, const char *text) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));
    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);

    if (object->text) {
        ParrotVideo_object_clear_text(handle);
    }

    object->text = malloc(sizeof(ParrotVideoObjectText));
    memset(object->text, 0, sizeof(*object->text));

    object->text->font = font;
    object->text->size = size;
    object->text->text = text;
}

void ParrotVideo_object_clear_text(ParrotVideoObjectHandle handle) {
    PARROT_FAIL_COND(!ParrotVideo_does_object_exist(handle));
    ParrotVideoObject *object = hmget(self->hm_pointers, handle.index);

    free(object->text);
    object->text = NULL;
}

void ParrotVideo_render(void) {
    PARROT_FAIL_COND(!ParrotVideo_is_initialized());

    ParrotVideo_render_object(
        self->root_object_handle, NULL, ParrotMat_identity(), ParrotMat_identity(), ParrotColor_WHITE);
}
