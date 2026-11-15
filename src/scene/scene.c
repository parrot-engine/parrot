#include "parrot/scene/scene.h"
#include "parrot/core/array.h"
#include "parrot/core/hash.h"
#include "parrot/core/math.h"
#include "parrot/core/reflect.h"
#include "parrot/core/scope.h"
#include "parrot/core/util.h"
#include "src/ds.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COMPONENT_DATA_BLOCK_SIZE (256)

#define LIFE_FLAG_EXISTS (1 << 0)
#define LIFE_FLAG_DELETION_QUEUED (1 << 1)
#define LIFE_FLAG_READY (1 << 2)

#define MIN_ARR_SIZE_VALUE(arr, size, fill)                                                                             \
    do {                                                                                                                \
        while (ParrotArray_size(arr) < size) {                                                                          \
            ParrotArray_push((arr), fill);                                                                              \
        }                                                                                                               \
    } while (0);

#define MAKE_ENTITY(index, gen) ((ParrotSceneEntity)(((ParrotSceneEntity)(gen)) << 32 | (index)))
#define ENTITY_INDEX(entity) ((uint32_t)((entity) & 0xFFFFFFFF))
#define ENTITY_GEN(entity) ((uint32_t)(((entity) >> 32)))

typedef struct {
    ParrotScope *scope;

    char *key;

    ParrotSceneComponentProfile profile;

    uint8_t ***p_arr_data_blocks;
    uint8_t **p_arr_flags;
} ParrotSceneRegisteredComponent;

typedef struct {
    ParrotScope *scope;

    size_t *arr_entity_indices;

    uint64_t component_gen;
    uint64_t tree_gen;
} ParrotSceneCachedQuery;

struct ParrotScene {
    ParrotScope *scope;

    ParrotSceneRegisteredComponent *sh_registered_components;

    ParrotSceneEntity *arr_entity_parents;

    uint8_t *arr_entity_flags;
    uint32_t *arr_entity_gens;

    uint32_t *arr_free_indices;
    uint32_t next_new_index;

    ParrotSceneCachedQuery **hm_query_cache;

    uint64_t tree_gen;
    uint64_t component_gen;
};

static void ParrotSceneRegisteredComponent_min_size(ParrotSceneRegisteredComponent *self, size_t size) {
    size_t required_block_count = PARROT_ALIGN_UP(size, COMPONENT_DATA_BLOCK_SIZE) / COMPONENT_DATA_BLOCK_SIZE;
    while (ParrotArray_size(*self->p_arr_data_blocks) < required_block_count) {
        void *block = malloc(COMPONENT_DATA_BLOCK_SIZE * self->profile.size);
        ParrotScope_push_free(self->scope, block);
        ParrotArray_push(*self->p_arr_data_blocks, block);
    }

    MIN_ARR_SIZE_VALUE(*self->p_arr_flags, size, 0);
}

ParrotScene *ParrotScene_new(void) {
    ParrotScene *self = PARROT_ALLOC(ParrotScene);

    self->scope = ParrotScope_new(NULL);
    ParrotScope_push_free(self->scope, self);

    ParrotScope_push_arrfree(self->scope, self->sh_registered_components);

    ParrotScope_push_arrfree(self->scope, self->arr_entity_parents);

    ParrotScope_push_arrfree(self->scope, self->arr_entity_flags);
    ParrotScope_push_arrfree(self->scope, self->arr_entity_gens);

    ParrotScope_push_arrfree(self->scope, self->arr_free_indices);
    self->next_new_index = 1;

    ParrotScope_push_arrfree(self->scope, self->hm_query_cache);

    self->tree_gen = 1;
    self->component_gen = 1;

    ParrotScene_register_component(self, ParrotScene_ParrotMat_profile);
    ParrotScene_register_component(self, ParrotScene_ParrotTransform_profile);

    return self;
}

