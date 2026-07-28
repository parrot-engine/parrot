#include "parrot/video/scene.h"
#include "parrot/core/util.h"
#include "parrot/scene/world.h"
#include "parrot/video/video.h"

static void ParrotVideoSceneWindowComponent_constructor(ParrotSceneWorldEntity entity, void *self_ptr, void *user_data) {
    PARROT_FAIL_COND(!ParrotVideo_is_initialized());

    (void)entity;
    (void)user_data;

    ParrotVideoSceneWindowComponent *self = (ParrotVideoSceneWindowComponent *)self_ptr;

    self->width = 1;
    self->height = 1;
}

static void
ParrotVideoSceneRenderableComponent_constructor(ParrotSceneWorldEntity entity, void *self_ptr, void *user_data) {
    PARROT_FAIL_COND(!ParrotVideo_is_initialized());

    (void)entity;
    (void)user_data;

    ParrotVideoSceneRenderableComponent *self = (ParrotVideoSceneRenderableComponent *)self_ptr;

    self->object_handle = ParrotVideo_create_object();

    self->visible = true;
}

void ParrotVideoSceneSystem_register_components(ParrotSceneWorld *world) {
    ParrotSceneWorld_register_component(world,
                                        PARROT_TYPE_STRING(ParrotVideoSceneWindowComponent),
                                        (ParrotSceneWorldComponentDescription){
                                            .size = sizeof(ParrotVideoSceneWindowComponent),

                                            .constructor = ParrotVideoSceneWindowComponent_constructor,
                                        });

    ParrotSceneWorld_register_component(world,
                                        PARROT_TYPE_STRING(ParrotVideoSceneRenderableComponent),
                                        (ParrotSceneWorldComponentDescription){
                                            .size = sizeof(ParrotVideoSceneRenderableComponent),

                                            .constructor = ParrotVideoSceneRenderableComponent_constructor,
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

    ParrotVideoSceneWindowComponent *window =
        ParrotSceneWorld_get_component(world, entity, ParrotVideoSceneWindowComponent);
    if (window) {
        if (!ParrotVideo_object_has_window(renderable->object_handle)) {
            ParrotVideo_object_add_window(renderable->object_handle, window->width, window->height);
        }

        ParrotVideo_object_set_window_title(renderable->object_handle, window->title ? window->title : "");

        if (ParrotVideo_object_get_window_width(renderable->object_handle) != window->width ||
            ParrotVideo_object_get_window_height(renderable->object_handle) != window->height) {
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