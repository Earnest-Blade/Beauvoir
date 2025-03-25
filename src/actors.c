#include <BVR/actors.h>

#include <BVR/file.h>

#include <stdlib.h>

static void bvri_calculate_transform(struct bvr_actor_s* actor){
    BVR_ASSERT(actor);

    BVR_IDENTITY_MAT4(actor->transform.matrix);
    actor->transform.matrix[3][0] = actor->transform.position[0];
    actor->transform.matrix[3][1] = actor->transform.position[1];
    actor->transform.matrix[3][2] = actor->transform.position[2];
}

void bvr_create_actor(struct bvr_actor_s* actor, const char* name, bvr_actor_type_t type){
    BVR_ASSERT(actor);

    actor->type = type;
    actor->id = (uint32_t)actor;
    
    BVR_IDENTITY_VEC3(actor->transform.position);
    BVR_IDENTITY_VEC3(actor->transform.rotation);
    actor->transform.scale[0] = 1.0f;
    actor->transform.scale[1] = 1.0f;
    actor->transform.scale[2] = 1.0f;

    BVR_IDENTITY_MAT4(actor->transform.matrix);
    bvr_create_string(&actor->name, name);
}

void bvr_destroy_actor(struct bvr_actor_s* actor){
    bvr_destroy_string(&actor->name);
}

void bvr_draw_static_model(bvr_static_model_t* actor, int drawmode){
    bvri_calculate_transform((struct bvr_actor_s*)actor);

    bvr_shader_enable(&actor->shader);
    bvr_shader_use_uniform(&actor->shader.uniforms[0], &actor->object.transform.matrix[0][0]);

    bvr_mesh_draw(&actor->mesh, drawmode);
}