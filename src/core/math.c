#include "parrot/core/math.h"
#include <math.h> // IWYU pragma: keep

void ParrotReal_to_float_array(const ParrotReal *src, ParrotReal *dest, size_t count) {
    for (size_t i = 0; i < count; i++) {
        dest[i] = src[i];
    }
}

ParrotVec2 ParrotVec2_n(ParrotReal n) {
    return (ParrotVec2){n, n};
}

ParrotVec2 ParrotVec2_add(ParrotVec2 a, ParrotVec2 b) {
    return (ParrotVec2){a.x + b.x, a.y + b.y};
}

ParrotVec2 ParrotVec2_sub(ParrotVec2 a, ParrotVec2 b) {
    return (ParrotVec2){a.x - b.x, a.y - b.y};
}

ParrotVec2 ParrotVec2_mul(ParrotVec2 a, ParrotVec2 b) {
    return (ParrotVec2){a.x * b.x, a.y * b.y};
}

ParrotVec2 ParrotVec2_div(ParrotVec2 a, ParrotVec2 b) {
    return (ParrotVec2){a.x / b.x, a.y / b.y};
}

ParrotVec2 ParrotVec2_scale(ParrotVec2 a, ParrotReal b) {
    return (ParrotVec2){a.x * b, a.y * b};
}

ParrotVec2 ParrotVec2_normalize(ParrotVec2 self) {
    ParrotReal length = ParrotVec2_length(self);

    if (length == 0) {
        return ParrotVec2_n(0);
    }

    return (ParrotVec2){self.x / length, self.y / length};
}

ParrotReal ParrotVec2_length(ParrotVec2 self) {
    return ParrotReal_sqrt(self.x * self.x + self.y * self.y);
}

ParrotReal ParrotVec2_dot(ParrotVec2 self, ParrotVec2 other) {
    return self.x * other.x + self.y * other.y;
}

ParrotVec3 ParrotVec3_n(ParrotReal n) {
    return (ParrotVec3){n, n, n};
}

