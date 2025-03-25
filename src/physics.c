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

    collider->capsule.elemsize = sizeof(float);
    collider->capsule.size = count * sizeof(float);
    collider->capsule.data = NULL;

    if(vertices){
        collider->capsule.data = malloc(collider->capsule.size);
        BVR_ASSERT(collider->capsule.data); 
    }
}

void bvr_destroy_collider(bvr_collider_t* collider){
    BVR_ASSERT(collider);
    
    free(collider->capsule.data);
}