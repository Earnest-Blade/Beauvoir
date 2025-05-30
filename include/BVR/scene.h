#pragma once

#include <BVR/buffer.h>
#include <BVR/math.h>
#include <BVR/actors.h>

#include <BVR/window.h>
#include <BVR/audio.h>

#include <stdint.h>

#define BVR_CAMERA_ORTHOGRAPHIC 0x1
#define BVR_CAMERA_PERSPECTIVE  0x2

#ifndef BVR_MAX_SCENE_ACTOR_COUNT
    #define BVR_MAX_SCENE_ACTOR_COUNT 64
#endif

#ifndef BVR_NO_FPS_CAP
    #ifndef BVR_TARGET_FRAMERATE
        #define BVR_TARGET_FRAMERATE 60
    #endif

    #define BVR_FRAMERATE (1000 / BVR_TARGET_FRAMERATE)
#endif

typedef struct bvr_camera_s {
    uint32_t mode;

    struct bvr_transform_s transform;
    bvr_framebuffer_t* framebuffer;
    uint32_t buffer; /* uniform buffer object reference */
    
    float near;
    float far;
    union bvr_camera_field_of_view_u
    {
        // ortho scale
        float scale;

        // perspective scale
        float fov;
    } field_of_view;
} bvr_camera_t;

/*
    Contains all world's informations and data
*/
typedef struct bvr_page_s {
    bvr_string_t name;
    
    bvr_camera_t camera;

    // all world's actors
    bvr_pool_t actors;

    // all world's colliders
    bvr_collider_collection_t colliders;
} bvr_page_t;

/*
    Contains all game's related data
*/
typedef struct bvr_book_s {
    bvr_window_t window;
    bvr_pipeline_t pipeline;
    bvr_audio_stream_t audio;

    bvr_page_t page;

    float delta_time;
    uint64_t prev_time, current_time;
} bvr_book_t;

/*
    Create a new game context
*/
int bvr_create_book(bvr_book_t* book);

/*
    Return BVR_OK if the game is still running.
*/
BVR_H_FUNC int bvr_is_awake(bvr_book_t* book){
    return book->window.awake;
}

/*
    ask Beauvoir to prepare a new frame
*/
void bvr_new_frame(bvr_book_t* book);

void bvr_update(bvr_book_t* book);
/*
    push Beauvoir's graphics to the window
*/
void bvr_render(bvr_book_t* book);

void bvr_destroy_book(bvr_book_t* book);

/*
    Create a new scene
*/
int bvr_create_page(bvr_page_t* page);

bvr_camera_t* bvr_create_orthographic_camera(bvr_page_t* page, bvr_framebuffer_t* framebuffer, float near, float far, float scale);

void bvr_camera_lookat(bvr_page_t* page, vec3 target, vec3 up);

/*
    Set the view matrix of the camera.
*/
BVR_H_FUNC void bvr_camera_set_view(bvr_page_t* page, mat4x4 matrix){
    bvr_enable_uniform_buffer(page->camera.buffer);
    bvr_uniform_buffer_set(sizeof(mat4x4), sizeof(mat4x4), &matrix[0][0]);
    bvr_enable_uniform_buffer(0);
}

/*
    Transpose a screen-space coords into a world-space coord.
*/
void bvr_screen_to_world_coords(bvr_book_t* book, vec2 screen, vec3 world);

/*
    Register a new actor inside page's pool. 
    Return NULL if cannot register actor.
*/
struct bvr_actor_s* bvr_link_actor_to_page(bvr_page_t* page, struct bvr_actor_s* actor);

/*
    Register a new non-actor collider inside page's pool.
    Return NULL if cannot register collider.
*/
bvr_collider_t* bvr_link_collider_to_page(bvr_page_t* page, bvr_collider_t* collider);

void bvr_destroy_page(bvr_page_t* page);