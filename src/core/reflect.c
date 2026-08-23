#include "parrot/core/reflect.h"
#include "parrot/core/scope.h"
#include "parrot/core/util.h"
#include "src/ds.h"
#include "stb_ds.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FAIL_INVALID_TYPE(type) PARROT_FAIL_FMT("Unknown entry type: %d", type)
#define FAIL_REGISTERED_TYPE(type)                                                                                      \
    PARROT_FAIL_COND_MSG(ParrotReflect_resolve_type(self, type) >= 0, "Type is registered")
#define FAIL_HEADER() PARROT_FAIL_MSG("Attempt to use header twice")

#define MAX_ARRAY_DIMENSIONS 6

typedef struct {
    char *key;

    ParrotScope *scope;

    size_t offset;

    char *type;
    char *tags[ParrotReflect_MAX_TAGS];

    size_t size[MAX_ARRAY_DIMENSIONS + 1];
    size_t dimension_count;
} ParrotReflectTypeFieldInfo;

typedef struct {
    char *key;

    ParrotScope *scope;

    char *tags[ParrotReflect_MAX_TAGS];

    size_t size;

    ParrotReflectTypeFieldInfo **sh_fields;
} ParrotReflectTypeInfo;

typedef struct {
    char *key;

    ParrotScope *scope;

    char *value;
} ParrotReflectAliasInfo;

struct ParrotReflect {
    ParrotScope *scope;

    ParrotReflectTypeInfo *sh_types;
    ParrotReflectAliasInfo *sh_aliases;
};

#define BUILTIN_TYPES                                                                                                   \
    X(char, char)                                                                                                       \
    X(int, int)                                                                                                         \
    X(short, short)                                                                                                     \
    X(long, long)                                                                                                       \
    X(ll, long long)                                                                                                    \
    X(uchar, unsigned char)                                                                                             \
    X(uint, unsigned int)                                                                                               \
    X(ushort, unsigned short)                                                                                           \
    X(ulong, unsigned long)                                                                                             \
    X(ull, unsigned long long)                                                                                          \
    X(bool, bool)                                                                                                       \
    X(int8, int8_t)                                                                                                     \
    X(int16, int16_t)                                                                                                   \
    X(int32, int32_t)                                                                                                   \
    X(int64, int64_t)                                                                                                   \
    X(uint8, uint8_t)                                                                                                   \
    X(uint16, uint16_t)                                                                                                 \
    X(uint32, uint32_t)                                                                                                 \
    X(uint64, uint64_t)                                                                                                 \
    X(size, size_t)                                                                                                     \
    X(ptrdiff, ptrdiff_t)                                                                                               \
    X(float, float)                                                                                                     \
    X(double, double)

#define X(name, type)                                                                                                   \
    static const ParrotReflectDescription builtin_##name##_description[] = {                                            \
        PARROT_REFLECT_TYPE_HEADER(type),                                                                               \
        PARROT_REFLECT_END(),                                                                                           \
    };
BUILTIN_TYPES
#undef X

static const ParrotReflectDescription builtin_collection[] = {
    PARROT_REFLECT_COLLECTION_HEADER(),

#define X(name, type) PARROT_REFLECT_COLLECTION_DESCRIPTION(builtin_##name##_description),
    BUILTIN_TYPES
#undef X

        PARROT_REFLECT_END(),
};

ParrotReflect *ParrotReflect_new(void) {
    ParrotReflect *self = malloc(sizeof(ParrotReflect));
    memset(self, 0, sizeof(ParrotReflect));

    self->scope = ParrotScope_new(NULL);

    ParrotScope_push_shfree(self->scope, self->sh_types);
    ParrotScope_push_shfree(self->scope, self->sh_aliases);

    ParrotReflect_register(self, builtin_collection);
    return self;
}

void ParrotReflect_delete(ParrotReflect *self) {
    PARROT_FAIL_NULL(self);

    ParrotScope_delete(self->scope);
    free(self);
}

