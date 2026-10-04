#include "parrot/scene/world.h"
#include "parrot/core/array.h"
#include "parrot/core/hash.h"
#include "parrot/core/math.h"
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
    ParrotCRC32 key;

    ParrotSceneWorldEntity *arr_results;

    const char **arr_components;
} ParrotSceneWorldCachedQuery;

typedef struct {
    const char *key;
    ParrotCRC32Set *value;
} ParrotSceneWorldInitialCachedQueries;

typedef struct {
    char *key;

    uint8_t **arr_data_blocks;

    bool *arr_exists;
    bool *arr_delete_queued;

    ParrotSceneWorldComponentDescription description;

    ParrotCRC32Set *shm_queries;
} ParrotSceneWorldRegisteredComponent;

struct ParrotSceneWorld {
    ParrotSceneWorldRegisteredComponent *sh_registered_components;

    ParrotSceneWorldEntity *arr_entity_parents;

    bool *arr_entity_exists;
    bool *arr_entity_delete_queued;
    uint32_t *arr_entity_gens;

    uint32_t *arr_free_indices;
    uint32_t next_new_index;

    ParrotSceneWorldCachedQuery *hm_queries;
    ParrotSceneWorldInitialCachedQueries *sh_initial_cached_queries;
    ParrotCRC32 *arr_entity_tree_queries;
};

static void ParrotSceneWorld_invalidate_cached_query(ParrotSceneWorld *self, ParrotCRC32 query_crc32) {
    ptrdiff_t cached_query_index = ParrotArray_find(self->hm_queries, query_crc32);
    PARROT_RET_COND(cached_query_index < 0);

    ParrotSceneWorldCachedQuery *cached_query = &self->hm_queries[cached_query_index];

    ParrotArray_free(cached_query->arr_results);
    ParrotArray_free(cached_query->arr_components);

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
    while (ParrotArray_size(self->arr_data_blocks) < required_block_count) {
        ParrotArray_push(self->arr_data_blocks, malloc(COMPONENT_DATA_BLOCK_SIZE * self->description.size));
    }

    MIN_ARR_SIZE_VALUE(self->arr_exists, size, false);
    MIN_ARR_SIZE_VALUE(self->arr_delete_queued, size, false);
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

    self->next_new_index = 1;

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

    while (ParrotArray_size(self->hm_queries) > 0) {
        ParrotSceneWorld_invalidate_cached_query(self, self->hm_queries[0].key);
    }

    for (size_t i = 0; i < ParrotArray_size(self->arr_entity_gens); i++) {
        ParrotSceneWorldEntity entity = MAKE_ENTITY(i, self->arr_entity_gens[i]);
        ParrotSceneWorld_queue_delete_entity(self, entity);
    }
    ParrotSceneWorld_delete_queued(self);

    while (ParrotArray_size(self->sh_registered_components) > 0) {
        ParrotSceneWorld_unregister_component(self, self->sh_registered_components[0].key);
    }
    ParrotArray_free(self->sh_registered_components);

    ParrotArray_free(self->arr_entity_parents);

    ParrotArray_free(self->arr_entity_exists);
    ParrotArray_free(self->arr_entity_delete_queued);
    ParrotArray_free(self->arr_entity_gens);

    ParrotArray_free(self->arr_free_indices);

    ParrotArray_free(self->hm_queries);
    ParrotArray_free(self->arr_entity_tree_queries);

    while (ParrotArray_size(self->sh_initial_cached_queries) > 0) {
        ParrotArray_free(self->sh_initial_cached_queries[0].value);
        ParrotArray_del(self->sh_initial_cached_queries, 0);
    }
    ParrotArray_free(self->sh_initial_cached_queries);

    free(self);
}

void ParrotSceneWorld_vdelete(void *self) {
    ParrotSceneWorld_delete((ParrotSceneWorld *)self);
}

