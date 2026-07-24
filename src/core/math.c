#include "parrot/core/math.h"
#include <math.h> // IWYU pragma: keep

ParrotVec2f ParrotVec2f_n(ParrotReal n) {
    return (ParrotVec2f){n, n};
}

ParrotVec2f ParrotVec2f_add(ParrotVec2f a, ParrotVec2f b) {
    return (ParrotVec2f){a.x + b.x, a.y + b.y};
}

ParrotVec2f ParrotVec2f_sub(ParrotVec2f a, ParrotVec2f b) {
    return (ParrotVec2f){a.x - b.x, a.y - b.y};
}

ParrotVec2f ParrotVec2f_mul(ParrotVec2f a, ParrotVec2f b) {
    return (ParrotVec2f){a.x * b.x, a.y * b.y};
}

ParrotVec2f ParrotVec2f_div(ParrotVec2f a, ParrotVec2f b) {
    return (ParrotVec2f){a.x / b.x, a.y / b.y};
}

ParrotVec2f ParrotVec2f_scale(ParrotVec2f a, ParrotReal b) {
    return (ParrotVec2f){a.x * b, a.y * b};
}

ParrotVec2f ParrotVec2f_normalize(ParrotVec2f self) {
    ParrotReal length = ParrotVec2f_length(self);

    if (length == 0) {
        return ParrotVec2f_n(0);
    }

    return (ParrotVec2f){self.x / length, self.y / length};
}

ParrotReal ParrotVec2f_length(ParrotVec2f self) {
    return sqrt(self.x * self.x + self.y * self.y);
}

ParrotReal ParrotVec2f_dot(ParrotVec2f self, ParrotVec2f other) {
    return self.x * other.x + self.y * other.y;
}

ParrotVec3f ParrotVec3f_n(ParrotReal n) {
    return (ParrotVec3f){n, n, n};
}

ParrotVec3f ParrotVec3f_add(ParrotVec3f a, ParrotVec3f b) {
    return (ParrotVec3f){a.x + b.x, a.y + b.y, a.z + b.z};
}

ParrotVec3f ParrotVec3f_sub(ParrotVec3f a, ParrotVec3f b) {
    return (ParrotVec3f){a.x - b.x, a.y - b.y, a.z - b.z};
}

ParrotVec3f ParrotVec3f_mul(ParrotVec3f a, ParrotVec3f b) {
    return (ParrotVec3f){a.x * b.x, a.y * b.y, a.z * b.z};
}

ParrotVec3f ParrotVec3f_div(ParrotVec3f a, ParrotVec3f b) {
    return (ParrotVec3f){a.x / b.x, a.y / b.y, a.z / b.z};
}

ParrotVec3f ParrotVec3f_scale(ParrotVec3f a, ParrotReal b) {
    return (ParrotVec3f){a.x * b, a.y * b, a.z * b};
}

ParrotVec3f ParrotVec3f_normalize(ParrotVec3f self) {
    ParrotReal length = ParrotVec3f_length(self);

    if (length == 0) {
        return ParrotVec3f_n(0);
    }

    return (ParrotVec3f){self.x / length, self.y / length, self.z / length};
}

ParrotReal ParrotVec3f_length(ParrotVec3f self) {
    return sqrt(self.x * self.x + self.y * self.y + self.z * self.z);
}

ParrotReal ParrotVec3f_dot(ParrotVec3f self, ParrotVec3f other) {
    return self.x * other.x + self.y * other.y + self.z * other.z;
}

ParrotVec4f ParrotVec4f_n(ParrotReal n) {
    return (ParrotVec4f){n, n, n, n};
}

ParrotVec4f ParrotVec4f_add(ParrotVec4f a, ParrotVec4f b) {
    return (ParrotVec4f){a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

ParrotVec4f ParrotVec4f_sub(ParrotVec4f a, ParrotVec4f b) {
    return (ParrotVec4f){a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}

ParrotVec4f ParrotVec4f_mul(ParrotVec4f a, ParrotVec4f b) {
    return (ParrotVec4f){a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
}

ParrotVec4f ParrotVec4f_div(ParrotVec4f a, ParrotVec4f b) {
    return (ParrotVec4f){a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
}

ParrotVec4f ParrotVec4f_scale(ParrotVec4f a, ParrotReal b) {
    return (ParrotVec4f){a.x * b, a.y * b, a.z * b, a.w * b};
}

ParrotVec4f ParrotVec4f_normalize(ParrotVec4f self) {
    ParrotReal length = ParrotVec4f_length(self);

    if (length == 0) {
        return ParrotVec4f_n(0);
    }

    return (ParrotVec4f){self.x / length, self.y / length, self.z / length, self.w / length};
}

ParrotReal ParrotVec4f_length(ParrotVec4f self) {
    return sqrt(self.x * self.x + self.y * self.y + self.z * self.z + self.w * self.w);
}

ParrotReal ParrotVec4f_dot(ParrotVec4f self, ParrotVec4f other) {
    return self.x * other.x + self.y * other.y + self.z * other.z + self.w * other.w;
}