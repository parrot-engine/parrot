#include "parrot/video/scene.h"
#include "parrot/core/math.h"
#include "parrot/core/scope.h"
#include "parrot/core/util.h"
#include "parrot/scene/world.h"
#include "stb_image.h"

typedef struct {
    ParrotVideoSceneViewportActor base;

    ParrotScope *scope;

    ParrotVideoSceneCameraActor *camera;
    ParrotMat view_matrix;
    ParrotMat projection_matrix;
} _ParrotVideoSceneViewportActor;

static void ParrotVideoSceneViewportActor_init(void *selfx) {
    _ParrotVideoSceneViewportActor *self = selfx;

    self->scope = ParrotScope_new(NULL);
}

static void ParrotVideoSceneViewportActor_destroy(void *selfx) {
    _ParrotVideoSceneViewportActor *self = selfx;

    ParrotScope_delete(self->scope);
}

static _ParrotVideoSceneViewportActor *find_viewport_parent_or_null(void *actor) {
    _ParrotVideoSceneViewportActor *viewport = ParrotScene_get_actor_parent(actor);
    while (viewport) {
        const ParrotSceneActorProfile *profile = ParrotScene_get_actor_profile(viewport);
        for (size_t i = 0; profile->type[i]; i++) {
            if (strcmp(profile->type[i], PARROT_TYPE_STRING(ParrotVideoSceneViewportActor)) == 0) {
                return viewport;
            }
        }

        viewport = ParrotScene_get_actor_parent(viewport);
    }

    return NULL;
}

static void propagate_event(ParrotScene *scene, ParrotSceneActorEvent event, void *actor) {
    const ParrotSceneActorProfile *profile = ParrotScene_get_actor_profile(actor);

    void *children = ParrotScene_get_actor_child(scene, actor);
    for (void *child = children; child; child = ParrotScene_get_next_actor(child)) {
        propagate_event(scene, event, child);
    }

    for (size_t i = 0; profile->type[i]; i++) {
        const char *begin = "ParrotVideoScene";
        if (strncmp(profile->type[i], begin, strlen(begin)) == 0) {
            ParrotScene_send_actor_event(actor, event);
            break;
        }
    }
}

static void handle_renderable_event(void *actor, ParrotSceneActorEvent event) {
    ParrotScene *scene = ParrotScene_get_actor_scene(actor);

    switch ((int)event) {
    case ParrotVideoSceneEvent_PRE_RENDER:
    case ParrotVideoSceneEvent_RENDER: {
        void *children = ParrotScene_get_actor_child(scene, actor);
        for (void *child = children; child; child = ParrotScene_get_next_actor(child)) {
            propagate_event(scene, event, child);
        }
    } break;
    default:
        break;
    }
}

static void ParrotVideoSceneViewportActor_event(void *selfx, ParrotSceneActorEvent event) {
    _ParrotVideoSceneViewportActor *self = selfx;

    switch ((int)event) {
    case ParrotVideoSceneEvent_PRE_RENDER: {
        self->camera = NULL;
    } break;
    case ParrotVideoSceneEvent_RENDER: {
        if (self->camera && self->camera->use_clear_color) {
            self->base.driver->clear_viewport(self->base.viewport, self->camera->clear_color);
        }
    } break;
    }

    handle_renderable_event(self, event);

    switch ((int)event) {
    case ParrotSceneActorEvent_TREE_ENTER: {
        PARROT_FAIL_NULL(self->base.driver);

        self->base.viewport = self->base.driver->create_viewport(self->base.driver, self->base.width, self->base.height);

        if (self->base.window.driver) {
            self->base.window.window = self->base.window.driver->create_window(
                self->base.window.driver, self->base.window.width, self->base.window.height);
        }
    } break;
    case ParrotVideoSceneEvent_RENDER: {
        if (self->base.window.driver) {
            self->base.window.close_requested = false;

            ParrotWindowDriverEvent event;
            while (self->base.window.driver->poll_events(self->base.window.window, &event)) {
                switch (event.type) {
                case ParrotWindowDriverEventType_QUIT:
                    self->base.window.close_requested = true;
                    break;
                default:
                    break;
                }
            }

            self->base.window.driver->set_title(self->base.window.window, self->base.window.title);

            ParrotColorFormat image_format = self->base.window.driver->get_native_image_format(self->base.window.window);
            const uint8_t *image = self->base.driver->get_viewport_pixels(self->base.viewport, image_format);
            self->base.window.driver->set_image_native(self->base.window.window, image);
        }
    } break;
    default:
        break;
    }
}

const ParrotSceneActorProfile ParrotVideoSceneViewportActor_profile = {
    .size = sizeof(_ParrotVideoSceneViewportActor),
    .type =
        (const char *[]){
            PARROT_TYPE_STRING(ParrotVideoSceneViewportActor),
            NULL,
        },

    .init = ParrotVideoSceneViewportActor_init,
    .event = ParrotVideoSceneViewportActor_event,
    .destroy = ParrotVideoSceneViewportActor_destroy,
};

