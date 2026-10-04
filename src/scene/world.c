#include "parrot/scene/world.h"
#include "parrot/core/array.h"
#include "parrot/core/hash.h"
#include "parrot/core/math.h"
#include "parrot/core/scope.h"
#include "src/ds.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COMPONENT_DATA_BLOCK_SIZE (64)

#define MIN_ARR_SIZE_VALUE(arr, size, fill)                                                                             \
    do {                                                                                                                \
        while (ParrotArray_size(arr) < size) {                                                                          \
            ParrotArray_push((arr), fill);                                                                              \
        }                                                                                                               \
    } while (0);

#define MAKE_ENTITY(index, gen) ((ParrotSceneWorldEntity)(((ParrotSceneWorldEntity)(gen)) << 32 | (index)))
#define ENTITY_INDEX(entity) ((uint32_t)((entity) & 0xFFFFFFFF))
#define ENTITY_GEN(entity) ((uint32_t)(((entity) >> 32)))

typedef struct {
    ParrotScope *scope;

    ParrotCRC32 key;

    ParrotSceneWorldEntity **p_arr_results;

    const char ***p_arr_components;
} ParrotSceneWorldCachedQuery;

typedef struct {
    ParrotScope *scope;

    char *key;

    ParrotSceneWorldComponentDescription description;

    uint8_t ***p_arr_data_blocks;

    bool **p_arr_exists;
    bool **p_arr_delete_queued;

    ParrotCRC32Set **p_shm_queries;
} ParrotSceneWorldRegisteredComponent;

struct ParrotSceneWorld {
    ParrotScope *scope;

    ParrotSceneWorldRegisteredComponent *sh_registered_components;

    ParrotSceneWorldEntity *arr_entity_parents;

    bool *arr_entity_exists;
    bool *arr_entity_delete_queued;
    uint32_t *arr_entity_gens;

    uint32_t *arr_free_indices;
    uint32_t next_new_index;

    ParrotSceneWorldCachedQuery *hm_queries;
    ParrotCRC32 *arr_entity_tree_queries;
};

static void ParrotSceneWorld_invalidate_cached_query(ParrotSceneWorld *self, ParrotCRC32 query_crc32) {
    ptrdiff_t cached_query_index = ParrotArray_find(self->hm_queries, query_crc32);
    PARROT_RET_COND(cached_query_index < 0);

    ParrotScope_delete(self->hm_queries[cached_query_index].scope);
    ParrotArray_delk(self->hm_queries, query_crc32);
}

static void ParrotSceneWorld_invalidate_tree_queries(ParrotSceneWorld *self) {
    for (size_t i = 0; i < ParrotArray_size(self->arr_entity_tree_queries); i++) {
        ParrotSceneWorld_invalidate_cached_query(self, self->arr_entity_tree_queries[i]);
    }
    ParrotArray_free(self->arr_entity_tree_queries);
}

static void ParrotSceneWorldRegisteredComponent_min_size(ParrotSceneWorldRegisteredComponent *self, size_t size) {
    size_t required_block_count = PARROT_ALIGN_UP(size, COMPONENT_DATA_BLOCK_SIZE) / COMPONENT_DATA_BLOCK_SIZE;
    while (ParrotArray_size(*self->p_arr_data_blocks) < required_block_count) {
        void *block = malloc(COMPONENT_DATA_BLOCK_SIZE * self->description.size);
        ParrotScope_push_free(self->scope, block);
        ParrotArray_push(*self->p_arr_data_blocks, block);
    }

    MIN_ARR_SIZE_VALUE(*self->p_arr_exists, size, false);
    MIN_ARR_SIZE_VALUE(*self->p_arr_delete_queued, size, false);
}

static void ParrotSceneWorld_ParrotMat_constructor(ParrotSceneWorldEntity entity, void *component_ptr, void *user_data) {
    (void)entity;
    (void)user_data;
    *(ParrotMat *)component_ptr = ParrotMat_identity();
}

static void
ParrotSceneWorld_ParrotTransform_constructor(ParrotSceneWorldEntity entity, void *component_ptr, void *user_data) {
    (void)entity;
    (void)user_data;
    *(ParrotTransform *)component_ptr = ParrotTransform_new();
}

static void
ParrotSceneWorld_ParrotColor_constructor(ParrotSceneWorldEntity entity, void *component_ptr, void *user_data) {
    (void)entity;
    (void)user_data;
    *(ParrotColor *)component_ptr = ParrotColor_WHITE;
}

