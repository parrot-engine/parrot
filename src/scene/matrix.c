#include "parrot/scene/matrix.h"
#include "parrot/core/math.h"
#include "parrot/core/util.h"
#include "parrot/scene/world.h"

static void ParrotMat_constructor(ParrotSceneWorldEntity entity, void *self_ptr, void *user_data) {
    (void)entity;
    (void)user_data;

    ParrotMat *self = (ParrotMat *)self_ptr;

    *self = ParrotMat_identity();
}

void ParrotMat_scene_register(ParrotSceneWorld *world) {
    ParrotSceneWorld_register_component(world,
                                        PARROT_TYPE_STRING(ParrotMat),
                                        (ParrotSceneWorldComponentDescription){
                                            .size = sizeof(ParrotMat),

                                            .constructor = ParrotMat_constructor,
                                        });
}

ParrotMat ParrotMat_get_entity_global_matrix(ParrotSceneWorld *world, ParrotSceneWorldEntity entity) {
    ParrotMat matrix = ParrotMat_identity();

    ParrotMat *matrix_component = ParrotSceneWorld_get_component(world, entity, ParrotMat);
    if (matrix_component) {
        matrix = ParrotMat_mul(*matrix_component, matrix);
    }

    ParrotSceneWorldEntity parent = ParrotSceneWorld_get_entity_parent(world, entity);
    if (parent != ParrotSceneWorldEntity_NULL) {
        matrix = ParrotMat_mul(ParrotMat_get_entity_global_matrix(world, parent), matrix);
    }

    return matrix;
}