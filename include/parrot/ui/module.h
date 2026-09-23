#ifndef PARROT_PARROT_INCLUDE_PARROT_UI_MODULE_H_
#define PARROT_PARROT_INCLUDE_PARROT_UI_MODULE_H_

#include "parrot/core/reflect.h"
#include "parrot/ui/ui.h"

static const ParrotReflectDescription Parrot_ui_collection[] = {
    PARROT_REFLECT_COLLECTION_HEADER(),

    PARROT_REFLECT_COLLECTION_DESCRIPTION(Parrot_ui_ui_collection),

    PARROT_REFLECT_END(),
};

#endif // PARROT_PARROT_INCLUDE_PARROT_UI_MODULE_H_
