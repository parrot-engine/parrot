#ifndef PARROT_PARROT_INCLUDE_PARROT_CORE_SERIALIZE_H_
#define PARROT_PARROT_INCLUDE_PARROT_CORE_SERIALIZE_H_

#include "parrot/core/buffer.h"
#include "parrot/core/file.h"
#include <stddef.h>
#include <stdint.h>

PARROT_API void
Parrot_serialize_bytes(ParrotBuffer *output, ParrotReflect *reflect, size_t type, const void *data, bool with_ptrs);
/// Return type is allocated with malloc() that the caller takes ownership of or NULL if could not be deserialized
PARROT_API void *Parrot_deserialize_bytes(ParrotBuffer *input, ParrotReflect *reflect, size_t *out_type, bool with_ptrs);

#endif // PARROT_PARROT_INCLUDE_PARROT_CORE_SERIALIZE_H_