void ParrotScene_delete(ParrotScene *self) {
    PARROT_FAIL_NULL(self);

    for (size_t i = 0; i < ParrotArray_size(self->sh_registered_components); i++) {
        ParrotSceneRegisteredComponent *component = &self->sh_registered_components[i];
        for (size_t j = 0; j < ParrotArray_size(*component->p_arr_flags); j++) {
            uint8_t flags = (*component->p_arr_flags)[j];
            if (!(flags & LIFE_FLAG_EXISTS)) {
                continue;
            }

            void *data = &(*component->p_arr_data_blocks)[j / COMPONENT_DATA_BLOCK_SIZE]
                                                         [(j % COMPONENT_DATA_BLOCK_SIZE) * component->profile.size];

            if (component->profile.destroy) {
                component->profile.destroy(data,
                                           (ParrotSceneFuncParams){
                                               .scene = self,
                                               .entity = MAKE_ENTITY(j, self->arr_entity_gens[j]),
                                           });
            }
        }
    }

    ParrotScope_delete(self->scope);
}

void ParrotScene_vdelete(void *self) {
    ParrotScene_delete((ParrotScene *)self);
}

typedef enum {
    UpdateStage_READY = 0,

    UpdateStage_EARLY,
    UpdateStage_UPDATE,
    UpdateStage_LATE,
} UpdateStage;

static void update_entity(ParrotScene *self, UpdateStage stage, ParrotSceneEntity entity, float delta) {
    ParrotScope *scope = ParrotScope_new(NULL);

    if (!ParrotScene_does_entity_exist(self, entity)) {
        goto propogate;
    }

    ParrotSceneFuncParams params = {
        .scene = self,
        .entity = entity,
    };

    size_t index = ENTITY_INDEX(entity);

    // OPTIMIZE: Precompute list of components with these handlers
    for (size_t i = 0; i < ParrotArray_size(self->sh_registered_components); i++) {
        ParrotSceneRegisteredComponent *component = &self->sh_registered_components[i];
        uint8_t flags = (*component->p_arr_flags)[index];

        if (!(flags & LIFE_FLAG_EXISTS)) {
            continue;
        }

        void *data = &(*component->p_arr_data_blocks)[index / COMPONENT_DATA_BLOCK_SIZE]
                                                     [(index % COMPONENT_DATA_BLOCK_SIZE) * component->profile.size];

        switch (stage) {
        case UpdateStage_READY: {
            if (flags & LIFE_FLAG_READY) {
                continue;
            }

            flags |= LIFE_FLAG_READY;
            if (component->profile.ready) {
                component->profile.ready(data, params);
            }
        } break;
        case UpdateStage_EARLY: {
            if (component->profile.early_update) {
                component->profile.early_update(data, params, delta);
            }
        } break;
        case UpdateStage_UPDATE: {
            if (component->profile.update) {
                component->profile.update(data, params, delta);
            }
        } break;
        case UpdateStage_LATE: {
            if (component->profile.late_update) {
                component->profile.late_update(data, params, delta);
            }
        } break;
        }

        (*component->p_arr_flags)[index] = flags;
    }

propogate: {
    ParrotSceneEntity *arr_children = NULL;
    ParrotScope_push_arrfree(scope, arr_children);

    ParrotSceneQuery child_query[] = {
        PARROT_SCENE_QUERY_WITH_PARENT(entity),
        PARROT_SCENE_QUERY_END(),
    };

    ParrotScene_query(self, child_query, &arr_children);

    for (size_t i = 0; i < ParrotArray_size(arr_children); i++) {
        update_entity(self, stage, arr_children[i], delta);
    }

    ParrotScope_delete(scope);
}
}

void ParrotScene_update(ParrotScene *self, float delta) {
    PARROT_FAIL_NULL(self);

    update_entity(self, UpdateStage_READY, ParrotSceneEntity_NULL, delta);

    update_entity(self, UpdateStage_EARLY, ParrotSceneEntity_NULL, delta);
    ParrotScene_delete_queued(self);
    update_entity(self, UpdateStage_UPDATE, ParrotSceneEntity_NULL, delta);
    update_entity(self, UpdateStage_LATE, ParrotSceneEntity_NULL, delta);
}

