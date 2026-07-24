#ifndef __SRC_PARROT_INCLUDE_PARROT_SCENE_WORLD_VIEW_H_
#define __SRC_PARROT_INCLUDE_PARROT_SCENE_WORLD_VIEW_H_

#include "parrot/core/api.h"
#include "parrot/scene/world.h"

typedef ParrotSceneWorldEntity ParrotSceneWorldView;

PARROT_API ParrotSceneWorldView *ParrotSceneWorldView_new(ParrotSceneWorld *world);
PARROT_API ParrotSceneWorldView *ParrotSceneWorldView_copy(const ParrotSceneWorldView *self);
PARROT_API ParrotSceneWorldView *ParrotSceneWorldView_new_with_inactive(ParrotSceneWorld *world);
PARROT_API void ParrotSceneWorldView_delete(ParrotSceneWorldView *self);

PARROT_API void ParrotSceneWorldView_with_parent(ParrotSceneWorldView *self, ParrotSceneWorldEntity parent, bool tree);
PARROT_API void
ParrotSceneWorldView_without_parent(ParrotSceneWorldView *self, ParrotSceneWorldEntity parent, bool tree);

#define ParrotSceneWorldView_with_component(self, type, tree)                                                           \
    ParrotSceneWorldView_with_component_name(self, (const char *)(const type *)#type, tree)
PARROT_API void ParrotSceneWorldView_with_component_name(ParrotSceneWorldView *self, const char *name, bool tree);

#define ParrotSceneWorldView_without_component(self, type, tree)                                                        \
    ParrotSceneWorldView_without_component_name(self, (const char *)(const type *)#type, tree)
PARROT_API void ParrotSceneWorldView_without_component_name(ParrotSceneWorldView *self, const char *name, bool tree);

#endif // __SRC_PARROT_INCLUDE_PARROT_SCENE_WORLD_VIEW_H_