#ifndef __SRC_PARROT_INCLUDE_PARROT_VIDEO_SCENE_H_
#define __SRC_PARROT_INCLUDE_PARROT_VIDEO_SCENE_H_

#include "parrot/core/math.h"
#include "parrot/core/reflect.h"
#include "parrot/core/scope.h"
#include "parrot/core/util.h"
#include "parrot/drivers/video_driver.h"
#include "parrot/drivers/window_driver.h"
#include "parrot/scene/scene.h"
#include <stdbool.h>

typedef enum {
    ParrotVideoSceneRenderStage_PRE = 0,
    ParrotVideoSceneRenderStage_RENDER,
    ParrotVideoSceneRenderStage_POST,
} ParrotVideoSceneRenderStage;

typedef void (*ParrotVideoSceneRenderFunc)(ParrotScope *scope);

typedef struct {
    bool visible;
} ParrotVideoSceneRenderableComponent;

PARROT_API extern const ParrotSceneComponentProfile ParrotVideoSceneRenderableComponent_profile;

static const ParrotReflectDescription ParrotVideoSceneRenderableComponent_description[] = {
    PARROT_REFLECT_TYPE_HEADER_TAG(
        ParrotVideoSceneRenderableComponent,
        ((ParrotReflectDescriptionTag[]){
            {PARROT_TYPE_STRING(ParrotSceneComponentProfile), &ParrotVideoSceneRenderableComponent_profile},
            {NULL, NULL},
        })),

    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRenderableComponent, bool, visible, ),

    PARROT_REFLECT_END(),
};

PARROT_API void ParrotVideoSceneRenderableComponent_add(ParrotVideoSceneRenderableComponent *self,
                                                        ParrotScope *scope,
                                                        ParrotVideoSceneRenderStage stage,
                                                        ParrotVideoSceneRenderFunc render);
PARROT_API void ParrotVideoSceneRenderableComponent_render(ParrotScene *scene, ParrotSceneEntity entity);

typedef struct {
    bool corner_aligned;

    bool use_clear_color;
    ParrotColor clear_color;

    ParrotMat last_view_matrix;
    ParrotMat last_projection_matrix;
} ParrotVideoSceneCameraComponent;

PARROT_API extern const ParrotSceneComponentProfile ParrotVideoSceneCameraComponent_profile;

static const ParrotReflectDescription ParrotVideoSceneCameraComponent_description[] = {
    PARROT_REFLECT_TYPE_HEADER_TAG(
        ParrotVideoSceneCameraComponent,
        ((ParrotReflectDescriptionTag[]){
            {PARROT_TYPE_STRING(ParrotSceneComponentProfile), &ParrotVideoSceneCameraComponent_profile},
            {NULL, NULL},
        })),

    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneCameraComponent, bool, corner_aligned, ),

    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneCameraComponent, bool, use_clear_color, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneCameraComponent, ParrotColor, clear_color, ),

    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneCameraComponent, ParrotMat, last_view_matrix, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneCameraComponent, ParrotMat, last_projection_matrix, ),

    PARROT_REFLECT_END(),
};

PARROT_API ParrotSceneEntity ParrotVideoSceneCameraComponent_find_or_null(ParrotScene *scene, ParrotSceneEntity entity);

typedef struct {
    const char *title;

    int width;
    int height;
    bool resize;

    bool close_requested;

    ParrotWindowDriver *driver;
    ParrotWindowDriverWindow *window;
} ParrotVideoSceneRenderableWindowComponent;

PARROT_API extern const ParrotSceneComponentProfile ParrotVideoSceneRenderableWindowComponent_profile;

