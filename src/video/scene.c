#include "parrot/video/scene.h"
#include "parrot/core/math.h"
#include "parrot/core/scope.h"
#include "parrot/core/util.h"
#include "parrot/scene/world.h"
#include "parrot/video/video.h"
#include "stb_image.h"

struct ParrotVideoSceneSystem {
    ParrotScope *scope;

    ParrotSceneWorld *world;
    ParrotVideoObjectHandle root_handle;
};

static void ParrotVideoSceneCameraComponent_constructor(ParrotSceneWorldEntity entity, void *self_ptr, void *user_data) {
    PARROT_FAIL_COND(!ParrotVideo_is_initialized());

    (void)entity;
    (void)user_data;

    ParrotVideoSceneCameraComponent *self = self_ptr;

    self->use_clear_color = true;
    self->clear_color = ParrotColor_newf(0.3, 0.3, 0.3);
}

static void
ParrotVideoSceneRenderableComponent_constructor(ParrotSceneWorldEntity entity, void *self_ptr, void *user_data) {
    PARROT_FAIL_COND(!ParrotVideo_is_initialized());

    (void)entity;
    (void)user_data;

    ParrotVideoSceneRenderableComponent *self = self_ptr;

    self->object_handle = ParrotVideo_create_object();

    self->visible = true;
    self->tint = ParrotColor_WHITE;
}

void ParrotVideoSceneSystem_register_components(ParrotSceneWorld *world) {
    ParrotSceneWorld_register_component(world,
                                        PARROT_TYPE_STRING(ParrotVideoSceneRenderableComponent),
                                        (ParrotSceneWorldComponentDescription){
                                            .size = sizeof(ParrotVideoSceneRenderableComponent),
                                            .constructor = ParrotVideoSceneRenderableComponent_constructor,
                                        });

    ParrotSceneWorld_register_component(world,
                                        PARROT_TYPE_STRING(ParrotVideoSceneCameraComponent),
                                        (ParrotSceneWorldComponentDescription){
                                            .size = sizeof(ParrotVideoSceneCameraComponent),
                                            .constructor = ParrotVideoSceneCameraComponent_constructor,
                                        });
}

static ParrotVideoObjectHandle ParrotVideoSceneSystem_get_parent(ParrotSceneWorld *world,
                                                                 ParrotVideoObjectHandle root_handle,
                                                                 ParrotSceneWorldEntity entity) {
    entity = ParrotSceneWorld_get_entity_parent(world, entity);
    for (; entity != ParrotSceneWorldEntity_NULL; entity = ParrotSceneWorld_get_entity_parent(world, entity)) {
        ParrotVideoSceneRenderableComponent *renderable =
            ParrotSceneWorld_get_component(world, entity, ParrotVideoSceneRenderableComponent);
        if (!renderable) {
            continue;
        }

        return renderable->object_handle;
    }

    return root_handle;
}

static void ParrotVideoSceneSystem_update_tree(ParrotSceneWorld *world,
                                               ParrotSceneWorldQuery *query,
                                               ParrotVideoObjectHandle root_handle) {
    for (size_t i = 0; i < ParrotSceneWorld_query_result_count(world, query); i++) {
        ParrotSceneWorldEntity entity = ParrotSceneWorld_query_result_at(world, query, i);

        ParrotVideoSceneRenderableComponent *renderable =
            ParrotSceneWorld_get_component(world, entity, ParrotVideoSceneRenderableComponent);
        PARROT_FAIL_NULL(renderable);

        ParrotVideo_set_object_parent(renderable->object_handle,
                                      ParrotVideoSceneSystem_get_parent(world, root_handle, entity));
    }
}