static void register_type(ParrotReflect *self, const ParrotReflectDescription *description) {
    ParrotReflectTypeInfo type_info = {0};
    type_info.scope = ParrotScope_new(self->scope);

    type_info.key = strcpy(calloc(strlen(description->name) + 1, sizeof(char)), description->name);
    ParrotScope_push_free(type_info.scope, type_info.key);

    for (size_t i = 0; i < ParrotReflect_MAX_TAGS; i++) {
        if (!description->tags[i]) {
            break;
        }

        type_info.tags[i] = strcpy(calloc(strlen(description->tags[i]) + 1, sizeof(char)), description->tags[i]);
        ParrotScope_push_free(type_info.scope, type_info.tags[i]);
    }

    type_info.size = description->data.type_header_data.size;

    type_info.sh_fields = malloc(sizeof(type_info.sh_fields));
    *type_info.sh_fields = NULL;
    ParrotScope_push_free(type_info.scope, type_info.sh_fields);
    ParrotScope_push_shfree(type_info.scope, *type_info.sh_fields);

    description++;

    while (description->type != ParrotReflectEntryType_END) {
        switch (description->type) {
        case ParrotReflectEntryType_END: {
        } break;

        case ParrotReflectEntryType_TYPE_HEADER: {
            FAIL_HEADER();
        } break;

        case ParrotReflectEntryType_TYPE_FIELD: {
            ParrotReflectTypeFieldInfo field_info = {0};

            field_info.scope = ParrotScope_new(type_info.scope);

            field_info.key = strcpy(calloc(strlen(description->name) + 1, sizeof(char)), description->name);
            ParrotScope_push_free(field_info.scope, field_info.key);

            field_info.offset = description->data.type_field_data.offset;

            field_info.type = strcpy(calloc(strlen(description->data.type_field_data.type) + 1, sizeof(char)),
                                     description->data.type_field_data.type);
            ParrotScope_push_free(field_info.scope, field_info.type);

            for (size_t i = 0; i < ParrotReflect_MAX_TAGS; i++) {
                if (!description->tags[i]) {
                    break;
                }

                field_info.tags[i] =
                    strcpy(calloc(strlen(description->tags[i]) + 1, sizeof(char)), description->tags[i]);
                ParrotScope_push_free(field_info.scope, field_info.tags[i]);
            }

            field_info.size[field_info.dimension_count++] = description->data.type_field_data.field_base_size;

            const char *ptr = description->data.type_field_data.suffix;
            for (;;) {
                PARROT_FAIL_COND_FMT(field_info.dimension_count >= MAX_ARRAY_DIMENSIONS + 1,
                                     "Too many dimensions in array, max is %d",
                                     MAX_ARRAY_DIMENSIONS);

                ptr = strchr(ptr, '[');
                if (!ptr) {
                    break;
                }
                ptr++;

                size_t size = strtol(ptr, NULL, 0);
                PARROT_FAIL_COND_MSG(size == 0, "Unexpected dimension with size of 0");
                field_info.size[field_info.dimension_count++] = size;

                ptr = strchr(ptr, ']');
                PARROT_FAIL_COND_FMT(!ptr, "Expected ']' in suffix: \"%s\"", description->data.type_field_data.suffix);
            }

            shputs(*type_info.sh_fields, field_info);
        } break;

        default:
            FAIL_INVALID_TYPE(description->type);
            break;
        }

        description++;
    }

    shputs(self->sh_types, type_info);
}

static void register_collection(ParrotReflect *self, const ParrotReflectDescription *description) {
    description++;

    while (description->type != ParrotReflectEntryType_END) {
        switch (description->type) {
        case ParrotReflectEntryType_COLLECTION_HEADER: {
            FAIL_HEADER();
        } break;
        case ParrotReflectEntryType_COLLECTION_DESCRIPTION: {
            ParrotReflect_register(self, description->data.collection_description_data.description);
        } break;
        default: {
            FAIL_INVALID_TYPE(description->type);
        } break;
        }

        description++;
    }
}

void ParrotReflect_register(ParrotReflect *self, const ParrotReflectDescription *description) {
    PARROT_FAIL_NULL(self);
    PARROT_FAIL_NULL(description);

    switch (description->type) {
    case ParrotReflectEntryType_TYPE_HEADER: {
        FAIL_REGISTERED_TYPE(description->name);

        register_type(self, description);
    } break;
    case ParrotReflectEntryType_ALIAS: {
        FAIL_REGISTERED_TYPE(description->name);

        ParrotReflectAliasInfo alias_info = {0};

        alias_info.scope = ParrotScope_new(self->scope);

        alias_info.key = strcpy(calloc(strlen(description->data.alias_data.name) + 1, sizeof(char)),
                                description->data.alias_data.name);
        ParrotScope_push_free(alias_info.scope, alias_info.key);

        alias_info.value = strcpy(calloc(strlen(description->data.alias_data.alias_of_name) + 1, sizeof(char)),
                                  description->data.alias_data.alias_of_name);
        ParrotScope_push_free(alias_info.scope, alias_info.value);

        shputs(self->sh_aliases, alias_info);
    } break;
    case ParrotReflectEntryType_COLLECTION_HEADER: {
        register_collection(self, description);
    } break;
    default: {
        PARROT_FAIL_MSG("First entry of description must be a header");
    } break;
    }
}

void ParrotReflect_unregister(ParrotReflect *self, const char *type) {
    ParrotReflectTypeInfo *type_info = shgetp_null(self->sh_types, type);
    if (type_info) {
        ParrotReflectTypeInfo local_type_info = *type_info;
        shdel(self->sh_types, type);

        ParrotScope_delete(local_type_info.scope);
        return;
    }

    ParrotReflectAliasInfo *alias_info = shgetp_null(self->sh_aliases, type);
    if (alias_info) {
        ParrotReflectAliasInfo local_alias_info = *alias_info;
        shdel(self->sh_aliases, type);

        ParrotScope_delete(local_alias_info.scope);
        return;
    }
}

size_t ParrotReflect_get_type_count(ParrotReflect *self) {
    PARROT_FAIL_NULL(self);
    return shlen(self->sh_types);
}

