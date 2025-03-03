#pragma once

#include <BVR/utils.h>
#include <BVR/buffer.h>

#include <stdint.h>

#define BVR_UNIFORM_CAMERA "bvr_camera"
#define BVR_UNIFORM_TRANSFORM "bvr_transform"

#define BVR_UNIFORM_BLOCK_CAMERA 0x0

#define BVR_VERTEX_SHADER 0x8B31
#define BVR_FRAGMENT_SHADER 0x8B30
#define BVR_GEOMETRY_SHADER 0x8DD9

typedef struct bvr_shader_uniform_s {
    struct bvr_buffer_s memory;
    const char* name;
    int location;
    int type;
} bvr_shader_uniform_t;

typedef struct bvr_shader_block_s {
    int location;
    int type;
    uint32_t count;
} bvr_shader_block_t;

typedef struct bvr_shader_s {
    bvr_shader_block_t camera;
    bvr_shader_uniform_t transform;

    struct bvr_buffer_s uniforms;
    struct bvr_buffer_s shaders;
    uint32_t program;
    
    int flags;
    
    // TODO: find another way to store initialization variables.
    bvr_string_t file_string;
    size_t header_length;
} bvr_shader_t;

int bvr_create_shaderf(bvr_shader_t* shader, FILE* file);
static inline int bvr_create_shader(bvr_shader_t* shader, const char* path){
    FILE* file = fopen(path, "rb");
    int a = bvr_create_shaderf(shader, file);
    fclose(file);
    return a;
} 

void bvr_shader_register_sub_shader(bvr_shader_t* shader, int type, const char* name);
void bvr_shader_compile(bvr_shader_t* shader);
void bvr_shader_register_uniform(bvr_shader_t* shader, int type, int count, const char* name);
void bvr_shader_set_uniformi(bvr_shader_t* shader, const int id, void* data);
void bvr_shader_set_uniform(bvr_shader_t* shader, const char* name, void* data);
void bvr_shader_use_uniform(bvr_shader_uniform_t* uniform, void* data);
static inline void bvr_shader_use_uniformi(bvr_shader_t* shader, int id, void* data){
    BVR_ASSERT(shader);
    BVR_ASSERT(id >= 0 && id < shader->uniforms.size / shader->uniforms.elemsize);
    
    bvr_shader_use_uniform((bvr_shader_uniform_t*)&shader->uniforms.data[id * shader->uniforms.elemsize], data);
}
static inline void bvr_shader_use_uniforms(bvr_shader_t* shader, const char* name, void* data){
    BVR_ASSERT(shader);
    BVR_ASSERT(name);

    bvr_shader_uniform_t* uniform;
    for (size_t i = 0; i < shader->shaders.size / shader->shaders.elemsize; i++)
    {
        uniform = (bvr_shader_uniform_t*)&shader->uniforms.data[i * shader->uniforms.elemsize];
        if(strcmp(name, uniform->name) == 0){
            bvr_shader_use_uniform(uniform, data);
            return;
        }
    }
}

void bvr_shader_enable(bvr_shader_t* shader);
void bvr_shader_disable(void);
void bvr_destroy_shader(bvr_shader_t* shader);