static void ParrotVideoSceneCameraActor_init(void *selfx) {
    ParrotVideoSceneCameraActor *self = selfx;

    self->clear_color = ParrotColor_newf(0.3, 0.3, 0.3);
    self->use_clear_color = true;
}

static void ParrotVideoSceneCameraActor_event(void *selfx, ParrotSceneActorEvent event) {
    ParrotVideoSceneCameraActor *self = selfx;

    handle_renderable_event(self, event);

    switch ((int)event) {
    case ParrotVideoSceneEvent_PRE_RENDER: {
        if (!ParrotScene_is_actor_active(self)) {
            break;
        }

        _ParrotVideoSceneViewportActor *viewport = find_viewport_parent_or_null(self);
        if (!viewport) {
            break;
        }

        viewport->camera = self;

        viewport->view_matrix = ParrotMat_inverse(ParrotScene_get_global_actor_matrix(self));
        viewport->projection_matrix =
            ParrotMat_ortho(!self->corner_aligned ? -viewport->base.width / 2 : 0,
                            !self->corner_aligned ? viewport->base.width / 2 : viewport->base.width,
                            !self->corner_aligned ? -viewport->base.height / 2 : 0,
                            !self->corner_aligned ? viewport->base.height / 2 : viewport->base.height,
                            -1000000,
                            1000000);
    } break;
    default:
        break;
    }
}

const ParrotSceneActorProfile ParrotVideoSceneCameraActor_profile = {
    .size = sizeof(ParrotVideoSceneCameraActor),
    .type =
        (const char *[]){
            PARROT_TYPE_STRING(ParrotVideoSceneCameraActor),
            NULL,
        },

    .init = ParrotVideoSceneCameraActor_init,
    .event = ParrotVideoSceneCameraActor_event,
};

static void ParrotVideoSceneRenderableActor2D_init(void *selfx) {
    ParrotVideoSceneRenderableActor2D *self = selfx;

    self->tint = ParrotColor_WHITE;
}

static void ParrotVideoSceneRenderableActor2D_event(void *selfx, ParrotSceneActorEvent event) {
    ParrotVideoSceneRenderableActor2D *self = selfx;

    switch ((int)event) {
    case ParrotVideoSceneEvent_RENDER: {
        if (!ParrotScene_is_actor_active(self)) {
            break;
        }

        _ParrotVideoSceneViewportActor *viewport = find_viewport_parent_or_null(self);
        if (!viewport) {
            break;
        }

        ParrotColor tint = self->tint;

        int width = self->width;
        int height = self->height;

        float u0 = 0; // self->texture ? self->texture_x / (float)self->texture->width : 0;
        float v0 = 0; // self->texture ? self->texture_y / (float)self->texture->height : 0;
        float u1 = 0; // self->texture ? x0 + self->width / (float)self->texture->width : 0;
        float v1 = 0; // self->texture ? y0 + self->height / (float)self->texture->height : 0;

        ParrotVideoVertex vertices[] = {
            (ParrotVideoVertex){.position = (ParrotVec3){0, 0, 0}, .uv = {u0, v0}, .tint = tint},
            (ParrotVideoVertex){.position = (ParrotVec3){self->width, 0, 0}, .uv = {u1, v0}, .tint = tint},
            (ParrotVideoVertex){.position = (ParrotVec3){0, self->height, 0}, .uv = {u0, v1}, .tint = tint},

            (ParrotVideoVertex){.position = (ParrotVec3){width, self->height, 0}, .uv = {u1, v1}, .tint = tint},
            (ParrotVideoVertex){.position = (ParrotVec3){0, height, 0}, .uv = {v0, v1}, .tint = tint},
            (ParrotVideoVertex){.position = (ParrotVec3){width, 0, 0}, .uv = {v1, v0}, .tint = tint},
        };

        /*if (self->texture) {
            self->video_driver->set_viewport_texture(viewport,
                                                     self->texture->width,
                                                     self->texture->height,
                                                     self->texture->rgba8888,
                                                     self->texture_nearest_filter);
        }*/
        viewport->base.driver->draw_viewport_vertices(viewport->base.viewport,
                                                      (ParrotGMatSet){
                                                          .model = ParrotScene_get_global_actor_matrix(self),
                                                          .view = viewport->view_matrix,
                                                          .projection = viewport->projection_matrix,
                                                      },
                                                      vertices,
                                                      6);
        // self->video_driver->clear_viewport_texture(viewport);
    } break;
    default:
        break;
    }
}

static void ParrotVideoSceneRenderableActor2D_destroy(void *selfx) {
    ParrotVideoSceneRenderableActor2D *self = selfx;

    (void)self;
}

const ParrotSceneActorProfile ParrotVideoSceneRenderableActor2D_profile = {
    .size = sizeof(ParrotVideoSceneRenderableActor2D),
    .type =
        (const char *[]){
            PARROT_TYPE_STRING(ParrotVideoSceneRenderableActor2D),
            NULL,
        },

    .init = ParrotVideoSceneRenderableActor2D_init,
    .event = ParrotVideoSceneRenderableActor2D_event,
    .destroy = ParrotVideoSceneRenderableActor2D_destroy,
};
