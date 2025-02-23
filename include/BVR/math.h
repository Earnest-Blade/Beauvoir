#pragma once

typedef float bvr_vec2[2];
typedef float bvr_vec3[3];
typedef float bvr_vec4[4];

typedef bvr_vec4 bvr_quat;

typedef bvr_vec4 bvr_mat4[4];

struct bvr_transform_s {
    bvr_vec3 position;
    bvr_vec3 scale;
    bvr_quat rotation;

    float matrix[4][4];
};

#ifndef BVR_MATH_IMPLEMENTATION
#define BVR_MATH_IMPLEMENTATION

static inline void bvr_create_default_transform(struct bvr_transform_s* transform){
    transform->position[0] = 0.0f;
    transform->position[1] = 0.0f;
    transform->position[2] = 0.0f;

    transform->rotation[0] = 0.0f;
    transform->rotation[1] = 0.0f;
    transform->rotation[2] = 0.0f;
    transform->rotation[3] = 0.0f;

    transform->scale[0] = 1.0f;
    transform->scale[1] = 1.0f;
    transform->scale[2] = 1.0f;
}

static void bvr_identity_mat4(bvr_mat4 mat){
    mat[0][0] = 1.0f;
    mat[0][1] = 0.0f;
    mat[0][2] = 0.0f;
    mat[0][3] = 0.0f;

    mat[1][0] = 0.0f;
    mat[1][1] = 1.0f;
    mat[1][2] = 0.0f;
    mat[1][3] = 0.0f;

    mat[2][0] = 0.0f;
    mat[2][1] = 0.0f;
    mat[2][2] = 1.0f;
    mat[2][3] = 0.0f;

    mat[3][0] = 0.0f;
    mat[3][1] = 0.0f;
    mat[3][2] = 0.0f;
    mat[3][3] = 1.0f;
}

#endif