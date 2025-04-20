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

/*
    Check for AABB collision between two in-mouvement bounds
*/
static int bvri_aabb(struct bvr_bounds_s* a, struct bvr_bounds_s* b, vec3 a_inertia, vec3 b_inertia){
    vec3 va, vb;
    BVR_IDENTITY_VEC3(va);
    BVR_IDENTITY_VEC3(vb);

    vec2_add(va, a->coords, a_inertia);
    vec2_add(vb, b->coords, b_inertia);

    va[0] -= a->width;
    va[1] -= a->height; 
    vb[0] -= b->width;
    vb[1] -= b->height;

    return !(
        vb[0] >= va[0] + a->width * 2.0f  || // right check
        vb[0] + b->width * 2.0f <= va[0]  || // left check
        vb[1] >= va[1] + a->height * 2.0f || // bottom check
        vb[1] + b->height * 2.0f <= va[1]    // top check
    );
}

static void bvri_compare_box_colliders(bvr_collider_t* a, bvr_collider_t* b, struct bvr_collision_result_s* result){
    struct bvr_bounds_s ba, bb;

    for (size_t ax = 0; ax < BVR_BUFFER_COUNT(a->geometry); ax++)
    {
        for (size_t bx = 0; bx < BVR_BUFFER_COUNT(b->geometry); bx++)
        {
            memcpy(&ba, a->geometry.data + ax * sizeof(struct bvr_bounds_s), sizeof(struct bvr_bounds_s));
            memcpy(&bb, b->geometry.data + bx * sizeof(struct bvr_bounds_s), sizeof(struct bvr_bounds_s));

            vec2_add(ba.coords, ba.coords, a->transform->position);
            vec2_add(bb.coords, bb.coords, b->transform->position);

            vec3 intertia_a, intertia_b;
            vec3_scale(intertia_a, a->body.direction, a->body.acceleration);
            vec3_scale(intertia_b, b->body.direction, b->body.acceleration);

            if(bvri_aabb(&ba, &bb, intertia_a, intertia_b)){
                result->collide = BVR_OK;
                result->other = b;
            
                return;
            }
        }
    }
}

void bvr_compare_colliders(bvr_collider_t* a, bvr_collider_t* b, struct bvr_collision_result_s* result){
    BVR_ASSERT(a);
    BVR_ASSERT(b);
    BVR_ASSERT(result);

    result->collide = 0;
    result->distance = 0.0f;
    result->other = NULL;
    BVR_IDENTITY_VEC3(result->direction);

    if(a->shape == BVR_COLLIDER_EMPTY || b->shape == BVR_COLLIDER_EMPTY){
        return;
    }

    if(a == b){
        result->collide = -1;
        return;
    }

    if(a->shape == b->shape && a->shape == BVR_COLLIDER_BOX){
        bvri_compare_box_colliders(a, b, result);
    }

    
}

void bvr_destroy_collider(bvr_collider_t* collider){
    BVR_ASSERT(collider);
    
    free(collider->geometry.data);
}