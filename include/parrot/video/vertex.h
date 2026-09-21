#ifndef PARROT_PARROT_INCLUDE_PARROT_VIDEO_VERTEX_H_
#define PARROT_PARROT_INCLUDE_PARROT_VIDEO_VERTEX_H_

#include "parrot/core/math.h"

typedef struct {
    ParrotVec3 position;
    ParrotVec3 normal;
    ParrotVec2 uv;
    ParrotColor tint;
} ParrotVideoVertex;

#endif // PARROT_PARROT_INCLUDE_PARROT_VIDEO_VERTEX_H_