ParrotSceneWorld *ParrotSceneWorld_new(void) {
    ParrotSceneWorld *self = PARROT_ALLOC(ParrotSceneWorld);

    self->scope = ParrotScope_new(NULL);
    ParrotScope_push_free(self->scope, self);

    ParrotScope_push_arrfree(self->scope, self->sh_registered_components);

    ParrotScope_push_arrfree(self->scope, self->arr_entity_parents);

    ParrotScope_push_arrfree(self->scope, self->arr_entity_exists);
    ParrotScope_push_arrfree(self->scope, self->arr_entity_delete_queued);
    ParrotScope_push_arrfree(self->scope, self->arr_entity_gens);

    ParrotScope_push_arrfree(self->scope, self->arr_free_indices);
    self->next_new_index = 1;

    ParrotScope_push_arrfree(self->scope, self->hm_queries);
    ParrotScope_push_arrfree(self->scope, self->arr_entity_tree_queries);

    ParrotSceneWorld_register_component(self,
                                        "ParrotMat",
                                        (ParrotSceneWorldComponentDescription){
                                            .size = sizeof(ParrotMat),
                                            .constructor = ParrotSceneWorld_ParrotMat_constructor,
                                        });
    ParrotSceneWorld_register_component(self,
                                        "ParrotTransform",
                                        (ParrotSceneWorldComponentDescription){
                                            .size = sizeof(ParrotTransform),
                                            .constructor = ParrotSceneWorld_ParrotTransform_constructor,
                                        });

    ParrotSceneWorld_register_component(self,
                                        "ParrotColor",
                                        (ParrotSceneWorldComponentDescription){
                                            .size = sizeof(ParrotColor),
                                            .constructor = ParrotSceneWorld_ParrotColor_constructor,
                                        });

    return self;
}

void ParrotSceneWorld_delete(ParrotSceneWorld *self) {
    PARROT_FAIL_NULL(self);

    ParrotScope_delete(self->scope);
}

void ParrotSceneWorld_vdelete(void *self) {
    ParrotSceneWorld_delete((ParrotSceneWorld *)self);
}

void ParrotSceneWorld_delete_queued(ParrotSceneWorld *self) {
    for (size_t i = 0; i < ParrotArray_size(self->sh_registered_components); i++) {
        ParrotSceneWorldRegisteredComponent *component = &self->sh_registered_components[i];
        for (size_t j = 0; j < ParrotArray_size(self->arr_entity_gens); j++) {
            if (!(*component->p_arr_delete_queued)[j]) {
                continue;
            }

            (*component->p_arr_exists)[j] = false;
            (*component->p_arr_delete_queued)[j] = false;

            for (size_t i = 0; i < ParrotArray_size(*component->p_shm_queries); i++) {
                ParrotSceneWorld_invalidate_cached_query(self, (*component->p_shm_queries)[i].key);
            }
            ParrotArray_free(*component->p_shm_queries);
        }
    }

    for (size_t i = 0; i < ParrotArray_size(self->arr_entity_gens); i++) {
        if (!self->arr_entity_delete_queued[i]) {
            continue;
        }

        ParrotSceneWorld_set_entity_parent(self, MAKE_ENTITY(i, self->arr_entity_gens[i]), ParrotSceneWorldEntity_NULL);

        self->arr_entity_exists[i] = false;

        ParrotSceneWorld_invalidate_tree_queries(self);
        ParrotArray_push(self->arr_free_indices, i);

        self->arr_entity_delete_queued[i] = false;
    }
}

bool ParrotSceneWorld_is_entity_deletion_queued(ParrotSceneWorld *self, ParrotSceneWorldEntity entity) {
    PARROT_FAIL_NULL(self);
    PARROT_RET_COND_V(!ParrotSceneWorld_does_entity_exist(self, entity), false);

    return self->arr_entity_delete_queued[ENTITY_INDEX(entity)];
}

ParrotSceneWorldEntity ParrotSceneWorld_create_entity(ParrotSceneWorld *self) {
    PARROT_FAIL_NULL(self);

    uint32_t index = self->next_new_index;
    if (ParrotArray_size(self->arr_free_indices) > 0) {
        index = self->arr_free_indices[ParrotArray_size(self->arr_free_indices) - 1];
        ParrotArray_del(self->arr_free_indices, 0);
        self->arr_entity_gens[index]++;
    } else {
        self->next_new_index++;
    }

    MIN_ARR_SIZE_VALUE(self->arr_entity_parents, index + 1, ParrotSceneWorldEntity_NULL);

    MIN_ARR_SIZE_VALUE(self->arr_entity_exists, index + 1, false);
    MIN_ARR_SIZE_VALUE(self->arr_entity_delete_queued, index + 1, false);
    MIN_ARR_SIZE_VALUE(self->arr_entity_gens, index + 1, 0);

    for (size_t i = 0; i < ParrotArray_size(self->sh_registered_components); i++) {
        ParrotSceneWorldRegisteredComponent_min_size(&self->sh_registered_components[i], index + 1);

        (*self->sh_registered_components[i].p_arr_exists)[index] = false;
    }

    self->arr_entity_exists[index] = true;

    ParrotSceneWorld_invalidate_tree_queries(self);

    return MAKE_ENTITY(index, self->arr_entity_gens[index]);
}

