#include "parrot/core/scope.h"
#include "parrot/core/util.h"
#include "stb_ds.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint32_t key;

    void (*func)(void *ctx);
    void *ctx;
} ParrotScopeEntry;

struct ParrotScope {
    ParrotScopeEntry *hm_stack;
    uint32_t next_id;

    ParrotScope *parent;
    uint32_t parent_delete_id;
};

static void scope_delete_wrapper(void *ctx) {
    ParrotScope_delete((ParrotScope *)ctx);
}

ParrotScope *ParrotScope_new(ParrotScope *parent) {
    ParrotScope *self = malloc(sizeof(ParrotScope));
    memset(self, 0, sizeof(ParrotScope));

    if (parent) {
        self->parent = parent;
        self->parent_delete_id = ParrotScope_push(parent, scope_delete_wrapper, self);
    }

    return self;
}

void ParrotScope_delete(ParrotScope *self) {
    PARROT_FAIL_NULL(self);

    if (self->parent) {
        ParrotScope_cancel(self->parent, self->parent_delete_id);
    }

    while (hmlen(self->hm_stack) > 0) {
        size_t index = hmlen(self->hm_stack) - 1;
        ParrotScopeEntry entry = self->hm_stack[index];
        hmdel(self->hm_stack, index);

        entry.func(entry.ctx);
    }

    hmfree(self->hm_stack);
    free(self);
}

uint32_t ParrotScope_push(ParrotScope *self, void (*func)(void *ctx), void *ctx) {
    PARROT_FAIL_NULL(self);

    ParrotScopeEntry entry = {0};
    entry.key = self->next_id++;
    entry.func = func;
    entry.ctx = ctx;

    hmputs(self->hm_stack, entry);
    return entry.key;
}

void ParrotScope_cancel(ParrotScope *self, uint32_t id) {
    PARROT_FAIL_NULL(self);

    hmdel(self->hm_stack, id);
}
