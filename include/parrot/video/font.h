#ifndef PARROT_PARROT_INCLUDE_PARROT_VIDEO_FONT_H_
#define PARROT_PARROT_INCLUDE_PARROT_VIDEO_FONT_H_

#include "parrot/core/api.h"
#include "parrot/core/file.h"

typedef struct {
    int width;
    int height;

    int x;
    int y;

    int advance;

    /// an array of 8-bit alphas allocated with malloc()
    uint8_t *bitmap;
} ParrotVideoFontChar;

typedef struct ParrotVideoFont ParrotVideoFont;

PARROT_API ParrotVideoFont *ParrotVideoFont_new(ParrotBinaryImage font_data);
PARROT_API void ParrotVideoFont_delete(ParrotVideoFont *self);

PARROT_API ParrotVideoFontChar ParrotVideoFont_char(ParrotVideoFont *self, float size, char c);

#endif // PARROT_PARROT_INCLUDE_PARROT_VIDEO_FONT_H_
