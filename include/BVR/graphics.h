#pragma once

#include <BVR/math.h>
#include <BVR/buffer.h>

#include <BVR/mesh.h>
#include <BVR/shader.h>
#include <BVR/image.h>

#define BVR_BLEND_DISABLE   0x000
#define BVR_BLEND_ENABLE    0x001

// Cs*As+Cd*(1-As)
#define BVR_BLEND_FUNC_ALPHA_ONE_MINUS  0x002

// Cd=Cs*1+Cd*1
#define BVR_BLEND_FUNC_ALPHA_ADD        0x004

// Cd=Cs*Cd+Cd*0
#define BVR_BLEND_FUNC_ALPHA_MULT       0x008

#define BVR_DEPTH_TEST_DISABLE  0x000
#define BVR_DEPTH_TEST_ENABLE   0x001

#define BVR_DEPTH_FUNC_NEVER    0x002
#define BVR_DEPTH_FUNC_ALWAYS   0x004
#define BVR_DEPTH_FUNC_LESS     0x008
#define BVR_DEPTH_FUNC_GREATER  0x010
#define BVR_DEPTH_FUNC_LEQUAL   0x020
#define BVR_DEPTH_FUNC_GEQUAL   0x040
#define BVR_DEPTH_FUNC_NOTEQUAL 0x080
#define BVR_DEPTH_FUNC_EQUAL    0x100

#define BVR_MAX_DRAW_COMMAND 258

struct bvr_pipeline_state_s {
    int blending;
    int depth;
    int flags;
};

struct bvr_draw_command_s {
    int order;

    uint32_t array_buffer;
    uint32_t vertex_buffer;
    uint32_t element_buffer;

    uint32_t element_offset;
    uint32_t element_count;
    uint32_t attrib_count;
    
    uint8_t draw_mode;

    int texture_type;

    bvr_shader_t* shader;
    bvr_texture_t* texture;    

    void* user_data;
};

typedef struct bvr_pipeline_s {
    /*
        State use for default rendering
    */
    struct bvr_pipeline_state_s rendering_pass;
    
    /*
        State use to push the rendering framebuffer 
        to the window framebuffer.
    */
    struct bvr_pipeline_state_s swap_pass;

    int command_count;
    struct bvr_draw_command_s commands[BVR_MAX_DRAW_COMMAND];

    vec3 clear_color;
} bvr_pipeline_t;

typedef struct bvr_framebuffer_s {
    int width, target_width;
    int height, target_height;

    uint32_t buffer;
    uint32_t color_buffer, depth_buffer, stencil_buffer;
    uint32_t vertex_buffer, array_buffer;

    bvr_shader_t shader;
} bvr_framebuffer_t;

void bvr_pipeline_state_enable(struct bvr_pipeline_state_s* state);
void bvr_pipeline_draw_cmd(struct bvr_draw_command_s* cmd);
void bvr_pipeline_add_draw_cmd(struct bvr_draw_command_s* cmd);
void bvr_error();

BVR_H_FUNC int bvr_pipeline_compare_commands(const void* a, const void* b){
    return (((struct bvr_draw_command_s*)a)->order - ((struct bvr_draw_command_s*)b)->order);
}

int bvr_create_framebuffer(bvr_framebuffer_t* framebuffer, int width, int height, const char* shader);
void bvr_framebuffer_enable(bvr_framebuffer_t* framebuffer);
void bvr_framebuffer_disable(bvr_framebuffer_t* framebuffer);
void bvr_framebuffer_clear(bvr_framebuffer_t* framebuffer, vec3 color);
void bvr_framebuffer_blit(bvr_framebuffer_t* framebuffer);
void bvr_destroy_framebuffer(bvr_framebuffer_t* framebuffer);