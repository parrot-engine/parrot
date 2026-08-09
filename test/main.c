#include "parrot/core/math.h"
#include "parrot/scene/matrix.h"
#include "parrot/scene/world.h"
#include "parrot/video/scene.h"
#include <stdio.h>

int main(void) {
    ParrotVideo_init();

    printf("Test 123\n");

    ParrotSceneWorld *world = ParrotSceneWorld_new();

    ParrotMat_scene_register(world);

    ParrotVideoSceneSystem_register_components(world);

    ParrotSceneWorldEntity root = ParrotSceneWorld_create_entity(world);
    ParrotSceneWorld_add_component(world, root, ParrotVideoSceneWindowComponent);
    ParrotSceneWorld_add_component(world, root, ParrotVideoSceneViewportComponent);
    ParrotSceneWorld_add_component(world, root, ParrotVideoSceneRenderableComponent);

    {
        ParrotVideoSceneWindowComponent *window =
            ParrotSceneWorld_get_component(world, root, ParrotVideoSceneWindowComponent);

        window->title = "Test Window";
        window->width = 320;
        window->height = 240;
        window->resize = true;
    }

    {
        ParrotVideoSceneViewportComponent *viewport =
            ParrotSceneWorld_get_component(world, root, ParrotVideoSceneViewportComponent);

        viewport->width = 320;
        viewport->height = 240;
    }

    ParrotSceneWorldEntity camera = ParrotSceneWorld_create_entity(world);
    ParrotSceneWorld_set_entity_parent(world, camera, root);
    ParrotSceneWorld_add_component(world, camera, ParrotVideoSceneRenderableComponent);
    ParrotSceneWorld_add_component(world, camera, ParrotVideoSceneCameraComponent);

    ParrotSceneWorldEntity object = ParrotSceneWorld_create_entity(world);
    ParrotSceneWorld_set_entity_parent(world, object, root);
    ParrotSceneWorld_add_component(world, object, ParrotMat);
    ParrotSceneWorld_add_component(world, object, ParrotVideoSceneRenderableComponent);
    ParrotSceneWorld_add_component(world, object, ParrotVideoSceneRectComponent);

    {
        ParrotMat *matrix = ParrotSceneWorld_get_component(world, object, ParrotMat);
        *matrix = ParrotMat_set_position(*matrix, (ParrotVec3){100, 100, 0});
        *matrix = ParrotMat_set_rotation(*matrix, (ParrotVec3){0, 0, 45});

        for (int y = 0; y < 4; y++) {
            printf("%.02f %.02f %.02f %.02f\n",
                   matrix->data[0][y],
                   matrix->data[1][y],
                   matrix->data[2][y],
                   matrix->data[3][y]);
        }
    }

    {
        ParrotVideoSceneRenderableComponent *renderable =
            ParrotSceneWorld_get_component(world, object, ParrotVideoSceneRenderableComponent);
        renderable->tint = ParrotColor_RED;

        ParrotVideoSceneRectComponent *rect =
            ParrotSceneWorld_get_component(world, object, ParrotVideoSceneRectComponent);

        rect->width = 100;
        rect->height = 100;
    }

    while (!ParrotSceneWorld_get_component(world, root, ParrotVideoSceneWindowComponent)->close_requested) {
        {
            ParrotMat *matrix = ParrotSceneWorld_get_component(world, object, ParrotMat);
            ParrotVec3 position = ParrotMat_get_position(*matrix);
            // position.x += 1;
            *matrix = ParrotMat_set_position(*matrix, position);
        }

        ParrotVideoSceneSystem_update(world, ParrotVideo_get_root());
        ParrotVideo_render();
        printf("Loop\n");
    }

    printf("End\n");
    ParrotSceneWorld_delete(world);

    ParrotVideo_shutdown();
    return 0;
}
