#ifndef __SRC_PARROT_ECS_INCLUDE_PARROT_ECS_WORLD_H_
#define __SRC_PARROT_ECS_INCLUDE_PARROT_ECS_WORLD_H_

#include "parrot/core/api.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ParrotSceneWorldEntity_NULL ((ParrotSceneWorldEntity)0)

typedef uint64_t ParrotSceneWorldEntity;
typedef struct ParrotSceneWorld ParrotSceneWorld;

typedef void (*ParrotSceneWorldComponentConstructor)(void *component, void *user_data);
typedef void (*ParrotSceneWorldComponentDestructor)(void *component, void *user_data);

typedef struct {
    size_t size;

    ParrotSceneWorldComponentConstructor constructor;
    ParrotSceneWorldComponentDestructor destructor;
    void *user_data;
} ParrotSceneWorldComponentDescription;

PARROT_API ParrotSceneWorld *ParrotSceneWorld_new(void);
PARROT_API void ParrotSceneWorld_delete(ParrotSceneWorld *self);

PARROT_API ParrotSceneWorldEntity ParrotSceneWorld_create_entity(ParrotSceneWorld *self);
PARROT_API void ParrotSceneWorld_delete_entity(ParrotSceneWorld *self, ParrotSceneWorldEntity entity);

PARROT_API bool ParrotSceneWorld_does_entity_exist(ParrotSceneWorld *self, ParrotSceneWorldEntity entity);

PARROT_API void ParrotSceneWorld_set_entity_parent(ParrotSceneWorld *self,
                                                   ParrotSceneWorldEntity child,
                                                   /* Nullable */ ParrotSceneWorldEntity new_parent);
PARROT_API ParrotSceneWorldEntity ParrotSceneWorld_get_entity_parent(ParrotSceneWorld *self,
                                                                     ParrotSceneWorldEntity entity);
PARROT_API size_t ParrotSceneWorld_get_entity_child_count(ParrotSceneWorld *self,
                                                          /* NULL = list all */ ParrotSceneWorldEntity entity);
PARROT_API ParrotSceneWorldEntity ParrotSceneWorld_get_entity_child(ParrotSceneWorld *self,
                                                                    /* NULL = list all */ ParrotSceneWorldEntity entity,
                                                                    size_t index);

#endif // __SRC_PARROT_ECS_INCLUDE_PARROT_ECS_WORLD_H_