void ParrotScene_delete_queued(ParrotScene *self) {
    PARROT_FAIL_NULL(self);

    for (size_t i = 0; i < ParrotArray_size(self->arr_entity_gens); i++) {
        if (!(self->arr_entity_flags[i] & LIFE_FLAG_DELETION_QUEUED)) {
            continue;
        }

        ParrotScene_set_entity_parent(self, MAKE_ENTITY(i, self->arr_entity_gens[i]), ParrotSceneEntity_NULL);

        ParrotArray_push(self->arr_free_indices, i);

        self->arr_entity_flags[i] = 0;
    }

    for (size_t i = 0; i < ParrotArray_size(self->sh_registered_components); i++) {
        ParrotSceneRegisteredComponent *component = &self->sh_registered_components[i];

        for (size_t j = 0; j < ParrotArray_size(self->arr_entity_gens); j++) {
            if (!((*component->p_arr_flags)[j] & LIFE_FLAG_DELETION_QUEUED)) {
                continue;
            }

            void *data = &(*component->p_arr_data_blocks)[j / COMPONENT_DATA_BLOCK_SIZE]
                                                         [(j % COMPONENT_DATA_BLOCK_SIZE) * component->profile.size];

            if (component->profile.destroy) {
                component->profile.destroy(data,
                                           (ParrotSceneFuncParams){
                                               .scene = self,
                                               .entity = MAKE_ENTITY(j, self->arr_entity_gens[j]),
                                           });
            }

            (*component->p_arr_flags)[j] = 0;
        }
    }
}

bool ParrotScene_is_entity_deletion_queued(ParrotScene *self, ParrotSceneEntity entity) {
    PARROT_FAIL_NULL(self);
    PARROT_RET_COND_V(!ParrotScene_does_entity_exist(self, entity), false);

    return self->arr_entity_flags[ENTITY_INDEX(entity)] & LIFE_FLAG_DELETION_QUEUED;
}

ParrotSceneEntity ParrotScene_create_entity(ParrotScene *self) {
    PARROT_FAIL_NULL(self);

    self->tree_gen++;

    uint32_t index = self->next_new_index;
    if (ParrotArray_size(self->arr_free_indices) > 0) {
        index = self->arr_free_indices[ParrotArray_size(self->arr_free_indices) - 1];
        ParrotArray_del(self->arr_free_indices, 0);
        self->arr_entity_gens[index]++;
    } else {
        self->next_new_index++;
    }

    MIN_ARR_SIZE_VALUE(self->arr_entity_parents, index + 1, ParrotSceneEntity_NULL);

    MIN_ARR_SIZE_VALUE(self->arr_entity_flags, index + 1, false);
    MIN_ARR_SIZE_VALUE(self->arr_entity_gens, index + 1, 0);

    for (size_t i = 0; i < ParrotArray_size(self->sh_registered_components); i++) {
        ParrotSceneRegisteredComponent_min_size(&self->sh_registered_components[i], index + 1);
    }

    self->arr_entity_flags[index] |= LIFE_FLAG_EXISTS;
    self->arr_entity_flags[index] |= LIFE_FLAG_READY;

    return MAKE_ENTITY(index, self->arr_entity_gens[index]);
}

void ParrotScene_queue_delete_entity(ParrotScene *self, ParrotSceneEntity entity) {
    PARROT_RET_COND(!ParrotScene_does_entity_exist(self, entity));

    self->tree_gen++;

    ParrotScope *scope = ParrotScope_new(NULL);

    ParrotSceneQuery child_query[] = {
        PARROT_SCENE_QUERY_WITH_PARENT(entity),
        PARROT_SCENE_QUERY_END(),
    };

    ParrotSceneEntity *arr_children = NULL;
    ParrotScope_push_arrfree(scope, arr_children);

    ParrotScene_query(self, child_query, &arr_children);

    for (size_t i = ParrotArray_size(arr_children); i > 0; i--) {
        ParrotScene_queue_delete_entity(self, arr_children[i - 1]);
    }

    for (size_t i = 0; i < ParrotArray_size(self->sh_registered_components); i++) {
        ParrotSceneRegisteredComponent *component = &self->sh_registered_components[i];
        if ((*component->p_arr_flags)[ENTITY_INDEX(entity)] & LIFE_FLAG_EXISTS) {
            ParrotScene_queue_delete_componentx(self, entity, component->key);
        }
    }

    self->arr_entity_flags[ENTITY_INDEX(entity)] |= LIFE_FLAG_DELETION_QUEUED;

    ParrotScope_delete(scope);
}

