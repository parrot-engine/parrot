#ifndef PARROT_PARROT_INCLUDE_PARROT_CORE_FILE_H_
#define PARROT_PARROT_INCLUDE_PARROT_CORE_FILE_H_

#include "parrot/core/reflect.h"
#include <stddef.h>
#include <stdint.h>

typedef struct {
    const uint8_t *data;
    size_t size;
} ParrotBinaryImage;

static const ParrotReflectDescription ParrotBinaryImage_description[] = {
    PARROT_REFLECT_TYPE_HEADER(ParrotBinaryImage),

    PARROT_REFLECT_TYPE_FIELD(ParrotBinaryImage, const uint8_t *, data, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotBinaryImage, size_t, size, ),

    PARROT_REFLECT_END(),
};

typedef struct {
    uint8_t *data;
    size_t size;
} ParrotMutableBinaryImage;

static const ParrotReflectDescription ParrotMutableBinaryImage_description[] = {
    PARROT_REFLECT_TYPE_HEADER(ParrotMutableBinaryImage),

    PARROT_REFLECT_TYPE_FIELD(ParrotMutableBinaryImage, const uint8_t *, data, ),
    PARROT_REFLECT_TYPE_FIELD(ParrotMutableBinaryImage, size_t, size, ),

    PARROT_REFLECT_END(),
};

static const ParrotReflectDescription Parrot_core_file_collection[] = {
    PARROT_REFLECT_COLLECTION_HEADER(),

    PARROT_REFLECT_COLLECTION_DESCRIPTION(ParrotBinaryImage_description),
    PARROT_REFLECT_COLLECTION_DESCRIPTION(ParrotMutableBinaryImage_description),

    PARROT_REFLECT_END(),
};

#endif // PARROT_PARROT_INCLUDE_PARROT_CORE_FILE_H_
