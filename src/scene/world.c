#include "parrot/scene/world.h"
#include "parrot/core/hash.h"
#include "parrot/core/math.h"
#include "parrot/core/util.h"
#include "src/ds.h"
#include "stb_ds.h"
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

    uint8_t *arr_data;

    bool *arr_exists;

    ParrotSceneWorldComponentDescription description;

    ParrotCRC32Set *shm_queries;
} ParrotSceneWorldRegisteredComponent;

struct ParrotSceneWorld {
    ParrotSceneWorldRegisteredComponent *sh_registered_components;

    ParrotSceneWorldEntity *arr_entity_parents;

    bool *arr_entity_exists;
    uint32_t *arr_entity_gens;

    uint32_t *arr_free_indices;
    uint32_t next_new_index;

    ParrotSceneWorldCachedQuery *hm_queries;
    ParrotSceneWorldInitialCachedQueries *sh_initial_cached_queries;
    ParrotCRC32 *arr_entity_tree_queries;
};

static void ParrotSceneWorld_invalidate_cached_query(ParrotSceneWorld *self, ParrotCRC32 query_crc32) {
    ParrotSceneWorldCachedQuery *cached_query = hmgetp_null(self->hm_queries, query_crc32);
    PARROT_RET_COND(!cached_query);

    arrfree(cached_query->arr_results);
    arrfree(cached_query->arr_components);

    hmdel(self->hm_queries, query_crc32);
}

static void ParrotSceneWorld_invalidate_tree_queries(ParrotSceneWorld *self) {
    for (size_t i = 0; i < arrlen(self->arr_entity_tree_queries); i++) {
        ParrotSceneWorld_invalidate_cached_query(self, self->arr_entity_tree_queries[i]);
    }
    arrfree(self->arr_entity_tree_queries);
}

static void ParrotSceneWorldRegisteredComponent_min_size(ParrotSceneWorldRegisteredComponent *self, size_t size) {
    if (self->description.size * size > arrlen(self->arr_data)) {
        arrsetlen(self->arr_data, self->description.size * size);
    }

    MIN_ARR_SIZE_VALUE(self->arr_exists, size, false);
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
    ParrotSceneWorld *self = malloc(sizeof(ParrotSceneWorld));
    PARROT_RET_COND_V(!self, NULL);
    memset(self, 0, sizeof(ParrotSceneWorld));

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

    while (hmlen(self->hm_queries) > 0) {
        ParrotSceneWorld_invalidate_cached_query(self, self->hm_queries[0].key);
    }

    for (size_t i = 0; i < arrlen(self->arr_entity_gens); i++) {
        ParrotSceneWorldEntity entity = MAKE_ENTITY(i, self->arr_entity_gens[i]);
        ParrotSceneWorld_delete_entity(self, entity);
    }

    while (shlen(self->sh_registered_components) > 0) {
        ParrotSceneWorld_unregister_component(self, self->sh_registered_components[0].key);
    }
    shfree(self->sh_registered_components);

    arrfree(self->arr_entity_parents);

    arrfree(self->arr_entity_exists);
    arrfree(self->arr_entity_gens);

    arrfree(self->arr_free_indices);

    shfree(self->hm_queries);
    arrfree(self->arr_entity_tree_queries);

    while (shlen(self->sh_initial_cached_queries) > 0) {
        hmfree(self->sh_initial_cached_queries[0].value);
        shdel(self->sh_initial_cached_queries, self->sh_initial_cached_queries[0].key);
    }
    shfree(self->sh_initial_cached_queries);

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

    MIN_ARR_SIZE_VALUE(self->arr_entity_exists, index + 1, false);
    MIN_ARR_SIZE_VALUE(self->arr_entity_gens, index + 1, 0);

    for (size_t i = 0; i < shlen(self->sh_registered_components); i++) {
        ParrotSceneWorldRegisteredComponent_min_size(&self->sh_registered_components[i], index + 1);

        self->sh_registered_components[i].arr_exists[index] = false;
    }

    self->arr_entity_exists[index] = true;

    ParrotSceneWorld_invalidate_tree_queries(self);

    return MAKE_ENTITY(index, self->arr_entity_gens[index]);
}

void ParrotSceneWorld_delete_entity(ParrotSceneWorld *self, ParrotSceneWorldEntity entity) {
    PARROT_RET_COND(!ParrotSceneWorld_does_entity_exist(self, entity));

    ParrotSceneWorldQuery child_query[] = {
        PARROT_SCENE_WORLD_QUERY_WITH_PARENT(entity),
        PARROT_SCENE_WORLD_QUERY_END(),
    };
    for (size_t i = ParrotSceneWorld_query_result_count(self, child_query); i > 0; i--) {
        ParrotSceneWorld_delete_entity(self, ParrotSceneWorld_query_result_at(self, child_query, i - 1));
    }

    for (size_t i = 0; i < shlen(self->sh_registered_components); i++) {
        ParrotSceneWorldRegisteredComponent *component = &self->sh_registered_components[i];
        if (component->arr_exists && component->arr_exists[ENTITY_INDEX(entity)]) {
            ParrotSceneWorld_delete_component_name(self, entity, component->key);
        }
    }

    ParrotSceneWorld_set_entity_parent(self, entity, ParrotSceneWorldEntity_NULL);

    self->arr_entity_exists[ENTITY_INDEX(entity)] = false;

    ParrotSceneWorld_invalidate_tree_queries(self);
    arrpush(self->arr_free_indices, ENTITY_INDEX(entity));
}

