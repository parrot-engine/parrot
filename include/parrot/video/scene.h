#ifndef __SRC_PARROT_INCLUDE_PARROT_VIDEO_SCENE_H_
#define __SRC_PARROT_INCLUDE_PARROT_VIDEO_SCENE_H_

#include "parrot/core/math.h"
#include "parrot/core/reflect.h"
#include "parrot/drivers/video_driver.h"
#include "parrot/drivers/window_driver.h"
#include "parrot/scene/scene.h"
#include <stdbool.h>

typedef enum {
    /// Send this event to a renderable before drawing it
    ParrotVideoSceneEvent_PRE_RENDER = ParrotSceneActorEvent_USER_BEGIN,
    /// Send this event to a renderable to draw it and it's children
    ParrotVideoSceneEvent_RENDER,

    ParrotVideoSceneEvent_USER_BEGIN = ParrotSceneActorEvent_USER_BEGIN + 0x1000,
} ParrotVideoSceneEvent;

typedef struct {
    const char *title;

    int width;
    int height;
    bool resize;

    bool close_requested;

    /// Creates a window using the driver if not NULL at TREE_ENTER event.
    ParrotWindowDriver *driver;
    ParrotWindowDriverWindow *window;
} ParrotVideoSceneViewportWindow;

static const ParrotReflectDescription ParrotVideoSceneViewportWindow_description[] = {
    PARROT_REFLECT_TYPE_HEADER(ParrotVideoSceneViewportWindow),

    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneViewportWindow, const char *, title, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneViewportWindow, bool, close_requested, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneViewportWindow, bool, resize, ),

    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneViewportWindow, ParrotVideoDriver *, driver, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneViewportWindow, ParrotWindowDriverWindow *, window, ),

    PARROT_REFLECT_END(),
};

typedef struct {
    int width;
    int height;

    ParrotVideoDriver *driver;
    ParrotVideoDriverViewport *viewport;

    ParrotVideoSceneViewportWindow window;
} ParrotVideoSceneViewportActor;

PARROT_API extern const ParrotSceneActorProfile ParrotVideoSceneViewportActor_profile;

static const ParrotReflectDescription ParrotVideoSceneViewportActor_description[] = {
    PARROT_REFLECT_TYPE_HEADER(ParrotVideoSceneViewportActor),

    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneViewportActor, int, width, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneViewportActor, int, height, ),

    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneViewportActor, ParrotWindowDriverWindow *, window, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneViewportActor, ParrotVideoSceneViewportWindow, window, ),

    PARROT_REFLECT_END(),
};

typedef struct {
    bool corner_aligned;

    bool use_clear_color;
    ParrotColor clear_color;
} ParrotVideoSceneCameraActor;

PARROT_API extern const ParrotSceneActorProfile ParrotVideoSceneCameraActor_profile;

static const ParrotReflectDescription ParrotVideoSceneCameraActor_description[] = {
    PARROT_REFLECT_TYPE_HEADER(ParrotVideoSceneCameraActor),

    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneCameraActor, bool, corner_aligned, ),

    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneCameraActor, bool, use_clear_color, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneCameraActor, ParrotColor, clear_color, ),

    PARROT_REFLECT_END(),
};

typedef struct {
    ParrotColor tint;

    /**
     * Normal: Used as rectangle width.
     * Textured: Used as region width or 0 for the texture width.
     */
    int width;
    /**
     * Normal: Used as rectangle height.
     * Textured: Used as region height or 0 for the texture height.
     */
    int height;

    // ParrotVideoTexture *texture;
    int texture_x, texture_y;
    bool texture_nearest_filter;
} ParrotVideoSceneRenderableActor2D;

PARROT_API extern const ParrotSceneActorProfile ParrotVideoSceneRenderableActor2D_profile;

static const ParrotReflectDescription ParrotVideoSceneRenderableActor2D_description[] = {
    PARROT_REFLECT_TYPE_HEADER(ParrotVideoSceneRenderableActor2D),

    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRenderableActor2D, ParrotReal, width, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRenderableActor2D, ParrotReal, height, ),

    /*PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRenderableActor2D, bool, texture_nearest_filter, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRenderableActor2D, int, texture_x, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRenderableActor2D, int, texture_y, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRenderableActor2D, bool, texture_use_region, ),*/

    PARROT_REFLECT_END(),
};

static const ParrotReflectDescription Parrot_video_scene_collection[] = {
    PARROT_REFLECT_COLLECTION_HEADER(),

    PARROT_REFLECT_COLLECTION_DESCRIPTION(ParrotVideoSceneViewportWindow_description),

    PARROT_REFLECT_COLLECTION_DESCRIPTION(ParrotVideoSceneViewportActor_description),
    PARROT_REFLECT_COLLECTION_DESCRIPTION(ParrotVideoSceneCameraActor_description),
    PARROT_REFLECT_COLLECTION_DESCRIPTION(ParrotVideoSceneRenderableActor2D_description),

    PARROT_REFLECT_END(),
};

#endif // __SRC_PARROT_INCLUDE_PARROT_VIDEO_SCENE_H_
