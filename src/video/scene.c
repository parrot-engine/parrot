#include "parrot/video/scene.h"
#include "parrot/core/array.h"
#include "parrot/core/math.h"
#include "parrot/core/scope.h"
#include "parrot/core/util.h"
#include "parrot/scene/scene.h"
#include "stb_image.h"

typedef struct {
    ParrotScope *scope;
    ParrotVideoSceneRenderStage stage;
    ParrotVideoSceneRenderFunc func;
} ParrotVideoSceneRenderableComponentCommand;

typedef struct {
    ParrotVideoSceneRenderableComponent pub;
    ParrotScope *scope;

    ParrotVideoSceneRenderableComponentCommand *arr_commands;
} _ParrotVideoSceneRenderableComponent;

static void ParrotVideoSceneRenderableComponent_init(void *selfx, ParrotSceneFuncParams params) {
    _ParrotVideoSceneRenderableComponent *self = selfx;
    (void)params;

    self->scope = ParrotScope_new(NULL);

    ParrotScope_push_arrfree(self->scope, self->arr_commands);

    self->pub.visible = true;
}

static void ParrotVideoSceneRenderableComponent_destroy(void *selfx, ParrotSceneFuncParams params) {
    _ParrotVideoSceneRenderableComponent *self = selfx;
    (void)params;

    ParrotScope_delete(self->scope);
}

void ParrotVideoSceneRenderableComponent_add(ParrotVideoSceneRenderableComponent *selfx,
                                             ParrotScope *scope,
                                             ParrotVideoSceneRenderStage stage,
                                             ParrotVideoSceneRenderFunc render) {
    _ParrotVideoSceneRenderableComponent *self = (void *)selfx;
    PARROT_FAIL_NULL(self);
    PARROT_FAIL_NULL(scope);

    ParrotScope_set_parent(scope, self->scope);

    ParrotArray_push(self->arr_commands,
                     ((ParrotVideoSceneRenderableComponentCommand){
                         .scope = scope,
                         .stage = stage,
                         .func = render,
                     }));
}

static void propagate_render(ParrotScene *scene, ParrotSceneEntity entity, ParrotVideoSceneRenderStage stage) {
    _ParrotVideoSceneRenderableComponent *renderable =
        (void *)ParrotScene_get_component_or_null(scene, entity, ParrotVideoSceneRenderableComponent);
    PARROT_RET_COND(renderable && !renderable->pub.visible);

    const ParrotSceneQuery children_query[] = {
        PARROT_SCENE_QUERY_WITH_PARENT(entity),
        PARROT_SCENE_QUERY_END(),
    };

    ParrotSceneEntity *arr_children = NULL;
    ParrotScene_query(scene, children_query, &arr_children);

    for (size_t i = 0; i < ParrotArray_size(arr_children); i++) {
        propagate_render(scene, arr_children[i], stage);
    }
    ParrotArray_free(arr_children);

    PARROT_RET_COND(!renderable);
    for (size_t i = 0; i < ParrotArray_size(renderable->arr_commands); i++) {
        if (renderable->arr_commands[i].stage == stage) {
            renderable->arr_commands[i].func(renderable->arr_commands[i].scope);

            if (renderable->arr_commands[i].scope) {
                ParrotScope_delete(renderable->arr_commands[i].scope);
            }

            ParrotArray_del(renderable->arr_commands, i);
            i--;
        }
    }
}

void ParrotVideoSceneRenderableComponent_render(ParrotScene *scene, ParrotSceneEntity entity) {
    PARROT_FAIL_NULL(scene);

    propagate_render(scene, entity, ParrotVideoSceneRenderStage_PRE);
    propagate_render(scene, entity, ParrotVideoSceneRenderStage_RENDER);
    propagate_render(scene, entity, ParrotVideoSceneRenderStage_POST);
}

typedef struct {
    void *self;
    ParrotSceneFuncParams params;
} BasicEntityRenderContext;

static void ParrotVideoSceneCameraComponent_pre_render(ParrotScope *scope);

static void ParrotVideoSceneCameraComponent_init(void *selfx, ParrotSceneFuncParams params) {
    ParrotVideoSceneCameraComponent *self = selfx;
    (void)params;

    self->clear_color = ParrotColor_newf(0.3, 0.3, 0.3);
    self->use_clear_color = true;

    self->last_view_matrix = ParrotMat_identity();
    self->last_projection_matrix = ParrotMat_identity();
}

