#include "parrot/config.h"
#include "parrot/core/main_loop.h"
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

static ParrotVideoSceneViewportActor *viewport;

static float fps_update_timer = 0;

static void TestObjectActor_update(void *selfx, float delta) {
    ParrotVideoSceneRenderableActor2D *self = selfx;

    ParrotTransform transform = ParrotScene_get_actor_transform(self);
    transform.position.x += 50 * delta;
    transform.rotation.z += delta;
    ParrotScene_set_actor_transform(self, transform);

    if (ParrotVideoSceneRenderableActor2D_profile.update) {
        ParrotVideoSceneRenderableActor2D_profile.update(self, delta);
    }
}

static ParrotSceneActorProfile TestObjectActor_profile = {};

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

    TestObjectActor_profile = ParrotVideoSceneRenderableActor2D_profile;
    TestObjectActor_profile.update = TestObjectActor_update;

    viewport = ParrotScene_actor_spawn(scene, &ParrotVideoSceneViewportActor_profile);

    viewport->driver = video_driver;
    viewport->width = SCREEN_WIDTH;
    viewport->height = SCREEN_HEIGHT;

    viewport->window.driver = window_driver;
    viewport->window.title = "Test Window";
    viewport->window.width = SCREEN_WIDTH;
    viewport->window.height = SCREEN_HEIGHT;

    ParrotVideoSceneCameraActor *camera = ParrotScene_actor_spawn(scene, &ParrotVideoSceneCameraActor_profile);
    ParrotScene_set_actor_parent(camera, viewport);
    {
        ParrotTransform transform = ParrotScene_get_actor_transform(camera);
        transform.position = (ParrotVec3){RECT_SIZE / 2.0, RECT_SIZE / 2.0, 0};
        ParrotScene_set_actor_transform(camera, transform);
    }

    ParrotVideoSceneRenderableActor2D *object = ParrotScene_actor_spawn(scene, &TestObjectActor_profile);
    ParrotScene_set_actor_parent(object, viewport);
    object->tint = ParrotColor_RED;
    object->width = RECT_SIZE;
    object->height = RECT_SIZE;
    {
        ParrotTransform transform = ParrotScene_get_actor_transform(object);
        transform.rotation = (ParrotVec3){0, 0, 45};
        ParrotScene_set_actor_transform(object, transform);
    }
}

static bool update(ParrotMainLoopRunSettings *settings, float delta, bool should_close) {
    if (!should_close) {
        should_close = viewport->window.close_requested;
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
    ParrotScene_send_actor_event(viewport, (ParrotSceneActorEvent)ParrotVideoSceneEvent_PRE_RENDER);
    ParrotScene_send_actor_event(viewport, (ParrotSceneActorEvent)ParrotVideoSceneEvent_RENDER);
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
