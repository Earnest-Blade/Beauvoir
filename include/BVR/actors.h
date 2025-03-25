#pragma once

#include <BVR/image.h>
#include <BVR/shader.h>
#include <BVR/mesh.h>
#include <BVR/physics.h>
#include <BVR/math.h>

#include <stdio.h>

typedef enum bvr_actor_type_e {
    BVR_EMPTY_ACTOR = 0x00,
    BVR_LAYER_ACTOR = 0x01,
    BVR_STATIC_ACTOR = 0x02,
    BVR_DYNAMIC_ACTOR = 0x04
} bvr_actor_type_t;


/*
    Each actor based struct must start by 
    the actor struct.
*/

struct bvr_actor_s {
    bvr_string_t name;
    bvr_actor_type_t type;
    size_t id;

    struct bvr_transform_s transform;
};

typedef struct bvr_static_model_s {
    struct bvr_actor_s object;

    bvr_mesh_t mesh;
    bvr_shader_t shader;
} bvr_static_model_t;

typedef struct bvr_dynamic_model_s {
    struct bvr_actor_s object;

    bvr_mesh_t mesh;
    bvr_shader_t shader;

    bvr_collider_t collider;
} bvr_dynamic_model_t;

/*
    Initialize a generic actor.
*/
void bvr_create_actor(struct bvr_actor_s* actor, const char* name, bvr_actor_type_t type);
void bvr_destroy_actor(struct bvr_actor_s* actor);

void bvr_draw_static_model(bvr_static_model_t* actor, int drawmode);