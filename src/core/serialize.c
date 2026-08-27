#include "parrot/core/serialize.h"
#include "parrot/core/binary.h"
#include "parrot/core/buffer.h"
#include "parrot/core/file.h"
#include "parrot/core/math.h"
#include "parrot/core/reflect.h"
#include "parrot/core/scope.h"
#include "parrot/core/util.h"
#include "src/core/buffer.h"
#include "src/ds.h"
#include "stb_ds.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef uint16_t ObjectEntryType;
#define ObjectEntryType_OBJECT (0x00)
#define ObjectEntryType_SINT (0x10)
#define ObjectEntryType_UINT (0x18)
#define ObjectEntryType_ASCII (0x20)
#define ObjectEntryType_NUMBER (0x28)

typedef struct {
    char *name;
    uint32_t index;
} ObjectEntryFieldEntry;

typedef struct {
    ParrotScope *scope;

    ObjectEntryType type;

    union {
        struct {
            ptrdiff_t type;
            const uint8_t *data;
            ObjectEntryFieldEntry **p_arr_fields;
        } object;
        int64_t sint;
        uint64_t uint;
        char ascii;
        double number;
    } data;
} ObjectEntry;

typedef struct {
    const uint8_t *ptr;
    size_t type;
} ObjectPtrMapKey;

typedef struct {
    ObjectPtrMapKey key;
    uint32_t value;
} ObjectPtrMap;

static uint32_t queue_object(ObjectEntry **p_arr_objects,
                             ObjectPtrMap **p_hm_ptr_map,
                             ParrotScope *scope,
                             ParrotReflect *reflect,
                             size_t type,
                             const uint8_t *data,
                             bool with_ptrs) {
#define arr_objects (*p_arr_objects)
#define hm_ptr_map (*p_hm_ptr_map)

    ParrotScope *work_scope = ParrotScope_new(scope);

    if (hmgeti(hm_ptr_map, data) >= 0) {
        ParrotScope_delete(work_scope);
        return hmgeti(hm_ptr_map, data);
    }

    ObjectPtrMapKey entry_key = {
        .ptr = data,
        .type = type,
    };

    uint32_t entry_index = arrlen(arr_objects);
    {
        ObjectEntry entry_temp = {0};

        entry_temp.scope = ParrotScope_new(scope);

        entry_temp.type = ObjectEntryType_OBJECT;
        entry_temp.data.object.type = type;
        entry_temp.data.object.data = data;

        entry_temp.data.object.p_arr_fields = malloc(sizeof(*entry_temp.data.object.p_arr_fields));
        ParrotScope_push_free(entry_temp.scope, entry_temp.data.object.p_arr_fields);
        *entry_temp.data.object.p_arr_fields = NULL;
        ParrotScope_push_arrfree(entry_temp.scope, *entry_temp.data.object.p_arr_fields);

        arrpush(arr_objects, entry_temp);
    }
    hmput(hm_ptr_map, entry_key, entry_index);

    for (size_t i = 0; i < ParrotReflect_get_type_field_count(reflect, type); i++) {
        char *field_typename = ParrotReflect_get_type_field_typename(reflect, type, i);
        ParrotScope_push_free(work_scope, field_typename);

        ptrdiff_t field_type = ParrotReflect_resolve_type(reflect, field_typename);
        if (field_type < 0) {
            continue;
        }

        size_t ptr_level = 0;
        free(ParrotReflect_parse_type(field_typename, &ptr_level, NULL));

        if (ptr_level > 0 && !with_ptrs) {
            continue;
        }

        const uint8_t *field_data = data + ParrotReflect_get_type_field_offset(reflect, type, i);
        for (size_t j = 0; j < ptr_level; j++) {
            field_data = *(const uint8_t **)field_data;
            if (!field_data) {
                break;
            }
        }

        if (!field_data) {
            continue;
        }

        ObjectEntryFieldEntry field = {0};

        field.name = ParrotReflect_get_type_field_name(reflect, type, i);
        ParrotScope_push_free(arr_objects[entry_index].scope, field.name);

        field.index = queue_object(p_arr_objects, p_hm_ptr_map, scope, reflect, field_type, field_data, with_ptrs);

        arrpush(*arr_objects[entry_index].data.object.p_arr_fields, field);
    }

    ParrotScope_delete(work_scope);
    return entry_index;

#undef hm_ptr_map
#undef arr_objects
}

