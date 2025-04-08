#pragma once

#include <BVR/buffer.h>
#include <BVR/math.h>

#ifndef BVR_COLLIDER_COLLECTION_SIZE
    #define BVR_COLLIDER_COLLECTION_SIZE 128
#endif

#define BVR_COLLISION_DISABLE 0x0
#define BVR_COLLISION_ENABLE 0x1

/*
    Contains an array of collider pointers.
*/
typedef struct bvr_pool_s bvr_collider_collection_t;

struct bvr_body_s {
    float acceleration;
    vec3 direction;

    /* is the body passive or aggressive? */
    char mode;
};

typedef struct bvr_collider_s {
    struct bvr_body_s body;
    struct bvr_buffer_s geometry;

    struct bvr_transform_s* transform;
} bvr_collider_t;

struct bvr_collision_result_s {
    int collide;

    float distance;
    vec3 direction;

    bvr_collider_t* other;
};

void bvr_body_add_force(struct bvr_body_s* body, float x, float y, float z);
void bvr_body_apply_motion(struct bvr_body_s* body, struct bvr_transform_s* transform);

void bvr_create_collider(bvr_collider_t* collider, float* vertices, size_t count);
void bvr_compare_colliders(bvr_collider_t* a, bvr_collider_t* b, struct bvr_collision_result_s* result);
void bvr_destroy_collider(bvr_collider_t* collider);
