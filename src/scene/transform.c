#include "parrot/core/math.h"
#include "parrot/core/util.h"
#include "parrot/scene/transform.h"
#include "parrot/scene/world.h"

static void ParrotTransform_constructor(ParrotSceneWorldEntity entity, void *self_ptr, void *user_data) {
    (void)entity;
    (void)user_data;

    ParrotTransform *self = (ParrotTransform *)self_ptr;
    *self = ParrotTransform_new();
}

void ParrotTransform_scene_register(ParrotSceneWorld *world) {
    ParrotSceneWorld_register_component(world,
                                        PARROT_TYPE_STRING(ParrotTransform),
                                        (ParrotSceneWorldComponentDescription){
                                            .size = sizeof(ParrotTransform),

                                            .constructor = ParrotTransform_constructor,
                                        });
}
