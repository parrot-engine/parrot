#include "parrot/video/scene.h"
#include "parrot/scene/world.h"
#include "parrot/video/video.h"
#include "stb_image.h"

struct ParrotVideoSceneSystem {
    ParrotScope *scope;

    ParrotSceneWorld *world;
    ParrotVideoObjectHandle root_handle;
};

static void ParrotVideoSceneRenderableComponent_constructor(ParrotSceneWorldEntity entity, void *user_data) {
    PARROT_FAIL_COND(!ParrotVideo_is_initialized());

    ParrotSceneWorld *world = user_data;
    ParrotVideoSceneRenderableComponent *self =
        ParrotSceneWorld_get_component(world, entity, ParrotVideoSceneRenderableComponent);

    self->object_handle = ParrotVideo_create_object();

    self->visible = true;
    self->tint = ParrotColor_WHITE;
}

static void ParrotVideoSceneRenderableComponent_destructor(ParrotSceneWorldEntity entity, void *user_data) {
    PARROT_FAIL_COND(!ParrotVideo_is_initialized());

    ParrotSceneWorld *world = user_data;

    ParrotVideo_delete_object(
        ParrotSceneWorld_get_component(world, entity, ParrotVideoSceneRenderableComponent)->object_handle);
}

static void ParrotVideoSceneRenderableComponent_update(ParrotSceneWorldEntity entity, void *user_data, float delta) {
    (void)delta;

    PARROT_FAIL_COND(!ParrotVideo_is_initialized());

    ParrotSceneWorld *world = user_data;
    ParrotVideoSceneRenderableComponent *self =
        ParrotSceneWorld_get_component(world, entity, ParrotVideoSceneRenderableComponent);

    if (!self->manual_parent) {
        ParrotVideo_set_object_parent(self->object_handle, ParrotVideo_get_root());

        ParrotSceneWorldEntity parent = ParrotSceneWorld_get_entity_parent(world, entity);
        while (ParrotSceneWorld_does_entity_exist(world, parent)) {
            ParrotVideoSceneRenderableComponent *renderable;
            if ((renderable = ParrotSceneWorld_get_component(world, parent, ParrotVideoSceneRenderableComponent))) {
                ParrotVideo_set_object_parent(self->object_handle, renderable->object_handle);
                break;
            }

            parent = ParrotSceneWorld_get_entity_parent(world, parent);
        }
    }

    ParrotVideo_set_object_visible(self->object_handle, self->visible);
    ParrotVideo_set_object_tint(self->object_handle, self->tint);

    ParrotTransform *transform;
    if ((transform = ParrotSceneWorld_get_component(world, entity, ParrotTransform))) {
        ParrotVideo_set_object_matrix(self->object_handle, ParrotTransform_calculate_matrix(transform));
    }

    ParrotVideoSceneWindowComponent *window;
    if ((window = ParrotSceneWorld_get_component(world, entity, ParrotVideoSceneWindowComponent))) {
        if (!ParrotVideo_object_has_window(self->object_handle)) {
            ParrotVideo_object_add_window(self->object_handle, window->width, window->height);
        }

        ParrotVideo_object_set_window_title(self->object_handle, window->title ? window->title : "");

        if (!window->resize) {
            window->width = ParrotVideo_object_get_window_width(self->object_handle);
            window->height = ParrotVideo_object_get_window_height(self->object_handle);
        } else {
            ParrotVideo_object_set_window_size(self->object_handle, window->width, window->height);
        }

        if (ParrotVideo_object_is_window_close_requested(self->object_handle)) {
            window->close_requested = true;
        }
    } else {
        if (ParrotVideo_object_has_window(self->object_handle)) {
            ParrotVideo_object_remove_window(self->object_handle);
        }
    }

    ParrotVideoSceneViewportComponent *viewport;
    if ((viewport = ParrotSceneWorld_get_component(world, entity, ParrotVideoSceneViewportComponent))) {
        if (!ParrotVideo_object_has_viewport(self->object_handle)) {
            ParrotVideo_object_add_viewport(self->object_handle, viewport->width, viewport->height);
        }
    } else {
        if (ParrotVideo_object_has_viewport(self->object_handle)) {
            ParrotVideo_object_remove_viewport(self->object_handle);
        }
    }

    ParrotVideoSceneCameraComponent *camera;
    if ((camera = ParrotSceneWorld_get_component(world, entity, ParrotVideoSceneCameraComponent))) {
        if (!ParrotVideo_object_has_camera(self->object_handle)) {
            ParrotVideo_object_add_camera(self->object_handle);
        }

        ParrotVideo_object_set_camera_corner_aligned(self->object_handle, camera->corner_aligned);

        if (camera->use_clear_color) {
            ParrotVideo_object_set_camera_clear_color(self->object_handle, camera->clear_color);
        } else {
            ParrotVideo_object_clear_camera_clear_color(self->object_handle);
        }
    } else {
        if (ParrotVideo_object_has_camera(self->object_handle)) {
            ParrotVideo_object_remove_camera(self->object_handle);
        }
    }

    ParrotVideoSceneRectComponent *rect;
    if ((rect = ParrotSceneWorld_get_component(world, entity, ParrotVideoSceneRectComponent))) {
        if (!ParrotVideo_object_has_rect(self->object_handle)) {
            ParrotVideo_object_add_rect(self->object_handle);
        }

        ParrotVideo_object_set_rect_texture(self->object_handle, rect->texture, rect->texture_nearest_filter);

        if (rect->texture) {
            if (rect->texture_use_region) {
                ParrotVideo_object_set_rect_texture_region(self->object_handle,
                                                           rect->texture_region_x,
                                                           rect->texture_region_y,
                                                           rect->texture_region_width,
                                                           rect->texture_region_height);
            }
        }

        ParrotVideo_object_set_rect_size(self->object_handle, rect->width, rect->height);
    } else {
        if (ParrotVideo_object_has_rect(self->object_handle)) {
            ParrotVideo_object_remove_rect(self->object_handle);
        }
    }
}

static void ParrotVideoSceneCameraComponent_constructor(ParrotSceneWorldEntity entity, void *user_data) {
    PARROT_FAIL_COND(!ParrotVideo_is_initialized());

    ParrotSceneWorld *world = user_data;
    ParrotVideoSceneCameraComponent *self =
        ParrotSceneWorld_get_component(world, entity, ParrotVideoSceneCameraComponent);

    self->use_clear_color = true;
    self->clear_color = ParrotColor_newf(0.3, 0.3, 0.3);
}

void ParrotVideoSceneSystem_register(ParrotSceneWorld *world) {
    ParrotSceneWorld_register_component(world,
                                        PARROT_TYPE_STRING(ParrotVideoSceneRenderableComponent),
                                        (ParrotSceneWorldComponentDescription){
                                            .size = sizeof(ParrotVideoSceneRenderableComponent),
                                            .constructor = ParrotVideoSceneRenderableComponent_constructor,
                                            .destructor = ParrotVideoSceneRenderableComponent_destructor,
                                            .update = ParrotVideoSceneRenderableComponent_update,
                                            .user_data = world,
                                        });

    ParrotSceneWorld_register_component(world,
                                        PARROT_TYPE_STRING(ParrotVideoSceneCameraComponent),
                                        (ParrotSceneWorldComponentDescription){
                                            .size = sizeof(ParrotVideoSceneCameraComponent),
                                            .constructor = ParrotVideoSceneCameraComponent_constructor,
                                            .user_data = world,
                                        });
}
