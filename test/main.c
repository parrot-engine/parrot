#include "parrot/config.h"
#include "parrot/core/main_loop.h"
#include "parrot/core/math.h"
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

static ParrotScope *scope;

static ParrotReflect *reflect;

static ParrotScene *scene;

static ParrotSceneEntity root;

static ParrotVideoSceneRenderableWindowComponent *window;

static float fps_update_timer = 0;

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
    scope = ParrotScope_new(NULL);

    reflect = ParrotReflect_new();
    ParrotReflect_register(reflect, Parrot_collection);
    ParrotScope_push(scope, ParrotReflect_vdelete, reflect);

#ifdef PARROT_PLATFORM_HAS_OS
#ifdef PARROT_PLATFORM_LINUX
    ParrotWindowDriver *window_driver = Parrot_x11_window_driver_new_or_null();
    ParrotScope_push(scope, Parrot_x11_window_driver_vdelete, window_driver);

    ParrotGLDriver *gl_driver = Parrot_x11_gl_driver_new_or_null();
    ParrotScope_push(scope, Parrot_x11_gl_driver_vdelete, gl_driver);
#endif
    ParrotVideoDriver *video_driver = Parrot_gl11_video_driver_new(gl_driver);
    ParrotScope_push(scope, Parrot_gl11_video_driver_vdelete, video_driver);
#else
#ifdef PARROT_PLATFORM_N64
    ParrotWindowDriver *window_driver = NULL;
    ParrotGLDriver *gl_driver = NULL;
    ParrotVideoDriver *video_driver = NULL;
#endif
#endif

    scene = ParrotScene_new();
    ParrotScope_push(scope, ParrotScene_vdelete, scene);

    ParrotScene_auto_register_components(scene, reflect);
    ParrotScene_register_component(scene, MovementComponent_profile);

    root = ParrotScene_create_entity(scene);
    ParrotScene_add_component_or_null(scene, root, ParrotVideoSceneRenderableComponent);
    {
        window = ParrotScene_add_component_or_null(scene, root, ParrotVideoSceneRenderableWindowComponent);
        window->driver = window_driver;
        window->title = "Test Window";
        window->width = SCREEN_WIDTH;
        window->height = SCREEN_HEIGHT;

        ParrotVideoSceneRenderableViewportComponent *viewport =
            ParrotScene_add_component_or_null(scene, root, ParrotVideoSceneRenderableViewportComponent);
        viewport->driver = video_driver;
        viewport->width = SCREEN_WIDTH;
        viewport->height = SCREEN_HEIGHT;
    }

    ParrotSceneEntity camera_entity = ParrotScene_create_entity(scene);
    ParrotScene_set_entity_parent(scene, camera_entity, root);
    ParrotScene_add_component_or_null(scene, camera_entity, ParrotVideoSceneRenderableComponent);
    {
        ParrotTransform *transform = ParrotScene_add_component_or_null(scene, camera_entity, ParrotTransform);
        transform->position = (ParrotVec3){RECT_SIZE / 2.0, RECT_SIZE / 2.0, 0};

        ParrotScene_add_component_or_null(scene, camera_entity, ParrotVideoSceneCameraComponent);
    }

    ParrotSceneEntity object = ParrotScene_create_entity(scene);
    ParrotScene_set_entity_parent(scene, object, root);
    ParrotScene_add_component_or_null(scene, object, ParrotVideoSceneRenderableComponent);
    {
        MovementComponent *movement = ParrotScene_add_component_or_null(scene, object, MovementComponent);
        movement->direction = (ParrotVec3){50, 0, 0};
        movement->rotation = (ParrotVec3){0, 0, 1};

        ParrotTransform *transform = ParrotScene_add_component_or_null(scene, object, ParrotTransform);
        transform->rotation = (ParrotVec3){0, 0, 45};

        ParrotVideoSceneRenderable2DComponent *renderable =
            ParrotScene_add_component_or_null(scene, object, ParrotVideoSceneRenderable2DComponent);
        renderable->tint = ParrotColor_RED;
        renderable->width = RECT_SIZE;
        renderable->height = RECT_SIZE;
    }
}

static bool update(ParrotMainLoopRunSettings *settings, float delta, bool should_close) {
    if (!should_close) {
        should_close = window->close_requested;
    }

    if (fps_update_timer > 1.0) {
        printf("FPS: %.02f\n", 1 / delta);
        fps_update_timer = 0;
    } else {
        fps_update_timer += delta;
    }

    ParrotScene_update(scene, delta);
    return should_close;
}

static void render(ParrotMainLoopRunSettings *settings) {
    ParrotVideoSceneRenderableComponent_render(scene, root);
}

static void shutdown(ParrotMainLoopRunSettings *settings) {
    ParrotScope_delete(scope);
}

int main(void) {
    ParrotMainLoop *main_loop = ParrotMainLoop_new();
    ParrotMainLoop_run(main_loop, NULL, init, update, render, shutdown);
    ParrotMainLoop_delete(main_loop);
    return 0;
}
