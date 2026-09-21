#include "parrot/core/main_loop.h"
#include "parrot/core/math.h"
#include "parrot/core/reflect.h"
#include "parrot/drivers/gl_driver.h"
#include "parrot/drivers/video_driver.h"
#include "parrot/drivers/window_driver.h"
#include "parrot/drivers/window_driver.keys.h"
#include "parrot/module.h"
#include "parrot/scene/world.h"
#include "parrot/video/scene.h"
#include "parrot/video/video.h"
#include <stdio.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define RECT_SIZE 150

ParrotReflect *reflect = NULL;
ParrotSceneWorld *world = NULL;

ParrotVideoObjectHandle root_handle;
ParrotVideoSceneWindowComponent *window;

ParrotTransform *object_transform;
ParrotTransform *camera_transform;

ParrotWindowDriver *window_driver;
ParrotGLDriver *gl_driver;
ParrotVideoDriver *video_driver;

float fps_update_timer = 0;

static void print_type(const char *typename) {
    ptrdiff_t type = ParrotReflect_resolve_type(reflect, typename);
    printf("Type \"%s\":\n", typename);
    printf("  Size: %td\n", ParrotReflect_get_type_size(reflect, type));

    for (size_t i = 0; i < ParrotReflect_get_type_field_count(reflect, type); i++) {
        char *name = ParrotReflect_get_type_field_name(reflect, type, i);
        char *typename = ParrotReflect_get_type_field_typename(reflect, type, i);

        printf("  Field \"%s\":\n", name);
        printf("    Type: %s", typename);
        printf("\n");
        printf("    Offset: %td\n", ParrotReflect_get_type_field_offset(reflect, type, i));
        printf("    Size: %td\n", ParrotReflect_get_type_field_size(reflect, type, i));

        free(typename);
        free(name);
    }

    for (size_t i = 0; i < ParrotReflect_get_type_field_count(reflect, type); i++) {
        char *typename = ParrotReflect_get_type_field_typename(reflect, type, i);

        if (ParrotReflect_resolve_type(reflect, typename) >= 0) {
            print_type(typename);
        }

        free(typename);
    }
}

static void init(ParrotMainLoopRunSettings *settings) {
    settings->max_fps = 60;

    reflect = ParrotReflect_new();

    window_driver = Parrot_x11_window_driver_new();
    gl_driver = Parrot_x11_gl_driver_new();
    video_driver = Parrot_gl11_video_driver_new(gl_driver);

    ParrotVideo_init(window_driver, video_driver);

    world = ParrotSceneWorld_new();

    ParrotReflect_register(reflect, Parrot_collection);

    ParrotVideoSceneSystem_register_components(world);

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
        object_transform->parent = object_transform;
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
    ParrotSceneWorld_add_component(world, window, ParrotVideoSceneRenderableComponent);
    ParrotSceneWorld_add_component(world, window, ParrotVideoSceneUIWindowComponent);
    {
        ParrotVideoSceneUIWindowComponent *ui_window =
            ParrotSceneWorld_get_component(world, window, ParrotVideoSceneUIWindowComponent);

        ui_window->width = 640;
        ui_window->height = 480;
        ui_window->title = "Test window";
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
            ParrotVideo_object_is_window_key_down(root_handle, ParrotWindowDriverEventKey_D) -
                ParrotVideo_object_is_window_key_down(root_handle, ParrotWindowDriverEventKey_A),
            ParrotVideo_object_is_window_key_down(root_handle, ParrotWindowDriverEventKey_S) -
                ParrotVideo_object_is_window_key_down(root_handle, ParrotWindowDriverEventKey_W),
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

    ParrotSceneWorld_delete_queued(world);
    return should_close;
}

static void render(ParrotMainLoopRunSettings *settings) {
    ParrotVideoSceneSystem_update(world, ParrotVideo_get_root());
    ParrotVideo_render();
}

static void shutdown(ParrotMainLoopRunSettings *settings) {
    ParrotSceneWorld_delete(world);

    ParrotVideo_shutdown();

    Parrot_gl11_video_driver_delete(video_driver);
    Parrot_x11_gl_driver_delete(gl_driver);
    Parrot_x11_window_driver_delete(window_driver);

    ParrotReflect_delete(reflect);
}

int main(void) {
    ParrotMainLoop *main_loop = ParrotMainLoop_new();
    ParrotMainLoop_run(main_loop, NULL, init, update, render, shutdown);
    ParrotMainLoop_delete(main_loop);
    return 0;
}
