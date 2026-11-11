#include "parrot/scene/world.h"
#include "parrot/core/array.h"
#include "parrot/core/math.h"
#include "parrot/core/scope.h"
#include "parrot/core/util.h"
#include <stddef.h>
#include <stdio.h>

typedef struct ParrotSceneActorHeader {
    ParrotScope *scope;

    ParrotScene *scene;
    size_t index;

    struct ParrotSceneActorHeader *prev;
    struct ParrotSceneActorHeader *next;

    struct ParrotSceneActorHeader *parent;
    struct ParrotSceneActorHeader *child;

    const ParrotSceneActorProfile *profile;

    bool entered_tree;
    bool ready;
    bool kill;

    bool active;

    ParrotTransform transform;

    ParrotMat matrix;
    bool recompute_matrix;
} ParrotSceneActorHeader;

typedef struct {
    ParrotSceneActorHeader *actor;
    size_t generation;
} ParrotSceneActorEntry;

struct ParrotScene {
    ParrotScope *scope;

    ParrotSceneActorHeader *root_actors;
    ParrotSceneActorEntry *arr_actors;
};

ParrotScene *ParrotScene_new(void) {
    ParrotScene *self = PARROT_ALLOC(ParrotScene);

    self->scope = ParrotScope_new(NULL);
    ParrotScope_push_free(self->scope, self);

    ParrotScope_push_arrfree(self->scope, self->arr_actors);

    return self;
}

void ParrotScene_delete(ParrotScene *self) {
    PARROT_FAIL_NULL(self);

    ParrotScope_delete(self->scope);
}

void ParrotScene_vdelete(void *self) {
    ParrotScene_delete((ParrotScene *)self);
}

static void isolate_actor(ParrotSceneActorHeader *header) {
    ParrotScene *self = header->scene;

    if (!header->prev) {
        if (header->parent) {
            header->parent->child = header->next;
        }

        if (header == self->root_actors) {
            self->root_actors = header->next;
        }
    } else {
        header->prev->next = header->next;
    }

    if (header->next) {
        header->next->prev = header->prev;
    }

    header->prev = NULL;
    header->next = NULL;
}

static void prioritize_actor(ParrotSceneActorHeader *header) {
    ParrotScene *self = header->scene;

    isolate_actor(header);

    if (self->root_actors) {
        self->root_actors->prev = header;
    }

    header->next = self->root_actors;
    self->root_actors = header;
}

static void update_actor(ParrotSceneActorHeader *header, float delta) {
    ParrotScene *self = header->scene;

    if (header->kill) {
        isolate_actor(header);
        self->arr_actors[header->index].actor = NULL;

        ParrotScope_delete(header->scope);
        return;
    }

    if (!header->ready) {
        ParrotScene_set_actor_parent(PARROT_META_USER(sizeof(ParrotSceneActorHeader), header),
                                     PARROT_META_USER(sizeof(ParrotSceneActorHeader), header->parent));
        ParrotScene_send_actor_event(PARROT_META_USER(sizeof(ParrotSceneActorHeader), header),
                                     ParrotSceneActorEvent_READY);

        header->ready = true;
    }

    PARROT_RET_COND(!header->active);

    if (header->profile->update) {
        header->profile->update(PARROT_META_USER(sizeof(ParrotSceneActorHeader), header), delta);
    }

    for (ParrotSceneActorHeader *child = header->child; child; child = child->next) {
        update_actor(child, delta);
    }
}

void ParrotScene_update(ParrotScene *self, float delta) {
    PARROT_FAIL_NULL(self);

    for (ParrotSceneActorHeader *actor = self->root_actors; actor != NULL; actor = actor->next) {
        update_actor(actor, delta);
    }
}

