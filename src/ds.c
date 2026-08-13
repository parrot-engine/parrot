#include "src/ds.h"
#include "stb_ds.h"

void Parrot_free_scope_wrapper(void *ctx) {
    free(ctx);
}

void Parrot_arrfree_scope_wrapper(void *ctx) {
    arrfree(*(void **)ctx);
}

typedef struct {
    char *key;
} HashDummy;

void Parrot_shfree_scope_wrapper(void *ctx) {
    HashDummy *dummy = *(void **)ctx;
    shfree(dummy);
}

void Parrot_hmfree_scope_wrapper(void *ctx) {
    HashDummy *dummy = *(void **)ctx;
    hmfree(dummy);
}