void ParrotSceneWorld_delete_queued(ParrotSceneWorld *self) {
    for (size_t i = 0; i < ParrotArray_size(self->sh_registered_components); i++) {
        ParrotSceneWorldRegisteredComponent *component = &self->sh_registered_components[i];
        for (size_t j = 0; j < ParrotArray_size(self->arr_entity_gens); j++) {
            if (!component->arr_delete_queued[j]) {
                continue;
            }

            component->arr_exists[j] = false;
            component->arr_delete_queued[j] = false;

            for (size_t i = 0; i < ParrotArray_size(component->shm_queries); i++) {
                ParrotSceneWorld_invalidate_cached_query(self, component->shm_queries[0].key);
            }
            ParrotArray_free(component->shm_queries);
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
        index = self->arr_free_indices[ParrotArray_size(self->arr_free_indices)];
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

        self->sh_registered_components[i].arr_exists[index] = false;
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
        if (component->arr_exists && component->arr_exists[ENTITY_INDEX(entity)]) {
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

    component.key = strcpy(calloc(strlen(name) + 1, sizeof(char)), name);
    component.description = description;

    ParrotSceneWorldRegisteredComponent_min_size(&component, self->next_new_index);

    ptrdiff_t initial_cached_query_index = ParrotArray_find(self->sh_initial_cached_queries, name);
    if (initial_cached_query_index >= 0) {
        ParrotSceneWorldInitialCachedQueries *initial_cached_query =
            &self->sh_initial_cached_queries[initial_cached_query_index];
        component.shm_queries = initial_cached_query->value;
        ParrotArray_del(self->sh_initial_cached_queries, initial_cached_query_index);
    }

    ParrotArray_puts(self->sh_registered_components, component);
}

void ParrotSceneWorld_unregister_component(ParrotSceneWorld *self, const char *name) {
    PARROT_FAIL_COND(!ParrotSceneWorld_is_component_registered(self, name));

    ptrdiff_t component_index = ParrotArray_finds(self->sh_registered_components, name);
    PARROT_FAIL_COND(component_index < 0);
    ParrotSceneWorldRegisteredComponent *component = &self->sh_registered_components[component_index];

    for (size_t i = 0; i < ParrotArray_size(component->arr_exists); i++) {
        if (component->arr_exists[i]) {
            ParrotSceneWorld_queue_delete_component_name(self, MAKE_ENTITY(i, self->arr_entity_gens[i]), component->key);
        }
    }

    {
        for (size_t i = 0; i < ParrotArray_size(component->arr_data_blocks); i++) {
            free(component->arr_data_blocks[i]);
        }
        ParrotArray_free(component->arr_data_blocks);

        ParrotArray_free(component->arr_exists);
        ParrotArray_free(component->arr_delete_queued);
    }

    ParrotArray_free(component->shm_queries);

    free(component->key);

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

    void *data =
        &component->arr_data_blocks[ENTITY_INDEX(entity) / COMPONENT_DATA_BLOCK_SIZE]
                                   [(ENTITY_INDEX(entity) % COMPONENT_DATA_BLOCK_SIZE) * component->description.size];
    memset(data, 0, component->description.size);

    component->arr_exists[ENTITY_INDEX(entity)] = true;

    if (component->description.constructor) {
        component->description.constructor(entity, data, component->description.user_data);
    }

    while (ParrotArray_size(component->shm_queries) > 0) {
        ParrotSceneWorld_invalidate_cached_query(self, component->shm_queries[0].key);
    }
    ParrotArray_free(component->shm_queries);
}

void *ParrotSceneWorld_get_component_name(ParrotSceneWorld *self, ParrotSceneWorldEntity entity, const char *name) {
    PARROT_RET_COND_V(!ParrotSceneWorld_is_component_registered(self, name), NULL);
    PARROT_RET_COND_V(!ParrotSceneWorld_does_entity_exist(self, entity), NULL);

    ParrotSceneWorldRegisteredComponent *component = ParrotArray_findsp(self->sh_registered_components, name);
    PARROT_FAIL_NULL(component);

    void *data =
        &component->arr_data_blocks[ENTITY_INDEX(entity) / COMPONENT_DATA_BLOCK_SIZE]
                                   [(ENTITY_INDEX(entity) % COMPONENT_DATA_BLOCK_SIZE) * component->description.size];
    return component->arr_exists[ENTITY_INDEX(entity)] ? data : NULL;
}

bool ParrotSceneWorld_is_component_deletion_queued_name(ParrotSceneWorld *self,
                                                        ParrotSceneWorldEntity entity,
                                                        const char *name) {
    PARROT_RET_COND_V(!ParrotSceneWorld_is_component_registered(self, name), false);
    PARROT_RET_COND_V(!ParrotSceneWorld_does_entity_exist(self, entity), false);

    ParrotSceneWorldRegisteredComponent *component = ParrotArray_findsp(self->sh_registered_components, name);
    PARROT_FAIL_NULL(component);

    return component->arr_delete_queued[ENTITY_INDEX(entity)];
}

void ParrotSceneWorld_queue_delete_component_name(ParrotSceneWorld *self,
                                                  ParrotSceneWorldEntity entity,
                                                  const char *name) {
    PARROT_RET_COND(!ParrotSceneWorld_is_component_registered(self, name));
    PARROT_RET_COND(!ParrotSceneWorld_does_entity_exist(self, entity));

    ParrotSceneWorldRegisteredComponent *component = ParrotArray_findsp(self->sh_registered_components, name);
    PARROT_FAIL_NULL(component);

    component->arr_delete_queued[ENTITY_INDEX(entity)] = true;
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

    ParrotSceneWorldCachedQuery cached_query = {
        .key = query_crc32,
    };

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
                if ((component && component->arr_exists[i]) == invert) {
                    ParrotArray_delk(shm_entity_indices, i);
                }
            }

            ParrotCRC32Set **cache = NULL;

            if (!component) {
                if (ParrotArray_find(self->sh_initial_cached_queries, filter->data.component.name) < 0) {
                    ParrotArray_put(self->sh_initial_cached_queries,
                                    ((ParrotSceneWorldInitialCachedQueries){filter->data.component.name, NULL}));
                }
                cache = &((ParrotSceneWorldInitialCachedQueries *)ParrotArray_findsp(self->sh_initial_cached_queries,
                                                                                     filter->data.component.name))
                             ->value;
            } else {
                cache = &component->shm_queries;
            }
            ParrotArray_put(*cache, (ParrotCRC32Set){query_crc32});

            ParrotArray_push(cached_query.arr_components, filter->data.component.name);
        } break;
        case ParrotSceneWorldQueryType_END: {
        } break;
        }
    }

    for (size_t i = 0; i < ParrotArray_size(shm_entity_indices); i++) {
        size_t index = shm_entity_indices[i].key;
        ParrotArray_push(cached_query.arr_results, MAKE_ENTITY(index, self->arr_entity_gens[index]));
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

    return ParrotArray_size(cached_query->arr_results);
}

ParrotSceneWorldEntity
ParrotSceneWorld_query_result_at(ParrotSceneWorld *self, const ParrotSceneWorldQuery *query, size_t idx) {
    PARROT_FAIL_NULL(self);
    PARROT_FAIL_NULL(query);

    ParrotCRC32 query_crc32 = ParrotSceneWorld_query(self, query);

    ParrotSceneWorldCachedQuery *cached_query = ParrotArray_findp(self->hm_queries, query_crc32);
    PARROT_FAIL_NULL(cached_query);

    PARROT_FAIL_COND(idx >= ParrotArray_size(cached_query->arr_results));
    return cached_query->arr_results[idx];
}