static void generate_object_queue(ObjectEntry **p_arr_objects,
                                  ParrotScope *scope,
                                  ParrotReflect *reflect,
                                  size_t type,
                                  const uint8_t *data,
                                  bool with_ptrs) {
    ObjectPtrMap *hm_ptr_map = NULL;

    queue_object(p_arr_objects, &hm_ptr_map, scope, reflect, type, data, with_ptrs);

    hmfree(hm_ptr_map);
}

static void scope_ParrotBuffer_free_wrapper(void *ctx) {
    ParrotBuffer_delete(ctx);
}

ParrotMutableBinaryImage
Parrot_serialize_bytes(ParrotReflect *reflect, size_t type, const void *data_ptr, bool with_ptrs) {
    PARROT_FAIL_NULL(reflect);
    PARROT_FAIL_NULL(data_ptr);

    ParrotScope *scope = ParrotScope_new(NULL);

    ObjectEntry *arr_objects = NULL;
    ParrotScope_push_arrfree(scope, arr_objects);
    generate_object_queue(&arr_objects, scope, reflect, type, data_ptr, with_ptrs);

    uint8_t *arr_data = NULL;
    ParrotScope_push_arrfree(scope, arr_data);

    ParrotBuffer *buffer = ParrotBuffer_new_stbds_array(arr_data);
    ParrotScope_push(scope, scope_ParrotBuffer_free_wrapper, buffer);

    ParrotBuffer_set_endian(buffer, ParrotBufferEndian_BIG);

    uint32_t object_count = arrlen(arr_objects);
    ParrotBuffer_write32(buffer, object_count);

    for (size_t i = 0; i < arrlen(arr_objects); i++) {
        char *typename = ParrotReflect_get_type_name(reflect, arr_objects[i].data.object.type);
        ParrotScope_push_free(scope, typename);

        bool primitive = false;

#define X(type_, signed_)                                                                                               \
    do {                                                                                                                \
        if (strcmp(typename, #type_) == 0) {                                                                            \
            ParrotBuffer_write16(buffer, (signed_) ? ObjectEntryType_SINT : ObjectEntryType_UINT);                      \
            if (signed_) {                                                                                              \
                int64_t val = 0;                                                                                        \
                switch (sizeof(type_)) {                                                                                \
                case 1:                                                                                                 \
                    val = (int64_t)*(int8_t *)arr_objects[i].data.object.data;                                          \
                    break;                                                                                              \
                case 2:                                                                                                 \
                    val = (int64_t)*(int16_t *)arr_objects[i].data.object.data;                                         \
                    break;                                                                                              \
                case 4:                                                                                                 \
                    val = (int64_t)*(int32_t *)arr_objects[i].data.object.data;                                         \
                    break;                                                                                              \
                case 8:                                                                                                 \
                    val = (int64_t)*(int64_t *)arr_objects[i].data.object.data;                                         \
                    break;                                                                                              \
                default:                                                                                                \
                    val = 0;                                                                                            \
                    break;                                                                                              \
                }                                                                                                       \
                ParrotBuffer_write64s(buffer, val);                                                                     \
            } else {                                                                                                    \
                uint64_t val = 0;                                                                                       \
                switch (sizeof(type_)) {                                                                                \
                case 1:                                                                                                 \
                    val = (uint64_t)*(uint8_t *)arr_objects[i].data.object.data;                                        \
                    break;                                                                                              \
                case 2:                                                                                                 \
                    val = (uint64_t)*(uint16_t *)arr_objects[i].data.object.data;                                       \
                    break;                                                                                              \
                case 4:                                                                                                 \
                    val = (uint64_t)*(uint32_t *)arr_objects[i].data.object.data;                                       \
                    break;                                                                                              \
                case 8:                                                                                                 \
                    val = (uint64_t)*(uint64_t *)arr_objects[i].data.object.data;                                       \
                    break;                                                                                              \
                default:                                                                                                \
                    val = 0;                                                                                            \
                    break;                                                                                              \
                }                                                                                                       \
                ParrotBuffer_write64(buffer, val);                                                                      \
            }                                                                                                           \
        }                                                                                                               \
    } while (0)
        X(short, true);
        X(int, true);
        X(long, true);
        X(long long, true);

        X(signed short, true);
        X(signed int, true);
        X(signed long, true);
        X(signed long long, true);

        X(unsigned short, false);
        X(unsigned int, false);
        X(unsigned long, false);
        X(unsigned long long, false);

        X(int8_t, true);
        X(int16_t, true);
        X(int32_t, true);
        X(int64_t, true);

        X(uint8_t, false);
        X(uint16_t, false);
        X(uint32_t, false);
        X(uint64_t, false);
#undef X

        if (strcmp(typename, "char") == 0 || strcmp(typename, "signed char") == 0 ||
            strcmp(typename, "unsigned char") == 0) {
            ParrotBuffer_write16(buffer, ObjectEntryType_ASCII);
            ParrotBuffer_write8(buffer, Parrot_char_to_ascii(*(char *)arr_objects[i].data.object.data));
        }

        if (strcmp(typename, "float") == 0) {
            primitive = true;
            ParrotBuffer_write16(buffer, ObjectEntryType_NUMBER);
            ParrotBuffer_write64(buffer, ParrotFixed64i_from_double((double)*(float *)arr_objects[i].data.object.data));
        }

        if (strcmp(typename, "double") == 0) {
            primitive = true;
            ParrotBuffer_write16(buffer, ObjectEntryType_NUMBER);
            ParrotBuffer_write64(buffer, ParrotFixed64i_from_double(*(double *)arr_objects[i].data.object.data));
        }

        if (primitive) {
            continue;
        }

        ParrotBuffer_write16(buffer, ObjectEntryType_OBJECT);

        ParrotBuffer_write_ascii(buffer, typename);
        size_t field_count = arrlen(*arr_objects[i].data.object.p_arr_fields);
        ParrotBuffer_write32(buffer, field_count);

        for (size_t j = 0; j < field_count; j++) {
            ObjectEntryFieldEntry *field = (*arr_objects[i].data.object.p_arr_fields) + j;

            ParrotBuffer_write_ascii(buffer, field->name);
            ParrotBuffer_write32(buffer, field->index);
        }
    }

// #define ENABLE
#ifdef ENABLE
#undef ENABLE
    {
        FILE *file = fopen("serialization_dump.pdebug.bin", "wb");
        fwrite(arr_data, arrlen(arr_data), sizeof(uint8_t), file);
        fclose(file);
    }

    for (size_t i = 0; i < object_count; i++) {
        char *typename = ParrotReflect_get_type_name(reflect, arr_objects[i].data.object.type);
        ParrotScope_push_free(scope, typename);
        printf("[%zu] type=%s", i, typename);

        if (strcmp(typename, "float") == 0) {
            printf(" value=%f\n", *(float *)arr_objects[i].data.object.data);
            continue;
        }

        printf(" children=%zu\n", arrlen(*arr_objects[i].data.object.p_arr_fields));
        for (size_t j = 0; j < arrlen(*arr_objects[i].data.object.p_arr_fields); j++) {
            ObjectEntryFieldEntry *f = (*arr_objects[i].data.object.p_arr_fields) + j;
            printf("  field \"%s\" -> %u\n", f->name, f->index);
        }
    }
#endif

    ParrotMutableBinaryImage ret = {
        .data = memcpy(malloc(arrlen(arr_data)), arr_data, arrlen(arr_data)),
        .size = arrlen(arr_data),
    };

    ParrotScope_delete(scope);
    return ret;
}

typedef struct {
    const ObjectEntry *object;

    ptrdiff_t type;

    uint8_t *data;
} DeserializeStateObject;

typedef enum {
    DeserializeStateRelocationType_MOVE = 0,
    DeserializeStateRelocationType_POINT,
} DeserializeStateRelocationType;

typedef struct {
    DeserializeStateRelocationType type;

    size_t dest_object;
    size_t dest_offset;
    size_t src_object;

    union {
        struct {
            size_t ptr_level;
        } point;
    } unique_data;
} DeserializeStateRelocation;

typedef struct {
    DeserializeStateObject *arr_objects;
    DeserializeStateRelocation *arr_relocations;
} DeserializeState;

static void
deserialize_object(DeserializeState *state, size_t object, ptrdiff_t type, ParrotReflect *reflect, bool with_ptrs) {
    PARROT_RET_COND(state->arr_objects[object].data);

    ParrotScope *scope = ParrotScope_new(NULL);

    state->arr_objects[object].type = type;

    state->arr_objects[object].data = malloc(ParrotReflect_get_type_size(reflect, type));
    memset(state->arr_objects[object].data, 0, ParrotReflect_get_type_size(reflect, type));

    char *typename = ParrotReflect_get_type_name(reflect, type);
    ParrotScope_push_free(scope, typename);

    switch (state->arr_objects[object].object->type) {
    case ObjectEntryType_OBJECT: {
        for (size_t i = 0; i < arrlen(*state->arr_objects[object].object->data.object.p_arr_fields); i++) {
            ParrotScope *field_scope = ParrotScope_new(scope);

            ptrdiff_t field = ParrotReflect_get_type_field(
                reflect, type, (*state->arr_objects[object].object->data.object.p_arr_fields)[i].name);

            if (field < 0) {
                ParrotScope_delete(field_scope);
                continue;
            }

            char *field_typename = ParrotReflect_get_type_field_typename(reflect, type, field);
            ParrotScope_push_free(scope, field_typename);

            size_t ptr_level = 0;
            ptrdiff_t field_type = ParrotReflect_resolve_type_ex(reflect, field_typename, NULL, &ptr_level, NULL);
            if (field_type < 0) {
                ParrotScope_delete(field_scope);
                continue;
            }

            DeserializeStateRelocation relocation = {
                .dest_object = object,
                .dest_offset = ParrotReflect_get_type_field_offset(reflect, type, field),
                .src_object = (*state->arr_objects[object].object->data.object.p_arr_fields)[i].index,
            };
            relocation.type =
                ptr_level <= 0 ? DeserializeStateRelocationType_MOVE : DeserializeStateRelocationType_POINT;

            if (relocation.type == DeserializeStateRelocationType_POINT) {
                relocation.unique_data.point.ptr_level = ptr_level;
            }

            deserialize_object(state, relocation.src_object, field_type, reflect, with_ptrs);

            ParrotReflect_get_type_field_size(reflect, type, field);
            arrpush(state->arr_relocations, relocation);

            ParrotScope_delete(field_scope);
        }
    } break;
    case ObjectEntryType_SINT: {
        if (strcmp(typename, "int8_t") == 0) {
            *(int8_t *)state->arr_objects[object].data = state->arr_objects[object].object->data.sint;
        } else if (strcmp(typename, "int16_t") == 0) {
            *(int16_t *)state->arr_objects[object].data = state->arr_objects[object].object->data.sint;
        } else if (strcmp(typename, "int32_t") == 0) {
            *(int32_t *)state->arr_objects[object].data = state->arr_objects[object].object->data.sint;
        } else if (strcmp(typename, "int64_t") == 0) {
            *(int64_t *)state->arr_objects[object].data = state->arr_objects[object].object->data.sint;
        }
    } break;
    case ObjectEntryType_UINT: {
        if (strcmp(typename, "uint8_t") == 0) {
            *(uint8_t *)state->arr_objects[object].data = state->arr_objects[object].object->data.uint;
        } else if (strcmp(typename, "uint16_t") == 0) {
            *(uint16_t *)state->arr_objects[object].data = state->arr_objects[object].object->data.uint;
        } else if (strcmp(typename, "uint32_t") == 0) {
            *(uint32_t *)state->arr_objects[object].data = state->arr_objects[object].object->data.uint;
        } else if (strcmp(typename, "uint64_t") == 0) {
            *(uint64_t *)state->arr_objects[object].data = state->arr_objects[object].object->data.uint;
        }
    } break;
    case ObjectEntryType_ASCII:
        if (strcmp(typename, "char") == 0 || strcmp(typename, "signed char") == 0 ||
            strcmp(typename, "unsigned char") == 0) {
            memcpy(state->arr_objects[object].data, &state->arr_objects[object].object->data.ascii, sizeof(char));
        }
        break;
    case ObjectEntryType_NUMBER: {
        if (strcmp(typename, "float") == 0) {
            float value = state->arr_objects[object].object->data.number;
            memcpy(state->arr_objects[object].data, &value, sizeof(float));
        } else if (strcmp(typename, "double") == 0) {
            memcpy(state->arr_objects[object].data, &state->arr_objects[object].object->data.number, sizeof(double));
        }
    } break;
    }

    ParrotScope_delete(scope);
}

static void *
deserialize(ObjectEntry *objects, size_t object_count, ParrotReflect *reflect, size_t *out_type, bool with_ptrs) {
    PARROT_RET_COND_V(object_count <= 0, NULL);

    ParrotScope *scope = ParrotScope_new(NULL);

    *out_type = objects[0].data.object.type;

    DeserializeState state = {0};
    ParrotScope_push_arrfree(scope, state.arr_objects);

    for (size_t i = 0; i < object_count; i++) {
        DeserializeStateObject object = {0};

        object.object = &objects[i];

        arrpush(state.arr_objects, object);
    }

    deserialize_object(&state, 0, *out_type, reflect, with_ptrs);

    size_t pool_size = ParrotReflect_get_type_size(reflect, state.arr_objects[0].type);
    for (size_t i = 0; i < arrlen(state.arr_relocations); i++) {
        DeserializeStateRelocation relocation = state.arr_relocations[i];

        if (relocation.type != DeserializeStateRelocationType_POINT) {
            continue;
        }

        pool_size += (relocation.unique_data.point.ptr_level - 1) * sizeof(void *);

        // OPTIMIZE: Reduce unneeded size increase by only increasing pool size once per point of that object
        pool_size += ParrotReflect_get_type_size(reflect, state.arr_objects[relocation.src_object].type);
    }
    state.arr_objects[0].data = realloc(state.arr_objects[0].data, pool_size);
    uint8_t *pool_ptr = state.arr_objects[0].data + ParrotReflect_get_type_size(reflect, state.arr_objects[0].type);

    for (size_t i = 0; i < arrlen(state.arr_relocations); i++) {
        DeserializeStateRelocation relocation = state.arr_relocations[i];

        if (relocation.type != DeserializeStateRelocationType_MOVE) {
            continue;
        }

        memcpy(state.arr_objects[relocation.dest_object].data + relocation.dest_offset,
               state.arr_objects[relocation.src_object].data,
               ParrotReflect_get_type_size(reflect, state.arr_objects[relocation.src_object].type));
        free(state.arr_objects[relocation.src_object].data);
        state.arr_objects[relocation.src_object].data =
            state.arr_objects[relocation.dest_object].data + relocation.dest_offset;
    }

    ParrotSizeSet *shm_relocated = NULL;
    ParrotScope_push_hmfree(scope, shm_relocated);

    hmputs(shm_relocated,
           ((ParrotSizeSet){
               .key = 0,
           }));

    for (size_t i = 0; i < arrlen(state.arr_relocations); i++) {
        DeserializeStateRelocation relocation = state.arr_relocations[i];

        if (relocation.type != DeserializeStateRelocationType_POINT) {
            continue;
        }

        if (hmgeti(shm_relocated, relocation.src_object) >= 0) {
            continue;
        }
        memcpy(pool_ptr,
               state.arr_objects[relocation.src_object].data,
               ParrotReflect_get_type_size(reflect, state.arr_objects[relocation.src_object].type));
        free(state.arr_objects[relocation.src_object].data);

        state.arr_objects[relocation.src_object].data = pool_ptr;
        pool_ptr += ParrotReflect_get_type_size(reflect, state.arr_objects[relocation.src_object].type);

        hmputs(shm_relocated,
               ((ParrotSizeSet){
                   .key = relocation.src_object,
               }));
    }

    for (size_t i = 0; i < arrlen(state.arr_relocations); i++) {
        DeserializeStateRelocation relocation = state.arr_relocations[i];

        if (relocation.type != DeserializeStateRelocationType_POINT) {
            continue;
        }

        memcpy(state.arr_objects[relocation.dest_object].data + relocation.dest_offset,
               &state.arr_objects[relocation.src_object].data,
               sizeof(void *));
    }

    void *ret = state.arr_objects[0].data;
    ParrotScope_delete(scope);
    return ret;
}

void *Parrot_deserialize_bytes(ParrotBinaryImage image, ParrotReflect *reflect, size_t *out_type, bool with_ptrs) {
    PARROT_FAIL_NULL(image.data);
    PARROT_FAIL_NULL(reflect);
    PARROT_FAIL_NULL(out_type);

    ParrotScope *scope = ParrotScope_new(NULL);

    ParrotBuffer *buffer = ParrotBuffer_new_bytearray(NULL, image.data, image.size, NULL);
    ParrotScope_push(scope, scope_ParrotBuffer_free_wrapper, buffer);

    ObjectEntry *arr_objects = NULL;
    ParrotScope_push_arrfree(scope, arr_objects);

    ParrotBuffer_set_endian(buffer, ParrotBufferEndian_BIG);

#define FAIL()                                                                                                          \
    do {                                                                                                                \
        ParrotScope_delete(scope);                                                                                      \
        return NULL;                                                                                                    \
    } while (0)
#define CHECK_FAIL(expr)                                                                                                \
    do {                                                                                                                \
        if (!(expr)) {                                                                                                  \
            FAIL();                                                                                                     \
        }                                                                                                               \
    } while (0)

    uint32_t object_count = 0;
    CHECK_FAIL(ParrotBuffer_read32(buffer, &object_count));

    for (size_t i = 0; i < object_count; i++) {
        ParrotScope *object_scope = ParrotScope_new(scope);

        ObjectEntry entry = {0};
        entry.scope = ParrotScope_new(object_scope);

        CHECK_FAIL(ParrotBuffer_read(buffer, &entry.type, sizeof(entry.type)));

        switch (entry.type) {
        case ObjectEntryType_OBJECT: {
            char *arr_object_type = NULL;
            ParrotScope_push_arrfree(object_scope, arr_object_type);
            for (;;) {
                uint8_t ascii = 0;
                CHECK_FAIL(ParrotBuffer_read8(buffer, &ascii));

                arrpush(arr_object_type, Parrot_ascii_to_char(ascii));

                if (ascii == 0) {
                    break;
                }
            }

            entry.data.object.type = ParrotReflect_resolve_type(reflect, arr_object_type);
            if (entry.data.object.type < 0) {
                FAIL();
            }

            uint32_t field_count = 0;
            CHECK_FAIL(ParrotBuffer_read32(buffer, &field_count));

            entry.data.object.p_arr_fields = malloc(sizeof(*entry.data.object.p_arr_fields));
            ParrotScope_push_arrfree(entry.scope, *entry.data.object.p_arr_fields);
            *entry.data.object.p_arr_fields = NULL;

            for (size_t j = 0; j < field_count; j++) {
                ParrotScope *field_scope = ParrotScope_new(object_scope);

                char *arr_field_name = NULL;
                ParrotScope_push_arrfree(field_scope, arr_field_name);
                for (;;) {
                    uint8_t ascii = 0;
                    CHECK_FAIL(ParrotBuffer_read8(buffer, &ascii));

                    arrpush(arr_field_name, Parrot_ascii_to_char(ascii));

                    if (ascii == 0) {
                        break;
                    }
                }

                uint32_t index = 0;
                CHECK_FAIL(ParrotBuffer_read32(buffer, &index));

                ObjectEntryFieldEntry field_entry = {0};

                field_entry.name = calloc(arrlen(arr_field_name), sizeof(char));
                ParrotScope_push_free(entry.scope, field_entry.name);
                strcpy(field_entry.name, arr_field_name);

                field_entry.index = index;

                arrpush(*entry.data.object.p_arr_fields, field_entry);

                ParrotScope_delete(field_scope);
            }
        } break;
        case ObjectEntryType_SINT:
            ParrotBuffer_read64s(buffer, &entry.data.sint);
            break;

        case ObjectEntryType_UINT:
            ParrotBuffer_read64(buffer, &entry.data.uint);
            break;
        case ObjectEntryType_ASCII: {
            uint8_t ascii = 0;
            ParrotBuffer_read8(buffer, &ascii);
            entry.data.ascii = Parrot_ascii_to_char(ascii);
            break;
        }
        case ObjectEntryType_NUMBER: {
            ParrotFixed64i number = 0;
            CHECK_FAIL(ParrotBuffer_read64s(buffer, &number));
            entry.data.number = ParrotFixed64i_to_double(number);
        } break;
        default:
            FAIL();
            break;
        }

        ParrotScope_set_parent(entry.scope, scope);
        ParrotScope_delete(object_scope);

        arrpush(arr_objects, entry);
    }

#undef CHECK_FAIL
#undef FAIL

    void *ret = deserialize(arr_objects, arrlen(arr_objects), reflect, out_type, with_ptrs);
    ParrotScope_delete(scope);
    return ret;
}
