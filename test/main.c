#include "parrot/config.h"
#include "parrot/core/main_loop.h"
#include "parrot/core/math.h"
#include "parrot/core/scope.h"
#include "parrot/core/util.h"
#include "parrot/drivers/gl_driver.h"
#include "parrot/drivers/video_driver.h"
#include "parrot/drivers/window_driver.h"
#include "parrot/module.h"
#include "parrot/scene/scene.h"
#include "parrot/video/scene.h"
#include <stdio.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define RECT_SIZE 150

typedef struct {
    ParrotScope *scope;
    ParrotReflect *reflect;

    ParrotScene *scene;
    ParrotSceneEntity root;

    ParrotVideoSceneRenderableWindowComponent *window;

    float fps_update_timer;
} RunCtx;

typedef struct {
    ParrotVec3 direction;
    ParrotVec3 rotation;
} MovementComponent;

static void MovementComponent_update(void *selfx, ParrotSceneFuncParams params, float delta) {
    MovementComponent *self = selfx;

    ParrotTransform *transform = ParrotScene_get_component_or_null(params.scene, params.entity, ParrotTransform);
    PARROT_RET_COND(!transform);

    transform->position = ParrotVec3_add(transform->position, ParrotVec3_scale(self->direction, delta));
    transform->rotation = ParrotVec3_add(transform->rotation, ParrotVec3_scale(self->rotation, delta));
}

static ParrotSceneComponentProfile MovementComponent_profile = {
    .size = sizeof(MovementComponent),
    .type = PARROT_TYPE_STRING(MovementComponent),

    .update = MovementComponent_update,
};

static void init(ParrotMainLoopRunSettings *settings) {
    settings->user_data = PARROT_ALLOC(RunCtx);
    RunCtx *ctx = settings->user_data;

    ctx->scope = ParrotScope_new(NULL);
    ParrotScope_push_free(ctx->scope, ctx);

    ctx->reflect = ParrotReflect_new();
    ParrotReflect_register(ctx->reflect, Parrot_collection);
    ParrotScope_push(ctx->scope, ParrotReflect_vdelete, ctx->reflect);

#ifdef PARROT_PLATFORM_HAS_OS
#ifdef PARROT_PLATFORM_LINUX
    ParrotWindowDriver *window_driver = Parrot_x11_window_driver_new_or_null();
    ParrotScope_push(ctx->scope, Parrot_x11_window_driver_vdelete, window_driver);

    ParrotGLDriver *gl_driver = Parrot_x11_gl_driver_new_or_null();
    ParrotScope_push(ctx->scope, Parrot_x11_gl_driver_vdelete, gl_driver);
#endif
    ParrotVideoDriver *video_driver = Parrot_gl11_video_driver_new(gl_driver);
    ParrotScope_push(ctx->scope, Parrot_gl11_video_driver_vdelete, video_driver);
#else
#ifdef PARROT_PLATFORM_N64
    ParrotWindowDriver *window_driver = NULL;
    ParrotGLDriver *gl_driver = NULL;
    ParrotVideoDriver *video_driver = NULL;
#endif
#endif

    ctx->scene = ParrotScene_new();
    ParrotScope_push(ctx->scope, ParrotScene_vdelete, ctx->scene);

    ParrotScene_auto_register_components(ctx->scene, ctx->reflect);
    ParrotScene_register_component(ctx->scene, MovementComponent_profile);

    ctx->root = ParrotScene_create_entity(ctx->scene);
    ParrotScene_add_component_or_null(ctx->scene, ctx->root, ParrotVideoSceneRenderableComponent);
    {
#ifndef PARROT_PLATFORM_N64
        ctx->window =
            ParrotScene_add_component_or_null(ctx->scene, ctx->root, ParrotVideoSceneRenderableWindowComponent);
        ctx->window->driver = window_driver;
        ctx->window->title = "Test Window";
        ctx->window->width = SCREEN_WIDTH;
        ctx->window->height = SCREEN_HEIGHT;

        ParrotVideoSceneRenderableViewportComponent *viewport =
            ParrotScene_add_component_or_null(ctx->scene, ctx->root, ParrotVideoSceneRenderableViewportComponent);
        viewport->driver = video_driver;
        viewport->width = SCREEN_WIDTH;
        viewport->height = SCREEN_HEIGHT;
#endif
    }

    ParrotSceneEntity camera_entity = ParrotScene_create_entity(ctx->scene);
    ParrotScene_set_entity_parent(ctx->scene, camera_entity, ctx->root);
    ParrotScene_add_component_or_null(ctx->scene, camera_entity, ParrotVideoSceneRenderableComponent);
    {
        ParrotTransform *transform = ParrotScene_add_component_or_null(ctx->scene, camera_entity, ParrotTransform);
        transform->position = (ParrotVec3){RECT_SIZE / 2.0, RECT_SIZE / 2.0, 0};

        ParrotScene_add_component_or_null(ctx->scene, camera_entity, ParrotVideoSceneCameraComponent);
    }

    ParrotSceneEntity object = ParrotScene_create_entity(ctx->scene);
    ParrotScene_set_entity_parent(ctx->scene, object, ctx->root);
    ParrotScene_add_component_or_null(ctx->scene, object, ParrotVideoSceneRenderableComponent);
    {
        MovementComponent *movement = ParrotScene_add_component_or_null(ctx->scene, object, MovementComponent);
        movement->direction = (ParrotVec3){50, 0, 0};
        movement->rotation = (ParrotVec3){0, 0, 1};

        ParrotTransform *transform = ParrotScene_add_component_or_null(ctx->scene, object, ParrotTransform);
        transform->rotation = (ParrotVec3){0, 0, 45};

        ParrotVideoSceneRenderable2DComponent *renderable =
            ParrotScene_add_component_or_null(ctx->scene, object, ParrotVideoSceneRenderable2DComponent);
        renderable->tint = ParrotColor_RED;
        renderable->width = RECT_SIZE;
        renderable->height = RECT_SIZE;
    }
}

static bool update(ParrotMainLoopRunSettings *settings, float delta, bool should_close) {
    RunCtx *ctx = settings->user_data;

#ifdef PARROT_PLATFORM_N64
    printf("Update frame\n");
#endif

    if (!should_close) {
#ifndef PARROT_PLATFORM_N64
        should_close = ctx->window->close_requested;
#endif
    }

    if (ctx->fps_update_timer > 1.0) {
        printf("FPS: %.02f\n", 1 / delta);
        ctx->fps_update_timer = 0;
    } else {
        ctx->fps_update_timer += delta;
    }

    ParrotScene_update(ctx->scene, delta);
    return should_close;
}

static void render(ParrotMainLoopRunSettings *settings) {
    RunCtx *ctx = settings->user_data;

#ifndef PARROT_PLATFORM_N64
    ParrotVideoSceneRenderableComponent_render(ctx->scene, ctx->root);
#endif
}

static void shutdown(ParrotMainLoopRunSettings *settings) {
    RunCtx *ctx = settings->user_data;

    ParrotScope_delete(ctx->scope);
}

int main(void) {
    ParrotMainLoop *main_loop = ParrotMainLoop_new();
    ParrotMainLoop_run(main_loop, NULL, init, update, render, shutdown);
    ParrotMainLoop_delete(main_loop);
    return 0;
}
