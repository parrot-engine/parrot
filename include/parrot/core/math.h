#ifndef __SRC_PARROT_INCLUDE_PARROT_CORE_MATH_H_
#define __SRC_PARROT_INCLUDE_PARROT_CORE_MATH_H_

#include "parrot/config.h" // IWYU pragma: keep
#include "parrot/core/api.h"
#include <stddef.h>

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

#define PARROT_MIN(a, b) ((a) > (b) ? (b) : (a))
#define PARROT_MAX(a, b) ((a) > (b) ? (a) : (b))

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

PARROT_API ParrotMat
ParrotMat_ortho(ParrotReal left, ParrotReal right, ParrotReal down, ParrotReal up, ParrotReal near, ParrotReal far);

PARROT_API ParrotMat ParrotMat_set_position(ParrotMat matrix, ParrotVec3 position);
PARROT_API ParrotMat ParrotMat_set_rotation(ParrotMat matrix, ParrotVec3 rotation);
PARROT_API ParrotMat ParrotMat_set_scale(ParrotMat matrix, ParrotVec3 scale);

PARROT_API ParrotVec3 ParrotMat_get_position(ParrotMat matrix);
PARROT_API ParrotVec3 ParrotMat_get_rotation(ParrotMat matrix);
PARROT_API ParrotVec3 ParrotMat_get_scale(ParrotMat matrix);

#endif // __SRC_PARROT_INCLUDE_PARROT_CORE_MATH_H_