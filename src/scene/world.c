#include "parrot/scene/world.h"
#include "parrot/core/util.h"
#include "src/stb_ds.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN_ARR_SIZE_VALUE(arr, size, fill)                                                                             \
    do {                                                                                                                \
        while (arrlen(arr) < size) {                                                                                    \
            arrpush((arr), fill);                                                                                       \
        }                                                                                                               \
    } while (0);

#define MAKE_ENTITY(index, gen) ((ParrotSceneWorldEntity)(((ParrotSceneWorldEntity)(gen)) << 32 | (index)))
#define ENTITY_INDEX(entity) ((uint32_t)((entity) & 0xFFFFFFFF))
#define ENTITY_GEN(entity) ((uint32_t)(((entity) >> 32) & 0xFFFFFFFF))

typedef struct {
    char *key;

    ParrotSceneWorldComponentDescription description;
} ParrotSceneWorldRegisteredComponent;

typedef struct {
    ParrotSceneWorldEntity key;
} ParrotSceneWorldEntityChild;

struct ParrotSceneWorld {
    ParrotSceneWorldRegisteredComponent *sh_registered_components;

    ParrotSceneWorldEntity *arr_entity_parents;
    ParrotSceneWorldEntityChild **arr_hm_entity_children;

    bool *arr_entity_exists;
    uint32_t *arr_entity_gens;

    uint32_t *arr_free_indices;
    uint32_t next_new_index;
};

void ParrotSceneWorldRegisteredComponent_min_size(ParrotSceneWorldRegisteredComponent *self, size_t size) {
    (void)self;
    (void)size;
}

ParrotSceneWorld *ParrotSceneWorld_new(void) {
    ParrotSceneWorld *self = malloc(sizeof(ParrotSceneWorld));
    memset(self, 0, sizeof(ParrotSceneWorld));

    self->next_new_index = 1;

    return self;
}

void ParrotSceneWorld_delete(ParrotSceneWorld *self) {
    PARROT_FAIL_NULL(self);

    for (size_t i = 0; i < arrlen(self->arr_entity_gens); i++) {
        ParrotSceneWorldEntity entity = MAKE_ENTITY(i, self->arr_entity_gens[i]);
        ParrotSceneWorld_delete_entity(self, entity);
    }

    for (size_t i = 0; i < shlen(self->sh_registered_components); i++) {
    }
    shfree(self->sh_registered_components);

    arrfree(self->arr_entity_parents);
    for (size_t i = 0; i < arrlen(self->arr_hm_entity_children); i++) {
        hmfree(self->arr_hm_entity_children[i]);
    }
    arrfree(self->arr_hm_entity_children);

    arrfree(self->arr_entity_exists);
    arrfree(self->arr_entity_gens);

    arrfree(self->arr_free_indices);

    free(self);
}

ParrotSceneWorldEntity ParrotSceneWorld_create_entity(ParrotSceneWorld *self) {
    PARROT_FAIL_NULL(self);

    uint32_t index = self->next_new_index;
    if (arrlen(self->arr_free_indices) > 0) {
        index = arrpop(self->arr_free_indices);
        self->arr_entity_gens[index]++;
    } else {
        self->next_new_index++;
    }

    MIN_ARR_SIZE_VALUE(self->arr_entity_parents, index + 1, ParrotSceneWorldEntity_NULL);
    MIN_ARR_SIZE_VALUE(self->arr_hm_entity_children, index + 1, NULL);

    MIN_ARR_SIZE_VALUE(self->arr_entity_exists, index + 1, false);
    MIN_ARR_SIZE_VALUE(self->arr_entity_gens, index + 1, 0);

    self->arr_entity_exists[index] = true;

    for (size_t i = 0; i < shlen(self->sh_registered_components); i++) {
        ParrotSceneWorldRegisteredComponent_min_size(&self->sh_registered_components[i], index + 1);
    }

    return MAKE_ENTITY(index, self->arr_entity_gens[index]);
}

