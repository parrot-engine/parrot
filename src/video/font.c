#include "parrot/video/font.h"
#include "parrot/core/util.h"
#include "stb_truetype.h"
#include <stdint.h>
#include <string.h>

struct ParrotVideoFont {
    stbtt_fontinfo font;
};

ParrotVideoFont *ParrotVideoFont_new(ParrotBinaryImage font_data) {
    ParrotVideoFont *self = malloc(sizeof(ParrotVideoFont));
    memset(self, 0, sizeof(*self));

    stbtt_InitFont(&self->font, font_data.data, stbtt_GetFontOffsetForIndex(font_data.data, 0));

    return self;
}

void ParrotVideoFont_delete(ParrotVideoFont *self) {
    PARROT_FAIL_NULL(self);

    free(self);
}

ParrotVideoFontChar ParrotVideoFont_char(ParrotVideoFont *self, float size, char c) {
    PARROT_FAIL_NULL(self);

    ParrotVideoFontChar character = {0};

    float scale = stbtt_ScaleForPixelHeight(&self->font, size);

    uint8_t *bitmap = stbtt_GetCodepointBitmap(
        &self->font, 0, scale, c, &character.width, &character.height, &character.x, &character.y);

    int left_side_bearing;
    stbtt_GetCodepointHMetrics(&self->font, c, &character.advance, &left_side_bearing);
    character.advance *= scale;

    character.bitmap = calloc(character.width * character.height, sizeof(uint8_t));
    memcpy(character.bitmap, bitmap, character.width * character.height * sizeof(uint8_t));

    stbtt_FreeBitmap(bitmap, NULL);

    return character;
}
