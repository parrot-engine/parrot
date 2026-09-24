#include "parrot/core/main_loop.h"
#include "parrot/core/math.h"
#include "parrot/core/reflect.h"
#include "parrot/drivers/gl_driver.h"
#include "parrot/drivers/video_driver.h"
#include "parrot/drivers/window_driver.h"
#include "parrot/drivers/window_driver.keys.h"
#include "parrot/module.h"
#include "parrot/scene/world.h"
#include "parrot/ui/ui.h"
#include "parrot/video/font.h"
#include "parrot/video/scene.h"
#include "parrot/video/video.h"
#include <stdio.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define RECT_SIZE 150

static ParrotScope *scope;

static ParrotReflect *reflect;

static ParrotSceneWorld *world;

static ParrotUISystem *ui_system;

static ParrotVideoObjectHandle root_handle;
static ParrotVideoSceneWindowComponent *window;

static ParrotTransform *object_transform;
static ParrotTransform *camera_transform;

static float fps_update_timer = 0;

static void init(ParrotMainLoopRunSettings *settings) {
    scope = ParrotScope_new(NULL);

    reflect = ParrotReflect_new();
    ParrotReflect_register(reflect, Parrot_collection);
    ParrotScope_push(scope, ParrotReflect_vdelete, reflect);

    ParrotWindowDriver *window_driver = Parrot_x11_window_driver_new();
    ParrotScope_push(scope, Parrot_x11_window_driver_vdelete, window_driver);

    ParrotGLDriver *gl_driver = Parrot_x11_gl_driver_new();
    ParrotScope_push(scope, Parrot_x11_gl_driver_vdelete, gl_driver);

    ParrotVideoDriver *video_driver = Parrot_gl11_video_driver_new(gl_driver);
    ParrotScope_push(scope, Parrot_gl11_video_driver_vdelete, video_driver);

    ParrotVideo_init(window_driver, video_driver);
    ParrotScope_push(scope, ParrotVideo_vshutdown, NULL);

    ParrotVideoFont *font = ParrotVideoFont_new_default(ParrotVideoFontDefaultStyle_MODERN);
    ParrotScope_push(scope, ParrotVideoFont_vdelete, font);

    world = ParrotSceneWorld_new();
    ParrotScope_push(scope, ParrotSceneWorld_vdelete, world);
    ParrotVideoSceneSystem_register_components(world);

    ui_system = ParrotUISystem_new(world, ParrotVideo_get_root());
    ParrotScope_push(scope, ParrotUISystem_vdelete, ui_system);

    ParrotSceneWorldEntity root = ParrotSceneWorld_create_entity(world);
    ParrotVideoSceneRenderableComponent *root_renderable = NULL;
    ParrotSceneWorld_add_component(world, root, ParrotVideoSceneWindowComponent);
    ParrotSceneWorld_add_component(world, root, ParrotVideoSceneViewportComponent);
    ParrotSceneWorld_add_component(world, root, ParrotVideoSceneRenderableComponent);
    {
        root_renderable = ParrotSceneWorld_get_component(world, root, ParrotVideoSceneRenderableComponent);

        window = ParrotSceneWorld_get_component(world, root, ParrotVideoSceneWindowComponent);

        window->title = "Test Window";
        window->width = SCREEN_WIDTH;
        window->height = SCREEN_HEIGHT;
    }

    {
        ParrotVideoSceneViewportComponent *viewport =
            ParrotSceneWorld_get_component(world, root, ParrotVideoSceneViewportComponent);

        viewport->width = SCREEN_WIDTH;
        viewport->height = SCREEN_HEIGHT;
    }

    ParrotSceneWorldEntity camera = ParrotSceneWorld_create_entity(world);
    ParrotSceneWorld_set_entity_parent(world, camera, root);
    ParrotSceneWorld_add_component(world, camera, ParrotTransform);
    ParrotSceneWorld_add_component(world, camera, ParrotVideoSceneRenderableComponent);
    ParrotSceneWorld_add_component(world, camera, ParrotVideoSceneCameraComponent);
    {
        camera_transform = ParrotSceneWorld_get_component(world, camera, ParrotTransform);
        camera_transform->position = (ParrotVec3){RECT_SIZE / 2.0, RECT_SIZE / 2.0, 0};
    }

    ParrotSceneWorldEntity object = ParrotSceneWorld_create_entity(world);
    ParrotSceneWorld_set_entity_parent(world, object, root);
    ParrotSceneWorld_add_component(world, object, ParrotTransform);
    ParrotSceneWorld_add_component(world, object, ParrotVideoSceneRenderableComponent);
    ParrotSceneWorld_add_component(world, object, ParrotVideoSceneRectComponent);
    {
        object_transform = ParrotSceneWorld_get_component(world, object, ParrotTransform);
        object_transform->rotation = (ParrotVec3){0, 0, 45};

        ParrotVideoSceneRenderableComponent *renderable =
            ParrotSceneWorld_get_component(world, object, ParrotVideoSceneRenderableComponent);
        renderable->tint = ParrotColor_RED;

        ParrotVideoSceneRectComponent *rect =
            ParrotSceneWorld_get_component(world, object, ParrotVideoSceneRectComponent);

        rect->width = RECT_SIZE;
        rect->height = RECT_SIZE;
    }

    ParrotSceneWorldEntity window = ParrotSceneWorld_create_entity(world);
    ParrotSceneWorld_set_entity_parent(world, window, root);
    ParrotSceneWorld_add_component(world, window, ParrotTransform);
    ParrotSceneWorld_add_component(world, window, ParrotUIComponent);
    ParrotSceneWorld_add_component(world, window, ParrotUIWindowComponent);
    {
        ParrotTransform *transform = ParrotSceneWorld_get_component(world, window, ParrotTransform);
        transform->position.z = 1;

        ParrotUIComponent *ui = ParrotSceneWorld_get_component(world, window, ParrotUIComponent);
        ui->font = font;
        ui->min_width = 640;
        ui->min_height = 480;

        ParrotUIWindowComponent *ui_window = ParrotSceneWorld_get_component(world, window, ParrotUIWindowComponent);
        ui_window->title = "Test (sub) window";
    }

    ParrotVideoSceneSystem_update(world, ParrotVideo_get_root());
    root_handle = root_renderable->object_handle;
}

