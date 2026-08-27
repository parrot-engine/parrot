#ifndef PARROT_PARROT_INCLUDE_PARROT_CORE_BUFFER_H_
#define PARROT_PARROT_INCLUDE_PARROT_CORE_BUFFER_H_

#include "parrot/core/api.h"
#include "parrot/core/scope.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef bool (*ParrotBufferRead)(ParrotScope *scope, size_t offset, uint8_t *out);
typedef void (*ParrotBufferWrite)(ParrotScope *scope, uint8_t data);

typedef enum {
    ParrotBufferEndian_HOST = 0,
    ParrotBufferEndian_BIG,
    ParrotBufferEndian_LITTLE,
} ParrotBufferEndian;

typedef struct ParrotBuffer ParrotBuffer;

PARROT_API ParrotBuffer *ParrotBuffer_new(/* Auto-deleted at end if not NULL */ ParrotScope *scope,
                                          /* NULL = no read */ ParrotBufferRead read,
                                          /* NULL = no write */ ParrotBufferWrite write);
PARROT_API ParrotBuffer *ParrotBuffer_new_bytearray(/* Auto-deleted at end if not NULL */ ParrotScope *scope,
                                                    const void *data,
                                                    size_t size,
                                                    /* NULL = no write */ ParrotBufferWrite write);
PARROT_API void ParrotBuffer_delete(ParrotBuffer *self);

PARROT_API void ParrotBuffer_set_endian(ParrotBuffer *self, ParrotBufferEndian endian);
PARROT_API ParrotBufferEndian ParrotBuffer_get_endian(ParrotBuffer *self);

PARROT_API void ParrotBuffer_rseek(ParrotBuffer *self, size_t position);
PARROT_API size_t ParrotBuffer_rtell(ParrotBuffer *self);
PARROT_API void ParrotBuffer_pad(ParrotBuffer *self, uint8_t data, size_t count);
PARROT_API void ParrotBuffer_pad_until(ParrotBuffer *self, uint8_t data, size_t until_position);

PARROT_API size_t ParrotBuffer_read(ParrotBuffer *self, void *out, size_t size);
PARROT_API void ParrotBuffer_write(ParrotBuffer *self, const void *data, size_t size);

PARROT_API bool ParrotBuffer_read8(ParrotBuffer *self, uint8_t *out);
PARROT_API bool ParrotBuffer_read16(ParrotBuffer *self, uint16_t *out);
PARROT_API bool ParrotBuffer_read32(ParrotBuffer *self, uint32_t *out);
PARROT_API bool ParrotBuffer_read64(ParrotBuffer *self, uint64_t *out);

PARROT_API void ParrotBuffer_write8(ParrotBuffer *self, uint8_t data);
PARROT_API void ParrotBuffer_write16(ParrotBuffer *self, uint16_t data);
PARROT_API void ParrotBuffer_write32(ParrotBuffer *self, uint32_t data);
PARROT_API void ParrotBuffer_write64(ParrotBuffer *self, uint64_t data);

PARROT_API bool ParrotBuffer_read8s(ParrotBuffer *self, int8_t *out);
PARROT_API bool ParrotBuffer_read16s(ParrotBuffer *self, int16_t *out);
PARROT_API bool ParrotBuffer_read32s(ParrotBuffer *self, int32_t *out);
PARROT_API bool ParrotBuffer_read64s(ParrotBuffer *self, int64_t *out);

PARROT_API void ParrotBuffer_write8s(ParrotBuffer *self, int8_t data);
PARROT_API void ParrotBuffer_write16s(ParrotBuffer *self, int16_t data);
PARROT_API void ParrotBuffer_write32s(ParrotBuffer *self, int32_t data);
PARROT_API void ParrotBuffer_write64s(ParrotBuffer *self, int64_t data);

PARROT_API void ParrotBuffer_write_ascii(ParrotBuffer *self, const char *str);

#endif // PARROT_PARROT_INCLUDE_PARROT_CORE_BUFFER_H_
