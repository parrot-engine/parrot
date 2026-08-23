#ifndef PARROT_PARROT_INCLUDE_PARROT_CORE_SCOPE_H_
#define PARROT_PARROT_INCLUDE_PARROT_CORE_SCOPE_H_

#include "parrot/core/api.h"
#include <stdint.h>

typedef struct ParrotScope ParrotScope;

PARROT_API ParrotScope *ParrotScope_new(/* NULL = no parent */ ParrotScope *parent);
PARROT_API void ParrotScope_delete(ParrotScope *self);

PARROT_API uint32_t ParrotScope_push(ParrotScope *self, void (*func)(void *ctx), void *ctx);
/// Pushes a free(ptr) function
PARROT_API uint32_t ParrotScope_push_free(ParrotScope *self, void *ptr);
/// Does not call pushed function. Does nothing on non-existant id
PARROT_API void ParrotScope_cancel(ParrotScope *self, uint32_t id);

#endif // PARROT_PARROT_INCLUDE_PARROT_CORE_SCOPE_H_
