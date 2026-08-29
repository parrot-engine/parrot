#include "src/ds.h"
#include "parrot/core/scope.h"
#include "stb_ds.h"
#include <stdlib.h>

typedef struct {
    void **data;
    size_t element_size;
} STBDSFreeCtx;

static void arrfree_wrapper(void *ctx_ptr) {
    STBDSFreeCtx *ctx = ctx_ptr;
    if (*ctx->data) {
        stbds_arrfreef(*ctx->data);
        *ctx->data = NULL;
    }
}

void ParrotScope_push_arrfree_raw(ParrotScope *self, void **arr, size_t element_size) {
    ParrotScope *scope = ParrotScope_new(self);
    STBDSFreeCtx *ctx = ParrotScope_alloc_ctx(scope, STBDSFreeCtx);
    ctx->data = arr;
    ctx->element_size = element_size;

    ParrotScope_push(scope, arrfree_wrapper, ctx);
}

static void hmfree_wrapper(void *ctx_ptr) {
    STBDSFreeCtx *ctx = ctx_ptr;
    if (*ctx->data) {
        stbds_hmfree_func(((uint8_t *)*ctx->data) - ctx->element_size, ctx->element_size);
        *ctx->data = NULL;
    }
}

void ParrotScope_push_hmfree_raw(ParrotScope *self, void **hm, size_t element_size) {
    ParrotScope *scope = ParrotScope_new(self);
    STBDSFreeCtx *ctx = ParrotScope_alloc_ctx(scope, STBDSFreeCtx);
    ctx->data = hm;
    ctx->element_size = element_size;

    ParrotScope_push(scope, hmfree_wrapper, ctx);
}