ParrotVec3 ParrotVec3_add(ParrotVec3 a, ParrotVec3 b) {
    return (ParrotVec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

ParrotVec3 ParrotVec3_sub(ParrotVec3 a, ParrotVec3 b) {
    return (ParrotVec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

ParrotVec3 ParrotVec3_mul(ParrotVec3 a, ParrotVec3 b) {
    return (ParrotVec3){a.x * b.x, a.y * b.y, a.z * b.z};
}

ParrotVec3 ParrotVec3_div(ParrotVec3 a, ParrotVec3 b) {
    return (ParrotVec3){a.x / b.x, a.y / b.y, a.z / b.z};
}

ParrotVec3 ParrotVec3_scale(ParrotVec3 a, ParrotReal b) {
    return (ParrotVec3){a.x * b, a.y * b, a.z * b};
}

ParrotVec3 ParrotVec3_normalize(ParrotVec3 self) {
    ParrotReal length = ParrotVec3_length(self);

    if (length == 0) {
        return ParrotVec3_n(0);
    }

    return (ParrotVec3){self.x / length, self.y / length, self.z / length};
}

ParrotReal ParrotVec3_length(ParrotVec3 self) {
    return ParrotReal_sqrt(self.x * self.x + self.y * self.y + self.z * self.z);
}

ParrotReal ParrotVec3_dot(ParrotVec3 self, ParrotVec3 other) {
    return self.x * other.x + self.y * other.y + self.z * other.z;
}

ParrotVec4 ParrotVec4_n(ParrotReal n) {
    return (ParrotVec4){n, n, n, n};
}

ParrotVec4 ParrotVec4_add(ParrotVec4 a, ParrotVec4 b) {
    return (ParrotVec4){a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

ParrotVec4 ParrotVec4_sub(ParrotVec4 a, ParrotVec4 b) {
    return (ParrotVec4){a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}

ParrotVec4 ParrotVec4_mul(ParrotVec4 a, ParrotVec4 b) {
    return (ParrotVec4){a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
}

ParrotVec4 ParrotVec4_div(ParrotVec4 a, ParrotVec4 b) {
    return (ParrotVec4){a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
}

ParrotVec4 ParrotVec4_scale(ParrotVec4 a, ParrotReal b) {
    return (ParrotVec4){a.x * b, a.y * b, a.z * b, a.w * b};
}

ParrotVec4 ParrotVec4_normalize(ParrotVec4 self) {
    ParrotReal length = ParrotVec4_length(self);

    if (length == 0) {
        return ParrotVec4_n(0);
    }

    return (ParrotVec4){self.x / length, self.y / length, self.z / length, self.w / length};
}

ParrotReal ParrotVec4_length(ParrotVec4 self) {
    return ParrotReal_sqrt(self.x * self.x + self.y * self.y + self.z * self.z + self.w * self.w);
}

ParrotReal ParrotVec4_dot(ParrotVec4 self, ParrotVec4 other) {
    return self.x * other.x + self.y * other.y + self.z * other.z + self.w * other.w;
}

ParrotMat ParrotMat_identity(void) {
    ParrotMat result = {0};

    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            if (x == y) {
                result.data[x][y] = 1;
            }
        }
    }

    return result;
}

ParrotMat ParrotMat_inverse(ParrotMat matrix) {
    ParrotMat result = {0};

    float aug[4][8];
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            aug[y][x] = matrix.data[x][y];
            aug[y][x + 4] = (x == y) ? 1.0f : 0.0f;
        }
    }

    for (int col = 0; col < 4; col++) {
        int pivot = col;
        for (int row = col + 1; row < 4; row++) {
            if (fabsf(aug[row][col]) > fabsf(aug[pivot][col]))
                pivot = row;
        }

        if (pivot != col) {
            for (int k = 0; k < 8; k++) {
                float tmp = aug[col][k];
                aug[col][k] = aug[pivot][k];
                aug[pivot][k] = tmp;
            }
        }

        if (fabsf(aug[col][col]) < 1e-6f)
            return result;

        float scale = aug[col][col];
        for (int k = 0; k < 8; k++)
            aug[col][k] /= scale;

        for (int row = 0; row < 4; row++) {
            if (row == col)
                continue;
            float factor = aug[row][col];
            for (int k = 0; k < 8; k++)
                aug[row][k] -= factor * aug[col][k];
        }
    }

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            result.data[x][y] = aug[y][x + 4];
        }
    }

    return result;
}

ParrotMat ParrotMat_transpose(ParrotMat matrix) {
    ParrotMat result = {0};

    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            result.data[x][y] = matrix.data[y][x];
        }
    }

    return result;
}

ParrotMat ParrotMat_add(ParrotMat a, ParrotMat b) {
    ParrotMat result = {0};

    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            result.data[x][y] = a.data[x][y] + b.data[x][y];
        }
    }

    return result;
}

ParrotMat ParrotMat_mul(ParrotMat a, ParrotMat b) {
    ParrotMat result = {0};

    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            for (int z = 0; z < 4; z++) {
                result.data[x][y] += a.data[z][y] * b.data[x][z];
            }
        }
    }

    return result;
}

ParrotMat
ParrotMat_ortho(ParrotReal left, ParrotReal right, ParrotReal bottom, ParrotReal top, ParrotReal near, ParrotReal far) {
    ParrotMat matrix = {0};

    ParrotReal width = right - left;
    ParrotReal height = top - bottom;
    ParrotReal total_distance = far - near;

    matrix.data[0][0] = 2.0 / width;

    matrix.data[1][1] = 2.0 / height;

    matrix.data[2][2] = -2.0 / total_distance;

    matrix.data[3][0] = -(right + left) / width;
    matrix.data[3][1] = -(top + bottom) / height;
    matrix.data[3][2] = -(far + near) / total_distance;
    matrix.data[3][3] = 1.0;

    return matrix;
}

static ParrotReal ParrotMat_col_length(ParrotMat matrix, int col) {
    ParrotReal x = matrix.data[col][0];
    ParrotReal y = matrix.data[col][1];
    ParrotReal z = matrix.data[col][2];
    return ParrotReal_sqrt(x * x + y * y + z * z);
}

ParrotMat ParrotMat_set_position(ParrotMat matrix, ParrotVec3 position) {
    matrix.data[3][0] = position.x;
    matrix.data[3][1] = position.y;
    matrix.data[3][2] = position.z;
    return matrix;
}

