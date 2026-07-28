#include "parrot/scene/world.h"
#include "parrot/video/scene.h"
#include "parrot/video/video.h"

int main(void) {
    ParrotVideo_init();

    ParrotSceneWorld *world = ParrotSceneWorld_new();

    ParrotVideoSceneSystem_register_components(world);

    ParrotSceneWorldEntity root = ParrotSceneWorld_create_entity(world);
    ParrotSceneWorld_add_component(world, root, ParrotVideoSceneWindowComponent);
    ParrotSceneWorld_add_component(world, root, ParrotVideoSceneRenderableComponent);

    ParrotVideoSceneWindowComponent *window =
        ParrotSceneWorld_get_component(world, root, ParrotVideoSceneWindowComponent);

    window->title = "Test Window";
    window->width = 1280;
    window->height = 720;

    while (!window->close_requested) {
        ParrotVideoSceneSystem_update(world, ParrotVideo_get_root());
        ParrotVideo_render();
    }

    ParrotSceneWorld_delete(world);

    ParrotVideo_shutdown();
    return 0;
}