bool ParrotScene_does_entity_exist(ParrotScene *self, ParrotSceneEntity entity) {
    PARROT_FAIL_NULL(self);
    PARROT_RET_COND_V(ENTITY_INDEX(entity) >= ParrotArray_size(self->arr_entity_flags), false);

    return (self->arr_entity_flags[ENTITY_INDEX(entity)] & LIFE_FLAG_EXISTS) &&
           self->arr_entity_gens[ENTITY_INDEX(entity)] == ENTITY_GEN(entity);
}

void ParrotScene_set_entity_parent(ParrotScene *self,
                                   ParrotSceneEntity child,
                                   /* Nullable */ ParrotSceneEntity new_parent) {
    PARROT_RET_COND(!ParrotScene_does_entity_exist(self, child));

    // TODO: Cycle detection

    self->tree_gen++;
    self->arr_entity_parents[ENTITY_INDEX(child)] = new_parent;
}

ParrotSceneEntity ParrotScene_get_entity_parent_or_null(ParrotScene *self, ParrotSceneEntity entity) {
    PARROT_RET_COND_V(!ParrotScene_does_entity_exist(self, entity), ParrotSceneEntity_NULL);

    return self->arr_entity_parents[ENTITY_INDEX(entity)];
}

void ParrotScene_auto_register_components(ParrotScene *self, ParrotReflect *reflect) {
    PARROT_FAIL_NULL(self);
    PARROT_FAIL_NULL(reflect);

    for (size_t i = 0; i < ParrotReflect_get_type_count(reflect); i++) {
        ptrdiff_t type = ParrotReflect_resolve_type_by_index_or_neg(reflect, i);
        if (type < 0) {
            continue;
        }

        ParrotReflectDescriptionTag tag;
        if (!ParrotReflect_get_type_tag_or_null(reflect, type, PARROT_TYPE_STRING(ParrotSceneComponentProfile), &tag)) {
            continue;
        }

        const ParrotSceneComponentProfile *profile = tag.value;
        if (!ParrotScene_is_component_registered(self, profile->type)) {
            ParrotScene_register_component(self, *profile);
        }
    }
}

void ParrotScene_register_component(ParrotScene *self, ParrotSceneComponentProfile profile) {
    PARROT_FAIL_COND(ParrotScene_is_component_registered(self, profile.type));

    self->component_gen++;

    ParrotSceneRegisteredComponent component = {0};

    component.scope = ParrotScope_new(self->scope);

    component.key = strcpy(calloc(strlen(profile.type) + 1, sizeof(char)), profile.type);
    ParrotScope_push_free(component.scope, component.key);

    component.profile = profile;

    ParrotScope_alloc_ptr_arrfree(component.scope, component.p_arr_data_blocks);
    ParrotScope_alloc_ptr_arrfree(component.scope, component.p_arr_flags);

    ParrotSceneRegisteredComponent_min_size(&component, self->next_new_index);

    ParrotArray_puts(self->sh_registered_components, component);
}

void ParrotScene_unregister_component(ParrotScene *self, const char *type) {
    PARROT_FAIL_COND(!ParrotScene_is_component_registered(self, type));

    self->component_gen++;

    ptrdiff_t component_index = ParrotArray_finds_or_neg(self->sh_registered_components, type);
    PARROT_FAIL_COND(component_index < 0);
    ParrotSceneRegisteredComponent *component = &self->sh_registered_components[component_index];

    ParrotScope_delete(component->scope);
    ParrotArray_del(self->sh_registered_components, component_index);
}

bool ParrotScene_is_component_registered(ParrotScene *self, const char *type) {
    PARROT_FAIL_NULL(self);
    PARROT_FAIL_NULL(type);

    return ParrotArray_finds_or_neg(self->sh_registered_components, type) >= 0;
}

