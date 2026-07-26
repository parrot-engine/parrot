#ifndef __SRC_PARROT_INCLUDE_PARROT_CORE_MATH_H_
#define __SRC_PARROT_INCLUDE_PARROT_CORE_MATH_H_

#include "parrot/config.h"
#include "parrot/core/api.h"

typedef struct {
    ParrotReal x;
    ParrotReal y;
} ParrotVec2f;

PARROT_API ParrotVec2f ParrotVec2f_n(ParrotReal n);

PARROT_API ParrotVec2f ParrotVec2f_add(ParrotVec2f a, ParrotVec2f b);
PARROT_API ParrotVec2f ParrotVec2f_sub(ParrotVec2f a, ParrotVec2f b);
PARROT_API ParrotVec2f ParrotVec2f_mul(ParrotVec2f a, ParrotVec2f b);
PARROT_API ParrotVec2f ParrotVec2f_div(ParrotVec2f a, ParrotVec2f b);
PARROT_API ParrotVec2f ParrotVec2f_scale(ParrotVec2f a, ParrotReal b);

PARROT_API ParrotVec2f ParrotVec2f_normalize(ParrotVec2f self);

PARROT_API ParrotReal ParrotVec2f_length(ParrotVec2f self);
PARROT_API ParrotReal ParrotVec2f_dot(ParrotVec2f self, ParrotVec2f other);

typedef struct {
    ParrotReal x;
    ParrotReal y;
    ParrotReal z;
} ParrotVec3f;

PARROT_API ParrotVec3f ParrotVec3f_n(ParrotReal n);

PARROT_API ParrotVec3f ParrotVec3f_add(ParrotVec3f a, ParrotVec3f b);
PARROT_API ParrotVec3f ParrotVec3f_sub(ParrotVec3f a, ParrotVec3f b);
PARROT_API ParrotVec3f ParrotVec3f_mul(ParrotVec3f a, ParrotVec3f b);
PARROT_API ParrotVec3f ParrotVec3f_div(ParrotVec3f a, ParrotVec3f b);
PARROT_API ParrotVec3f ParrotVec3f_scale(ParrotVec3f a, ParrotReal b);

PARROT_API ParrotVec3f ParrotVec3f_normalize(ParrotVec3f self);

PARROT_API ParrotReal ParrotVec3f_length(ParrotVec3f self);
PARROT_API ParrotReal ParrotVec3f_dot(ParrotVec3f self, ParrotVec3f other);

typedef struct {
    ParrotReal x;
    ParrotReal y;
    ParrotReal z;
    ParrotReal w;
} ParrotVec4f;

PARROT_API ParrotVec4f ParrotVec4f_n(ParrotReal n);

PARROT_API ParrotVec4f ParrotVec4f_add(ParrotVec4f a, ParrotVec4f b);
PARROT_API ParrotVec4f ParrotVec4f_sub(ParrotVec4f a, ParrotVec4f b);
PARROT_API ParrotVec4f ParrotVec4f_mul(ParrotVec4f a, ParrotVec4f b);
PARROT_API ParrotVec4f ParrotVec4f_div(ParrotVec4f a, ParrotVec4f b);
PARROT_API ParrotVec4f ParrotVec4f_scale(ParrotVec4f a, ParrotReal b);

PARROT_API ParrotVec4f ParrotVec4f_normalize(ParrotVec4f self);

PARROT_API ParrotReal ParrotVec4f_length(ParrotVec4f self);
PARROT_API ParrotReal ParrotVec4f_dot(ParrotVec4f self, ParrotVec4f other);

#endif // __SRC_PARROT_INCLUDE_PARROT_CORE_MATH_H_