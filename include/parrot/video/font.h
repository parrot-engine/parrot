#ifndef PARROT_PARROT_INCLUDE_PARROT_VIDEO_FONT_H_
#define PARROT_PARROT_INCLUDE_PARROT_VIDEO_FONT_H_

#include "parrot/core/api.h"
#include "parrot/core/file.h"
#include "parrot/core/math.h"

typedef struct {
    int width;
    int height;

    int x;
    int y;

    int advance;

    /// an array of 8-bit alphas allocated with malloc() which must be freed
    uint8_t *bitmap;
} ParrotVideoFontChar;

typedef enum {
    ParrotVideoFontDefaultStyle_MODERN = 0,
} ParrotVideoFontDefaultStyle;

typedef struct ParrotVideoFont ParrotVideoFont;

PARROT_API ParrotVideoFont *ParrotVideoFont_new(ParrotBinaryImage font_data);
PARROT_API ParrotVideoFont *ParrotVideoFont_new_default(ParrotVideoFontDefaultStyle style);
PARROT_API void ParrotVideoFont_delete(ParrotVideoFont *self);
PARROT_API void ParrotVideoFont_vdelete(void *self);

PARROT_API ParrotVideoFontChar ParrotVideoFont_char(ParrotVideoFont *self, float size, char c);
PARROT_API ParrotVec2 ParrotVideoFont_measure_text(ParrotVideoFont *self, float size, const char *text);

#endif // PARROT_PARROT_INCLUDE_PARROT_VIDEO_FONT_H_
