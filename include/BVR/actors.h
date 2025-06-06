#pragma once

#include <BVR/image.h>
#include <BVR/shader.h>
#include <BVR/mesh.h>
#include <BVR/physics.h>
#include <BVR/math.h>

#include <stdio.h>

#define BVR_ACTOR_NOT_FREE                          0x00001

#define BVR_DYNACTOR_PASSIVE                        0x00100
#define BVR_DYNACTOR_AGGRESSIVE                     0x00200
#define BVR_DYNACTOR_CREATE_COLLIDER_FROM_VERTICES  0x00400

#define BVR_BITMAP_CREATE_COLLIDER                  0x00800

typedef enum bvr_actor_type_e {
    BVR_EMPTY_ACTOR,
    BVR_LAYER_ACTOR,
    BVR_BITMAP_ACTOR,
    BVR_STATIC_ACTOR,
    BVR_DYNAMIC_ACTOR
} bvr_actor_type_t;

/*
    Each actor based struct must start by 
    an actor struct.
*/

struct bvr_actor_s {
    bvr_string_t name;
    bvr_actor_type_t type;
    bvr_uuid_t id;
    int flags;

    uint8_t active;
    uint16_t order_in_layer;

    // TODO find why tranform overwrite order in layer
    uint32_t padding;

    struct bvr_transform_s transform;
};

typedef struct bvr_actor_s bvr_emty_actor_t;

typedef struct bvr_layer_actor_s {
    struct bvr_actor_s object;

    bvr_mesh_t mesh;
    bvr_shader_t shader;
    bvr_layered_texture_t texture;
} bvr_layer_actor_t;

typedef struct bvr_static_actor_s {
    struct bvr_actor_s object;

    bvr_mesh_t mesh;
    bvr_shader_t shader;
} bvr_static_actor_t;

typedef struct bvr_dynamic_actor_s {
    struct bvr_actor_s object;

    bvr_mesh_t mesh;
    bvr_shader_t shader;
    bvr_collider_t collider;

} bvr_dynamic_actor_t;

typedef struct bvr_bitmap_layer_s {
    struct bvr_actor_s object;

    bvr_mesh_t mesh;
    bvr_shader_t shader;
    bvr_collider_t collider;

    bvr_texture_t bitmap;
} bvr_bitmap_layer_t;

/*
    Initialize a generic actor.
*/
void bvr_create_actor(struct bvr_actor_s* actor, const char* name, bvr_actor_type_t type, int flags);
void bvr_destroy_actor(struct bvr_actor_s* actor);

void bvr_draw_actor(struct bvr_actor_s* actor, int drawmode);