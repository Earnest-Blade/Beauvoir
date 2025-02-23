#pragma once

#include <BVR/image.h>
#include <BVR/shader.h>
#include <BVR/mesh.h>
#include <bvr/math.h>

#include <stdio.h>

typedef enum bvr_actor_type_e {
    BVR_EMPTY_ACTOR = 0x00,
    BVR_MODEL_ACTOR = 0x01,
    BVR_LAYER_ACTOR = 0x02
} bvr_actor_type_t;

struct bvr_actor_s {
    bvr_string_t name;
    bvr_actor_type_t type;
    size_t id;

    struct bvr_buffer_s data;
    struct bvr_transform_s transform;
};

typedef struct bvr_model_s {
    bvr_mesh_t mesh;
    bvr_shader_t shader;

    struct bvr_transform_s transform;
} bvr_model_t;

void bvr_model_draw(bvr_model_t* model, int drawmode);