void *ParrotScene_add_component_or_nullx(ParrotScene *self, ParrotSceneEntity entity, const char *type) {
    PARROT_FAIL_COND(!ParrotScene_is_component_registered(self, type));
    PARROT_RET_COND_V(!ParrotScene_does_entity_exist(self, entity), NULL);

    self->component_gen++;

    ParrotSceneRegisteredComponent *component = ParrotArray_findsp_or_null(self->sh_registered_components, type);
    PARROT_FAIL_NULL(component);

    void *data =
        &(*component->p_arr_data_blocks)[ENTITY_INDEX(entity) / COMPONENT_DATA_BLOCK_SIZE]
                                        [(ENTITY_INDEX(entity) % COMPONENT_DATA_BLOCK_SIZE) * component->profile.size];
    memset(data, 0, component->profile.size);

    (*component->p_arr_flags)[ENTITY_INDEX(entity)] = LIFE_FLAG_EXISTS;

    if (component->profile.init) {
        component->profile.init(data,
                                (ParrotSceneFuncParams){
                                    .scene = self,
                                    .entity = entity,
                                });
    }

    return data;
}

void *ParrotScene_get_component_or_nullx(ParrotScene *self, ParrotSceneEntity entity, const char *type) {
    PARROT_RET_COND_V(!ParrotScene_is_component_registered(self, type), NULL);
    PARROT_RET_COND_V(!ParrotScene_does_entity_exist(self, entity), NULL);

    ParrotSceneRegisteredComponent *component = ParrotArray_findsp_or_null(self->sh_registered_components, type);
    PARROT_FAIL_NULL(component);

    void *data =
        &(*component->p_arr_data_blocks)[ENTITY_INDEX(entity) / COMPONENT_DATA_BLOCK_SIZE]
                                        [(ENTITY_INDEX(entity) % COMPONENT_DATA_BLOCK_SIZE) * component->profile.size];
    bool exists = (*component->p_arr_flags)[ENTITY_INDEX(entity)] & LIFE_FLAG_EXISTS;
    return exists ? data : NULL;
}

bool ParrotScene_is_component_deletion_queuedx(ParrotScene *self, ParrotSceneEntity entity, const char *type) {
    PARROT_RET_COND_V(!ParrotScene_is_component_registered(self, type), false);
    PARROT_RET_COND_V(!ParrotScene_does_entity_exist(self, entity), false);

    ParrotSceneRegisteredComponent *component = ParrotArray_findsp_or_null(self->sh_registered_components, type);
    PARROT_FAIL_NULL(component);

    return (*component->p_arr_flags)[ENTITY_INDEX(entity)] & LIFE_FLAG_DELETION_QUEUED;
}

void ParrotScene_queue_delete_componentx(ParrotScene *self, ParrotSceneEntity entity, const char *type) {
    PARROT_RET_COND(!ParrotScene_is_component_registered(self, type));
    PARROT_RET_COND(!ParrotScene_does_entity_exist(self, entity));

    self->component_gen++;

    ParrotSceneRegisteredComponent *component = ParrotArray_findsp_or_null(self->sh_registered_components, type);
    PARROT_FAIL_NULL(component);

    (*component->p_arr_flags)[ENTITY_INDEX(entity)] |= LIFE_FLAG_DELETION_QUEUED;
}

