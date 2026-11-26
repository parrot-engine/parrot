#ifndef PARROT_PARROT_INCLUDE_PARROT_SCENE_SCENE_H_
#define PARROT_PARROT_INCLUDE_PARROT_SCENE_SCENE_H_

#include "parrot/core/reflect.h"
#include "parrot/core/util.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ParrotSceneEntity_NULL ((ParrotSceneEntity)0)

typedef uint64_t ParrotSceneEntity;
typedef struct ParrotScene ParrotScene;

typedef enum {
    ParrotSceneQueryType_END = 0,

    ParrotSceneQueryType_SET_INVERT,

    ParrotSceneQueryType_PARENT,
    ParrotSceneQueryType_COMPONENT,
} ParrotSceneQueryType;

typedef struct {
    ParrotSceneQueryType type;
    union {
        bool invert;

        struct {
            ParrotSceneEntity entity;
        } parent;

        struct {
            const char *type;
        } component;
    } data;
} ParrotSceneQuery;

#define PARROT_SCENE_QUERY_END()                                                                                        \
    ((ParrotSceneQuery){                                                                                                \
        .type = ParrotSceneQueryType_END,                                                                               \
    })

#define PARROT_SCENE_QUERY_SET_INVERT(p_invert)                                                                         \
    ((ParrotSceneQuery){                                                                                                \
        .type = ParrotSceneQueryType_SET_INVERT,                                                                        \
        .data =                                                                                                         \
            {                                                                                                           \
                .invert = p_invert,                                                                                     \
            },                                                                                                          \
    })

#define PARROT_SCENE_QUERY_WITH_PARENT(p_entity)                                                                        \
    ((ParrotSceneQuery){                                                                                                \
        .type = ParrotSceneQueryType_PARENT,                                                                            \
        .data =                                                                                                         \
            {                                                                                                           \
                .parent =                                                                                               \
                    {                                                                                                   \
                        .entity = p_entity,                                                                             \
                    },                                                                                                  \
            },                                                                                                          \
    })

#define PARROT_SCENE_QUERY_WITH_COMPONENT_type(p_type)                                                                  \
    ((ParrotSceneQuery){                                                                                                \
        .type = ParrotSceneQueryType_COMPONENT,                                                                         \
        .data =                                                                                                         \
            {                                                                                                           \
                .component =                                                                                            \
                    {                                                                                                   \
                        .type = p_type,                                                                                 \
                    },                                                                                                  \
            },                                                                                                          \
    })
#define PARROT_SCENE_QUERY_WITH_COMPONENT(type) PARROT_SCENE_QUERY_WITH_COMPONENT_type(PARROT_TYPE_STRING(type))

typedef struct {
    ParrotScene *scene;
    ParrotSceneEntity entity;
} ParrotSceneFuncParams;

typedef struct {
    size_t size;
    const char *type;

    void (*init_or_null)(void *self, ParrotSceneFuncParams params);
    void (*destroy_or_null)(void *self, ParrotSceneFuncParams params);

    void (*ready_or_null)(void *self, ParrotSceneFuncParams params);

    void (*early_update_or_null)(void *self, ParrotSceneFuncParams params, float delta);
    void (*update_or_null)(void *self, ParrotSceneFuncParams params, float delta);
    void (*late_update_or_null)(void *self, ParrotSceneFuncParams params, float delta);
} ParrotSceneComponentProfile;

PARROT_API ParrotScene *ParrotScene_new(void);
PARROT_API void ParrotScene_delete(ParrotScene *self);
PARROT_API void ParrotScene_vdelete(void *self);

PARROT_API void ParrotScene_update(ParrotScene *self, float delta);
PARROT_API void ParrotScene_delete_queued(ParrotScene *self);

PARROT_API ParrotSceneEntity ParrotScene_create_entity(ParrotScene *self);
PARROT_API void ParrotScene_queue_delete_entity(ParrotScene *self, ParrotSceneEntity entity);
PARROT_API bool ParrotScene_is_entity_deletion_queued(ParrotScene *self, ParrotSceneEntity entity);

PARROT_API bool ParrotScene_does_entity_exist(ParrotScene *self, ParrotSceneEntity entity);

PARROT_API void ParrotScene_set_entity_parent(ParrotScene *self,
                                              ParrotSceneEntity child,
                                              /* Nullable */ ParrotSceneEntity new_parent);
PARROT_API ParrotSceneEntity ParrotScene_get_entity_parent_or_null(ParrotScene *self, ParrotSceneEntity entity);

#define ParrotScene_simple_register_component(self, type)                                                               \
    ParrotScene_register_component(self,                                                                                \
                                   PARROT_TYPE_STRING(type),                                                            \
                                   (ParrotSceneComponentProfile){                                                       \
                                       .size = sizeof(type),                                                            \
                                   })

/// Checks for a ParrotSceneComponentProfile tag on each type with a value as the pointer to the profile
PARROT_API void ParrotScene_auto_register_components(ParrotScene *self, ParrotReflect *reflect);
PARROT_API void ParrotScene_register_component(ParrotScene *self, ParrotSceneComponentProfile profile);
PARROT_API void ParrotScene_unregister_component(ParrotScene *self, const char *type);
PARROT_API bool ParrotScene_is_component_registered(ParrotScene *self, const char *type);

#define ParrotScene_add_component_or_null(self, entity, type)                                                           \
    ((type *)ParrotScene_add_component_or_nullx(self, entity, #type))
#define ParrotScene_get_component_or_null(self, entity, type)                                                           \
    ((type *)ParrotScene_get_component_or_nullx(self, entity, #type))
#define ParrotScene_is_component_deletion_queued(self, entity, type)                                                    \
    ParrotScene_is_component_deletion_queuedx(self, entity, PARROT_TYPE_STRING(type))
#define ParrotScene_queue_delete_component(self, entity, type)                                                          \
    ParrotScene_queue_delete_componentx(self, entity, PARROT_TYPE_STRING(type))

PARROT_API void *ParrotScene_add_component_or_nullx(ParrotScene *self, ParrotSceneEntity entity, const char *type);
PARROT_API void *ParrotScene_get_component_or_nullx(ParrotScene *self, ParrotSceneEntity entity, const char *type);
PARROT_API bool ParrotScene_is_component_deletion_queuedx(ParrotScene *self, ParrotSceneEntity entity, const char *type);
PARROT_API void ParrotScene_queue_delete_componentx(ParrotScene *self, ParrotSceneEntity entity, const char *type);

PARROT_API void ParrotScene_query(ParrotScene *self, const ParrotSceneQuery *query, ParrotSceneEntity **p_arr_results);

/// Auto-registered at scene creation
PARROT_API extern const ParrotSceneComponentProfile ParrotScene_ParrotMat_profile;
/// Auto-registered at scene creation
PARROT_API extern const ParrotSceneComponentProfile ParrotScene_ParrotTransform_profile;

#endif // PARROT_PARROT_INCLUDE_PARROT_SCENE_SCENE_H_
