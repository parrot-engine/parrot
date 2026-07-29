#ifndef __SRC_PARROT_INCLUDE_PARROT_VIDEO_SCENE_H_
#define __SRC_PARROT_INCLUDE_PARROT_VIDEO_SCENE_H_

#include "parrot/core/math.h"
#include "parrot/scene/world.h"
#include "parrot/video/video.h"
#include <stdbool.h>

typedef struct {
    ParrotVideoObjectHandle object_handle;

    bool visible;
    ParrotVec4 tint;
} ParrotVideoSceneRenderableComponent;

typedef struct {
    const char *title;

    int width;
    int height;

    bool close_requested;
} ParrotVideoSceneWindowComponent;

typedef struct {
    int width;
    int height;
} ParrotVideoSceneViewportComponent;

typedef struct {
    bool use_clear_color;
    ParrotVec3 clear_color;
} ParrotVideoSceneCameraComponent;

typedef struct {
    ParrotReal width;
    ParrotReal height;
} ParrotVideoSceneRectComponent;

void ParrotVideoSceneSystem_register_components(ParrotSceneWorld *world);
void ParrotVideoSceneSystem_update(ParrotSceneWorld *world, ParrotVideoObjectHandle root_handle);

#endif // __SRC_PARROT_INCLUDE_PARROT_VIDEO_SCENE_H_