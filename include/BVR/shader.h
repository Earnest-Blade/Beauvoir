#pragma once

#include <BVR/utils.h>
#include <BVR/buffer.h>

#include <stdint.h>

#define BVR_UNIFORM_CAMERA_NAME "bvr_camera"
#define BVR_UNIFORM_TRANSFORM_NAME "bvr_transform"

#define BVR_UNIFORM_BLOCK_CAMERA 0x0

#define BVR_MAX_SHADER_COUNT 7
#define BVR_MAX_UNIFORM_COUNT 20
#define BVR_MAX_SHADER_BLOCK_COUNT 5

#define BVR_VERTEX_SHADER 0x001
#define BVR_FRAGMENT_SHADER 0x002
#define BVR_FRAMEBUFFER_SHADER 0x004

typedef struct bvr_shader_uniform_s {
    struct bvr_buffer_s memory;
    bvr_string_t name;
    short location;
    int type;
} bvr_shader_uniform_t;

typedef struct bvr_shader_stage_s {
    uint32_t shader;
    int type;
} bvr_shader_stage_t;

typedef struct bvr_shader_block_s {
    short location;
    int type;
    uint32_t count;
} bvr_shader_block_t;

typedef struct bvr_shader_s {
    uint32_t program;

    bvr_shader_stage_t shaders[BVR_MAX_SHADER_COUNT];
    bvr_shader_uniform_t uniforms[BVR_MAX_UNIFORM_COUNT];
    bvr_shader_block_t blocks[BVR_MAX_SHADER_BLOCK_COUNT];

    uint8_t shader_count;
    uint8_t uniform_count, block_count;
    
    int flags;
} bvr_shader_t;

int bvr_create_shaderf(bvr_shader_t* shader, FILE* file, int flags);
static inline int bvr_create_shader(bvr_shader_t* shader, const char* path, int flags){
    FILE* file = fopen(path, "rb");
    int a = bvr_create_shaderf(shader, file, flags);
    fclose(file);
    return a;
} 

bvr_shader_uniform_t* bvr_shader_register_uniform(bvr_shader_t* shader, int type, int count, const char* name);
bvr_shader_uniform_t* bvr_shader_register_texture(bvr_shader_t* shader, int type, int* id, int* layer, const char* name, const char* layer_name);

void bvr_shader_set_uniformi(bvr_shader_uniform_t* uniform, void* data);
void bvr_shader_set_texturei(bvr_shader_uniform_t* uniform, int* id, int* layer);
void bvr_shader_set_uniform(bvr_shader_t* shader, const char* name, void* data);
void bvr_shader_set_texture(bvr_shader_t* shader, const char* name, int* id, int* layer);
void bvr_shader_use_uniform(bvr_shader_uniform_t* uniform, void* data);

void bvr_shader_enable(bvr_shader_t* shader);
void bvr_shader_disable(void);
void bvr_destroy_shader(bvr_shader_t* shader);