static void ParrotVideoSceneCameraComponent_update(void *selfx, ParrotSceneFuncParams params, float delta) {
    ParrotVideoSceneCameraComponent *self = selfx;
    (void)params;
    (void)delta;

    ParrotVideoSceneRenderableComponent *renderable =
        ParrotScene_get_component_or_null(params.scene, params.entity, ParrotVideoSceneRenderableComponent);
    if (renderable) {
        ParrotScope *scope = ParrotScope_new(NULL);
        BasicEntityRenderContext *ctx = ParrotScope_alloc_ctx(scope, BasicEntityRenderContext);

        ctx->self = self;
        ctx->params = params;

        ParrotVideoSceneRenderableComponent_add(
            renderable, scope, ParrotVideoSceneRenderStage_PRE, ParrotVideoSceneCameraComponent_pre_render);
    }
}

static void ParrotVideoSceneCameraComponent_pre_render(ParrotScope *scope) {
    BasicEntityRenderContext *ctx = ParrotScope_get_ctx(scope, BasicEntityRenderContext *);
    ParrotVideoSceneCameraComponent *self = ctx->self;

    ParrotTransform *transform =
        ParrotScene_get_component_or_null(ctx->params.scene, ctx->params.entity, ParrotTransform);
    PARROT_RET_COND(!transform);

    ParrotSceneEntity viewport_entity =
        ParrotVideoSceneRenderableViewportComponent_find_or_null(ctx->params.scene, ctx->params.entity);
    ParrotVideoSceneRenderableViewportComponent *viewport = ParrotScene_get_component_or_null(
        ctx->params.scene, viewport_entity, ParrotVideoSceneRenderableViewportComponent);
    if (viewport) {
        self->last_view_matrix = ParrotMat_inverse(ParrotTransform_calculate_matrix(transform));
        self->last_projection_matrix = ParrotMat_ortho(!self->corner_aligned ? -viewport->width / 2 : 0,
                                                       !self->corner_aligned ? viewport->width / 2 : viewport->width,
                                                       !self->corner_aligned ? -viewport->height / 2 : 0,
                                                       !self->corner_aligned ? viewport->height / 2 : viewport->height,
                                                       -1000000,
                                                       1000000);
    } else {
        self->last_view_matrix = ParrotMat_identity();
        self->last_projection_matrix = ParrotMat_identity();
    }
}

ParrotSceneEntity ParrotVideoSceneCameraComponent_find_or_null(ParrotScene *scene, ParrotSceneEntity entity) {
    PARROT_FAIL_NULL(scene);

    if (ParrotScene_get_component_or_null(scene, entity, ParrotVideoSceneCameraComponent)) {
        return entity;
    }

    const ParrotSceneQuery children_query[] = {
        PARROT_SCENE_QUERY_WITH_PARENT(entity),
        PARROT_SCENE_QUERY_END(),
    };

    ParrotSceneEntity *arr_children = NULL;
    ParrotScene_query(scene, children_query, &arr_children);

    for (size_t i = 0; i < ParrotArray_size(arr_children); i++) {
        ParrotSceneEntity entity = ParrotVideoSceneCameraComponent_find_or_null(scene, arr_children[i]);
        if (entity) {
            ParrotArray_free(arr_children);
            return entity;
        }
    }

    ParrotArray_free(arr_children);
    return ParrotSceneEntity_NULL;
}

static void ParrotVideoSceneRenderableWindowComponent_ready(void *selfx, ParrotSceneFuncParams params) {
    ParrotVideoSceneRenderableWindowComponent *self = selfx;
    (void)params;

    PARROT_FAIL_NULL(self->driver);

    self->window = self->driver->create_window(self->driver, self->width, self->height);
}

static void ParrotVideoSceneRenderableWindowComponent_render(ParrotScope *scope);
static void ParrotVideoSceneRenderableWindowComponent_post_render(ParrotScope *scope);

static void ParrotVideoSceneRenderableWindowComponent_update(void *selfx, ParrotSceneFuncParams params, float delta) {
    ParrotVideoSceneRenderableWindowComponent *self = selfx;
    (void)params;
    (void)delta;

    ParrotVideoSceneRenderableComponent *renderable =
        ParrotScene_get_component_or_null(params.scene, params.entity, ParrotVideoSceneRenderableComponent);
    if (renderable) {
        ParrotScope *scope[2];
        for (size_t i = 0; i < PARROT_ARRAY_LEN(scope); i++) {
            scope[i] = ParrotScope_new(NULL);
            BasicEntityRenderContext *ctx = ParrotScope_alloc_ctx(scope[i], BasicEntityRenderContext);

            ctx->self = self;
            ctx->params = params;
        }

        ParrotVideoSceneRenderableComponent_add(
            renderable, scope[0], ParrotVideoSceneRenderStage_RENDER, ParrotVideoSceneRenderableWindowComponent_render);
        ParrotVideoSceneRenderableComponent_add(renderable,
                                                scope[1],
                                                ParrotVideoSceneRenderStage_POST,
                                                ParrotVideoSceneRenderableWindowComponent_post_render);
    }
}