void *ParrotScene_actor_spawn(ParrotScene *self, const ParrotSceneActorProfile *profile) {
    PARROT_FAIL_NULL(self);
    PARROT_FAIL_NULL(profile);

    ParrotSceneActorHeader *header = malloc(sizeof(ParrotSceneActorHeader) + profile->size);
    memset(header, 0, sizeof(ParrotSceneActorHeader) + profile->size);

    header->scope = ParrotScope_new(self->scope);
    ParrotScope_push_free(header->scope, header);

    header->scene = self;
    {

        for (header->index = 0; header->index < ParrotArray_size(self->arr_actors); header->index++) {
            if (!self->arr_actors[header->index].actor) {
                self->arr_actors[header->index].generation++;
                break;
            }
        }

        if (header->index == ParrotArray_size(self->arr_actors)) {
            ParrotArray_push(self->arr_actors,
                             ((ParrotSceneActorEntry){
                                 .actor = header,
                             }));
        }
    }

    // Temporary insertion to allow readying
    prioritize_actor(header);

    header->profile = profile;

    header->active = true;

    header->transform = ParrotTransform_new();

    header->recompute_matrix = true;

    void *actor = PARROT_META_USER(sizeof(ParrotSceneActorHeader), header);

    if (profile->init) {
        profile->init(actor);
    }

    if (profile->destroy) {
        ParrotScope_push(header->scope, profile->destroy, actor);
    }

    return actor;
}

void ParrotScene_actor_kill(void *actor) {
    PARROT_FAIL_NULL(actor);

    ParrotSceneActorHeader *header = PARROT_META(sizeof(ParrotSceneActorHeader), actor);

    PARROT_RET_COND(header->kill);

    for (ParrotSceneActorHeader *child = header->child; child; child = child->next) {
        ParrotScene_actor_kill(PARROT_META_USER(sizeof(ParrotSceneActorHeader), child));
    }

    ParrotScene_send_actor_event(actor, ParrotSceneActorEvent_TREE_EXIT);

    header->kill = true;
    prioritize_actor(header);
}

ParrotScene *ParrotScene_get_actor_scene(void *actor) {
    PARROT_FAIL_NULL(actor);

    return ((ParrotSceneActorHeader *)PARROT_META(sizeof(ParrotSceneActorHeader), actor))->scene;
}

const ParrotSceneActorProfile *ParrotScene_get_actor_profile(void *actor) {
    PARROT_FAIL_NULL(actor);

    return ((ParrotSceneActorHeader *)PARROT_META(sizeof(ParrotSceneActorHeader), actor))->profile;
}

ParrotSceneActorHandle ParrotScene_get_actor_handle(void *actor) {
    PARROT_FAIL_NULL(actor);

    ParrotSceneActorHeader *header = PARROT_META(sizeof(ParrotSceneActorHeader), actor);
    ParrotScene *self = header->scene;

    return (ParrotSceneActorHandle){
        .index = header->index,
        .generation = self->arr_actors[header->index].generation,
    };
}

void *ParrotScene_get_actor_from_handle_or_null(ParrotScene *self, ParrotSceneActorHandle handle) {
    PARROT_FAIL_NULL(self);

    PARROT_RET_COND_V(handle.index >= ParrotArray_size(self->arr_actors), NULL);
    PARROT_RET_COND_V(handle.index != self->arr_actors[handle.index].generation, NULL);

    return PARROT_META_USER(sizeof(ParrotSceneActorHeader), self->arr_actors[handle.index].actor);
}

void ParrotScene_send_actor_event(void *actor, ParrotSceneActorEvent event) {
    PARROT_FAIL_NULL(actor);

    ParrotSceneActorHeader *header = PARROT_META(sizeof(ParrotSceneActorHeader), actor);

    if (header->profile->event) {
        header->profile->event(actor, event);
    }
}