static const ParrotReflectDescription ParrotVideoSceneRenderableWindowComponent_description[] = {
    PARROT_REFLECT_TYPE_HEADER_TAG(
        ParrotVideoSceneRenderableWindowComponent,
        ((ParrotReflectDescriptionTag[]){
            {PARROT_TYPE_STRING(ParrotSceneComponentProfile), &ParrotVideoSceneRenderableWindowComponent_profile},
            {NULL, NULL},
        })),

    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRenderableWindowComponent, const char *, title, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRenderableWindowComponent, bool, close_requested, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRenderableWindowComponent, bool, resize, ),

    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRenderableWindowComponent, ParrotVideoDriver *, driver, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRenderableWindowComponent, ParrotWindowDriverWindow *, window, ),

    PARROT_REFLECT_END(),
};

typedef struct {
    int width;
    int height;

    ParrotVideoDriver *driver;
    ParrotVideoDriverViewport *viewport;

    ParrotSceneEntity last_camera_entity;
    ParrotVideoSceneCameraComponent *last_camera;
} ParrotVideoSceneRenderableViewportComponent;

PARROT_API extern const ParrotSceneComponentProfile ParrotVideoSceneRenderableViewportComponent_profile;

static const ParrotReflectDescription ParrotVideoSceneRenderableViewportComponent_description[] = {
    PARROT_REFLECT_TYPE_HEADER_TAG(
        ParrotVideoSceneRenderableViewportComponent,
        ((ParrotReflectDescriptionTag[]){
            {PARROT_TYPE_STRING(ParrotSceneComponentProfile), &ParrotVideoSceneRenderableViewportComponent_profile},
            {NULL, NULL},
        })),

    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRenderableViewportComponent, int, width, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRenderableViewportComponent, int, height, ),

    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRenderableViewportComponent, ParrotVideoDriver *, driver, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRenderableViewportComponent, ParrotVideoDriverViewport *, viewport, ),

    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRenderableViewportComponent, ParrotSceneEntity, last_camera_entity, ),
    PARROT_REFLECT_TYPE_FIELD(
        ParrotVideoSceneRenderableViewportComponent, ParrotVideoSceneCameraComponent *, last_camera, ),

    PARROT_REFLECT_END(),
};

PARROT_API ParrotSceneEntity ParrotVideoSceneRenderableViewportComponent_find_or_null(ParrotScene *scene,
                                                                                      ParrotSceneEntity entity);

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
} ParrotVideoSceneRenderable2DComponent;

PARROT_API extern const ParrotSceneComponentProfile ParrotVideoSceneRenderable2DComponent_profile;

static const ParrotReflectDescription ParrotVideoSceneRenderable2DComponent_description[] = {
    PARROT_REFLECT_TYPE_HEADER_TAG(
        ParrotVideoSceneRenderable2DComponent,
        ((ParrotReflectDescriptionTag[]){
            {PARROT_TYPE_STRING(ParrotSceneComponentProfile), &ParrotVideoSceneRenderable2DComponent_profile},
            {NULL, NULL},
        })),

    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRenderable2DComponent, ParrotReal, width, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRenderable2DComponent, ParrotReal, height, ),

    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRenderable2DComponent, int, texture_x, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRenderable2DComponent, int, texture_y, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotVideoSceneRenderable2DComponent, bool, texture_nearest_filter, ),

    PARROT_REFLECT_END(),
};

static const ParrotReflectDescription Parrot_video_scene_collection[] = {
    PARROT_REFLECT_COLLECTION_HEADER(),

    PARROT_REFLECT_COLLECTION_DESCRIPTION(ParrotVideoSceneRenderableComponent_description),

    PARROT_REFLECT_COLLECTION_DESCRIPTION(ParrotVideoSceneCameraComponent_description),

    PARROT_REFLECT_COLLECTION_DESCRIPTION(ParrotVideoSceneRenderableWindowComponent_description),
    PARROT_REFLECT_COLLECTION_DESCRIPTION(ParrotVideoSceneRenderableViewportComponent_description),

    PARROT_REFLECT_COLLECTION_DESCRIPTION(ParrotVideoSceneRenderable2DComponent_description),

    PARROT_REFLECT_END(),
};

#endif // __SRC_PARROT_INCLUDE_PARROT_VIDEO_SCENE_H_
