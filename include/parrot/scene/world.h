#ifndef __SRC_PARROT_ECS_INCLUDE_PARROT_ECS_WORLD_H_
#define __SRC_PARROT_ECS_INCLUDE_PARROT_ECS_WORLD_H_

#include "parrot/core/math.h"
#include "parrot/core/util.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct ParrotScene ParrotScene;

typedef enum {
    ParrotSceneActorEvent_TREE_ENTER = 0,
    ParrotSceneActorEvent_TREE_ENTERED,
    ParrotSceneActorEvent_TREE_EXIT,

    ParrotSceneActorEvent_READY,

    ParrotSceneActorEvent_REPARENT,
    ParrotSceneActorEvent_REPARENTED,

    ParrotSceneActorEvent_ENABLED,
    ParrotSceneActorEvent_DISABLED,

    ParrotSceneActorEvent_TRANSFORM_CHANGE,
    ParrotSceneActorEvent_TRANSFORM_CHANGED,

    ParrotSceneActorEvent_USER_BEGIN = 0x8000,
    /// For making sure the enum is large enough
    ParrotSceneActorEvent_USER_END = 0xFFFF,
} ParrotSceneActorEvent;

typedef struct {
    size_t index;
    size_t generation;
} ParrotSceneActorHandle;

typedef struct {
    size_t size;
    /**
     * Pointer to NULL-terminated list of strings where the current type is defined at the top. Mostly used by other
     * objects.
     */
    const char **type;
    void *user;

    /**
     * Do not use public properties in this function. Use the ready event instead for these purposes.
     *
     * NULL = no handler
     */
    void (*init)(void *self);
    /// NULL = no handler
    void (*event)(void *self, ParrotSceneActorEvent event);
    /// NULL = no handler
    void (*update)(void *self, float delta);
    /// NULL = no handler
    void (*destroy)(void *self);
} ParrotSceneActorProfile;

PARROT_API ParrotScene *ParrotScene_new(void);
PARROT_API void ParrotScene_delete(ParrotScene *self);
PARROT_API void ParrotScene_vdelete(void *self);

PARROT_API void ParrotScene_update(ParrotScene *self, float delta);

PARROT_API void *ParrotScene_actor_spawn(ParrotScene *self, const ParrotSceneActorProfile *profile);
PARROT_API void ParrotScene_actor_kill(void *actor);

PARROT_API ParrotScene *ParrotScene_get_actor_scene(void *actor);
PARROT_API const ParrotSceneActorProfile *ParrotScene_get_actor_profile(void *actor);

PARROT_API ParrotSceneActorHandle ParrotScene_get_actor_handle(void *actor);
PARROT_API void *ParrotScene_get_actor_from_handle_or_null(ParrotScene *self, ParrotSceneActorHandle handle);

PARROT_API void ParrotScene_send_actor_event(void *actor, ParrotSceneActorEvent event);

PARROT_API void ParrotScene_set_actor_parent(void *actor, /* NULL = root */ void *parent_actor);
PARROT_API void *ParrotScene_get_actor_parent(void *actor);
PARROT_API void *ParrotScene_get_actor_children_or_null(ParrotScene *self, /* NULL = root */ void *actor);
PARROT_API void *ParrotScene_get_next_actor_or_null(void *actor);

PARROT_API void ParrotScene_set_actor_active(void *actor, bool active);
PARROT_API bool ParrotScene_is_actor_active(void *actor);

PARROT_API void ParrotScene_set_actor_transform(void *actor, ParrotTransform transform);
PARROT_API ParrotTransform ParrotScene_get_actor_transform(void *actor);
PARROT_API ParrotMat ParrotScene_get_global_actor_matrix(void *actor);
PARROT_API ParrotMat ParrotScene_get_actor_matrix(void *actor);

#endif // __SRC_PARROT_ECS_INCLUDE_PARROT_ECS_WORLD_H_
