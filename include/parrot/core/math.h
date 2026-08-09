#ifndef __SRC_PARROT_INCLUDE_PARROT_CORE_MATH_H_
#define __SRC_PARROT_INCLUDE_PARROT_CORE_MATH_H_

#include "parrot/config.h" // IWYU pragma: keep
#include "parrot/core/api.h"
#include <stddef.h>
#include <stdint.h>

/// 24.8 fixed point
typedef int32_t ParrotFixed32i;
#define ParrotFixed32i_MIN (-8388607)
#define ParrotFixed32i_MAX (8388607)
#define ParrotFixed32i_to_float(n) ((float)(n) / 256.0f)
#define ParrotFixed32i_from_float(n)                                                                                    \
    ((ParrotFixed32i)(PARROT_CLAMP(ParrotFixed32i_MIN, n, ParrotFixed32i_MAX) * 256.0f + 0.5f) - ((n) < 0))

/// 16.16 fixed point
typedef int32_t ParrotFixed32s;
#define ParrotFixed32s_MIN (-32767)
#define ParrotFixed32s_MAX (32767)
#define ParrotFixed32s_to_float(n) ((float)(n) / 65536.0f)
#define ParrotFixed32s_from_float(n)                                                                                    \
    ((ParrotFixed32s)(PARROT_CLAMP(ParrotFixed32s_MIN, n, ParrotFixed32s_MAX) * 65536.0f + 0.5f) - ((n) < 0))

/// 8.24 fixed point
typedef int32_t ParrotFixed32f;
#define ParrotFixed32f_MIN (-127)
#define ParrotFixed32f_MAX (127)
#define ParrotFixed32f_to_float(n) ((float)(n) / 8388608.0f)
#define ParrotFixed32f_from_float(n)                                                                                    \
    ((ParrotFixed32f)(PARROT_CLAMP(ParrotFixed32f_MIN, n, ParrotFixed32f_MAX) * 8388608.0f + 0.5f) - ((n) < 0))

/// 12.4 fixed point
typedef int16_t ParrotFixed16i;
#define ParrotFixed16i_MIN (-4095)
#define ParrotFixed16i_MAX (4095)
#define ParrotFixed16i_to_float(n) ((float)(n) / 16.0f)
#define ParrotFixed16i_from_float(n)                                                                                    \
    ((ParrotFixed16i)(PARROT_CLAMP(ParrotFixed16i_MIN, n, ParrotFixed16i_MAX) * 16.0f + 0.5f) - ((n) < 0))

/// 8.8 fixed point
typedef int16_t ParrotFixed16s;
#define ParrotFixed16s_MIN (-255)
#define ParrotFixed16s_MAX (255)
#define ParrotFixed16s_to_float(n) ((float)(n) / 256.0f)
#define ParrotFixed16s_from_float(n)                                                                                    \
    ((ParrotFixed16s)(PARROT_CLAMP(ParrotFixed16s_MIN, n, ParrotFixed16s_MAX) * 256.0f + 0.5f) - ((n) < 0))

/// 4.12 fixed point
typedef int16_t ParrotFixed16f;
#define ParrotFixed16f_MIN (-15)
#define ParrotFixed16f_MAX (15)
#define ParrotFixed16f_to_float(n) ((float)(n) / 4096.0f)
#define ParrotFixed16f_from_float(n)                                                                                    \
    ((ParrotFixed16f)(PARROT_CLAMP(ParrotFixed16f_MIN, n, ParrotFixed16f_MAX) * 4096.0f + 0.5f) - ((n) < 0))

#ifndef PARROT_DOUBLE_PRECISION
typedef float ParrotReal;
#define ParrotReal_sqrt sqrtf
#define ParrotReal_sin sinf
#define ParrotReal_cos cosf
#define ParrotReal_atan2 atan2f
#else
typedef double ParrotReal;
#define ParrotReal_sqrt sqrt
#define ParrotReal_sin sin
#define ParrotReal_cos cos
#define ParrotReal_atan2 atan2
#endif

#define PARROT_CLAMP(min, value, max) PARROT_MAX(min, PARROT_MIN(value, max))
#define PARROT_MIN(a, b) ((a) > (b) ? (b) : (a))
#define PARROT_MAX(a, b) ((a) > (b) ? (a) : (b))