static void ParrotVideoSceneRenderableWindowComponent_destroy(void *selfx, ParrotSceneFuncParams params) {
    ParrotVideoSceneRenderableWindowComponent *self = selfx;
    (void)params;

    PARROT_FAIL_NULL(self->driver);

    self->driver->delete_window(self->window);
}

static void ParrotVideoSceneRenderableWindowComponent_render(ParrotScope *scope) {
    BasicEntityRenderContext *ctx = ParrotScope_get_ctx(scope, BasicEntityRenderContext *);
    ParrotVideoSceneRenderableWindowComponent *self = ctx->self;

    self->close_requested = false;

    PARROT_FAIL_NULL(self->driver);

    ParrotWindowDriverEvent event;
    while (self->driver->poll_events(self->window, &event)) {
        switch (event.type) {
        case ParrotWindowDriverEventType_QUIT:
            self->close_requested = true;
            break;
        default:
            break;
        }
    }

    self->driver->set_title(self->window, self->title);

    if (!self->resize) {
        self->width = self->driver->get_width(self->window);
        self->height = self->driver->get_height(self->window);
    } else {
        self->driver->set_size(self->window, self->width, self->height);
        self->resize = false;
    }
}

static void ParrotVideoSceneRenderableWindowComponent_post_render(ParrotScope *scope) {
    BasicEntityRenderContext *ctx = ParrotScope_get_ctx(scope, BasicEntityRenderContext *);
    ParrotVideoSceneRenderableWindowComponent *self = ctx->self;

    PARROT_FAIL_NULL(self->driver);

    ParrotVideoSceneRenderableViewportComponent *viewport = ParrotScene_get_component_or_null(
        ctx->params.scene, ctx->params.entity, ParrotVideoSceneRenderableViewportComponent);
    if (viewport) {
        ParrotColorFormat image_format = self->driver->get_native_image_format(self->window);
        const uint8_t *image = viewport->driver->get_viewport_pixels(viewport->viewport, image_format);
        self->driver->set_image_native(self->window, image);
    }
}

static void ParrotVideoSceneRenderableViewportComponent_pre_render(ParrotScope *scope);

static void ParrotVideoSceneRenderableViewportComponent_ready(void *selfx, ParrotSceneFuncParams params) {
    ParrotVideoSceneRenderableViewportComponent *self = selfx;
    (void)params;

    PARROT_FAIL_NULL(self->driver);

    self->viewport = self->driver->create_viewport(self->driver, self->width, self->height);
}

static void ParrotVideoSceneRenderableViewportComponent_update(void *selfx, ParrotSceneFuncParams params, float delta) {
    ParrotVideoSceneRenderableViewportComponent *self = selfx;
    (void)params;
    (void)delta;

    ParrotVideoSceneRenderableComponent *renderable =
        ParrotScene_get_component_or_null(params.scene, params.entity, ParrotVideoSceneRenderableComponent);
    if (renderable) {
        ParrotScope *scope = ParrotScope_new(NULL);
        BasicEntityRenderContext *ctx = ParrotScope_alloc_ctx(scope, BasicEntityRenderContext);

        ctx->self = self;
        ctx->params = params;

        ParrotVideoSceneRenderableComponent_add(
            renderable, scope, ParrotVideoSceneRenderStage_PRE, ParrotVideoSceneRenderableViewportComponent_pre_render);
    }
}

static void ParrotVideoSceneRenderableViewportComponent_destroy(void *selfx, ParrotSceneFuncParams params) {
    ParrotVideoSceneRenderableViewportComponent *self = selfx;
    (void)params;

    PARROT_FAIL_NULL(self->driver);

    self->driver->delete_viewport(self->viewport);
}

static void ParrotVideoSceneRenderableViewportComponent_pre_render(ParrotScope *scope) {
    BasicEntityRenderContext *ctx = ParrotScope_get_ctx(scope, BasicEntityRenderContext *);
    ParrotVideoSceneRenderableViewportComponent *self = ctx->self;

    PARROT_FAIL_NULL(self->driver);

    self->last_camera_entity = ParrotVideoSceneCameraComponent_find_or_null(ctx->params.scene, ctx->params.entity);
    self->last_camera =
        ParrotScene_get_component_or_null(ctx->params.scene, self->last_camera_entity, ParrotVideoSceneCameraComponent);
    if (self->last_camera) {
        self->driver->clear_viewport(self->viewport, self->last_camera->clear_color);
    }
}

ParrotSceneEntity ParrotVideoSceneRenderableViewportComponent_find_or_null(ParrotScene *scene,
                                                                           ParrotSceneEntity entity) {
    PARROT_FAIL_NULL(scene);

    while (entity) {
        if (ParrotScene_get_component_or_null(scene, entity, ParrotVideoSceneRenderableViewportComponent)) {
            return entity;
        }

        entity = ParrotScene_get_entity_parent_or_null(scene, entity);
    }

    return ParrotSceneEntity_NULL;
}

