#ifndef __SRC_PARROT_INCLUDE_PARROT_SCENE_TRANSFORM_H_
#define __SRC_PARROT_INCLUDE_PARROT_SCENE_TRANSFORM_H_

#include "parrot/core/math.h"
#include "parrot/scene/world.h"

void ParrotMat_scene_register(ParrotSceneWorld *world);

ParrotMat ParrotMat_get_entity_global_matrix(ParrotSceneWorld *world, ParrotSceneWorldEntity entity);

#endif // __SRC_PARROT_INCLUDE_PARROT_SCENE_TRANSFORM_H_