void ParrotSceneWorld_queue_delete_entity(ParrotSceneWorld *self, ParrotSceneWorldEntity entity) {
    PARROT_RET_COND(!ParrotSceneWorld_does_entity_exist(self, entity));

    ParrotSceneWorldQuery child_query[] = {
        PARROT_SCENE_WORLD_QUERY_WITH_PARENT(entity),
        PARROT_SCENE_WORLD_QUERY_END(),
    };

    for (size_t i = ParrotSceneWorld_query_result_count(self, child_query); i > 0; i--) {
        ParrotSceneWorld_queue_delete_entity(self, ParrotSceneWorld_query_result_at(self, child_query, i - 1));
    }

    for (size_t i = 0; i < ParrotArray_size(self->sh_registered_components); i++) {
        ParrotSceneWorldRegisteredComponent *component = &self->sh_registered_components[i];
        if ((*component->p_arr_exists) && (*component->p_arr_exists)[ENTITY_INDEX(entity)]) {
            ParrotSceneWorld_queue_delete_component_name(self, entity, component->key);
        }
    }

    self->arr_entity_delete_queued[ENTITY_INDEX(entity)] = true;
}

bool ParrotSceneWorld_does_entity_exist(ParrotSceneWorld *self, ParrotSceneWorldEntity entity) {
    PARROT_FAIL_NULL(self);
    PARROT_RET_COND_V(ENTITY_INDEX(entity) >= ParrotArray_size(self->arr_entity_exists), false);

    return self->arr_entity_exists[ENTITY_INDEX(entity)] &&
           self->arr_entity_gens[ENTITY_INDEX(entity)] == ENTITY_GEN(entity);
}

size_t ParrotSceneWorld_get_entity_count(ParrotSceneWorld *self) {
    PARROT_FAIL_NULL(self);

    return ParrotArray_size(self->arr_entity_gens);
}

ParrotSceneWorldEntity ParrotSceneWorld_get_entity(ParrotSceneWorld *self, size_t index) {
    PARROT_FAIL_NULL(self);
    PARROT_FAIL_COND_MSG(index >= ParrotArray_size(self->arr_entity_gens),
                         "Attempted to access out of bounds index in children");

    return MAKE_ENTITY(index, self->arr_entity_gens[index]);
}

void ParrotSceneWorld_set_entity_parent(ParrotSceneWorld *self,
                                        ParrotSceneWorldEntity child,
                                        /* Nullable */ ParrotSceneWorldEntity new_parent) {
    PARROT_RET_COND(!ParrotSceneWorld_does_entity_exist(self, child));

    // TODO: Cycle detection

    self->arr_entity_parents[ENTITY_INDEX(child)] = new_parent;

    ParrotSceneWorld_invalidate_tree_queries(self);
}

ParrotSceneWorldEntity ParrotSceneWorld_get_entity_parent(ParrotSceneWorld *self, ParrotSceneWorldEntity entity) {
    PARROT_RET_COND_V(!ParrotSceneWorld_does_entity_exist(self, entity), ParrotSceneWorldEntity_NULL);

    return self->arr_entity_parents[ENTITY_INDEX(entity)];
}

void ParrotSceneWorld_register_component(ParrotSceneWorld *self,
                                         const char *name,
                                         ParrotSceneWorldComponentDescription description) {
    PARROT_FAIL_COND(ParrotSceneWorld_is_component_registered(self, name));

    ParrotSceneWorldRegisteredComponent component = {0};

    component.scope = ParrotScope_new(self->scope);

    component.key = strcpy(calloc(strlen(name) + 1, sizeof(char)), name);
    ParrotScope_push_free(component.scope, component.key);

    component.description = description;

    ParrotScope_alloc_ptr_arrfree(component.scope, component.p_arr_data_blocks);

    ParrotScope_alloc_ptr_arrfree(component.scope, component.p_arr_exists);
    ParrotScope_alloc_ptr_arrfree(component.scope, component.p_arr_delete_queued);

    ParrotScope_alloc_ptr_arrfree(component.scope, component.p_shm_queries);

    ParrotSceneWorldRegisteredComponent_min_size(&component, self->next_new_index);

    ParrotArray_puts(self->sh_registered_components, component);
}