ParrotMat ParrotMat_set_rotation(ParrotMat matrix, ParrotVec3 rotation) {
    ParrotReal sx_scale = ParrotMat_col_length(matrix, 0);
    ParrotReal sy_scale = ParrotMat_col_length(matrix, 1);
    ParrotReal sz_scale = ParrotMat_col_length(matrix, 2);

    if (sx_scale < 1e-6f) {
        sx_scale = (ParrotReal)1.0;
    }
    if (sy_scale < 1e-6f) {
        sy_scale = (ParrotReal)1.0;
    }
    if (sz_scale < 1e-6f) {
        sz_scale = (ParrotReal)1.0;
    }

    ParrotReal sx = ParrotReal_sin(rotation.x), cx = ParrotReal_cos(rotation.x);
    ParrotReal sy = ParrotReal_sin(rotation.y), cy = ParrotReal_cos(rotation.y);
    ParrotReal sz = ParrotReal_sin(rotation.z), cz = ParrotReal_cos(rotation.z);

    matrix.data[0][0] = (cz * cy) * sx_scale;
    matrix.data[0][1] = (sz * cy) * sx_scale;
    matrix.data[0][2] = (-sy) * sx_scale;

    matrix.data[1][0] = (cz * sy * sx - sz * cx) * sy_scale;
    matrix.data[1][1] = (sz * sy * sx + cz * cx) * sy_scale;
    matrix.data[1][2] = (cy * sx) * sy_scale;

    matrix.data[2][0] = (cz * sy * cx + sz * sx) * sz_scale;
    matrix.data[2][1] = (sz * sy * cx - cz * sx) * sz_scale;
    matrix.data[2][2] = (cy * cx) * sz_scale;

    return matrix;
}

ParrotMat ParrotMat_set_scale(ParrotMat matrix, ParrotVec3 scale) {
    for (int col = 0; col < 3; col++) {
        ParrotReal new_scale = (col == 0) ? scale.x : (col == 1) ? scale.y : scale.z;
        ParrotReal len = ParrotMat_col_length(matrix, col);

        if (len > (ParrotReal)1e-6) {
            ParrotReal factor = new_scale / len;
            matrix.data[col][0] *= factor;
            matrix.data[col][1] *= factor;
            matrix.data[col][2] *= factor;
        } else {
            matrix.data[col][0] = (col == 0) ? new_scale : (ParrotReal)0.0;
            matrix.data[col][1] = (col == 1) ? new_scale : (ParrotReal)0.0;
            matrix.data[col][2] = (col == 2) ? new_scale : (ParrotReal)0.0;
        }
    }

    return matrix;
}

ParrotVec3 ParrotMat_get_position(ParrotMat matrix) {
    return (ParrotVec3){matrix.data[3][0], matrix.data[3][1], matrix.data[3][2]};
}

ParrotVec3 ParrotMat_get_rotation(ParrotMat matrix) {
    ParrotVec3 scale = ParrotMat_get_scale(matrix);

    ParrotReal r00 = matrix.data[0][0] / scale.x;
    ParrotReal r01 = matrix.data[0][1] / scale.x;
    ParrotReal r02 = matrix.data[0][2] / scale.x;
    ParrotReal r12 = matrix.data[1][2] / scale.y;
    ParrotReal r22 = matrix.data[2][2] / scale.z;
    ParrotReal r10 = matrix.data[1][0] / scale.y;
    ParrotReal r11 = matrix.data[1][1] / scale.y;

    ParrotVec3 result;

    ParrotReal cy = ParrotReal_sqrt(r00 * r00 + r01 * r01);
    if (cy > (ParrotReal)1e-6) {
        result.x = ParrotReal_atan2(r12, r22);
        result.y = ParrotReal_atan2(-r02, cy);
        result.z = ParrotReal_atan2(r01, r00);
    } else {
        result.x = ParrotReal_atan2(-r10, r11);
        result.y = ParrotReal_atan2(-r02, cy);
        result.z = (ParrotReal)0;
    }

    return result;
}

ParrotVec3 ParrotMat_get_scale(ParrotMat matrix) {
    ParrotVec3 result;
    result.x = ParrotReal_sqrt(matrix.data[0][0] * matrix.data[0][0] + matrix.data[0][1] * matrix.data[0][1] +
                               matrix.data[0][2] * matrix.data[0][2]);
    result.y = ParrotReal_sqrt(matrix.data[1][0] * matrix.data[1][0] + matrix.data[1][1] * matrix.data[1][1] +
                               matrix.data[1][2] * matrix.data[1][2]);
    result.z = ParrotReal_sqrt(matrix.data[2][0] * matrix.data[2][0] + matrix.data[2][1] * matrix.data[2][1] +
                               matrix.data[2][2] * matrix.data[2][2]);
    return result;
}