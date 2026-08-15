#ifndef __SRC_PARROT_INCLUDE_PARROT_VIDEO_SCENE_H_
#define __SRC_PARROT_INCLUDE_PARROT_VIDEO_SCENE_H_

#include "parrot/core/file.h"
#include "parrot/core/math.h"
#include "parrot/core/reflect.h"
#include "parrot/scene/world.h"
#include "parrot/video/video.h"
#include <stdbool.h>

typedef struct {
    ParrotVideoObjectHandle object_handle;

    bool visible;
    ParrotColor tint;
} ParrotVideoSceneRenderableComponent;

static const ParrotReflectDescription ParrotVideoSceneRenderableComponent_description[] = {
    PARROT_REFLECT_TYPE_HEADER(ParrotVideoSceneRenderableComponent),

    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRenderableComponent, ParrotVideoObjectHandle, object_handle, ),

    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRenderableComponent, bool, visible, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRenderableComponent, ParrotColor, tint, ),

    PARROT_REFLECT_END(),
};

typedef struct {
    const char *title;

    int width;
    int height;
    bool resize;

    bool close_requested;
} ParrotVideoSceneWindowComponent;

static const ParrotReflectDescription ParrotVideoSceneWindowComponent_description[] = {
    PARROT_REFLECT_TYPE_HEADER(ParrotVideoSceneWindowComponent),

    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneWindowComponent, const char *, title, ),

    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneWindowComponent, int, width, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneWindowComponent, int, height, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneWindowComponent, bool, resize, ),

    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneWindowComponent, bool, close_requested, ),

    PARROT_REFLECT_END(),
};

typedef struct {
    int width;
    int height;
} ParrotVideoSceneViewportComponent;

static const ParrotReflectDescription ParrotVideoSceneViewportComponent_description[] = {
    PARROT_REFLECT_TYPE_HEADER(ParrotVideoSceneViewportComponent),

    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneViewportComponent, int, width, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneViewportComponent, int, height, ),

    PARROT_REFLECT_END(),
};

typedef struct {
    bool corner_aligned;

    bool use_clear_color;
    ParrotColor clear_color;
} ParrotVideoSceneCameraComponent;

static const ParrotReflectDescription ParrotVideoSceneCameraComponent_description[] = {
    PARROT_REFLECT_TYPE_HEADER(ParrotVideoSceneCameraComponent),

    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneCameraComponent, bool, corner_aligned, ),

    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneCameraComponent, bool, use_clear_color, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneCameraComponent, ParrotColor, clear_color, ),

    PARROT_REFLECT_END(),
};

typedef struct {
    ParrotReal width;
    ParrotReal height;

    bool texture_nearest_filter;

    int texture_region_x, texture_region_y;
    int texture_region_width, texture_region_height;
    bool texture_use_region;
} ParrotVideoSceneRectComponent;

static const ParrotReflectDescription ParrotVideoSceneRectComponent_description[] = {
    PARROT_REFLECT_TYPE_HEADER(ParrotVideoSceneRectComponent),

    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRectComponent, ParrotReal, width, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRectComponent, ParrotReal, height, ),

    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRectComponent, bool, texture_nearest_filter, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRectComponent, int, texture_region_x, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRectComponent, int, texture_region_y, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRectComponent, int, texture_region_width, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRectComponent, int, texture_region_height, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRectComponent, bool, texture_use_region, ),

    PARROT_REFLECT_END(),
};

typedef struct {
    const uint32_t *rgba8888;
    int width;
    int height;
} ParrotVideoSceneRectRawTextureComponent;

static const ParrotReflectDescription ParrotVideoSceneRectRawTextureComponent_description[] = {
    PARROT_REFLECT_TYPE_HEADER(ParrotVideoSceneRectRawTextureComponent),

    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRectRawTextureComponent, const uint32_t *, rgba8888, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRectRawTextureComponent, int, width, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRectRawTextureComponent, int, height, ),

    PARROT_REFLECT_END(),
};

typedef struct {
    ParrotBinaryImage image;
    /// Set when image has changed
    bool image_dirty;

    uint32_t *_rgba8888;
    int _width;
    int _height;
} ParrotVideoSceneRectTextureComponent;

static const ParrotReflectDescription ParrotVideoSceneRectTextureComponent_description[] = {
    PARROT_REFLECT_TYPE_HEADER(ParrotVideoSceneRectTextureComponent),

    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRectTextureComponent, ParrotBinaryImage, image, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRectTextureComponent, bool, image_dirty, ),

    PARROT_REFLECT_END(),
};

void ParrotVideoSceneSystem_register_components(ParrotSceneWorld *world);
void ParrotVideoSceneSystem_update(ParrotSceneWorld *world, ParrotVideoObjectHandle root_handle);

static const ParrotReflectDescription Parrot_video_scene_collection[] = {
    PARROT_REFLECT_COLLECTION_HEADER(),

    PARROT_REFLECT_COLLECTION_DESCRIPTION(ParrotVideoSceneRenderableComponent_description),
    PARROT_REFLECT_COLLECTION_DESCRIPTION(ParrotVideoSceneWindowComponent_description),
    PARROT_REFLECT_COLLECTION_DESCRIPTION(ParrotVideoSceneViewportComponent_description),
    PARROT_REFLECT_COLLECTION_DESCRIPTION(ParrotVideoSceneCameraComponent_description),
    PARROT_REFLECT_COLLECTION_DESCRIPTION(ParrotVideoSceneRectComponent_description),
    PARROT_REFLECT_COLLECTION_DESCRIPTION(ParrotVideoSceneRectRawTextureComponent_description),
    PARROT_REFLECT_COLLECTION_DESCRIPTION(ParrotVideoSceneRectTextureComponent_description),

    PARROT_REFLECT_END(),
};

#endif // __SRC_PARROT_INCLUDE_PARROT_VIDEO_SCENE_H_
