#ifndef PARROT_PARROT_INCLUDE_PARROT_UI_UI_H_
#define PARROT_PARROT_INCLUDE_PARROT_UI_UI_H_

#include "parrot/core/api.h"
#include "parrot/core/reflect.h"
#include "parrot/scene/world.h"
#include "parrot/video/font.h"
#include "parrot/video/video.h"

typedef struct {
    ParrotVideoObjectHandle object_handle;

    /// NULL = parent's font or crash if root
    ParrotVideoFont *font;
} ParrotUIComponent;

static const ParrotReflectDescription ParrotUIComponent_description[] = {
    PARROT_REFLECT_TYPE_HEADER(ParrotUIComponent),

    PARROT_REFLECT_TYPE_FIELD(ParrotUIComponent, ParrotVideoObjectHandle, object_handle, ),

    PARROT_REFLECT_TYPE_FIELD(ParrotUIComponent, ParrotVideoFont *, font, ),

    PARROT_REFLECT_END(),
};

typedef struct {
    const char *title;

    int width;
    int height;
    bool resize;

    bool close_requested;
} ParrotUIWindowComponent;

static const ParrotReflectDescription ParrotUIWindowComponent_description[] = {
    PARROT_REFLECT_TYPE_HEADER(ParrotUIWindowComponent),

    PARROT_REFLECT_TYPE_FIELD(ParrotUIWindowComponent, const char *, title, ),

    PARROT_REFLECT_TYPE_FIELD(ParrotUIWindowComponent, int, width, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotUIWindowComponent, int, height, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotUIWindowComponent, bool, resize, ),

    PARROT_REFLECT_TYPE_FIELD(ParrotUIWindowComponent, bool, close_requested, ),

    PARROT_REFLECT_END(),
};

typedef struct ParrotUISystem ParrotUISystem;

PARROT_API ParrotUISystem *ParrotUISystem_new(ParrotSceneWorld *world, ParrotVideoObjectHandle root_handle);
PARROT_API void ParrotUISystem_delete(ParrotUISystem *self);
PARROT_API void ParrotUISystem_vdelete(void *self);

PARROT_API void ParrotUISystem_update(ParrotUISystem *self, ParrotVideoObjectHandle viewport_handle);

static const ParrotReflectDescription Parrot_ui_ui_collection[] = {
    PARROT_REFLECT_COLLECTION_HEADER(),

    PARROT_REFLECT_COLLECTION_DESCRIPTION(ParrotUIComponent_description),
    PARROT_REFLECT_COLLECTION_DESCRIPTION(ParrotUIWindowComponent_description),

    PARROT_REFLECT_END(),
};

#endif // PARROT_PARROT_INCLUDE_PARROT_UI_UI_H_
