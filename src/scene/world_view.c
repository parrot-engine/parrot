#include "parrot/scene/world_view.h"
#include "parrot/core/util.h"
#include "parrot/scene/world.h"
#include "src/stb_ds.h"
#include <stdlib.h>
#include <string.h>

#define HEADER(n) ((ParrotSceneWorldViewHeader *)(n) - 1)
#define DATA(n) ((ParrotSceneWorldView *)((ParrotSceneWorldViewHeader *)(n) + 1))

typedef struct {
    ParrotSceneWorld *world;
    size_t length;
} ParrotSceneWorldViewHeader;

static size_t _ParrotSceneWorldView_remove_at(ParrotSceneWorldView *self, size_t i);
static void _ParrotSceneWorldView_queue_tree(ParrotSceneWorldView *self, size_t **arr, ParrotSceneWorldEntity entity);

ParrotSceneWorldView *ParrotSceneWorldView_new(ParrotSceneWorld *world) {
    ParrotSceneWorldView *self = ParrotSceneWorldView_new_with_inactive(world);
    PARROT_RET_COND_V(!self, NULL);

    ParrotSceneWorldView_without_component(self, ParrotSceneWorldInactiveTag, true);
    return self;
}

ParrotSceneWorldView *ParrotSceneWorldView_new_with_inactive(ParrotSceneWorld *world) {
    PARROT_FAIL_NULL(world);

    size_t child_count = ParrotSceneWorld_get_entity_child_count(world, ParrotSceneWorldEntity_NULL);

    ParrotSceneWorldViewHeader *header =
        malloc(sizeof(ParrotSceneWorldViewHeader) + sizeof(ParrotSceneWorldEntity) * child_count);
    memset(header, 0, sizeof(ParrotSceneWorldViewHeader));

    header->world = world;
    header->length = child_count;

    ParrotSceneWorldView *data = DATA(header);
    for (size_t i = 0; i < child_count; i++) {
        data[i] = ParrotSceneWorld_get_entity_child(world, ParrotSceneWorldEntity_NULL, i);
    }

    return data;
}

ParrotSceneWorldView *ParrotSceneWorldView_copy(const ParrotSceneWorldView *self) {
    PARROT_FAIL_NULL(self);

    size_t data_size = sizeof(ParrotSceneWorldEntity) * HEADER(self)->length;

    ParrotSceneWorldViewHeader *header = malloc(sizeof(ParrotSceneWorldViewHeader) + data_size);
    memset(header, 0, sizeof(ParrotSceneWorldViewHeader));

    header->world = HEADER(self)->world;
    header->length = HEADER(self)->length;

    memcpy(DATA(header), self, data_size);

    return DATA(header);
}

void ParrotSceneWorldView_delete(ParrotSceneWorldView *self) {
    free(HEADER(self));
}

void ParrotSceneWorldView_with_parent(ParrotSceneWorldView *self, ParrotSceneWorldEntity parent, bool tree) {
    PARROT_FAIL_NULL(self);

    for (size_t i = 0; i < HEADER(self)->length; i++) {
        ParrotSceneWorldEntity current_parent = ParrotSceneWorld_get_entity_parent(HEADER(self)->world, self[i]);
        bool found = false;

        do {
            if (current_parent == parent) {
                found = true;
                break;
            }
            current_parent = ParrotSceneWorld_get_entity_parent(HEADER(self)->world, current_parent);
        } while (ParrotSceneWorld_does_entity_exist(HEADER(self)->world, current_parent) && tree);

        if (found) {
            continue;
        }

        i = _ParrotSceneWorldView_remove_at(self, i);
    }
}

void ParrotSceneWorldView_without_parent(ParrotSceneWorldView *self, ParrotSceneWorldEntity parent, bool tree) {
    PARROT_FAIL_NULL(self);

    for (size_t i = 0; i < HEADER(self)->length; i++) {
        ParrotSceneWorldEntity current_parent = ParrotSceneWorld_get_entity_parent(HEADER(self)->world, self[i]);
        bool found = true;

        do {
            if (current_parent == parent) {
                found = false;
                break;
            }
            current_parent = ParrotSceneWorld_get_entity_parent(HEADER(self)->world, current_parent);
        } while (ParrotSceneWorld_does_entity_exist(HEADER(self)->world, current_parent) && tree);

        if (found) {
            continue;
        }

        i = _ParrotSceneWorldView_remove_at(self, i);
    }
}

void ParrotSceneWorldView_with_component_name(ParrotSceneWorldView *self, const char *name, bool tree) {
    PARROT_FAIL_NULL(self);
    PARROT_FAIL_NULL(name);

    size_t *arr_remove_queue = NULL;
    for (size_t i = 0; i < HEADER(self)->length; i++) {
        if (ParrotSceneWorld_get_entity_component_name(HEADER(self)->world, self[i], name)) {
            continue;
        }

        arrpush(arr_remove_queue, i);
        if (tree) {
            _ParrotSceneWorldView_queue_tree(self, &arr_remove_queue, self[i]);
        }
    }

    for (size_t i = 0; i < arrlen(arr_remove_queue); i++) {
        _ParrotSceneWorldView_remove_at(self, arr_remove_queue[i]);
    }

    arrfree(arr_remove_queue);
}

void ParrotSceneWorldView_without_component_name(ParrotSceneWorldView *self, const char *name, bool tree) {
    PARROT_FAIL_NULL(self);
    PARROT_FAIL_NULL(name);

    size_t *arr_remove_queue = NULL;
    for (size_t i = 0; i < HEADER(self)->length; i++) {
        if (!ParrotSceneWorld_get_entity_component_name(HEADER(self)->world, self[i], name)) {
            continue;
        }

        arrpush(arr_remove_queue, i);
        if (tree) {
            _ParrotSceneWorldView_queue_tree(self, &arr_remove_queue, self[i]);
        }
    }

    for (size_t i = 0; i < arrlen(arr_remove_queue); i++) {
        _ParrotSceneWorldView_remove_at(self, arr_remove_queue[i]);
    }

    arrfree(arr_remove_queue);
}

static size_t _ParrotSceneWorldView_remove_at(ParrotSceneWorldView *self, size_t i) {
    memmove(&self[i], &self[i + 1], (HEADER(self)->length - i - 1) * sizeof(ParrotSceneWorldView));
    HEADER(self)->length--;
    return i - 1;
}

static void _ParrotSceneWorldView_queue_tree(ParrotSceneWorldView *self, size_t **arr, ParrotSceneWorldEntity entity) {
    for (size_t i = 0; i < ParrotSceneWorld_get_entity_child_count(HEADER(self)->world, entity); i++) {
        ParrotSceneWorldEntity child = ParrotSceneWorld_get_entity_child(HEADER(self)->world, entity, i);
        for (size_t j = 0; j < HEADER(self)->length; j++) {
            if (self[j] != child) {
                continue;
            }

            arrpush(*arr, j);
            _ParrotSceneWorldView_queue_tree(self, arr, child);
            break;
        }
    }
}