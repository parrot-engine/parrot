#ifndef PARROT_PARROT_INCLUDE_PARROT_VIDEO_MODULE_H_
#define PARROT_PARROT_INCLUDE_PARROT_VIDEO_MODULE_H_

#include "parrot/core/reflect.h"
#include "parrot/video/scene.h"

static const ParrotReflectDescription Parrot_video_collection[] = {
    PARROT_REFLECT_COLLECTION_HEADER(),

    PARROT_REFLECT_COLLECTION_DESCRIPTION(Parrot_video_scene_collection),

    PARROT_REFLECT_END(),
};

#endif // PARROT_PARROT_INCLUDE_PARROT_VIDEO_MODULE_H_
