#ifndef __SRC_PARROT_INCLUDE_PARROT_CORE_HASH_H_
#define __SRC_PARROT_INCLUDE_PARROT_CORE_HASH_H_

#include <stddef.h>
#include <stdint.h>

uint32_t Parrot_crc32(const uint8_t *data, size_t size);

#endif // __SRC_PARROT_INCLUDE_PARROT_CORE_HASH_H_