void ParrotSceneWorld_unregister_component(ParrotSceneWorld *self, const char *name) {
    PARROT_FAIL_COND(!ParrotSceneWorld_is_component_registered(self, name));

    ptrdiff_t component_index = ParrotArray_finds(self->sh_registered_components, name);
    PARROT_FAIL_COND(component_index < 0);
    ParrotSceneWorldRegisteredComponent *component = &self->sh_registered_components[component_index];

    ParrotScope_delete(component->scope);
    ParrotArray_del(self->sh_registered_components, component_index);
}

bool ParrotSceneWorld_is_component_registered(ParrotSceneWorld *self, const char *name) {
    PARROT_FAIL_NULL(self);
    PARROT_FAIL_NULL(name);

    return ParrotArray_finds(self->sh_registered_components, name) >= 0;
}

void ParrotSceneWorld_add_component_name(ParrotSceneWorld *self, ParrotSceneWorldEntity entity, const char *name) {
    PARROT_FAIL_COND(!ParrotSceneWorld_is_component_registered(self, name));
    PARROT_RET_COND(!ParrotSceneWorld_does_entity_exist(self, entity));

    ParrotSceneWorldRegisteredComponent *component = ParrotArray_findsp(self->sh_registered_components, name);
    PARROT_FAIL_NULL(component);

    void *data = &(
        *component->p_arr_data_blocks)[ENTITY_INDEX(entity) / COMPONENT_DATA_BLOCK_SIZE]
                                      [(ENTITY_INDEX(entity) % COMPONENT_DATA_BLOCK_SIZE) * component->description.size];
    memset(data, 0, component->description.size);

    (*component->p_arr_exists)[ENTITY_INDEX(entity)] = true;

    if (component->description.constructor) {
        component->description.constructor(entity, data, component->description.user_data);
    }

    while (ParrotArray_size(*component->p_shm_queries) > 0) {
        ParrotSceneWorld_invalidate_cached_query(self, (*component->p_shm_queries[0]).key);
    }
    ParrotArray_free(*component->p_shm_queries);
}

void *ParrotSceneWorld_get_component_name(ParrotSceneWorld *self, ParrotSceneWorldEntity entity, const char *name) {
    PARROT_RET_COND_V(!ParrotSceneWorld_is_component_registered(self, name), NULL);
    PARROT_RET_COND_V(!ParrotSceneWorld_does_entity_exist(self, entity), NULL);

    ParrotSceneWorldRegisteredComponent *component = ParrotArray_findsp(self->sh_registered_components, name);
    PARROT_FAIL_NULL(component);

    void *data = &(
        *component->p_arr_data_blocks)[ENTITY_INDEX(entity) / COMPONENT_DATA_BLOCK_SIZE]
                                      [(ENTITY_INDEX(entity) % COMPONENT_DATA_BLOCK_SIZE) * component->description.size];
    return (*component->p_arr_exists)[ENTITY_INDEX(entity)] ? data : NULL;
}

bool ParrotSceneWorld_is_component_deletion_queued_name(ParrotSceneWorld *self,
                                                        ParrotSceneWorldEntity entity,
                                                        const char *name) {
    PARROT_RET_COND_V(!ParrotSceneWorld_is_component_registered(self, name), false);
    PARROT_RET_COND_V(!ParrotSceneWorld_does_entity_exist(self, entity), false);

    ParrotSceneWorldRegisteredComponent *component = ParrotArray_findsp(self->sh_registered_components, name);
    PARROT_FAIL_NULL(component);

    return (*component->p_arr_delete_queued)[ENTITY_INDEX(entity)];
}

void ParrotSceneWorld_queue_delete_component_name(ParrotSceneWorld *self,
                                                  ParrotSceneWorldEntity entity,
                                                  const char *name) {
    PARROT_RET_COND(!ParrotSceneWorld_is_component_registered(self, name));
    PARROT_RET_COND(!ParrotSceneWorld_does_entity_exist(self, entity));

    ParrotSceneWorldRegisteredComponent *component = ParrotArray_findsp(self->sh_registered_components, name);
    PARROT_FAIL_NULL(component);

    (*component->p_arr_delete_queued)[ENTITY_INDEX(entity)] = true;
}