void ParrotScene_set_actor_parent(void *actor, void *parent_actor) {
    PARROT_FAIL_NULL(actor);

    ParrotSceneActorHeader *header = PARROT_META(sizeof(ParrotSceneActorHeader), actor);
    ParrotSceneActorHeader *parent_header =
        parent_actor ? PARROT_META(sizeof(ParrotSceneActorHeader), parent_actor) : NULL;

    ParrotScene *self = header->scene;

    ParrotScene_send_actor_event(
        actor, !header->entered_tree ? ParrotSceneActorEvent_TREE_ENTER : ParrotSceneActorEvent_TRANSFORM_CHANGE);

    isolate_actor(header);

    header->parent = parent_header;
    header->prev = NULL;
    header->next = NULL;

    ParrotSceneActorHeader *last_actor;

    if (parent_header) {
        if (!parent_header->child) {
            parent_header->child = header;
            goto finish;
        }

        last_actor = parent_header->child;
    } else {
        if (!self->root_actors) {
            self->root_actors = header;
            goto finish;
        }

        last_actor = self->root_actors;
    }

    while (last_actor->next) {
        last_actor = last_actor->next;
    }

    last_actor->next = header;
    header->prev = last_actor;

finish:
    ParrotScene_send_actor_event(
        actor, !header->entered_tree ? ParrotSceneActorEvent_TREE_ENTERED : ParrotSceneActorEvent_TRANSFORM_CHANGED);
    header->entered_tree = true;
}

void *ParrotScene_get_actor_parent(void *actor) {
    PARROT_FAIL_NULL(actor);

    ParrotSceneActorHeader *header = PARROT_META(sizeof(ParrotSceneActorHeader), actor);
    return header->parent ? PARROT_META_USER(sizeof(ParrotSceneActorHeader), header->parent) : NULL;
}

void *ParrotScene_get_actor_children_or_null(ParrotScene *self, void *actor) {
    PARROT_RET_COND_V(!actor, self->root_actors);

    ParrotSceneActorHeader *header = PARROT_META(sizeof(ParrotSceneActorHeader), actor);
    return header->child ? PARROT_META_USER(sizeof(ParrotSceneActorHeader), header->child) : NULL;
}

void ParrotScene_set_actor_active(void *actor, bool active) {
    PARROT_FAIL_NULL(actor);

    ParrotSceneActorHeader *header = PARROT_META(sizeof(ParrotSceneActorHeader), actor);

    header->active = active;

    ParrotScene_send_actor_event(actor, active ? ParrotSceneActorEvent_ENABLED : ParrotSceneActorEvent_DISABLED);
}

void *ParrotScene_get_next_actor_or_null(void *actor) {
    PARROT_FAIL_NULL(actor);

    ParrotSceneActorHeader *header = PARROT_META(sizeof(ParrotSceneActorHeader), actor);

    return header->next ? PARROT_META_USER(sizeof(ParrotSceneActorHeader), header->next) : NULL;
}

bool ParrotScene_is_actor_active(void *actor) {
    PARROT_FAIL_NULL(actor);

    return ((ParrotSceneActorHeader *)PARROT_META(sizeof(ParrotSceneActorHeader), actor))->active;
}

void ParrotScene_set_actor_transform(void *actor, ParrotTransform transform) {
    PARROT_FAIL_NULL(actor);

    ParrotSceneActorHeader *header = PARROT_META(sizeof(ParrotSceneActorHeader), actor);

    ParrotScene_send_actor_event(actor, ParrotSceneActorEvent_TRANSFORM_CHANGE);

    header->transform = transform;
    header->recompute_matrix = true;

    ParrotScene_send_actor_event(actor, ParrotSceneActorEvent_TRANSFORM_CHANGED);
}

ParrotTransform ParrotScene_get_actor_transform(void *actor) {
    PARROT_FAIL_NULL(actor);

    return ((ParrotSceneActorHeader *)PARROT_META(sizeof(ParrotSceneActorHeader), actor))->transform;
}

ParrotMat ParrotScene_get_global_actor_matrix(void *actor) {
    PARROT_FAIL_NULL(actor);

    void *parent_actor = ParrotScene_get_actor_parent(actor);
    ParrotMat parent_matrix = parent_actor ? ParrotScene_get_global_actor_matrix(parent_actor) : ParrotMat_identity();
    return ParrotMat_mul(ParrotScene_get_actor_matrix(actor), parent_matrix);
}

ParrotMat ParrotScene_get_actor_matrix(void *actor) {
    PARROT_FAIL_NULL(actor);

    ParrotSceneActorHeader *header = PARROT_META(sizeof(ParrotSceneActorHeader), actor);

    if (header->recompute_matrix) {
        header->matrix = ParrotTransform_calculate_matrix(&header->transform);
    }

    return header->matrix;
}
