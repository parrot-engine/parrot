#ifndef PARROT_PARROT_SRC_DS_H_
#define PARROT_PARROT_SRC_DS_H_

#include "parrot/core/hash.h"
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

#endif // PARROT_PARROT_SRC_DS_H_
