#include "parrot/ui/ui.h"
#include "parrot/core/math.h"
#include "parrot/core/scope.h"
#include "parrot/drivers/window_driver.h"
#include "parrot/scene/world.h"
#include "parrot/stb_ds.h"
#include "parrot/video/scene.h"
#include "parrot/video/video.h"
#include <string.h>

typedef struct {
    ParrotUISystem *self;
    ParrotSceneWorldEntity entity;
} State;

typedef struct {
    ParrotVec2 position;
    ParrotVec2 size;

    ParrotGMatSet matrix;

    void (*on_click)(uint64_t frames,
                     ParrotVideoObjectHandle viewport_handle,
                     ParrotWindowDriverEventMouseButton button,
                     ParrotSceneWorld *world,
                     ParrotSceneWorldEntity entity);
} ParrotUIInternalHitZone;

typedef struct {
    ParrotScope *scope;

    ParrotVec2 ui_window_last_mouse_position;

    ParrotUIInternalHitZone *arr_hit_zones;
} ParrotUIInternal;

struct ParrotUISystem {
    ParrotScope *scope;

    ParrotSceneWorld *world;

    ParrotVideoObjectHandle root_handle;

    ParrotUIInternalHitZone mouse_button_hit[ParrotWindowDriverEventMouseButton_COUNT];
    bool mouse_button_hit_found[ParrotWindowDriverEventMouseButton_COUNT];
    uint64_t mouse_button_frames[ParrotWindowDriverEventMouseButton_COUNT];
};

static void
draw_entity(ParrotScope *scope, ParrotVideoDriver *driver, ParrotVideoDriverViewport *viewport, ParrotGMatSet matrix);

static void ParrotUIComponent_constructor(ParrotSceneWorldEntity entity, void *self_ptr, void *user_data) {
    (void)entity;

    ParrotUISystem *self = user_data;

    ParrotSceneWorld_add_component(self->world, entity, ParrotUIInternal);

    ParrotUIComponent *ui = self_ptr;

    ParrotUIInternal *ui_internal = ParrotSceneWorld_get_component(self->world, entity, ParrotUIInternal);

    ui_internal->scope = ParrotScope_new(self->scope);
    ParrotScope_push_arrfree(ui_internal->scope, ui_internal->arr_hit_zones);

    PARROT_FAIL_COND(!ParrotVideo_is_initialized());

    ui->object_handle = ParrotVideo_create_object();

    ParrotScope *scope = ParrotScope_new(NULL);
    State *state = ParrotScope_alloc_ctx(scope, State);

    state->self = self;
    state->entity = entity;

    ParrotVideo_object_set_custom_draw(ui->object_handle, scope, draw_entity);
}

ParrotUISystem *ParrotUISystem_new(ParrotSceneWorld *world, ParrotVideoObjectHandle root_handle) {
    PARROT_FAIL_NULL(world);

    ParrotUISystem *self = PARROT_ALLOC(ParrotUISystem);

    self->scope = ParrotScope_new(NULL);
    ParrotScope_push_free(self->scope, self);

    self->world = world;

    self->root_handle = root_handle;

    ParrotSceneWorld_register_component(world,
                                        PARROT_TYPE_STRING(ParrotUIComponent),
                                        (ParrotSceneWorldComponentDescription){
                                            .size = sizeof(ParrotUIComponent),
                                            .constructor = ParrotUIComponent_constructor,
                                            .user_data = self,
                                        });

    return self;
}

void ParrotUISystem_delete(ParrotUISystem *self) {
    PARROT_FAIL_NULL(self);

    ParrotScope_delete(self->scope);
}

void ParrotUISystem_vdelete(void *self) {
    ParrotUISystem_delete(self);
}

static ParrotVideoObjectHandle
get_parent(ParrotSceneWorld *world, ParrotVideoObjectHandle root_handle, ParrotSceneWorldEntity entity) {
    entity = ParrotSceneWorld_get_entity_parent(world, entity);
    for (; entity != ParrotSceneWorldEntity_NULL; entity = ParrotSceneWorld_get_entity_parent(world, entity)) {
        ParrotUIComponent *ui_component = ParrotSceneWorld_get_component(world, entity, ParrotUIComponent);
        if (ui_component && !ParrotSceneWorld_is_component_deletion_queued(world, entity, ParrotUIComponent)) {
            return ui_component->object_handle;
        }

        ParrotVideoSceneRenderableComponent *renderable =
            ParrotSceneWorld_get_component(world, entity, ParrotVideoSceneRenderableComponent);
        if (!renderable) {
            continue;
        }

        if (ParrotSceneWorld_is_component_deletion_queued(world, entity, ParrotVideoSceneRenderableComponent)) {
            continue;
        }

        return renderable->object_handle;
    }

    return root_handle;
}

