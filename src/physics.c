#include <BVR/physics.h>

#include <BVR/utils.h>

#include <memory.h>
#include <malloc.h>

void bvr_body_add_force(struct bvr_body_s* body, float x, float y, float z){
    BVR_ASSERT(body);

    body->direction[0] = x;
    body->direction[1] = y;
    body->direction[2] = z;
    body->acceleration = vec3_len(body->direction);

    if(body->acceleration){
        vec3_norm(body->direction, body->direction);
    }
    else {
        BVR_IDENTITY_VEC3(body->direction);
    }
}

void bvr_body_apply_motion(struct bvr_body_s* body, struct bvr_transform_s* transform){
    BVR_ASSERT(body);
    BVR_ASSERT(transform);

    if(body->acceleration){
        vec3 translate;
        BVR_IDENTITY_VEC3(translate);
    
        vec3_add(translate, translate, body->direction);
        vec3_scale(translate, translate, body->acceleration);
        vec3_add(transform->position, transform->position, translate);
    
    }

    body->acceleration = 0.0f;
    BVR_IDENTITY_VEC3(body->direction);
}

/*
    TODO check for enhanced GTK aglo to check distance between two convex shapes :
        https://graphics.stanford.edu/courses/cs468-01-fall/Papers/cameron.pdf
*/

void bvr_create_collider(bvr_collider_t* collider, float* vertices, size_t count){
    BVR_ASSERT(collider);

    collider->body.acceleration = 0;
    collider->body.mode = 0;
    collider->geometry.elemsize = sizeof(float);
    collider->geometry.size = count * sizeof(float);
    collider->geometry.data = NULL;
    collider->transform = NULL;
    
    BVR_IDENTITY_VEC3(collider->body.direction);
}

void bvr_compare_colliders(bvr_collider_t* a, bvr_collider_t* b, struct bvr_collision_result_s* result){
    BVR_ASSERT(a);
    BVR_ASSERT(b);
    BVR_ASSERT(result);

    result->collide = 0;
    result->distance = 0.0f;
    result->other = NULL;
    BVR_IDENTITY_VEC3(result->direction);

    if(a == b){
        result->collide = -1;
        return;
    }

    float geometry_a[4], geometry_b[4];
    memcpy(&geometry_a[0], &((float*)a->geometry.data)[0], sizeof(float) * 2);
    memcpy(&geometry_b[0], &((float*)b->geometry.data)[0], sizeof(float) * 2);
    memcpy(&geometry_a[2], &((float*)a->geometry.data)[2], sizeof(float) * 2);
    memcpy(&geometry_b[2], &((float*)b->geometry.data)[2], sizeof(float) * 2);
    
    vec3_add(geometry_a, geometry_a, a->transform->position);
    vec3_add(geometry_b, geometry_b, b->transform->position);
    vec3_add(geometry_a, geometry_a, a->body.direction);
    vec3_add(geometry_b, geometry_b, b->body.direction);

    if(geometry_a[0] < geometry_b[0] + geometry_b[3] &&
        geometry_a[0] + geometry_a[3] > geometry_b[0] &&
        geometry_a[1] < geometry_b[1] + geometry_b[2] &&
        geometry_a[1] + geometry_a[2] > geometry_b[1]){

        result->collide = 1;
        result->other = b;
    }
}

void bvr_destroy_collider(bvr_collider_t* collider){
    BVR_ASSERT(collider);
    
    free(collider->geometry.data);
}