bool ParrotSceneWorld_does_entity_exist(ParrotSceneWorld *self, ParrotSceneWorldEntity entity) {
    PARROT_FAIL_NULL(self);
    PARROT_RET_COND_V(ENTITY_INDEX(entity) >= arrlen(self->arr_entity_exists), false);

    return self->arr_entity_exists[ENTITY_INDEX(entity)] &&
           self->arr_entity_gens[ENTITY_INDEX(entity)] == ENTITY_GEN(entity);
}

size_t ParrotSceneWorld_get_entity_count(ParrotSceneWorld *self) {
    PARROT_FAIL_NULL(self);

    return arrlen(self->arr_entity_gens);
}

ParrotSceneWorldEntity ParrotSceneWorld_get_entity(ParrotSceneWorld *self, size_t index) {
    PARROT_FAIL_NULL(self);
    PARROT_FAIL_COND_MSG(index >= arrlen(self->arr_entity_gens), "Attempted to access out of bounds index in children");

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

    component.key = calloc(strlen(name) + 1, sizeof(char));
    strcpy(component.key, name);

    component.description = description;

    ParrotSceneWorldRegisteredComponent_min_size(&component, self->next_new_index);

    ParrotSceneWorldInitialCachedQueries *initial_cached_query = hmgetp_null(self->sh_initial_cached_queries, name);
    if (initial_cached_query) {
        component.shm_queries = initial_cached_query->value;
        hmdel(self->sh_initial_cached_queries, initial_cached_query->key);
    }

    shputs(self->sh_registered_components, component);
}

void ParrotSceneWorld_unregister_component(ParrotSceneWorld *self, const char *name) {
    PARROT_FAIL_COND(!ParrotSceneWorld_is_component_registered(self, name));

    ParrotSceneWorldRegisteredComponent *component = shgetp_null(self->sh_registered_components, name);
    PARROT_FAIL_NULL(component);

    for (size_t i = 0; i < arrlen(component->arr_exists); i++) {
        if (component->arr_exists[i]) {
            ParrotSceneWorld_delete_component_name(self, MAKE_ENTITY(i, self->arr_entity_gens[i]), component->key);
        }
    }

    {
        arrfree(component->arr_data);

        arrfree(component->arr_exists);
    }

    hmfree(component->shm_queries);

    char *key = component->key;

    shdel(self->sh_registered_components, component->key);

    free(key);
}

bool ParrotSceneWorld_is_component_registered(ParrotSceneWorld *self, const char *name) {
    PARROT_FAIL_NULL(self);
    PARROT_FAIL_NULL(name);

    return shgeti(self->sh_registered_components, name) >= 0;
}

void ParrotSceneWorld_add_component_name(ParrotSceneWorld *self, ParrotSceneWorldEntity entity, const char *name) {
    PARROT_FAIL_COND(!ParrotSceneWorld_is_component_registered(self, name));
    PARROT_FAIL_COND(!ParrotSceneWorld_does_entity_exist(self, entity));

    ParrotSceneWorldRegisteredComponent *component = shgetp_null(self->sh_registered_components, name);
    PARROT_FAIL_NULL(component);

    void *data = &component->arr_data[component->description.size * ENTITY_INDEX(entity)];
    memset(data, 0, component->description.size);

    component->arr_exists[ENTITY_INDEX(entity)] = true;

    if (component->description.constructor) {
        component->description.constructor(entity, data, component->description.user_data);
    }

    while (hmlen(component->shm_queries) > 0) {
        ParrotSceneWorld_invalidate_cached_query(self, component->shm_queries[0].key);
    }
    hmfree(component->shm_queries);
}

void *ParrotSceneWorld_get_component_name(ParrotSceneWorld *self, ParrotSceneWorldEntity entity, const char *name) {
    PARROT_RET_COND_V(!ParrotSceneWorld_is_component_registered(self, name), NULL);
    PARROT_RET_COND_V(!ParrotSceneWorld_does_entity_exist(self, entity), NULL);

    ParrotSceneWorldRegisteredComponent *component = shgetp_null(self->sh_registered_components, name);
    PARROT_FAIL_NULL(component);

    return component->arr_exists[ENTITY_INDEX(entity)] ?
               &component->arr_data[component->description.size * ENTITY_INDEX(entity)] :
               NULL;
}

