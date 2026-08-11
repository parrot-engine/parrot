#include "parrot/core/main_loop.h"
#include "parrot/core/math.h"
#include "parrot/scene/transform.h"
#include "parrot/scene/world.h"
#include "parrot/video/scene.h"
#include "parrot/video/video.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

ParrotSceneWorld *world = NULL;
ParrotSceneWorldEntity root;
ParrotSceneWorldEntity camera;
ParrotSceneWorldEntity object;

float fps_update_timer = 0;

static void init(ParrotMainLoopRunSettings *settings) {
    settings->max_fps = 60;

    ParrotVideo_init();

    world = ParrotSceneWorld_new();

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
    ParrotSceneWorld_add_component(world, camera, ParrotVideoSceneRenderableComponent);
    ParrotSceneWorld_add_component(world, camera, ParrotVideoSceneCameraComponent);

    object = ParrotSceneWorld_create_entity(world);
    ParrotSceneWorld_set_entity_parent(world, object, root);
    ParrotSceneWorld_add_component(world, object, ParrotTransform);
    ParrotSceneWorld_add_component(world, object, ParrotVideoSceneRenderableComponent);
    ParrotSceneWorld_add_component(world, object, ParrotVideoSceneRectComponent);

    {
        ParrotTransform *transform = ParrotSceneWorld_get_component(world, object, ParrotTransform);
        transform->position = (ParrotVec3){SCREEN_WIDTH / 2.0, SCREEN_HEIGHT / 2.0, 0};
        transform->rotation = (ParrotVec3){0, 0, 45};
    }

    {
        ParrotVideoSceneRenderableComponent *renderable =
            ParrotSceneWorld_get_component(world, object, ParrotVideoSceneRenderableComponent);
        renderable->tint = ParrotColor_RED;

        ParrotVideoSceneRectComponent *rect =
            ParrotSceneWorld_get_component(world, object, ParrotVideoSceneRectComponent);

        rect->width = 150;
        rect->height = 150;
    }
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

    ParrotVideo_shutdown();
}

int main(void) {
    ParrotMainLoop *main_loop = ParrotMainLoop_new();
    ParrotMainLoop_run(main_loop, NULL, init, update, render, shutdown);
    ParrotMainLoop_delete(main_loop);
    return 0;
}
