#include "parrot/video/font.h"
#include "parrot/core/util.h"
#include "stb_truetype.h"
#include <stdint.h>
#include <string.h>

struct ParrotVideoFont {
    stbtt_fontinfo font;
};

extern const uint8_t ParrotVideoFont_default_jupiteroid_font[];
extern const uint32_t ParrotVideoFont_default_jupiteroid_font_len;

ParrotVideoFont *ParrotVideoFont_new(ParrotBinaryImage font_data) {
    ParrotVideoFont *self = PARROT_ALLOC(ParrotVideoFont);

    stbtt_InitFont(&self->font, font_data.data, stbtt_GetFontOffsetForIndex(font_data.data, 0));

    return self;
}

ParrotVideoFont *ParrotVideoFont_new_default(ParrotVideoFontDefaultStyle style) {
    switch (style) {
    case ParrotVideoFontDefaultStyle_MODERN:
        return ParrotVideoFont_new((ParrotBinaryImage){
            .data = ParrotVideoFont_default_jupiteroid_font,
            .size = ParrotVideoFont_default_jupiteroid_font_len,
        });
    }
    return NULL;
}

void ParrotVideoFont_delete(ParrotVideoFont *self) {
    PARROT_FAIL_NULL(self);

    free(self);
}

void ParrotVideoFont_vdelete(void *self) {
    ParrotVideoFont_delete((ParrotVideoFont *)self);
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
    if (bitmap) {
        memcpy(character.bitmap, bitmap, character.width * character.height * sizeof(uint8_t));
    }

    stbtt_FreeBitmap(bitmap, NULL);

    return character;
}

ParrotVec2 ParrotVideoFont_measure_text(ParrotVideoFont *self, float size, const char *text) {
    ParrotVec2 position = ParrotVec2_n(0);

    float max_character_height = 0;
    for (size_t i = 0; i < strlen(text); i++) {
        char c = text[i];

        switch (c) {
        case '\n': {
            position.y += max_character_height;
            position.x = 0;

            max_character_height = 0;
            continue;
        }
        default:
            break;
        }

        ParrotVideoFontChar character = ParrotVideoFont_char(self, size, c);
        free(character.bitmap);

        max_character_height = PARROT_MAX(max_character_height, character.height);
        position.x += character.advance;
    }

    return position;
}
