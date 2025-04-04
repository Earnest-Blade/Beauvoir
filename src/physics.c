#include <BVR/physics.h>

#include <BVR/utils.h>

#include <memory.h>
#include <malloc.h>

/*
    TODO check for enhanced GTK aglo to check distance between two convex shapes :
        https://graphics.stanford.edu/courses/cs468-01-fall/Papers/cameron.pdf
*/

void bvr_create_collider(bvr_collider_t* collider, float* vertices, size_t count){
    BVR_ASSERT(collider);

    collider->body.acceleration = 0;
    collider->body.aggressive = 0;
    collider->geometry.elemsize = sizeof(float);
    collider->geometry.size = count * sizeof(float);
    collider->geometry.data = NULL;
    collider->transform = NULL;
    
    BVR_IDENTITY_VEC3(collider->body.direction);

    if(vertices){
        collider->geometry.data = malloc(collider->geometry.size);
        BVR_ASSERT(collider->geometry.data); 
    }
}

void bvr_compare_colliders(bvr_collider_t* a, bvr_collider_t* b, struct bvr_collision_result_s* result){
    BVR_ASSERT(a);
    BVR_ASSERT(b);
    BVR_ASSERT(result);

    if(a == b){
        result->collide = -1;
        result->distance = 0.0f;
        BVR_IDENTITY_VEC3(result->direction);
        return;
    }

    /* for now, check check AABB */
    BVR_PRINTF("box0 %f %f %f %f box1 %f %f %f %f", 
        a->geometry.data[0], a->geometry.data[1], a->geometry.data[2], a->geometry.data[3],
        b->geometry.data[0], b->geometry.data[1], b->geometry.data[2], b->geometry.data[3]
    );
}

void bvr_destroy_collider(bvr_collider_t* collider){
    BVR_ASSERT(collider);
    
    free(collider->geometry.data);
}