void ParrotSceneWorld_delete_component_name(ParrotSceneWorld *self, ParrotSceneWorldEntity entity, const char *name) {
    PARROT_RET_COND(!ParrotSceneWorld_is_component_registered(self, name));
    PARROT_RET_COND(!ParrotSceneWorld_does_entity_exist(self, entity));

    ParrotSceneWorldRegisteredComponent *component = shgetp_null(self->sh_registered_components, name);
    PARROT_FAIL_NULL(component);

    if (component->description.destructor) {
        component->description.destructor(entity,
                                          &component->arr_data[component->description.size * ENTITY_INDEX(entity)],
                                          component->description.user_data);
    }

    component->arr_exists[ENTITY_INDEX(entity)] = false;

    while (hmlen(component->shm_queries) > 0) {
        ParrotSceneWorld_invalidate_cached_query(self, component->shm_queries[0].key);
    }
    hmfree(component->shm_queries);
}

static ParrotCRC32 ParrotSceneWorld_query(ParrotSceneWorld *self, const ParrotSceneWorldQuery *query) {
    size_t count = 0;
    for (count = 0; query[count].type != ParrotSceneWorldQueryType_END; count++)
        ;

    ParrotCRC32 query_crc32 = Parrot_crc32(query, count * sizeof(ParrotSceneWorldQuery));
    PARROT_RET_COND_V(hmgeti(self->hm_queries, query_crc32) >= 0, query_crc32);

    ParrotSceneWorldCachedQuery cached_query = {
        .key = query_crc32,
    };

    ParrotSizeSet *shm_entity_indices = NULL;
    for (size_t i = 0; i < arrlen(self->arr_entity_exists); i++) {
        if (self->arr_entity_exists[i]) {
            hmputs(shm_entity_indices, (ParrotSizeSet){i});
        }
    }

    bool invert = false;

    for (const ParrotSceneWorldQuery *filter = query; filter->type != ParrotSceneWorldQueryType_END; filter++) {
        switch (filter->type) {
        case ParrotSceneWorldQueryType_SET_INVERT:
            invert = filter->data.invert;
            break;
        case ParrotSceneWorldQueryType_PARENT: {
            for (size_t i = 0; i < arrlen(self->arr_entity_gens); i++) {
                if ((self->arr_entity_parents[i] == filter->data.parent.entity) == invert) {
                    hmdel(shm_entity_indices, i);
                }
            }

            arrpush(self->arr_entity_tree_queries, query_crc32);
        } break;
        case ParrotSceneWorldQueryType_COMPONENT: {
            ParrotSceneWorldRegisteredComponent *component =
                shgetp_null(self->sh_registered_components, filter->data.component.name);

            for (size_t i = 0; i < arrlen(self->arr_entity_gens); i++) {
                if ((component && component->arr_exists[i]) == invert) {
                    hmdel(shm_entity_indices, i);
                }
            }

            ParrotCRC32Set *cache = NULL;

            if (!component) {
                if (hmgeti(self->sh_initial_cached_queries, filter->data.component.name) < 0) {
                    hmput(self->sh_initial_cached_queries, filter->data.component.name, NULL);
                }
                cache = shgetp(self->sh_initial_cached_queries, filter->data.component.name)->value;
            } else {
                cache = component->shm_queries;
            }
            hmputs(cache, (ParrotCRC32Set){query_crc32});

            arrpush(cached_query.arr_components, filter->data.component.name);
        } break;
        case ParrotSceneWorldQueryType_END: {
        } break;
        }
    }

    for (size_t i = 0; i < hmlen(shm_entity_indices); i++) {
        size_t index = shm_entity_indices[i].key;
        arrpush(cached_query.arr_results, MAKE_ENTITY(index, self->arr_entity_gens[index]));
    }

    hmputs(self->hm_queries, cached_query);

    hmfree(shm_entity_indices);
    return query_crc32;
}

size_t ParrotSceneWorld_query_result_count(ParrotSceneWorld *self, const ParrotSceneWorldQuery *query) {
    PARROT_FAIL_NULL(self);
    PARROT_FAIL_NULL(query);

    ParrotCRC32 query_crc32 = ParrotSceneWorld_query(self, query);

    ParrotSceneWorldCachedQuery *cached_query = hmgetp_null(self->hm_queries, query_crc32);
    PARROT_FAIL_NULL(cached_query);

    return arrlen(cached_query->arr_results);
}

ParrotSceneWorldEntity
ParrotSceneWorld_query_result_at(ParrotSceneWorld *self, const ParrotSceneWorldQuery *query, size_t idx) {
    PARROT_FAIL_NULL(self);
    PARROT_FAIL_NULL(query);

    ParrotCRC32 query_crc32 = ParrotSceneWorld_query(self, query);

    ParrotSceneWorldCachedQuery *cached_query = hmgetp_null(self->hm_queries, query_crc32);
    PARROT_FAIL_NULL(cached_query);

    PARROT_FAIL_COND(idx >= arrlen(cached_query->arr_results));
    return cached_query->arr_results[idx];
}