static void update_tree(ParrotUISystem *self, ParrotSceneWorldQuery *query) {
    for (size_t i = 0; i < ParrotSceneWorld_query_result_count(self->world, query); i++) {
        ParrotSceneWorldEntity entity = ParrotSceneWorld_query_result_at(self->world, query, i);

        ParrotUIComponent *ui_component = ParrotSceneWorld_get_component(self->world, entity, ParrotUIComponent);
        ParrotVideo_set_object_parent(ui_component->object_handle, get_parent(self->world, self->root_handle, entity));
    }
}

void ParrotUISystem_update(ParrotUISystem *self, ParrotVideoObjectHandle viewport_handle) {
    PARROT_FAIL_NULL(self);

    PARROT_FAIL_COND(!ParrotVideo_is_initialized());

    ParrotSceneWorldQuery query[] = {
        PARROT_SCENE_WORLD_QUERY_WITH_COMPONENT(ParrotUIComponent),

        PARROT_SCENE_WORLD_QUERY_END(),
    };

    update_tree(self, query);

    for (size_t i = 0; i < ParrotSceneWorld_query_result_count(self->world, query); i++) {
        ParrotSceneWorldEntity entity = ParrotSceneWorld_query_result_at(self->world, query, i);

        ParrotUIComponent *ui = ParrotSceneWorld_get_component(self->world, entity, ParrotUIComponent);

        ParrotUIInternal *ui_internal = ParrotSceneWorld_get_component(self->world, entity, ParrotUIInternal);
        PARROT_FAIL_NULL(ui_internal);

        if (ParrotSceneWorld_is_component_deletion_queued(self->world, entity, ParrotUIComponent)) {
            ParrotScope_delete(ui_internal->scope);
            ParrotSceneWorld_queue_delete_component(self->world, entity, ParrotUIInternal);

            ParrotVideo_delete_object(ui->object_handle);
            continue;
        }

        ParrotTransform *transform = ParrotSceneWorld_get_component(self->world, entity, ParrotTransform);
        if (transform) {
            ParrotVideo_set_object_matrix(ui->object_handle, ParrotTransform_calculate_matrix(transform));
        }

        int screen_width = ParrotVideo_object_get_viewport_width(viewport_handle);
        int screen_height = ParrotVideo_object_get_viewport_height(viewport_handle);

        ParrotVec2 mouse_screen = ParrotVideo_object_get_viewport_mouse_position(viewport_handle);

        for (ParrotWindowDriverEventMouseButton btn = 0; btn < ParrotWindowDriverEventMouseButton_COUNT; btn++) {
            if (ParrotVideo_object_is_viewport_mouse_button_down(viewport_handle, btn)) {
                if (++self->mouse_button_frames[btn] != 1) {
                    continue;
                }

                for (size_t j = arrlen(ui_internal->arr_hit_zones); j > 0; j--) {
                    ParrotUIInternalHitZone zone = ui_internal->arr_hit_zones[j - 1];

                    ParrotMat combined = ParrotGMatSet_combine(&zone.matrix);
                    ParrotVec2 zone_min_ndc = ParrotMat_transform2(combined, zone.position);
                    ParrotVec2 zone_max_ndc = ParrotMat_transform2(
                        combined, (ParrotVec2){zone.position.x + zone.size.x, zone.position.y + zone.size.y});

                    ParrotVec2 zone_min_screen = {
                        (zone_min_ndc.x + 1) / 2 * screen_width,
                        (zone_min_ndc.y + 1) / 2 * screen_height,
                    };
                    ParrotVec2 zone_max_screen = {
                        (zone_max_ndc.x + 1) / 2 * screen_width,
                        (zone_max_ndc.y + 1) / 2 * screen_height,
                    };

                    float y_min = PARROT_MIN(zone_min_screen.y, zone_max_screen.y);
                    float y_max = PARROT_MAX(zone_min_screen.y, zone_max_screen.y);
                    float x_min = PARROT_MIN(zone_min_screen.x, zone_max_screen.x);
                    float x_max = PARROT_MAX(zone_min_screen.x, zone_max_screen.x);

                    bool hit = mouse_screen.x >= x_min && mouse_screen.x <= x_max && mouse_screen.y >= y_min &&
                               mouse_screen.y <= y_max;
                    if (hit) {
                        self->mouse_button_hit[btn] = zone;
                        self->mouse_button_hit_found[btn] = true;
                        break;
                    }
                }

                continue;
            }

            self->mouse_button_frames[btn] = 0;
            self->mouse_button_hit_found[btn] = false;
        }

        for (ParrotWindowDriverEventMouseButton btn = 0; btn < ParrotWindowDriverEventMouseButton_COUNT; btn++) {
            if (!self->mouse_button_frames[btn]) {
                continue;
            }

            if (!self->mouse_button_hit_found[btn]) {
                continue;
            }

            self->mouse_button_hit[btn].on_click(
                self->mouse_button_frames[btn], viewport_handle, btn, self->world, entity);
        }

        arrsetlen(ui_internal->arr_hit_zones, 0);
    }
}

