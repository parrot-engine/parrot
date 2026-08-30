#include "parrot/core/buffer.h"
#include "parrot/core/scope.h"
#include "parrot/core/util.h"
#include "src/core/buffer.h"
#include "stb_ds.h"
#include <string.h>

struct ParrotBuffer {
    ParrotBufferEndian endian;

    ParrotBufferRead read;
    ParrotBufferWrite write;
    ParrotScope *scope;

    size_t bytes_written;
    size_t read_position;
};

ParrotBuffer *ParrotBuffer_new(ParrotScope *scope, ParrotBufferRead read, ParrotBufferWrite write) {
    ParrotBuffer *self = PARROT_ALLOC(ParrotBuffer);

    self->read = read;
    self->write = write;
    self->scope = scope;

    return self;
}

typedef struct {
    const uint8_t *data;
    size_t size;

    ParrotScope *write_scope;
    ParrotBufferWrite write;
} BytearrayCtx;

static bool bytearray_read(ParrotScope *scope, size_t position, uint8_t *out) {
    BytearrayCtx *ctx = ParrotScope_get_ctx(scope, BytearrayCtx *);

    if (position >= ctx->size) {
        return false;
    }

    *out = ctx->data[position];
    return true;
}

static void bytearray_write_wrapper(ParrotScope *scope, uint8_t byte) {
    BytearrayCtx *ctx = ParrotScope_get_ctx(scope, BytearrayCtx *);

    if (ctx->write) {
        ctx->write(ctx->write_scope, byte);
    }
}

ParrotBuffer *
ParrotBuffer_new_bytearray(ParrotScope *write_scope, const void *data, size_t size, ParrotBufferWrite write) {
    ParrotScope *scope = ParrotScope_new(NULL);
    BytearrayCtx *ctx = ParrotScope_alloc_ctx(scope, BytearrayCtx);

    ctx->data = data;
    ctx->size = size;

    ctx->write_scope = write_scope;
    ctx->write = write;

    if (ctx->write_scope) {
        ParrotScope_set_parent(write_scope, scope);
    }

    return ParrotBuffer_new(scope, bytearray_read, bytearray_write_wrapper);
}

static bool stbds_array_read(ParrotScope *scope, size_t position, uint8_t *out) {
    uint8_t **p_arr_data = ParrotScope_get_ctx(scope, uint8_t **);

    if (position >= arrlen(*p_arr_data)) {
        return false;
    }

    *out = (*p_arr_data)[position];
    return true;
}

static void stbds_array_write(ParrotScope *scope, uint8_t byte) {
    uint8_t **p_arr_data = ParrotScope_get_ctx(scope, uint8_t **);
    arrpush(*p_arr_data, byte);
}

ParrotBuffer *ParrotBuffer_new_stbds_array_raw(uint8_t **p_arr_data) {
    ParrotScope *scope = ParrotScope_new(NULL);
    ParrotScope_set_ctx(scope, p_arr_data);
    return ParrotBuffer_new(scope, stbds_array_read, stbds_array_write);
}

void ParrotBuffer_delete(ParrotBuffer *self) {
    PARROT_FAIL_NULL(self);

    if (self->scope) {
        ParrotScope_delete(self->scope);
    }

    free(self);
}

void ParrotBuffer_set_endian(ParrotBuffer *self, ParrotBufferEndian endian) {
    PARROT_FAIL_NULL(self);

    self->endian = endian;
}

ParrotBufferEndian ParrotBuffer_get_endian(ParrotBuffer *self) {
    PARROT_FAIL_NULL(self);

    return self->endian;
}

void ParrotBuffer_rseek(ParrotBuffer *self, size_t position) {
    PARROT_FAIL_NULL(self);

    self->read_position = position;
}

size_t ParrotBuffer_rtell(ParrotBuffer *self) {
    PARROT_FAIL_NULL(self);

    return self->read_position;
}

void ParrotBuffer_pad(ParrotBuffer *self, uint8_t data, size_t count) {
    PARROT_FAIL_NULL(self);

    for (size_t i = 0; i < count; i++) {
        ParrotBuffer_write8(self, data);
    }
}

void ParrotBuffer_pad_until(ParrotBuffer *self, uint8_t data, size_t until_position) {
    PARROT_FAIL_NULL(self);

    while (until_position >= self->bytes_written) {
        ParrotBuffer_write8(self, data);
    }
}

size_t ParrotBuffer_read(ParrotBuffer *self, void *out_ptr, size_t size) {
    uint8_t *out = out_ptr;

    PARROT_FAIL_NULL(self);
    PARROT_FAIL_NULL(out);

    PARROT_RET_COND_V(!self->read, 0);

    for (size_t i = 0; i < size; i++) {
        if (!self->read(self->scope, self->read_position, &out[i])) {
            return i;
        }
        self->read_position++;
    }

    if (self->endian != ParrotBufferEndian_HOST) {
        uint16_t endian_test_big = 1;
        uint8_t endian_test = 0;
        memcpy(&endian_test, &endian_test_big, sizeof(uint8_t));

        ParrotBufferEndian host_endian = endian_test == 1 ? ParrotBufferEndian_LITTLE : ParrotBufferEndian_BIG;
        if (self->endian != host_endian) {
            for (size_t i = 0; i < size / 2; i++) {
                uint8_t tmp = out[i];
                out[i] = out[size - 1 - i];
                out[size - 1 - i] = tmp;
            }
        }
    }

    return size;
}