static void ParrotVideoSceneRenderable2DComponent_render(ParrotScope *scope);

static void ParrotVideoSceneRenderable2DComponent_init(void *selfx, ParrotSceneFuncParams params) {
    ParrotVideoSceneRenderable2DComponent *self = selfx;
    (void)params;

    self->tint = ParrotColor_WHITE;
}

static void ParrotVideoSceneRenderable2DComponent_update(void *selfx, ParrotSceneFuncParams params, float delta) {
    ParrotVideoSceneRenderable2DComponent *self = selfx;
    (void)params;
    (void)delta;

    ParrotVideoSceneRenderableComponent *renderable =
        ParrotScene_get_component_or_null(params.scene, params.entity, ParrotVideoSceneRenderableComponent);
    if (renderable) {
        ParrotScope *scope = ParrotScope_new(NULL);
        BasicEntityRenderContext *ctx = ParrotScope_alloc_ctx(scope, BasicEntityRenderContext);

        ctx->self = self;
        ctx->params = params;

        ParrotVideoSceneRenderableComponent_add(
            renderable, scope, ParrotVideoSceneRenderStage_RENDER, ParrotVideoSceneRenderable2DComponent_render);
    }
}

static void ParrotVideoSceneRenderable2DComponent_render(ParrotScope *scope) {
    BasicEntityRenderContext *ctx = ParrotScope_get_ctx(scope, BasicEntityRenderContext *);
    ParrotVideoSceneRenderable2DComponent *self = ctx->self;

    ParrotTransform *transform =
        ParrotScene_get_component_or_null(ctx->params.scene, ctx->params.entity, ParrotTransform);
    PARROT_RET_COND(!transform);

    ParrotSceneEntity viewport_entity =
        ParrotVideoSceneRenderableViewportComponent_find_or_null(ctx->params.scene, ctx->params.entity);
    ParrotVideoSceneRenderableViewportComponent *viewport = ParrotScene_get_component_or_null(
        ctx->params.scene, viewport_entity, ParrotVideoSceneRenderableViewportComponent);
    PARROT_RET_COND(!viewport);

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
    viewport->driver->draw_viewport_vertices(viewport->viewport,
                                             (ParrotGMatSet){
                                                 .model = ParrotTransform_calculate_matrix(transform),
                                                 .view = viewport->last_camera->last_view_matrix,
                                                 .projection = viewport->last_camera->last_projection_matrix,
                                             },
                                             vertices,
                                             6);
    // self->video_driver->clear_viewport_texture(viewport);
}

const ParrotSceneComponentProfile ParrotVideoSceneCameraComponent_profile = {
    .size = sizeof(ParrotVideoSceneCameraComponent),
    .type = PARROT_TYPE_STRING(ParrotVideoSceneCameraComponent),

    .init = ParrotVideoSceneCameraComponent_init,
    .update = ParrotVideoSceneCameraComponent_update,
};

const ParrotSceneComponentProfile ParrotVideoSceneRenderableComponent_profile = {
    .size = sizeof(_ParrotVideoSceneRenderableComponent),
    .type = PARROT_TYPE_STRING(ParrotVideoSceneRenderableComponent),

    .init = ParrotVideoSceneRenderableComponent_init,
    .destroy = ParrotVideoSceneRenderableComponent_destroy,
};

const ParrotSceneComponentProfile ParrotVideoSceneRenderableWindowComponent_profile = {
    .size = sizeof(ParrotVideoSceneRenderableWindowComponent),
    .type = PARROT_TYPE_STRING(ParrotVideoSceneRenderableWindowComponent),

    .ready = ParrotVideoSceneRenderableWindowComponent_ready,
    .update = ParrotVideoSceneRenderableWindowComponent_update,
    .destroy = ParrotVideoSceneRenderableWindowComponent_destroy,
};

const ParrotSceneComponentProfile ParrotVideoSceneRenderableViewportComponent_profile = {
    .size = sizeof(ParrotVideoSceneRenderableViewportComponent),
    .type = PARROT_TYPE_STRING(ParrotVideoSceneRenderableViewportComponent),

    .ready = ParrotVideoSceneRenderableViewportComponent_ready,
    .update = ParrotVideoSceneRenderableViewportComponent_update,
    .destroy = ParrotVideoSceneRenderableViewportComponent_destroy,
};

const ParrotSceneComponentProfile ParrotVideoSceneRenderable2DComponent_profile = {
    .size = sizeof(ParrotVideoSceneRenderable2DComponent),
    .type = PARROT_TYPE_STRING(ParrotVideoSceneRenderable2DComponent),

    .init = ParrotVideoSceneRenderable2DComponent_init,
    .update = ParrotVideoSceneRenderable2DComponent_update,
};