static ParrotVideoFont *get_font(ParrotSceneWorld *world, ParrotSceneWorldEntity entity) {
    ParrotUIComponent *ui_component = ParrotSceneWorld_get_component(world, entity, ParrotUIComponent);
    PARROT_RET_COND_V(ui_component && ui_component->font, ui_component->font);

    entity = ParrotSceneWorld_get_entity_parent(world, entity);
    PARROT_RET_COND_V(entity == ParrotSceneWorldEntity_NULL, NULL);

    return get_font(world, entity);
}

static void draw_text(ParrotVideoDriver *driver,
                      ParrotVideoDriverViewport *viewport,
                      ParrotGMatSet matrix_set,
                      ParrotVideoFont *font,
                      const char *text,
                      float size,
                      ParrotColor color) {
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
    for (size_t i = 0; i < strlen(text); i++) {
        char c = text[i];

        switch (c) {
        case '\n': {
            position.y += max_character_height;
            position.x = 0;

            max_character_height = 0;
            continue;
        }
        default:
            break;
        }

        if (hmgeti(hm_chars, text[i]) < 0) {
            hmput(hm_chars, c, ParrotVideoFont_char(font, size, c));
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

            ParrotVideoVertex a = {(ParrotVec3){x, y, 0}, .uv = {0, 0}, .tint = color};
            ParrotVideoVertex b = {(ParrotVec3){x + w, y, 0}, .uv = {1, 0}, .tint = color};
            ParrotVideoVertex c = {(ParrotVec3){x, y + h, 0}, .uv = {0, 1}, .tint = color};
            ParrotVideoVertex d = {(ParrotVec3){x + w, y + h, 0}, .uv = {1, 1}, .tint = color};
            ParrotVideoVertex e = {(ParrotVec3){x, y + h, 0}, .uv = {0, 1}, .tint = color};
            ParrotVideoVertex f = {(ParrotVec3){x + w, y, 0}, .uv = {1, 0}, .tint = color};

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
                    rgba8888[y * w + x] = ((uint32_t)character.bitmap[y * w + x] << 24) | 0xFFFFFF;
                }
            }
            free(character.bitmap);

            driver->set_viewport_texture(viewport, w, h, rgba8888, false);
            driver->draw_viewport_vertices(viewport, matrix_set, arr_vertices, arrlen(arr_vertices));
            driver->clear_viewport_texture(viewport);
        }
        free(rgba8888);

        arrfree(hm_arr_char_positions[i].value);
        arrfree(arr_vertices);
    }

    hmfree(hm_chars);
    hmfree(hm_arr_char_positions);
}

static void window_on_titlebar_click(uint64_t frames,
                                     ParrotVideoObjectHandle viewport_handle,
                                     ParrotWindowDriverEventMouseButton button,
                                     ParrotSceneWorld *world,
                                     ParrotSceneWorldEntity entity) {
    PARROT_RET_COND(button != ParrotWindowDriverEventMouseButton_LEFT);

    ParrotUIWindowComponent *ui_window = ParrotSceneWorld_get_component(world, entity, ParrotUIWindowComponent);
    PARROT_RET_COND(!ui_window);

    PARROT_RET_COND(ui_window->immovable);

    ParrotUIInternal *ui_internal = ParrotSceneWorld_get_component(world, entity, ParrotUIInternal);
    PARROT_FAIL_NULL(ui_internal);

    ParrotTransform *transform = ParrotSceneWorld_get_component(world, entity, ParrotTransform);
    PARROT_RET_COND(!transform);

    ParrotVec2 mouse_position = ParrotVideo_object_get_viewport_mouse_position(viewport_handle);
    ParrotVec2 movement = ParrotVec2_sub(mouse_position, ui_internal->ui_window_last_mouse_position);
    ui_internal->ui_window_last_mouse_position = mouse_position;

    PARROT_RET_COND(frames <= 1);

    transform->position.x += movement.x;
    transform->position.y += movement.y;
}

static void window_on_close_click(uint64_t frames,
                                  ParrotVideoObjectHandle viewport_handle,
                                  ParrotWindowDriverEventMouseButton button,
                                  ParrotSceneWorld *world,
                                  ParrotSceneWorldEntity entity) {
    (void)viewport_handle;

    PARROT_RET_COND(frames > 1);
    PARROT_RET_COND(button != ParrotWindowDriverEventMouseButton_LEFT);

    ParrotUIWindowComponent *ui_window = ParrotSceneWorld_get_component(world, entity, ParrotUIWindowComponent);
    PARROT_RET_COND(!ui_window);

    ui_window->close_requested = true;
}