void ParrotSceneWorld_delete_entity(ParrotSceneWorld *self, ParrotSceneWorldEntity entity) {
    PARROT_RET_COND(!ParrotSceneWorld_does_entity_exist(self, entity));

    for (size_t i = ParrotSceneWorld_get_entity_child_count(self, entity); i > 0; i--) {
        ParrotSceneWorld_delete_entity(self, ParrotSceneWorld_get_entity_child(self, entity, i - 1));
    }

    for (size_t i = 0; i < shlen(self->sh_registered_components); i++) {
        ParrotSceneWorldRegisteredComponent *component = &self->sh_registered_components[i];
        (void)component;
    }

    ParrotSceneWorld_set_entity_parent(self, entity, ParrotSceneWorldEntity_NULL);

    self->arr_entity_exists[ENTITY_INDEX(entity)] = false;
    arrpush(self->arr_free_indices, ENTITY_INDEX(entity));
}

bool ParrotSceneWorld_does_entity_exist(ParrotSceneWorld *self, ParrotSceneWorldEntity entity) {
    PARROT_FAIL_NULL(self);
    PARROT_RET_COND_V(ENTITY_INDEX(entity) >= arrlen(self->arr_entity_exists), false);

    return self->arr_entity_exists[ENTITY_INDEX(entity)] &&
           self->arr_entity_gens[ENTITY_INDEX(entity)] == ENTITY_GEN(entity);
}

void ParrotSceneWorld_set_entity_parent(ParrotSceneWorld *self,
                                        ParrotSceneWorldEntity child,
                                        /* Nullable */ ParrotSceneWorldEntity new_parent) {
    PARROT_RET_COND(!ParrotSceneWorld_does_entity_exist(self, child));

    // TODO: Cycle detection

    ParrotSceneWorldEntity old_parent = ParrotSceneWorld_get_entity_parent(self, child);
    if (ParrotSceneWorld_does_entity_exist(self, old_parent)) {
        hmdel(self->arr_hm_entity_children[ENTITY_INDEX(old_parent)], child);
    }

    if (ParrotSceneWorld_does_entity_exist(self, new_parent)) {
        ParrotSceneWorldEntityChild child_entry = {0};
        child_entry.key = child;
        hmputs(self->arr_hm_entity_children[ENTITY_INDEX(new_parent)], child_entry);
    }

    self->arr_entity_parents[ENTITY_INDEX(child)] = new_parent;
}

ParrotSceneWorldEntity ParrotSceneWorld_get_entity_parent(ParrotSceneWorld *self, ParrotSceneWorldEntity entity) {
    PARROT_RET_COND_V(!ParrotSceneWorld_does_entity_exist(self, entity), ParrotSceneWorldEntity_NULL);

    return self->arr_entity_parents[ENTITY_INDEX(entity)];
}

size_t ParrotSceneWorld_get_entity_child_count(ParrotSceneWorld *self, ParrotSceneWorldEntity entity) {
    PARROT_FAIL_NULL(self);

    if (!ParrotSceneWorld_does_entity_exist(self, entity)) {
        return arrlen(self->arr_entity_gens);
    }

    return hmlen(self->arr_hm_entity_children[ENTITY_INDEX(entity)]);
}

ParrotSceneWorldEntity
ParrotSceneWorld_get_entity_child(ParrotSceneWorld *self, ParrotSceneWorldEntity entity, size_t index) {
    PARROT_FAIL_NULL(self);

    bool parent_exists = ParrotSceneWorld_does_entity_exist(self, entity);
    size_t children_size =
        parent_exists ? hmlen(self->arr_hm_entity_children[ENTITY_INDEX(entity)]) : arrlen(self->arr_entity_gens);

    PARROT_FAIL_COND_MSG(index >= children_size, "Attempted to access out of bounds index in children");

    return parent_exists ? self->arr_hm_entity_children[ENTITY_INDEX(entity)][index].key :
                           MAKE_ENTITY(index, self->arr_entity_gens[index]);
}