static bool update(ParrotMainLoopRunSettings *settings, float delta, bool should_close) {
    if (!should_close) {
        should_close = window->close_requested;
    }

    {
        object_transform->position.x += delta * 50;
        object_transform->rotation.z += delta;
    }

    {
        ParrotVec2 direction = {
            ParrotVideo_object_is_viewport_key_down(root_handle, ParrotWindowDriverEventKey_D) -
                ParrotVideo_object_is_viewport_key_down(root_handle, ParrotWindowDriverEventKey_A),
            ParrotVideo_object_is_viewport_key_down(root_handle, ParrotWindowDriverEventKey_S) -
                ParrotVideo_object_is_viewport_key_down(root_handle, ParrotWindowDriverEventKey_W),
        };

        camera_transform->position =
            ParrotVec3_add(camera_transform->position, ParrotVec3_upgrade(ParrotVec2_scale(direction, delta * 500)));
    }

    ParrotVideoObjectEvent event = {0};
    while (ParrotVideo_poll_object_events(root_handle, &event)) {
        if (event.type == ParrotVideoObjectEventType_WINDOW) {
            ParrotWindowDriverEvent window_event = event.data.window;
            if (window_event.type == ParrotWindowDriverEventType_KEY) {
                if (window_event.data.key.key_down &&
                    window_event.data.key.logical_key == ParrotWindowDriverEventKey_ESCAPE) {
                    should_close = true;
                }
            }
        }
    }

    if (fps_update_timer > 1.0) {
        printf("FPS: %.02f\n", 1 / delta);
        fps_update_timer = 0;
    } else {
        fps_update_timer += delta;
    }

    ParrotUISystem_update(ui_system, root_handle);

    ParrotSceneWorld_delete_queued(world);
    return should_close;
}

static void render(ParrotMainLoopRunSettings *settings) {
    ParrotVideoSceneSystem_update(world, ParrotVideo_get_root());
    ParrotVideo_render();
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