static ParrotCRC32 ParrotSceneWorld_query(ParrotSceneWorld *self, const ParrotSceneWorldQuery *query) {
    size_t count = 0;
    for (count = 0; query[count].type != ParrotSceneWorldQueryType_END; count++)
        ;

    ParrotCRC32 query_crc32 = Parrot_crc32(query, count * sizeof(ParrotSceneWorldQuery));
    for (const ParrotSceneWorldQuery *filter = query; filter->type != ParrotSceneWorldQueryType_END; filter++) {
        switch (filter->type) {
        case ParrotSceneWorldQueryType_COMPONENT:
            query_crc32 = Parrot_crc32_combine(
                query_crc32, filter->data.component.name, strlen(filter->data.component.name) * sizeof(char));
            break;
        default:
            break;
        }
    }

    PARROT_RET_COND_V(ParrotArray_find(self->hm_queries, query_crc32) >= 0, query_crc32);

    ParrotSceneWorldCachedQuery cached_query = {0};

    cached_query.scope = ParrotScope_new(self->scope);

    cached_query.key = query_crc32;

    ParrotScope_alloc_ptr_arrfree(cached_query.scope, cached_query.p_arr_results);

    ParrotScope_alloc_ptr_arrfree(cached_query.scope, cached_query.p_arr_components);

    ParrotSizeSet *shm_entity_indices = NULL;
    for (size_t i = 0; i < ParrotArray_size(self->arr_entity_exists); i++) {
        if (self->arr_entity_exists[i]) {
            ParrotArray_put(shm_entity_indices, (ParrotSizeSet){i});
        }
    }

    bool invert = false;

    for (const ParrotSceneWorldQuery *filter = query; filter->type != ParrotSceneWorldQueryType_END; filter++) {
        switch (filter->type) {
        case ParrotSceneWorldQueryType_SET_INVERT:
            invert = filter->data.invert;
            break;
        case ParrotSceneWorldQueryType_PARENT: {
            for (size_t i = 0; i < ParrotArray_size(self->arr_entity_gens); i++) {
                if ((self->arr_entity_parents[i] == filter->data.parent.entity) == invert) {
                    ParrotArray_delk(shm_entity_indices, i);
                }
            }

            ParrotArray_push(self->arr_entity_tree_queries, query_crc32);
        } break;
        case ParrotSceneWorldQueryType_COMPONENT: {
            ParrotSceneWorldRegisteredComponent *component =
                ParrotArray_findsp(self->sh_registered_components, filter->data.component.name);

            for (size_t i = 0; i < ParrotArray_size(self->arr_entity_gens); i++) {
                if ((component && (*component->p_arr_exists)[i]) == invert) {
                    ParrotArray_delk(shm_entity_indices, i);
                }
            }

            if (component) {
                ParrotArray_put(*component->p_shm_queries, (ParrotCRC32Set){query_crc32});
            }

            char *component_name =
                strcpy(calloc(strlen(filter->data.component.name) + 1, sizeof(char)), filter->data.component.name);
            ParrotScope_push_free(cached_query.scope, component_name);
            ParrotArray_push(*cached_query.p_arr_components, component_name);
        } break;
        case ParrotSceneWorldQueryType_END: {
        } break;
        }
    }

    for (size_t i = 0; i < ParrotArray_size(shm_entity_indices); i++) {
        size_t index = shm_entity_indices[i].key;
        ParrotArray_push(*cached_query.p_arr_results, MAKE_ENTITY(index, self->arr_entity_gens[index]));
    }

    ParrotArray_put(self->hm_queries, cached_query);

    ParrotArray_free(shm_entity_indices);
    return query_crc32;
}

size_t ParrotSceneWorld_query_result_count(ParrotSceneWorld *self, const ParrotSceneWorldQuery *query) {
    PARROT_FAIL_NULL(self);
    PARROT_FAIL_NULL(query);

    ParrotCRC32 query_crc32 = ParrotSceneWorld_query(self, query);

    ParrotSceneWorldCachedQuery *cached_query = ParrotArray_findp(self->hm_queries, query_crc32);
    PARROT_FAIL_NULL(cached_query);

    return ParrotArray_size(*cached_query->p_arr_results);
}

ParrotSceneWorldEntity
ParrotSceneWorld_query_result_at(ParrotSceneWorld *self, const ParrotSceneWorldQuery *query, size_t idx) {
    PARROT_FAIL_NULL(self);
    PARROT_FAIL_NULL(query);

    ParrotCRC32 query_crc32 = ParrotSceneWorld_query(self, query);

    ParrotSceneWorldCachedQuery *cached_query = ParrotArray_findp(self->hm_queries, query_crc32);
    PARROT_FAIL_NULL(cached_query);

    PARROT_FAIL_COND(idx >= ParrotArray_size(*cached_query->p_arr_results));
    return (*cached_query->p_arr_results)[idx];
}
