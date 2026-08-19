#ifndef __SRC_PARROT_INCLUDE_PARROT_CORE_REFLECT_H_
#define __SRC_PARROT_INCLUDE_PARROT_CORE_REFLECT_H_

#include "parrot/core/api.h"
#include "parrot/core/util.h"
#include <stddef.h>
#include <stdint.h>

#define ParrotReflect_MAX_TAGS 16

typedef enum {
    ParrotReflectEntryType_END = 0,

    ParrotReflectEntryType_TYPE_HEADER = 0x100,
    ParrotReflectEntryType_TYPE_FIELD,

    ParrotReflectEntryType_ALIAS = 0x200,

    ParrotReflectEntryType_COLLECTION_HEADER = 0x300,
    ParrotReflectEntryType_COLLECTION_DESCRIPTION,
} ParrotReflectEntryType;

typedef struct ParrotReflectDescription ParrotReflectDescription;

struct ParrotReflectDescription {
    ParrotReflectEntryType type;

    const char *name;
    const char *tags[ParrotReflect_MAX_TAGS];

    union {
        struct {
            size_t size;
        } type_header_data;
        struct {
            size_t offset;

            size_t field_base_size;

            const char *type;
            const char *suffix;
        } type_field_data;

        struct {
            const char *name;

            const char *alias_of_name;
            size_t size;
        } alias_data;

        struct {
            const ParrotReflectDescription *description;
        } collection_description_data;
    } data;
};

#define PARROT_REFLECT_END()                                                                                            \
    {                                                                                                                   \
        .type = ParrotReflectEntryType_END,                                                                             \
    }

#define PARROT_REFLECT_TYPE_HEADER(p_type)                                                                              \
    {                                                                                                                   \
        .type = ParrotReflectEntryType_TYPE_HEADER,                                                                     \
        .name = PARROT_STRING(p_type),                                                                                  \
        .data =                                                                                                         \
            {                                                                                                           \
                .type_header_data =                                                                                     \
                    {                                                                                                   \
                        .size = sizeof(p_type),                                                                         \
                    },                                                                                                  \
            },                                                                                                          \
    }
#define PARROT_REFLECT_TYPE_HEADER_TAG(p_type, ...)                                                                     \
    {                                                                                                                   \
        .type = ParrotReflectEntryType_TYPE_HEADER,                                                                     \
        .name = PARROT_STRING(type),                                                                                    \
        .data{                                                                                                          \
            .type_header_data =                                                                                         \
                {                                                                                                       \
                    .size = sizeof(p_type),                                                                             \
                                                                                                                        \
                    .tags = {__VA_ARGS__},                                                                              \
                },                                                                                                      \
        },                                                                                                              \
    }
#define PARROT_REFLECT_TYPE_FIELD(p_type, p_field_type, p_field_name, p_field_suffix)                                   \
    {                                                                                                                   \
        .type = ParrotReflectEntryType_TYPE_FIELD,                                                                      \
        .name = PARROT_STRING(field_name),                                                                              \
        .data =                                                                                                         \
            {                                                                                                           \
                .type_field_data =                                                                                      \
                    {                                                                                                   \
                        .offset = offsetof(p_type, p_field_name),                                                       \
                        .field_base_size = sizeof(p_field_type),                                                        \
                        .type = PARROT_STRING(p_field_type),                                                            \
                        .suffix = PARROT_STRING(p_field_suffix),                                                        \
                    },                                                                                                  \
            },                                                                                                          \
    }
#define PARROT_REFLECT_TYPE_FIELD_TAG(p_type, p_field_type, p_field_name, p_field_suffix, ...)                          \
    {                                                                                                                   \
        .type = ParrotReflectEntryType_TYPE_FIELD,                                                                      \
        .name = PARROT_STRING(p_field_name),                                                                            \
        .tags = {__VA_ARGS__},                                                                                          \
        .data =                                                                                                         \
            {                                                                                                           \
                .type_field_data =                                                                                      \
                    {                                                                                                   \
                        .offset = offsetof(p_type, p_field_name),                                                       \
                        .field_base_size = sizeof(p_field_type),                                                        \
                        .type = PARROT_STRING(p_field_type),                                                            \
                        .suffix = PARROT_STRING(p_field_suffix),                                                        \
                    },                                                                                                  \
            },                                                                                                          \
    }

/// Immediately acts as the end as well
#define PARROT_REFLECT_ALIAS(p_type, p_alias_of)                                                                        \
    {                                                                                                                   \
        .type = ParrotReflectEntryType_ALIAS,                                                                           \
        .name = PARROT_STRING(p_type),                                                                                  \
        .data =                                                                                                         \
            {                                                                                                           \
                .alias_data =                                                                                           \
                    {                                                                                                   \
                        .alias_of_name = PARROT_STRING(p_alias_of),                                                     \
                        .size = sizeof(p_alias_of),                                                                     \
                    },                                                                                                  \
            },                                                                                                          \
    }

#define PARROT_REFLECT_COLLECTION_HEADER()                                                                              \
    {                                                                                                                   \
        .type = ParrotReflectEntryType_COLLECTION_HEADER,                                                               \
    }
#define PARROT_REFLECT_COLLECTION_DESCRIPTION(p_description)                                                            \
    {                                                                                                                   \
        .type = ParrotReflectEntryType_COLLECTION_DESCRIPTION,                                                          \
        .data =                                                                                                         \
            {                                                                                                           \
                .collection_description_data =                                                                          \
                    {                                                                                                   \
                        .description = p_description,                                                                   \
                    },                                                                                                  \
            },                                                                                                          \
    }

typedef struct ParrotReflect ParrotReflect;

PARROT_API ParrotReflect *ParrotReflect_new(void);
PARROT_API void ParrotReflect_delete(ParrotReflect *self);

PARROT_API void ParrotReflect_register(ParrotReflect *self, const ParrotReflectDescription *description);
/// Does not error on unregister of not registered type
PARROT_API void ParrotReflect_unregister(ParrotReflect *self, const char *type);

/// 0< = Not found
PARROT_API ptrdiff_t ParrotReflect_resolve_type(ParrotReflect *self, const char *type);

PARROT_API size_t ParrotReflect_get_type_size(ParrotReflect *self, size_t type);

PARROT_API size_t ParrotReflect_get_type_field_count(ParrotReflect *self, size_t type);
PARROT_API size_t ParrotReflect_get_type_field_offset(ParrotReflect *self, size_t type, size_t field);
/// Return type is allocated with malloc() that the caller tkes ownership of
PARROT_API char *ParrotReflect_get_type_field_typename(ParrotReflect *self, size_t type, size_t field);
/// Return type is allocated with malloc() that the caller takes ownership of
PARROT_API char *ParrotReflect_get_type_field_name(ParrotReflect *self, size_t type, size_t field);
/// Returns 0 if end of dimensions
PARROT_API size_t ParrotReflect_get_type_field_dimension_size(ParrotReflect *self,
                                                              size_t type,
                                                              size_t field,
                                                              /* 0 = Base size */ size_t n);
PARROT_API size_t ParrotReflect_get_type_field_size(ParrotReflect *self, size_t type, size_t field);

#endif // PARROT_PARROT_INCLUDE_PARROT_CORE_REFLECT_H_