void ParrotScene_query(ParrotScene *self, const ParrotSceneQuery *query, ParrotSceneEntity **p_arr_results) {
    ParrotCRC32 query_crc = 0;
    for (const ParrotSceneQuery *filter = query; filter->type != ParrotSceneQueryType_END; filter++) {
        query_crc = Parrot_crc32_combine(query_crc, filter, sizeof(ParrotSceneQuery));

        switch (filter->type) {
        case ParrotSceneQueryType_COMPONENT: {
            query_crc =
                Parrot_crc32_combine(query_crc, filter->data.component.type, strlen(filter->data.component.type));
        } break;
        default:
            break;
        }
    }

    ptrdiff_t cached_query_index = ParrotArray_find_or_neg(self->hm_query_cache, query_crc);
    ParrotSceneCachedQuery *cached_query = NULL;

    if (cached_query_index >= 0) {
        cached_query = self->hm_query_cache[cached_query_index];

        bool discard_conditions[] = {
            cached_query->component_gen > 0 && cached_query->component_gen != self->component_gen,
            cached_query->tree_gen > 0 && cached_query->tree_gen != self->tree_gen,
        };

        size_t i;
        for (i = 0; i < PARROT_ARRAY_LEN(discard_conditions) && !discard_conditions[i]; i++)
            ;

        if (i >= PARROT_ARRAY_LEN(discard_conditions)) {
            goto place_results;
        }
    }

    cached_query = PARROT_ALLOC(ParrotSceneCachedQuery);

    cached_query->scope = ParrotScope_new(self->scope);
    ParrotScope_push_free(cached_query->scope, cached_query);

    ParrotScope_push_arrfree(cached_query->scope, cached_query->arr_entity_indices);

    for (size_t i = 0; i < ParrotArray_size(self->arr_entity_flags); i++) {
        if (self->arr_entity_flags[i] & LIFE_FLAG_EXISTS) {
            ParrotArray_push(cached_query->arr_entity_indices, i);
        }
    }

    bool invert = false;

    for (const ParrotSceneQuery *filter = query; filter->type != ParrotSceneQueryType_END; filter++) {
        switch (filter->type) {
        case ParrotSceneQueryType_SET_INVERT:
            invert = filter->data.invert;
            break;
        case ParrotSceneQueryType_PARENT: {
            cached_query->tree_gen = self->tree_gen;
            for (size_t i = 0; i < ParrotArray_size(cached_query->arr_entity_indices); i++) {
                size_t index = cached_query->arr_entity_indices[i];
                if ((self->arr_entity_parents[index] == filter->data.parent.entity) == invert) {
                    ParrotArray_del(cached_query->arr_entity_indices, i);
                    i--;
                }
            }
        } break;
        case ParrotSceneQueryType_COMPONENT: {
            cached_query->component_gen = self->component_gen;
            ParrotSceneRegisteredComponent *component =
                ParrotArray_findsp_or_null(self->sh_registered_components, filter->data.component.type);

            for (size_t i = 0; i < ParrotArray_size(cached_query->arr_entity_indices); i++) {
                size_t index = cached_query->arr_entity_indices[i];
                if ((component && ((*component->p_arr_flags)[index] & LIFE_FLAG_EXISTS)) == invert) {
                    ParrotArray_del(cached_query->arr_entity_indices, i);
                    i--;
                }
            }
        } break;
        case ParrotSceneQueryType_END:
            break;
        }
    }

    if (cached_query_index >= 0) {
        ParrotScope_delete(self->hm_query_cache[cached_query_index]->scope);
    }
    ParrotArray_put_key(self->hm_query_cache, cached_query, &query_crc, sizeof(query_crc));

place_results:
    for (size_t i = 0; i < ParrotArray_size(cached_query->arr_entity_indices); i++) {
        size_t index = cached_query->arr_entity_indices[i];
        ParrotArray_push(*p_arr_results, MAKE_ENTITY(index, self->arr_entity_gens[index]));
    }
}

static void ParrotScene_ParrotMat_init(void *selfx, ParrotSceneFuncParams params) {
    ParrotMat *self = selfx;
    (void)params;

    *self = ParrotMat_identity();
}

const ParrotSceneComponentProfile ParrotScene_ParrotMat_profile = {
    .size = sizeof(ParrotMat),
    .type = PARROT_TYPE_STRING(ParrotMat),

    .init = ParrotScene_ParrotMat_init,
};

static void ParrotScene_ParrotTransform_init(void *selfx, ParrotSceneFuncParams params) {
    ParrotTransform *self = selfx;
    (void)params;

    *self = ParrotTransform_new();
}

const ParrotSceneComponentProfile ParrotScene_ParrotTransform_profile = {
    .size = sizeof(ParrotTransform),
    .type = PARROT_TYPE_STRING(ParrotTransform),

    .init = ParrotScene_ParrotTransform_init,
};