static void
draw_entity(ParrotScope *scope, ParrotVideoDriver *driver, ParrotVideoDriverViewport *viewport, ParrotGMatSet matrix) {
    (void)driver;
    (void)viewport;
    (void)matrix;

    State *state = ParrotScope_get_ctx(scope, State *);

    ParrotVideoFont *font = get_font(state->self->world, state->entity);
    PARROT_FAIL_NULL_MSG(font, "At least one parent must specify UI font");

    ParrotUIInternal *ui_internal = ParrotSceneWorld_get_component(state->self->world, state->entity, ParrotUIInternal);
    PARROT_FAIL_NULL(ui_internal);

    ParrotUIWindowComponent *ui_window =
        ParrotSceneWorld_get_component(state->self->world, state->entity, ParrotUIWindowComponent);
    if (ui_window) {
        const ParrotReal titlebar_height = 32;

        ParrotReal width = ui_window->width;
        ParrotReal height = ui_window->height;

        ParrotColor titlebar_color = ParrotColor_new(48, 48, 48);
        ParrotColor body_color = ParrotColor_new(56, 56, 56);

        ParrotVideoVertex vertices[] = {
            // Titlebar
            (ParrotVideoVertex){.position = (ParrotVec3){0, -titlebar_height, 0}, .tint = titlebar_color},
            (ParrotVideoVertex){.position = (ParrotVec3){width, -titlebar_height, 0}, .tint = titlebar_color},
            (ParrotVideoVertex){.position = (ParrotVec3){0, 0, 0}, .tint = titlebar_color},

            (ParrotVideoVertex){.position = (ParrotVec3){width, 0, 0}, .tint = titlebar_color},
            (ParrotVideoVertex){.position = (ParrotVec3){0, 0, 0}, .tint = titlebar_color},
            (ParrotVideoVertex){.position = (ParrotVec3){width, -titlebar_height, 0}, .tint = titlebar_color},

            // Body
            (ParrotVideoVertex){.position = (ParrotVec3){0, 0, 0}, .tint = body_color},
            (ParrotVideoVertex){.position = (ParrotVec3){width, 0, 0}, .tint = body_color},
            (ParrotVideoVertex){.position = (ParrotVec3){0, height, 0}, .tint = body_color},

            (ParrotVideoVertex){.position = (ParrotVec3){width, height, 0}, .tint = body_color},
            (ParrotVideoVertex){.position = (ParrotVec3){0, height, 0}, .tint = body_color},
            (ParrotVideoVertex){.position = (ParrotVec3){width, 0, 0}, .tint = body_color},
        };

        ParrotUIInternalHitZone titlebar_zone = {
            .position =
                {
                    0,
                    -titlebar_height,
                },
            .size = (ParrotVec2){ui_window->width, titlebar_height},
            .matrix = matrix,
            .on_click = window_on_titlebar_click,
        };
        arrpush(ui_internal->arr_hit_zones, titlebar_zone);

        driver->draw_viewport_vertices(viewport, matrix, vertices, PARROT_ARRAY_LEN(vertices));

        ParrotMat title_offset = ParrotMat_translation((ParrotVec3){8, -titlebar_height / 4, 0});
        draw_text(driver,
                  viewport,
                  (ParrotGMatSet){
                      .model = ParrotMat_mul(matrix.model, title_offset),
                      .view = matrix.view,
                      .projection = matrix.projection,
                  },
                  font,
                  ui_window->title,
                  24,
                  ParrotColor_WHITE);

        ParrotVec2 close_text_size = ParrotVideoFont_measure_text(font, 24, "X");
        ParrotVec2 close_text_offset = (ParrotVec2){ui_window->width - close_text_size.x - 8, -titlebar_height / 4};
        ParrotMat close_text_offset_matrix = ParrotMat_translation(ParrotVec3_upgrade(close_text_offset));

        ParrotUIInternalHitZone close_zone = {
            .position = {0, -close_text_size.y},
            .size = close_text_size,
            .matrix =
                (ParrotGMatSet){
                    .model = ParrotMat_mul(matrix.model, close_text_offset_matrix),
                    .view = matrix.view,
                    .projection = matrix.projection,
                },
            .on_click = window_on_close_click,
        };
        arrpush(ui_internal->arr_hit_zones, close_zone);

        draw_text(driver, viewport, close_zone.matrix, font, "X", 24, ParrotColor_WHITE);
    }
}
