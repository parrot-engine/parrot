#ifndef __SRC_PARROT_ECS_INCLUDE_PARROT_ECS_WORLD_H_
#define __SRC_PARROT_ECS_INCLUDE_PARROT_ECS_WORLD_H_

#include "parrot/core/api.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ParrotSceneWorldEntity_NULL ((ParrotSceneWorldEntity)0)

typedef struct {
    char pad;
} ParrotSceneWorldInactiveTag;

typedef uint64_t ParrotSceneWorldEntity;
typedef struct ParrotSceneWorld ParrotSceneWorld;

typedef void (*ParrotSceneWorldComponentConstructor)(void *component, void *user_data);
typedef void (*ParrotSceneWorldComponentDestructor)(void *component, void *user_data);

typedef struct {
    char *name;
    size_t size;

    ParrotSceneWorldComponentConstructor constructor;
    ParrotSceneWorldComponentDestructor destructor;
    void *user_data;
} ParrotSceneWorldComponentRegisterInfo;

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

#define ParrotSceneWorld_register_component(self, type)                                                                 \
    ParrotSceneWorld_register_component_manual(self,                                                                    \
                                               ((ParrotSceneWorldComponentRegisterInfo){                                \
                                                   .name = #type,                                                       \
                                                   .size = sizeof(type),                                                \
                                               }))
PARROT_API void ParrotSceneWorld_register_component_manual(ParrotSceneWorld *self,
                                                           ParrotSceneWorldComponentRegisterInfo register_info);
PARROT_API bool ParrotSceneWorld_is_component_registered(ParrotSceneWorld *self, const char *name);

#define ParrotSceneWorld_add_entity_component(self, entity, type)                                                       \
    do {                                                                                                                \
        if (!ParrotSceneWorld_is_component_registered(self, #type)) {                                                   \
            ParrotSceneWorld_register_component_manual(self,                                                            \
                                                       ((ParrotSceneWorldComponentRegisterInfo){                        \
                                                           .name = #type,                                               \
                                                           .size = sizeof(type),                                        \
                                                       }));                                                             \
        }                                                                                                               \
        ParrotSceneWorld_add_entity_component_raw(self, entity, #type)                                                  \
    } while (0)
PARROT_API void
ParrotSceneWorld_add_entity_component_name(ParrotSceneWorld *self, ParrotSceneWorldEntity entity, const char *name);

#define ParrotSceneWorld_get_entity_component(self, entity, type)                                                       \
    (ParrotSceneWorld_is_component_registered(self, #type) ?                                                            \
         (type *)ParrotSceneWorld_get_entity_component_raw(self, entity, #type) :                                       \
         NULL)
PARROT_API void *
ParrotSceneWorld_get_entity_component_name(ParrotSceneWorld *self, ParrotSceneWorldEntity entity, const char *name);

#define ParrotSceneWorld_delete_entity_component(self, entity, type)                                                    \
    do {                                                                                                                \
        if (ParrotSceneWorld_is_component_registed(self, #type)) {                                                      \
            ParrotSceneWorld_delete_entity_component_raw(self, entity, (const char *)(type *)#type);                    \
        }                                                                                                               \
    } while (0)
PARROT_API void
ParrotSceneWorld_delete_entity_component_name(ParrotSceneWorld *self, ParrotSceneWorldEntity entity, const char *name);

#endif // __SRC_PARROT_ECS_INCLUDE_PARROT_ECS_WORLD_H_