#include "parrot/core/main_loop.h"
#include "parrot/core/math.h"
#include "parrot/core/reflect.h"
#include "parrot/drivers/gl_driver.h"
#include "parrot/drivers/video_driver.h"
#include "parrot/drivers/window_driver.h"
#include "parrot/module.h"
#include "parrot/scene/transform.h"
#include "parrot/scene/world.h"
#include "parrot/video/scene.h"
#include "parrot/video/video.h"
#include <stdio.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define RECT_SIZE 150

ParrotReflect *reflect = NULL;
ParrotSceneWorld *world = NULL;
ParrotSceneWorldEntity root;
ParrotSceneWorldEntity camera;
ParrotSceneWorldEntity object;

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
        {
            size_t j = 1;
            for (;;) {
                size_t size = ParrotReflect_get_type_field_dimension_size(reflect, type, i, j++);
                if (size == 0) {
                    break;
                }
                printf("[%td]", size);
            }
        }
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

    Parrot_window_driver = &Parrot_x11_window_driver;
    Parrot_window_driver->init();

    Parrot_gl_driver = &Parrot_x11_gl_driver;
    Parrot_gl_driver->init();

    Parrot_video_driver = &Parrot_gl11_video_driver;
    Parrot_video_driver->init();

    ParrotVideo_init();

    reflect = ParrotReflect_new();
    world = ParrotSceneWorld_new();

    ParrotReflect_register(reflect, Parrot_collection);

    ParrotTransform_scene_register(world);
    ParrotVideoSceneSystem_register_components(world);

    root = ParrotSceneWorld_create_entity(world);
    ParrotSceneWorld_add_component(world, root, ParrotVideoSceneWindowComponent);
    ParrotSceneWorld_add_component(world, root, ParrotVideoSceneViewportComponent);
    ParrotSceneWorld_add_component(world, root, ParrotVideoSceneRenderableComponent);

    {
        ParrotVideoSceneWindowComponent *window =
            ParrotSceneWorld_get_component(world, root, ParrotVideoSceneWindowComponent);

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

    camera = ParrotSceneWorld_create_entity(world);
    ParrotSceneWorld_set_entity_parent(world, camera, root);
    ParrotSceneWorld_add_component(world, camera, ParrotTransform);
    ParrotSceneWorld_add_component(world, camera, ParrotVideoSceneRenderableComponent);
    ParrotSceneWorld_add_component(world, camera, ParrotVideoSceneCameraComponent);
    {
        ParrotTransform *transform = ParrotSceneWorld_get_component(world, camera, ParrotTransform);
        transform->position = (ParrotVec3){RECT_SIZE / 2.0, RECT_SIZE / 2.0, 0};
    }

    object = ParrotSceneWorld_create_entity(world);
    ParrotSceneWorld_set_entity_parent(world, object, root);
    ParrotSceneWorld_add_component(world, object, ParrotTransform);
    ParrotSceneWorld_add_component(world, object, ParrotVideoSceneRenderableComponent);
    ParrotSceneWorld_add_component(world, object, ParrotVideoSceneRectComponent);
    {
        ParrotTransform *transform = ParrotSceneWorld_get_component(world, object, ParrotTransform);
        transform->origin = (ParrotVec3){RECT_SIZE / 2.0, RECT_SIZE / 2.0, 0};
        transform->rotation = (ParrotVec3){0, 0, 45};

        ParrotVideoSceneRenderableComponent *renderable =
            ParrotSceneWorld_get_component(world, object, ParrotVideoSceneRenderableComponent);
        renderable->tint = ParrotColor_RED;

        ParrotVideoSceneRectComponent *rect =
            ParrotSceneWorld_get_component(world, object, ParrotVideoSceneRectComponent);

        rect->width = RECT_SIZE;
        rect->height = RECT_SIZE;
    }

    ParrotVideoSceneSystem_update(world, ParrotVideo_get_root());
}

static bool update(ParrotMainLoopRunSettings *settings, float delta, bool should_close) {
    if (!should_close) {
        should_close = ParrotSceneWorld_get_component(world, root, ParrotVideoSceneWindowComponent)->close_requested;
    }

    ParrotTransform *transform = ParrotSceneWorld_get_component(world, object, ParrotTransform);
    transform->position.x += delta * 50;
    transform->rotation.z += delta;

    if (fps_update_timer > 1.0) {
        printf("FPS: %.02f\n", 1 / delta);
        fps_update_timer = 0;
    } else {
        fps_update_timer += delta;
    }

    return should_close;
}

static void render(ParrotMainLoopRunSettings *settings) {
    ParrotVideoSceneSystem_update(world, ParrotVideo_get_root());
    ParrotVideo_render();
}

static void shutdown(ParrotMainLoopRunSettings *settings) {
    ParrotSceneWorld_delete(world);
    ParrotReflect_delete(reflect);

    ParrotVideo_shutdown();

    Parrot_video_driver->shutdown();
    Parrot_gl_driver->shutdown();
    Parrot_window_driver->shutdown();
}

int main(void) {
    ParrotMainLoop *main_loop = ParrotMainLoop_new();
    ParrotMainLoop_run(main_loop, NULL, init, update, render, shutdown);
    ParrotMainLoop_delete(main_loop);
    return 0;
}
