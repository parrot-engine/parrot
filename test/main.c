#include "parrot/core/math.h"
#include "parrot/scene/matrix.h"
#include "parrot/scene/world.h"
#include "parrot/video/scene.h"
#include "parrot/video/video.h"
#include <stdio.h>

int main(void) {
    // ParrotVideo_init();

    ParrotSceneWorld *world = ParrotSceneWorld_new();

    ParrotMat_scene_register(world);

    // ParrotVideoSceneSystem_register_components(world);

    ParrotSceneWorldEntity root = ParrotSceneWorld_create_entity(world);
    /*ParrotSceneWorld_add_component(world, root, ParrotVideoSceneWindowComponent);
     ParrotSceneWorld_add_component(world, root, ParrotVideoSceneViewportComponent);
     ParrotSceneWorld_add_component(world, root, ParrotVideoSceneRenderableComponent);

     {
        ParrotVideoSceneWindowComponent *window =
        ParrotSceneWorld_get_component(world, root, ParrotVideoSceneWindowComponent);

        window->title = "Test Window";
        window->width = 1280;
        window->height = 720;
        }

        {
            ParrotVideoSceneViewportComponent *viewport =
            ParrotSceneWorld_get_component(world, root, ParrotVideoSceneViewportComponent);

            viewport->width = 1280;
            viewport->height = 720;
            }

            ParrotSceneWorldEntity camera = ParrotSceneWorld_create_entity(world);
            ParrotSceneWorld_set_entity_parent(world, camera, root);
            ParrotSceneWorld_add_component(world, camera, ParrotVideoSceneRenderableComponent);
            ParrotSceneWorld_add_component(world, camera, ParrotVideoSceneCameraComponent);
            */

    ParrotSceneWorldEntity object = ParrotSceneWorld_create_entity(world);
    // ParrotSceneWorld_set_entity_parent(world, object, root);
    ParrotSceneWorld_add_component(world, object, ParrotMat);
    /*ParrotSceneWorld_add_component(world, object, ParrotVideoSceneRenderableComponent);
     ParrotSceneWorld_add_component(world, object, ParrotVideoSceneRectComponent);*/

    {
        ParrotMat *matrix = ParrotSceneWorld_get_component(world, object, ParrotMat);
        *matrix = ParrotMat_set_position(*matrix, (ParrotVec3){50, 50, 0});

        for (int y = 0; y < 4; y++) {
            printf("%.02f %.02f %.02f %.02f\n",
                   matrix->data[0][y],
                   matrix->data[1][y],
                   matrix->data[2][y],
                   matrix->data[3][y]);
        }
    }

    /*    ParrotVideoSceneRenderableComponent *renderable =
            ParrotSceneWorld_get_component(world, object, ParrotVideoSceneRenderableComponent);
        renderable->tint = ParrotVec4_n(0);
        renderable->tint.x = 1;
        renderable->tint.w = 1;

        ParrotVideoSceneRectComponent *rect =
            ParrotSceneWorld_get_component(world, object, ParrotVideoSceneRectComponent);

        rect->width = 100;
        rect->height = 100;
    }

    while (!ParrotSceneWorld_get_component(world, root, ParrotVideoSceneWindowComponent)->close_requested) {
        ParrotVideoSceneSystem_update(world, ParrotVideo_get_root());
        ParrotVideo_render();
    }*/

    ParrotSceneWorld_delete(world);

    // ParrotVideo_shutdown();
    return 0;
}