PARROT_API ParrotReal Parrot_lerp(ParrotReal a, ParrotReal b, ParrotReal t);
PARROT_API float Parrot_lerpf(float a, float b, float t);
PARROT_API double Parrot_lerpd(double a, double b, double t);

PARROT_API void ParrotReal_to_float_array(const ParrotReal *src, float *dest, size_t count);

typedef struct {
    ParrotReal x;
    ParrotReal y;
} ParrotVec2;

PARROT_API ParrotVec2 ParrotVec2_n(ParrotReal n);

PARROT_API ParrotVec2 ParrotVec2_add(ParrotVec2 a, ParrotVec2 b);
PARROT_API ParrotVec2 ParrotVec2_sub(ParrotVec2 a, ParrotVec2 b);
PARROT_API ParrotVec2 ParrotVec2_mul(ParrotVec2 a, ParrotVec2 b);
PARROT_API ParrotVec2 ParrotVec2_div(ParrotVec2 a, ParrotVec2 b);
PARROT_API ParrotVec2 ParrotVec2_scale(ParrotVec2 a, ParrotReal b);

PARROT_API ParrotVec2 ParrotVec2_normalize(ParrotVec2 self);

PARROT_API ParrotReal ParrotVec2_length(ParrotVec2 self);
PARROT_API ParrotReal ParrotVec2_dot(ParrotVec2 self, ParrotVec2 other);

typedef struct {
    ParrotReal x;
    ParrotReal y;
    ParrotReal z;
} ParrotVec3;

PARROT_API ParrotVec3 ParrotVec3_n(ParrotReal n);

PARROT_API ParrotVec3 ParrotVec3_add(ParrotVec3 a, ParrotVec3 b);
PARROT_API ParrotVec3 ParrotVec3_sub(ParrotVec3 a, ParrotVec3 b);
PARROT_API ParrotVec3 ParrotVec3_mul(ParrotVec3 a, ParrotVec3 b);
PARROT_API ParrotVec3 ParrotVec3_div(ParrotVec3 a, ParrotVec3 b);
PARROT_API ParrotVec3 ParrotVec3_scale(ParrotVec3 a, ParrotReal b);

PARROT_API ParrotVec3 ParrotVec3_normalize(ParrotVec3 self);

PARROT_API ParrotReal ParrotVec3_length(ParrotVec3 self);
PARROT_API ParrotReal ParrotVec3_dot(ParrotVec3 self, ParrotVec3 other);

typedef struct {
    ParrotReal x;
    ParrotReal y;
    ParrotReal z;
    ParrotReal w;
} ParrotVec4;

PARROT_API ParrotVec4 ParrotVec4_n(ParrotReal n);

PARROT_API ParrotVec4 ParrotVec4_add(ParrotVec4 a, ParrotVec4 b);
PARROT_API ParrotVec4 ParrotVec4_sub(ParrotVec4 a, ParrotVec4 b);
PARROT_API ParrotVec4 ParrotVec4_mul(ParrotVec4 a, ParrotVec4 b);
PARROT_API ParrotVec4 ParrotVec4_div(ParrotVec4 a, ParrotVec4 b);
PARROT_API ParrotVec4 ParrotVec4_scale(ParrotVec4 a, ParrotReal b);

PARROT_API ParrotVec4 ParrotVec4_normalize(ParrotVec4 self);

PARROT_API ParrotReal ParrotVec4_length(ParrotVec4 self);
PARROT_API ParrotReal ParrotVec4_dot(ParrotVec4 self, ParrotVec4 other);

typedef struct {
    // Column-major (accessed like data[x][y])
    ParrotReal data[4][4];
} ParrotMat;

PARROT_API ParrotMat ParrotMat_identity(void);

PARROT_API ParrotMat ParrotMat_inverse(ParrotMat matrix);
PARROT_API ParrotMat ParrotMat_transpose(ParrotMat matrix);

PARROT_API ParrotMat ParrotMat_add(ParrotMat a, ParrotMat b);
PARROT_API ParrotMat ParrotMat_mul(ParrotMat a, ParrotMat b);

