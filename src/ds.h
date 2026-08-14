#ifndef __SRC_PARROT_SRC_DS_H_
#define __SRC_PARROT_SRC_DS_H_

#include "parrot/core/hash.h"
#include <stddef.h>

typedef struct {
    size_t key;
} ParrotSizeSet;

typedef struct {
    ParrotCRC32 key;
} ParrotCRC32Set;

typedef struct {
    void **data;
    size_t element_size;
} ParrotSTBDSFreeCtx;

void Parrot_free_scope_wrapper(void *ctx);

#define Parrot_stbds_free_scope_wrapper_PACK_CTX(container)                                                             \
    memcpy(malloc(sizeof(ParrotSTBDSFreeCtx)),                                                                          \
           &((ParrotSTBDSFreeCtx){.data = (void **)&(container), .element_size = sizeof(*(container))}),                \
           sizeof(ParrotSTBDSFreeCtx))

#define Parrot_arrfree_scope_wrapper_PACK_CTX Parrot_stbds_free_scope_wrapper_PACK_CTX
#define Parrot_shfree_scope_wrapper_PACK_CTX Parrot_stbds_free_scope_wrapper_PACK_CTX
#define Parrot_hmfree_scope_wrapper_PACK_CTX Parrot_stbds_free_scope_wrapper_PACK_CTX

void Parrot_arrfree_scope_wrapper(void *ctx);
#define Parrot_shfree_scope_wrapper Parrot_hmfree_scope_wrapper
void Parrot_hmfree_scope_wrapper(void *ctx);

#endif // __SRC_PARROT_SRC_DS_H_
