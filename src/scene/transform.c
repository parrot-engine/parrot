#include "parrot/scene/transform.h"
#include "parrot/core/math.h"
#include "parrot/core/util.h"
#include "parrot/scene/world.h"

static void ParrotTransform_constructor(ParrotSceneWorldEntity entity, void *self_ptr, void *user_data) {
    (void)entity;
    (void)user_data;

    ParrotTransform *self = (ParrotTransform *)self_ptr;

    *self = ParrotTransform_create_default();
}

void ParrotTransform_scene_register(ParrotSceneWorld *world) {
    ParrotSceneWorld_register_component(world,
                                        PARROT_TYPE_STRING(ParrotTransform),
                                        (ParrotSceneWorldComponentDescription){
                                            .size = sizeof(ParrotTransform),

                                            .constructor = ParrotTransform_constructor,
                                        });
}

ParrotTransform ParrotTransform_get_entity_global_transform(ParrotSceneWorld *world, ParrotSceneWorldEntity entity) {
    ParrotTransform transform = ParrotTransform_create_default();

    ParrotTransform *transform_component = ParrotSceneWorld_get_component(world, entity, ParrotTransform);
    if (transform_component) {
        transform = *transform_component;
    }

    ParrotSceneWorldEntity parent = ParrotSceneWorld_get_entity_parent(world, entity);
    if (parent != ParrotSceneWorldEntity_NULL) {
        ParrotTransform parent_transform = ParrotTransform_get_entity_global_transform(world, parent);
        transform.position = ParrotVec3f_add(transform.position, parent_transform.position);
        transform.rotation = ParrotVec3f_add(transform.rotation, parent_transform.rotation);
        transform.scale = ParrotVec3f_mul(transform.scale, parent_transform.scale);
    }

    return transform;
}