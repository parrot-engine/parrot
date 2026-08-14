#include "src/ds.h"
#include "stb_ds.h"

void Parrot_free_scope_wrapper(void *ctx) {
    free(ctx);
}

void Parrot_arrfree_scope_wrapper(void *ctx_ptr) {
    ParrotSTBDSFreeCtx *ctx = ctx_ptr;
    if (*ctx->data) {
        stbds_arrfreef(*ctx->data);
    }
}

void Parrot_hmfree_scope_wrapper(void *ctx_ptr) {
    ParrotSTBDSFreeCtx *ctx = ctx_ptr;
    if (*ctx->data) {
        stbds_hmfree_func(((uint8_t *)*ctx->data) - ctx->element_size, ctx->element_size);
    }
}
