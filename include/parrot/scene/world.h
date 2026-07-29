#ifndef __SRC_PARROT_ECS_INCLUDE_PARROT_ECS_WORLD_H_
#define __SRC_PARROT_ECS_INCLUDE_PARROT_ECS_WORLD_H_

#include "parrot/core/api.h"
#include "parrot/core/util.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ParrotSceneWorldEntity_NULL ((ParrotSceneWorldEntity)0)

typedef uint64_t ParrotSceneWorldEntity;
typedef struct ParrotSceneWorld ParrotSceneWorld;

typedef enum {
    ParrotSceneWorldQueryType_END = 0,

    ParrotSceneWorldQueryType_COMPONENT,
} ParrotSceneWorldQueryType;

typedef union {
    struct {
        const char *name;
    } component;
} ParrotSceneWorldQueryData;

typedef struct {
    ParrotSceneWorldQueryType type;
    ParrotSceneWorldQueryData data;

    bool invert;
} ParrotSceneWorldQuery;

#define PARROT_SCENE_WORLD_QUERY_END()                                                                                  \
    ((ParrotSceneWorldQuery){                                                                                           \
        .type = ParrotSceneWorldQueryType_END,                                                                          \
    })

#define PARROT_SCENE_WORLD_QUERY_WITH_COMPONENT_NAME(p_name)                                                            \
    ((ParrotSceneWorldQuery){                                                                                           \
        .type = ParrotSceneWorldQueryType_COMPONENT,                                                                    \
        .data.component.name = p_name,                                                                                  \
    })
#define PARROT_SCENE_WORLD_QUERY_WITH_COMPONENT(type)                                                                   \
    PARROT_SCENE_WORLD_QUERY_WITH_COMPONENT_NAME(PARROT_TYPE_STRING(type))

#define PARROT_SCENE_WORLD_QUERY_WITHOUT_COMPONENT_NAME(p_name)                                                         \
    ((ParrotSceneWorldQuery){                                                                                           \
        .type = ParrotSceneWorldQueryType_COMPONENT,                                                                    \
        .data.component.name = p_name,                                                                                  \
        .invert = true,                                                                                                 \
    })
#define PARROT_SCENE_WORLD_QUERY_WITHOUT_COMPONENT(type)                                                                \
    PARROT_SCENE_WORLD_QUERY_WITHOUT_COMPONENT_NAME(PARROT_TYPE_STRING(type))

typedef void (*ParrotSceneWorldComponentConstructor)(ParrotSceneWorldEntity entity, void *component, void *user_data);
typedef void (*ParrotSceneWorldComponentDestructor)(ParrotSceneWorldEntity entity, void *component, void *user_data);

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

#define ParrotSceneWorld_simple_register_component(self, type)                                                          \
    ParrotSceneWorld_register_component(self,                                                                           \
                                        PARROT_TYPE_STRING(type),                                                       \
                                        (ParrotSceneWorldComponentDescription){                                         \
                                            .size = sizeof(type),                                                       \
                                        })

PARROT_API void ParrotSceneWorld_register_component(ParrotSceneWorld *self,
                                                    const char *name,
                                                    ParrotSceneWorldComponentDescription description);
PARROT_API void ParrotSceneWorld_unregister_component(ParrotSceneWorld *self, const char *name);
PARROT_API bool ParrotSceneWorld_is_component_registered(ParrotSceneWorld *self, const char *name);

#define ParrotSceneWorld_add_component(self, entity, type)                                                              \
    do {                                                                                                                \
        if (!ParrotSceneWorld_is_component_registered(self, #type)) {                                                   \
            ParrotSceneWorld_simple_register_component(self, type);                                                     \
        }                                                                                                               \
        ParrotSceneWorld_add_component_name(self, entity, #type);                                                       \
    } while (0)

// WARNING: The pointer returned from this function-style macro is unstable. Do not store long-term
#define ParrotSceneWorld_get_component(self, entity, type)                                                              \
    ((type *)(ParrotSceneWorld_is_component_registered(self, #type) ?                                                   \
                  ParrotSceneWorld_get_component_name(self, entity, #type) :                                            \
                  NULL))

#define ParrotSceneWorld_delete_component(self, entity, type)                                                           \
    do {                                                                                                                \
        if (ParrotSceneWorld_is_component_registered(self, PARROT_TYPE_STRING(type))) {                                 \
            ParrotSceneWorld_delete_component_name(self, entity, PARROT_TYPE_STRING(type));                             \
        }                                                                                                               \
    } while (0)

PARROT_API void
ParrotSceneWorld_add_component_name(ParrotSceneWorld *self, ParrotSceneWorldEntity entity, const char *name);
// WARNING: The pointer returned from this function is unstable. Do not store long-term
PARROT_API void *
ParrotSceneWorld_get_component_name(ParrotSceneWorld *self, ParrotSceneWorldEntity entity, const char *name);
PARROT_API void
ParrotSceneWorld_delete_component_name(ParrotSceneWorld *self, ParrotSceneWorldEntity entity, const char *name);

PARROT_API size_t ParrotSceneWorld_query_result_count(ParrotSceneWorld *self, const ParrotSceneWorldQuery *query);
PARROT_API ParrotSceneWorldEntity ParrotSceneWorld_query_result_at(ParrotSceneWorld *self,
                                                                   const ParrotSceneWorldQuery *query,
                                                                   size_t idx);

#endif // __SRC_PARROT_ECS_INCLUDE_PARROT_ECS_WORLD_H_