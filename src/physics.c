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

    return (
        vb[0] >= va[0] + a->width  || // right check
        vb[0] + b->width <= va[0]  || // left check
        vb[1] >= va[1] + a->height || // bottom check
        vb[1] + b->height <= va[1]    // top check
    );
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

    if(!BVR_HAS_FLAG(a->body.mode, BVR_COLLISION_AABB) || !BVR_HAS_FLAG(b->body.mode, BVR_COLLISION_AABB)){
        BVR_PRINT("collision type not supported :(");
        return;
    }

    struct bvr_bounds_s ba, bb;

    for (size_t ax = 0; ax < BVR_BUFFER_COUNT(a->geometry); ax++)
    {
        for (size_t bx = 0; bx < BVR_BUFFER_COUNT(b->geometry); bx++)
        {
            memcpy(&ba, a->geometry.data + ax * sizeof(struct bvr_bounds_s), sizeof(struct bvr_bounds_s));
            memcpy(&bb, b->geometry.data + bx * sizeof(struct bvr_bounds_s), sizeof(struct bvr_bounds_s));

            vec2_add(ba.coords, ba.coords, a->transform->position);
            vec2_add(bb.coords, bb.coords, b->transform->position);

            BVR_PRINTF("%f %f %i %i", ba.coords[0], ba.coords[1], ba.width, ba.height);
            BVR_PRINTF("%f %f %i %i", bb.coords[0], bb.coords[1], bb.width, bb.height);

            if(bvri_aabb(&ba, &bb, a->body.direction, b->body.direction)){
                result->collide = BVR_OK;
                result->other = b;
            
                return;
            }
        }
    }
}

void bvr_destroy_collider(bvr_collider_t* collider){
    BVR_ASSERT(collider);
    
    free(collider->geometry.data);
}