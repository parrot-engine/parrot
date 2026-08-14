#ifndef PARROT_PARROT_INCLUDE_PARROT_CORE_MODULE_H_
#define PARROT_PARROT_INCLUDE_PARROT_CORE_MODULE_H_

#include "parrot/core/main_loop.h"
#include "parrot/core/math.h"
#include "parrot/core/reflect.h"

static const ParrotReflectDescription Parrot_core_collection[] = {
    PARROT_REFLECT_COLLECTION_HEADER(),

    PARROT_REFLECT_COLLECTION_DESCRIPTION(Parrot_core_math_collection),
    PARROT_REFLECT_COLLECTION_DESCRIPTION(Parrot_core_main_loop_collection),

    PARROT_REFLECT_END(),
};

#endif // PARROT_PARROT_INCLUDE_PARROT_CORE_MODULE_H_
