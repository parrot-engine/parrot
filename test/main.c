#include "parrot/video/video.h"

int main(void) {
    ParrotVideo_init();

    ParrotVideo_object_add_window(ParrotVideo_get_root(), 1280, 720);
    ParrotVideo_object_add_viewport(ParrotVideo_get_root(), 1280, 720);

    ParrotVideo_object_set_window_title(ParrotVideo_get_root(), "Test Window");

    while (!ParrotVideo_object_is_window_close_requested(ParrotVideo_get_root())) {
        ParrotVideo_render();
    }

    ParrotVideo_shutdown();
    return 0;
}