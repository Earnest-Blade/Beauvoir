#pragma once

#include <BVR/buffer.h>
#include <BVR/math.h>

#ifndef BVR_COLLIDER_COLLECTION_SIZE
    #define BVR_COLLIDER_COLLECTION_SIZE 512
#endif

/*
    Contains an array of collider pointers.
*/
typedef struct bvr_pool_s bvr_collider_collection_t;

typedef struct bvr_collider_s {
    struct bvr_buffer_s capsule;
} bvr_collider_t;

void bvr_create_collider(bvr_collider_t* collider, float* vertices, size_t count);
void bvr_destroy_collider(bvr_collider_t* collider);