PARROT_API ParrotVec3 ParrotMat_transform3(ParrotMat matrix, ParrotVec3 vec);
PARROT_API ParrotVec2 ParrotMat_transform2(ParrotMat matrix, ParrotVec2 vec);

PARROT_API ParrotMat
ParrotMat_ortho(ParrotReal left, ParrotReal right, ParrotReal down, ParrotReal up, ParrotReal near, ParrotReal far);

PARROT_API ParrotMat ParrotMat_set_position(ParrotMat matrix, ParrotVec3 position);
PARROT_API ParrotMat ParrotMat_set_rotation(ParrotMat matrix, ParrotVec3 rotation);
PARROT_API ParrotMat ParrotMat_set_scale(ParrotMat matrix, ParrotVec3 scale);

PARROT_API ParrotVec3 ParrotMat_get_position(ParrotMat matrix);
PARROT_API ParrotVec3 ParrotMat_get_rotation(ParrotMat matrix);
PARROT_API ParrotVec3 ParrotMat_get_scale(ParrotMat matrix);

typedef struct {
    ParrotMat model;
    ParrotMat view;
    ParrotMat projection;
} ParrotGMatSet;

PARROT_API ParrotMat ParrotGMatSet_combine(const ParrotGMatSet *self);

typedef struct {
    // [0.0, 1.0]
    float r;
    // [0.0, 1.0]
    float g;
    // [0.0, 1.0]
    float b;
    // [0.0, 1.0]
    float a;
} ParrotColor;

PARROT_API ParrotColor ParrotColor_new(uint8_t r, uint8_t g, uint8_t b);
PARROT_API ParrotColor ParrotColor_newf(float r, float g, float b);
PARROT_API ParrotColor ParrotColor_newa(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
PARROT_API ParrotColor ParrotColor_newaf(float r, float g, float b, float a);

PARROT_API ParrotColor ParrotColor_from_rgba8888(uint32_t color);
PARROT_API ParrotColor ParrotColor_from_rgba5551(uint16_t color);
PARROT_API ParrotColor ParrotColor_from_rgba565(uint16_t color);

PARROT_API uint32_t ParrotColor_to_rgba8888(ParrotColor self);
PARROT_API uint16_t ParrotColor_to_rgba5551(ParrotColor self);
PARROT_API uint16_t ParrotColor_to_rgb565(ParrotColor self);

PARROT_API ParrotColor ParrotColor_mul(ParrotColor a, ParrotColor b);
PARROT_API ParrotColor ParrotColor_blend(ParrotColor a, ParrotColor b);
PARROT_API ParrotColor ParrotColor_lerp(ParrotColor a, ParrotColor b, float t);

#define ParrotColor_CLEAR ((ParrotColor){.r = 0.0, .g = 0.0, .b = 0.0, .a = 0.0})
#define ParrotColor_BLACK ((ParrotColor){.r = 0.0, .g = 0.0, .b = 0.0, .a = 1.0})
#define ParrotColor_WHITE ((ParrotColor){.r = 1.0, .g = 1.0, .b = 1.0, .a = 1.0})
#define ParrotColor_GRAY ((ParrotColor){.r = 0.5, .g = 0.5, .b = 0.5, .a = 1.0})
#define ParrotColor_GREY ParrotColor_GRAY
#define ParrotColor_RED ((ParrotColor){.r = 1.0, .g = 0.0, .b = 0.0, .a = 1.0})
#define ParrotColor_GREEN ((ParrotColor){.r = 0.0, .g = 1.0, .b = 0.0, .a = 1.0})
#define ParrotColor_BLUE ((ParrotColor){.r = 0.0, .g = 0.0, .b = 1.0, .a = 1.0})
#define ParrotColor_YELLOW ((ParrotColor){.r = 1.0, .g = 1.0, .b = 0.0, .a = 1.0})
#define ParrotColor_ORANGE ((ParrotColor){.r = 1.0, .g = 0.3, .b = 0.0, .a = 1.0})
#define ParrotColor_CYAN ((ParrotColor){.r = 0.0, .g = 1.0, .b = 1.0, .a = 1.0})

#endif // __SRC_PARROT_INCLUDE_PARROT_CORE_MATH_H_