void ParrotBuffer_write(ParrotBuffer *self, const void *data_ptr, size_t size) {
    const uint8_t *data = data_ptr;

    PARROT_FAIL_NULL(self);
    PARROT_FAIL_NULL(data);

    PARROT_RET_COND(!self->write);

    uint16_t endian_test_big = 1;
    uint8_t endian_test = 0;
    memcpy(&endian_test, &endian_test_big, sizeof(uint8_t));

    ParrotBufferEndian host_endian = endian_test == 1 ? ParrotBufferEndian_LITTLE : ParrotBufferEndian_BIG;
    if (self->endian != ParrotBufferEndian_HOST && self->endian != host_endian) {
        for (size_t i = size; i > 0; i--) {
            self->write(self->scope, data[i - 1]);
            self->bytes_written++;
        }
    } else {
        for (size_t i = 0; i < size; i++) {
            self->write(self->scope, data[i]);
            self->bytes_written++;
        }
    }
}

bool ParrotBuffer_read8(ParrotBuffer *self, uint8_t *out) {
    return ParrotBuffer_read(self, out, sizeof(*out));
}

bool ParrotBuffer_read16(ParrotBuffer *self, uint16_t *out) {
    return ParrotBuffer_read(self, out, sizeof(*out));
}

bool ParrotBuffer_read32(ParrotBuffer *self, uint32_t *out) {
    return ParrotBuffer_read(self, out, sizeof(*out));
}

bool ParrotBuffer_read64(ParrotBuffer *self, uint64_t *out) {
    return ParrotBuffer_read(self, out, sizeof(*out));
}

void ParrotBuffer_write8(ParrotBuffer *self, uint8_t data) {
    ParrotBuffer_write(self, &data, sizeof(data));
}

void ParrotBuffer_write16(ParrotBuffer *self, uint16_t data) {
    ParrotBuffer_write(self, &data, sizeof(data));
}

void ParrotBuffer_write32(ParrotBuffer *self, uint32_t data) {
    ParrotBuffer_write(self, &data, sizeof(data));
}

void ParrotBuffer_write64(ParrotBuffer *self, uint64_t data) {
    ParrotBuffer_write(self, &data, sizeof(data));
}

bool ParrotBuffer_read8s(ParrotBuffer *self, int8_t *out) {
    PARROT_FAIL_NULL(self);
    PARROT_FAIL_NULL(out);

    uint8_t value;

    if (!ParrotBuffer_read8(self, &value)) {
        return false;
    }

    if (value <= INT8_MAX) {
        *out = (int8_t)value;
    } else {
        *out = -(int8_t)(UINT8_MAX - value) - 1;
    }

    return true;
}

bool ParrotBuffer_read16s(ParrotBuffer *self, int16_t *out) {
    PARROT_FAIL_NULL(self);
    PARROT_FAIL_NULL(out);

    uint16_t value;

    if (!ParrotBuffer_read16(self, &value)) {
        return false;
    }

    if (value <= INT16_MAX) {
        *out = (int16_t)value;
    } else {
        *out = -(int16_t)(UINT16_MAX - value) - 1;
    }

    return true;
}

bool ParrotBuffer_read32s(ParrotBuffer *self, int32_t *out) {
    PARROT_FAIL_NULL(self);
    PARROT_FAIL_NULL(out);

    uint32_t value;

    if (!ParrotBuffer_read32(self, &value)) {
        return false;
    }

    if (value <= INT32_MAX) {
        *out = (int32_t)value;
    } else {
        *out = -(int32_t)(UINT32_MAX - value) - 1;
    }

    return true;
}

bool ParrotBuffer_read64s(ParrotBuffer *self, int64_t *out) {
    PARROT_FAIL_NULL(self);
    PARROT_FAIL_NULL(out);

    uint64_t value;

    if (!ParrotBuffer_read64(self, &value)) {
        return false;
    }

    if (value <= INT64_MAX) {
        *out = (int64_t)value;
    } else {
        *out = -(int64_t)(UINT64_MAX - value) - 1;
    }

    return true;
}

void ParrotBuffer_write8s(ParrotBuffer *self, int8_t data) {
    PARROT_FAIL_NULL(self);

    uint8_t value;

    if (data < 0) {
        value = (uint8_t)(UINT8_MAX - (uint8_t)(-(data + 1)));
    } else {
        value = (uint8_t)data;
    }

    ParrotBuffer_write8(self, value);
}

void ParrotBuffer_write16s(ParrotBuffer *self, int16_t data) {
    PARROT_FAIL_NULL(self);

    uint16_t value;

    if (data < 0) {
        value = (uint16_t)(UINT16_MAX - (uint16_t)(-(data + 1)));
    } else {
        value = (uint16_t)data;
    }

    ParrotBuffer_write16(self, value);
}

void ParrotBuffer_write32s(ParrotBuffer *self, int32_t data) {
    PARROT_FAIL_NULL(self);

    uint32_t value;

    if (data < 0) {
        value = (uint32_t)(UINT32_MAX - (uint32_t)(-(data + 1)));
    } else {
        value = (uint32_t)data;
    }

    ParrotBuffer_write32(self, value);
}

void ParrotBuffer_write64s(ParrotBuffer *self, int64_t data) {
    PARROT_FAIL_NULL(self);

    uint64_t value;

    if (data < 0) {
        value = (uint64_t)(UINT64_MAX - (uint64_t)(-(data + 1)));
    } else {
        value = (uint64_t)data;
    }

    ParrotBuffer_write64(self, value);
}

void ParrotBuffer_write_ascii(ParrotBuffer *self, const char *str) {
    PARROT_FAIL_NULL(self);
    PARROT_FAIL_NULL(str);

    while (*str) {
        ParrotBuffer_write8(self, *str++);
    }
    ParrotBuffer_write8(self, 0);
}
