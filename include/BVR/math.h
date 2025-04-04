#pragma once

#include <linmath.h>

struct bvr_transform_s {
    vec3 position;
    vec3 scale;
    quat rotation;

    float matrix[4][4];
};

#ifndef BVR_MATH_IMPLEMENTATION
#define BVR_MATH_IMPLEMENTATION

#define BVR_SCALE_VEC3(vec, a) vec[0] = a; vec[1] = a; vec[2] = a;
#define BVR_IDENTITY_VEC3(vec) BVR_SCALE_VEC3(vec, 0.0f);
#define BVR_IDENTITY_MAT4(mat)  mat[0][0] = 1.0f;\
                                mat[0][1] = 0.0f;\
                                mat[0][2] = 0.0f;\
                                mat[0][3] = 0.0f;\
                                mat[1][0] = 0.0f;\
                                mat[1][1] = 1.0f;\
                                mat[1][2] = 0.0f;\
                                mat[1][3] = 0.0f;\
                                mat[2][0] = 0.0f;\
                                mat[2][1] = 0.0f;\
                                mat[2][2] = 1.0f;\
                                mat[2][3] = 0.0f;\
                                mat[3][0] = 0.0f;\
                                mat[3][1] = 0.0f;\
                                mat[3][2] = 0.0f;\
                                mat[3][3] = 1.0f;\

#endif