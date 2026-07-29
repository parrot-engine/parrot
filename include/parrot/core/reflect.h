#ifndef __SRC_PARROT_INCLUDE_PARROT_CORE_REFLECT_H_
#define __SRC_PARROT_INCLUDE_PARROT_CORE_REFLECT_H_

#include "parrot/core/api.h"
#include <stddef.h>
#include <stdint.h>

typedef struct ParrotReflect ParrotReflect;

PARROT_API ParrotReflect *ParrotReflect_new(void);
PARROT_API void ParrotReflect_delete(ParrotReflect *self);

#define ParrotReflect_register_struct(self, type, size)                                                                 \
    ParrotReflect_register_struct_manual(self, PARROT_TYPE_STRING(type), sizeof(type))
PARROT_API void ParrotReflect_register_struct_manual(ParrotReflect *self, const char *type, size_t size);

#define ParrotReflect_register_field(self, type, field_type, field)                                                     \
    ParrotReflect_register_field_manual(self, PARROT_TYPE_STRING(type), field_type, field, offsetof(type, field))
PARROT_API void ParrotReflect_register_field_manual(
    ParrotReflect *self, const char *type, const char *field_type, const char *field, size_t offset);

#endif // __SRC_PARROT_INCLUDE_PARROT_CORE_REFLECT_H_