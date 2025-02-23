#pragma once

#include <BVR/buffer.h>
#include <BVR/math.h>
#include <BVR/actors.h>

#include <BVR/window.h>
#include <BVR/audio.h>

#include <stdint.h>

#define BVR_CAMERA_ORTHOGRAPHIC 1
#define BVR_CAMERA_PERSPECTIVE 2

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

typedef struct bvr_page_s {
    bvr_camera_t camera;
    struct bvr_buffer_s actors;
} bvr_page_t;

typedef struct bvr_book_s {
    bvr_window_t window;
    bvr_audio_stream_t audio;

    bvr_page_t page;

    float delta_time;
    uint64_t prev_time, current_time;
} bvr_book_t;

int bvr_create_book(bvr_book_t* book);
static inline int bvr_is_awake(bvr_book_t* book){
    return book->window.awake;
}
void bvr_new_frame(bvr_book_t* book);
void bvr_render(bvr_book_t* book);
void bvr_destroy_book(bvr_book_t* book);

int bvr_create_page(bvr_page_t* page);
bvr_camera_t* bvr_add_orthographic_camera(bvr_page_t* page, bvr_framebuffer_t* framebuffer, float near, float far, float scale);

void bvr_destroy_page(bvr_page_t* page);