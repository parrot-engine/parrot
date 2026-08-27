#ifndef PARROT_PARROT_INCLUDE_PARROT_CORE_BINARY_H_
#define PARROT_PARROT_INCLUDE_PARROT_CORE_BINARY_H_

#include "parrot/core/api.h"
#include "parrot/core/reflect.h"
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t *data;
    size_t size;
} ParrotMutableBinaryImage;

static const ParrotReflectDescription ParrotMutableBinaryImage_description[] = {
    PARROT_REFLECT_TYPE_HEADER(ParrotMutableBinaryImage),

    PARROT_REFLECT_TYPE_FIELD(ParrotMutableBinaryImage, uint8_t *, data, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotMutableBinaryImage, size_t, size, ),

    PARROT_REFLECT_END(),
};

typedef struct {
    const uint8_t *data;
    size_t size;
} ParrotBinaryImage;

PARROT_API ParrotBinaryImage ParrotBinaryImage_from_mutable(ParrotMutableBinaryImage image);

static const ParrotReflectDescription ParrotBinaryImage_description[] = {
    PARROT_REFLECT_TYPE_HEADER(ParrotBinaryImage),

    PARROT_REFLECT_TYPE_FIELD(ParrotBinaryImage, const uint8_t *, data, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotBinaryImage, size_t, size, ),

    PARROT_REFLECT_END(),
};

static const ParrotReflectDescription Parrot_core_binary_collection[] = {
    PARROT_REFLECT_COLLECTION_HEADER(),

    PARROT_REFLECT_COLLECTION_DESCRIPTION(ParrotMutableBinaryImage_description),
    PARROT_REFLECT_COLLECTION_DESCRIPTION(ParrotBinaryImage_description),

    PARROT_REFLECT_END(),
};

/**
 * The C99 standard does not guarntee ASCII repersentation of `char`. While most platforms do use ASCII for `char`, it
 * is not guarnteed.
 */
PARROT_API uint8_t Parrot_char_to_ascii(char c);
/**
 * The C99 standard does not guarntee ASCII repersentation of `char`. While most platforms do use ASCII for `char`, it
 * is not guarnteed.
 */
PARROT_API char Parrot_ascii_to_char(uint8_t ascii);

#endif // PARROT_PARROT_INCLUDE_PARROT_CORE_BINARY_H_
