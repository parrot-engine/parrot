#ifndef __SRC_PARROT_INCLUDE_PARROT_VIDEO_SCENE_H_
#define __SRC_PARROT_INCLUDE_PARROT_VIDEO_SCENE_H_

#include "parrot/scene/world.h"
#include "parrot/video/video.h"
#include <stdbool.h>

typedef struct {
    ParrotVideoObjectHandle object_handle;

    bool visible;
} ParrotVideoSceneRenderableComponent;

typedef struct {
    const char *title;

    int width;
    int height;

    bool close_requested;
} ParrotVideoSceneWindowComponent;

void ParrotVideoSceneSystem_register_components(ParrotSceneWorld *world);
void ParrotVideoSceneSystem_update(ParrotSceneWorld *world, ParrotVideoObjectHandle root_handle);

#endif // __SRC_PARROT_INCLUDE_PARROT_VIDEO_SCENE_H_