static void ParrotVideoSceneSystem_sync_entity(ParrotSceneWorld *world, ParrotSceneWorldEntity entity) {
    ParrotVideoSceneRenderableComponent *renderable =
        ParrotSceneWorld_get_component(world, entity, ParrotVideoSceneRenderableComponent);
    PARROT_FAIL_NULL(renderable);

    ParrotTransform *transform = ParrotSceneWorld_get_component(world, entity, ParrotTransform);
    if (transform) {
        ParrotVideo_set_object_matrix(renderable->object_handle, ParrotTransform_calculate_matrix(transform));
    }

    ParrotVideo_set_object_visible(renderable->object_handle, renderable->visible);
    ParrotVideo_set_object_tint(renderable->object_handle, renderable->tint);

    ParrotVideoSceneWindowComponent *window =
        ParrotSceneWorld_get_component(world, entity, ParrotVideoSceneWindowComponent);
    if (window) {
        if (!ParrotVideo_object_has_window(renderable->object_handle)) {
            ParrotVideo_object_add_window(renderable->object_handle, window->width, window->height);
        }

        ParrotVideo_object_set_window_title(renderable->object_handle, window->title ? window->title : "");

        if (!window->resize) {
            window->width = ParrotVideo_object_get_window_width(renderable->object_handle);
            window->height = ParrotVideo_object_get_window_height(renderable->object_handle);
        } else {
            ParrotVideo_object_set_window_size(renderable->object_handle, window->width, window->height);
        }

        if (ParrotVideo_object_is_window_close_requested(renderable->object_handle)) {
            window->close_requested = true;
        }
    } else {
        if (ParrotVideo_object_has_window(renderable->object_handle)) {
            ParrotVideo_object_remove_window(renderable->object_handle);
        }
    }

    ParrotVideoSceneViewportComponent *viewport =
        ParrotSceneWorld_get_component(world, entity, ParrotVideoSceneViewportComponent);
    if (viewport) {
        if (!ParrotVideo_object_has_viewport(renderable->object_handle)) {
            ParrotVideo_object_add_viewport(renderable->object_handle, viewport->width, viewport->height);
        }
    } else {
        if (ParrotVideo_object_has_viewport(renderable->object_handle)) {
            ParrotVideo_object_remove_viewport(renderable->object_handle);
        }
    }

    ParrotVideoSceneCameraComponent *camera =
        ParrotSceneWorld_get_component(world, entity, ParrotVideoSceneCameraComponent);
    if (camera) {
        if (!ParrotVideo_object_has_camera(renderable->object_handle)) {
            ParrotVideo_object_add_camera(renderable->object_handle);
        }

        ParrotVideo_object_set_camera_corner_aligned(renderable->object_handle, camera->corner_aligned);

        if (camera->use_clear_color) {
            ParrotVideo_object_set_camera_clear_color(renderable->object_handle, camera->clear_color);
        } else {
            ParrotVideo_object_clear_camera_clear_color(renderable->object_handle);
        }
    } else {
        if (ParrotVideo_object_has_camera(renderable->object_handle)) {
            ParrotVideo_object_remove_camera(renderable->object_handle);
        }
    }

    ParrotVideoSceneRectComponent *rect = ParrotSceneWorld_get_component(world, entity, ParrotVideoSceneRectComponent);
    if (rect) {
        if (!ParrotVideo_object_has_rect(renderable->object_handle)) {
            ParrotVideo_object_add_rect(renderable->object_handle);
        }

        ParrotVideoSceneRectTextureComponent *texture =
            ParrotSceneWorld_get_component(world, entity, ParrotVideoSceneRectTextureComponent);
        ParrotVideoSceneRectRawTextureComponent *raw_texture =
            ParrotSceneWorld_get_component(world, entity, ParrotVideoSceneRectRawTextureComponent);

        if (texture) {
            if (texture->image_dirty) {
                if (texture->_rgba8888) {
                    stbi_image_free(texture->_rgba8888);
                }

                int channels = 0;
                texture->_rgba8888 = (uint32_t *)stbi_load_from_memory(
                    texture->image.data, texture->image.size, &texture->_width, &texture->_height, &channels, 4);
            }

            if (!texture->_rgba8888) {
#define X {0xFF, 0x00, 0x00, 0xFF}
#define O {0x00, 0x00, 0x00, 0xFF}
                const uint8_t image_fail_texture_data[6][6][4] = {
                    {O, X, X, X, X, O},
                    {X, X, O, O, O, X},
                    {X, O, X, O, O, X},
                    {X, O, O, X, O, X},
                    {X, O, O, O, X, X},
                    {O, X, X, X, X, O},
                };
#undef O
#undef X
                ParrotVideo_object_set_rect_texture(
                    renderable->object_handle, 6, 6, (uint32_t *)image_fail_texture_data, true);
            } else {
                ParrotVideo_object_set_rect_texture(renderable->object_handle,
                                                    texture->_width,
                                                    texture->_height,
                                                    texture->_rgba8888,
                                                    rect->texture_nearest_filter);
            }
        } else if (raw_texture) {
            ParrotVideo_object_set_rect_texture(renderable->object_handle,
                                                raw_texture->width,
                                                raw_texture->height,
                                                raw_texture->rgba8888,
                                                rect->texture_nearest_filter);
        } else {
            ParrotVideo_object_clear_rect_texture(renderable->object_handle);
        }

        if (texture || raw_texture) {
            if (rect->texture_use_region) {
                ParrotVideo_object_set_rect_texture_region(renderable->object_handle,
                                                           rect->texture_region_x,
                                                           rect->texture_region_y,
                                                           rect->texture_region_width,
                                                           rect->texture_region_height);
            }
        }

        ParrotVideo_object_set_rect_size(renderable->object_handle, rect->width, rect->height);

        if (ParrotSceneWorld_is_component_deletion_queued(world, entity, ParrotVideoSceneRectTextureComponent)) {
            if (texture->_rgba8888) {
                stbi_image_free(texture->_rgba8888);
            }
        }
    } else {
        if (ParrotVideo_object_has_rect(renderable->object_handle)) {
            ParrotVideo_object_remove_rect(renderable->object_handle);
        }
    }

    ParrotVideoSceneTextComponent *text = ParrotSceneWorld_get_component(world, entity, ParrotVideoSceneTextComponent);
    if (text) {
        ParrotVideo_object_set_text(renderable->object_handle, text->font, text->size, text->text);
    } else {
        ParrotVideo_object_clear_text(renderable->object_handle);
    }
}

void ParrotVideoSceneSystem_update(ParrotSceneWorld *world, ParrotVideoObjectHandle root_handle) {
    PARROT_FAIL_COND(!ParrotVideo_is_initialized());

    ParrotSceneWorldQuery query[] = {
        PARROT_SCENE_WORLD_QUERY_WITH_COMPONENT(ParrotVideoSceneRenderableComponent),

        PARROT_SCENE_WORLD_QUERY_END(),
    };

    ParrotVideoSceneSystem_update_tree(world, query, root_handle);

    for (size_t i = 0; i < ParrotSceneWorld_query_result_count(world, query); i++) {
        ParrotSceneWorldEntity entity = ParrotSceneWorld_query_result_at(world, query, i);
        ParrotVideoSceneSystem_sync_entity(world, entity);
    }
}
