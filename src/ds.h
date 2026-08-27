#ifndef __SRC_PARROT_SRC_DS_H_
#define __SRC_PARROT_SRC_DS_H_

#include "parrot/core/hash.h"
#include "parrot/core/scope.h"
#include <stddef.h>

typedef struct {
    const void *key;
} ParrotPtrSet;

typedef struct {
    size_t key;
} ParrotSizeSet;

typedef struct {
    ParrotCRC32 key;
} ParrotCRC32Set;

#define ParrotScope_push_arrfree(p_self, p_arr) ParrotScope_push_arrfree_raw(p_self, (void **)&(p_arr), sizeof(*(p_arr)))
void ParrotScope_push_arrfree_raw(ParrotScope *self, void **arr, size_t element_size);

#define ParrotScope_push_hmfree(p_self, p_hm) ParrotScope_push_hmfree_raw(p_self, (void **)&(p_hm), sizeof(*(p_hm)))
void ParrotScope_push_hmfree_raw(ParrotScope *self, void **hm, size_t element_size);

#define ParrotScope_push_shfree_raw ParrotScope_push_hmfree_raw
#define ParrotScope_push_shfree ParrotScope_push_hmfree

#endif // __SRC_PARROT_SRC_DS_H_