ptrdiff_t ParrotReflect_resolve_type_ex(ParrotReflect *self, const char *type, bool *out_is_ptr, bool *out_is_const) {
    PARROT_FAIL_NULL(self);
    PARROT_FAIL_NULL(type);

    if (out_is_ptr) {
        *out_is_ptr = false;
    }

    if (out_is_const) {
        *out_is_const = false;
    }

    char *arr_type_str = NULL;

    while (*type && isspace(*type)) {
        type++;
    }

    if (out_is_const && strncmp(type, "const", strlen("const")) == 0) {
        *out_is_const = true;
    }

    while (*type) {
        arrpush(arr_type_str, *type++);
    }

    while (arr_type_str[arrlen(arr_type_str) - 1] == ' ' || arr_type_str[arrlen(arr_type_str) - 1] == '*') {
        if (out_is_ptr && arr_type_str[arrlen(arr_type_str) - 1] == '*') {
            *out_is_ptr = true;
        }
        arrdel(arr_type_str, arrlen(arr_type_str) - 1);
    }

    arrpush(arr_type_str, '\0');

    type = arr_type_str;
    while (shgeti(self->sh_aliases, type) >= 0) {
        type = shget(self->sh_aliases, type);
    }

    arrfree(arr_type_str);
    return shgeti(self->sh_types, type);
}

ptrdiff_t ParrotReflect_resolve_type_by_index(ParrotReflect *self, size_t index) {
    PARROT_FAIL_NULL(self);
    return shlen(self->sh_types) > index ? (ptrdiff_t)index : -1;
}

static ParrotReflectTypeInfo *resolve_type_info(ParrotReflect *self, size_t type) {
    PARROT_FAIL_NULL(self);
    PARROT_RET_COND_V(type >= shlen(self->sh_types), NULL);

    return &self->sh_types[type];
}

char *ParrotReflect_get_type_name(ParrotReflect *self, size_t type) {
    ParrotReflectTypeInfo *type_info = resolve_type_info(self, type);
    PARROT_FAIL_NULL(type_info);
    return strcpy(calloc(strlen(type_info->key) + 1, sizeof(char)), type_info->key);
}

size_t ParrotReflect_get_type_size(ParrotReflect *self, size_t type) {
    ParrotReflectTypeInfo *type_info = resolve_type_info(self, type);
    PARROT_FAIL_NULL(type_info);
    return type_info->size;
}

size_t ParrotReflect_get_type_field_count(ParrotReflect *self, size_t type) {
    ParrotReflectTypeInfo *type_info = resolve_type_info(self, type);
    PARROT_FAIL_NULL(type_info);
    return shlen(*type_info->sh_fields);
}

size_t ParrotReflect_get_type_field_offset(ParrotReflect *self, size_t type, size_t index) {
    ParrotReflectTypeInfo *type_info = resolve_type_info(self, type);
    PARROT_FAIL_NULL(type_info);
    PARROT_FAIL_COND(index >= shlen(*type_info->sh_fields));
    return (*type_info->sh_fields)[index].offset;
}

char *ParrotReflect_get_type_field_typename(ParrotReflect *self, size_t type, size_t index) {
    ParrotReflectTypeInfo *type_info = resolve_type_info(self, type);
    PARROT_FAIL_NULL(type_info);
    PARROT_FAIL_COND(index >= shlen(*type_info->sh_fields));

    ParrotReflectTypeFieldInfo *field_info = &(*type_info->sh_fields)[index];
    return strcpy(calloc(strlen(field_info->type) + 1, sizeof(char)), field_info->type);
}

char *ParrotReflect_get_type_field_name(ParrotReflect *self, size_t type, size_t index) {
    ParrotReflectTypeInfo *type_info = resolve_type_info(self, type);
    PARROT_FAIL_NULL(type_info);
    PARROT_FAIL_COND(index >= shlen(*type_info->sh_fields));

    ParrotReflectTypeFieldInfo *field_info = &(*type_info->sh_fields)[index];
    return strcpy(calloc(strlen(field_info->key) + 1, sizeof(char)), field_info->key);
}

size_t ParrotReflect_get_type_field_dimension_size(ParrotReflect *self, size_t type, size_t field, size_t n) {
    ParrotReflectTypeInfo *type_info = resolve_type_info(self, type);
    PARROT_FAIL_NULL(type_info);
    PARROT_FAIL_COND(field >= shlen(*type_info->sh_fields));

    ParrotReflectTypeFieldInfo *field_info = &(*type_info->sh_fields)[field];
    PARROT_RET_COND_V(n >= field_info->dimension_count, 0);

    return field_info->size[n];
}

size_t ParrotReflect_get_type_field_size(ParrotReflect *self, size_t type, size_t field) {
    size_t size = 1;
    size_t i = 0;
    for (;;) {
        size_t dimension_size = ParrotReflect_get_type_field_dimension_size(self, type, field, i++);
        if (dimension_size <= 0) {
            break;
        }
        size *= dimension_size;
    }
    return size;
}
