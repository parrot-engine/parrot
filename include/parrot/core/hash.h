#ifndef __SRC_PARROT_INCLUDE_PARROT_CORE_HASH_H_
#define __SRC_PARROT_INCLUDE_PARROT_CORE_HASH_H_

#include "parrot/core/api.h"
#include <stddef.h>
#include <stdint.h>

typedef uint32_t ParrotCRC32;

PARROT_API ParrotCRC32 Parrot_crc32(const void *data, size_t size);
PARROT_API ParrotCRC32 Parrot_crc32_combine(ParrotCRC32 crc, const void *data, size_t size);

#endif // __SRC_PARROT_INCLUDE_PARROT_CORE_HASH_H_
