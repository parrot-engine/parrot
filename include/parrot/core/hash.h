#ifndef __SRC_PARROT_INCLUDE_PARROT_CORE_HASH_H_
#define __SRC_PARROT_INCLUDE_PARROT_CORE_HASH_H_

#include <stddef.h>
#include <stdint.h>

typedef uint32_t ParrotCRC32;

ParrotCRC32 Parrot_crc32(const void *data, size_t size);

#endif // __SRC_PARROT_INCLUDE_PARROT_CORE_HASH_H_