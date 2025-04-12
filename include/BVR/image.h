#pragma once

#include <BVR/buffer.h>

#include <stdint.h>
#include <stdio.h>

#define BVR_RED     0x0
#define BVR_GREEN   0x1
#define BVR_BLUE    0x2
#define BVR_ALPHA   0x3

#define BVR_R       0x1903
#define BVR_RG      0x8227
#define BVR_RGB     0x1907
#define BVR_BGR     0x80E0
#define BVR_RGBA    0x1908
#define BVR_BGRA    0x80E1

#define BVR_RED8    0x8229
#define BVR_RG8     0x822B
#define BVR_RGB8    0x8051
#define BVR_RGBA8   0x8058
#define BVR_RED16   0x822A
#define BVR_RG16    0x822C
#define BVR_RGB16   0x8054
#define BVR_RGBA16  0x805B

#define BVR_TEXTURE_UNIT0   0x84C0
#define BVR_TEXTURE_UNIT1   0x84C1
#define BVR_TEXTURE_UNIT2   0x84C2
#define BVR_TEXTURE_UNIT3   0x84C3
#define BVR_TEXTURE_UNIT4   0x84C4
#define BVR_TEXTURE_UNIT5   0x84C5
#define BVR_TEXTURE_UNIT6   0x84C6
#define BVR_TEXTURE_UNIT7   0x84C7
#define BVR_TEXTURE_UNIT8   0x84C8
#define BVR_TEXTURE_UNIT9   0x84C9
#define BVR_TEXTURE_UNIT10  0x84CA
#define BVR_TEXTURE_UNIT11  0x84CB
#define BVR_TEXTURE_UNIT12  0x84CC
#define BVR_TEXTURE_UNIT13  0x84CD
#define BVR_TEXTURE_UNIT14  0x84CE
#define BVR_TEXTURE_UNIT15  0x84CF
#define BVR_TEXTURE_UNIT16  0x84D0
#define BVR_TEXTURE_UNIT17  0x84D1
#define BVR_TEXTURE_UNIT18  0x84D2
#define BVR_TEXTURE_UNIT19  0x84D3
#define BVR_TEXTURE_UNIT20  0x84D4
#define BVR_TEXTURE_UNIT21  0x84D5
#define BVR_TEXTURE_UNIT22  0x84D6
#define BVR_TEXTURE_UNIT23  0x84D7
#define BVR_TEXTURE_UNIT24  0x84D8
#define BVR_TEXTURE_UNIT25  0x84D9
#define BVR_TEXTURE_UNIT26  0x84DA
#define BVR_TEXTURE_UNIT27  0x84DB
#define BVR_TEXTURE_UNIT28  0x84DC
#define BVR_TEXTURE_UNIT29  0x84DD
#define BVR_TEXTURE_UNIT30  0x84DE
#define BVR_TEXTURE_UNIT31  0x84DF

#define BVR_TEXTURE_FILTER_NEAREST 0x2600
#define BVR_TEXTURE_FILTER_LINEAR 0x2601

#define BVR_TEXTURE_WRAP_REPEAT 0x2901
#define BVR_TEXTURE_WRAP_CLAMP_TO_EDGE 0x812F

/*
    Contains image layer informations
*/
typedef struct bvr_layer_s {
    bvr_string_t name;
    uint16_t flags;

    uint16_t blend_mode;
    int width, height;
    int anchor_x, anchor_y;
} bvr_layer_t;

/*
    Contains an image informations and data
*/
typedef struct bvr_image_s {
    int width, height, depth;
    int format;
    uint8_t channels;
    uint8_t* pixels;

    struct bvr_buffer_s layers;
} bvr_image_t;

/*
    2D texture.
*/
typedef struct bvr_texture_s {
    bvr_image_t image;

    uint32_t id;
    int filter, wrap;
} bvr_texture_t;

/*
    Represent an array of 2D textures
*/
typedef struct bvr_texture_atlas_s
{
    bvr_image_t image;

    uint32_t id;
    int filter, wrap;

    uint32_t tile_width, tile_height;
} bvr_texture_atlas_t;

/*
    Represent an array of 2D textures.
*/
typedef struct bvr_layered_texture_s {
    bvr_image_t image;

    uint32_t id;
    int filter, wrap;
} bvr_layered_texture_t;

int bvr_create_imagef(bvr_image_t* image, FILE* file);
static inline int bvr_create_image(bvr_image_t* image, const char* path){
    FILE* file = fopen(path, "rb");
    int success = bvr_create_imagef(image, file);
    fclose(file);
    return success;
}

int bvr_create_bitmap(bvr_image_t* image, const char* path, int channel);

/*
    Flip a pixel buffer vertically
*/
void bvr_flip_image_vertically(bvr_image_t* image);

/*
    Copy a specific image channel over another pixel buffer.
    The targeted pixel buffer must be allocated.
*/
int bvr_image_get_channel_pixels(bvr_image_t* image, int channel, uint8_t* buffer);

void bvr_destroy_image(bvr_image_t* image);

/* 2D TEXTURE */
int bvr_create_texture_from_image(bvr_texture_t* texture, bvr_image_t* image, int filter, int wrap);
int bvr_create_texturef(bvr_texture_t* texture, FILE* file, int filter, int wrap);
static inline int bvr_create_texture(bvr_texture_t* texture, const char* path, int filter, int wrap){
    FILE* file = fopen(path, "rb");
    int success = bvr_create_texturef(texture, file, filter, wrap);
    fclose(file);
    return success;
}

/*
    Bind a texture. 
*/
void bvr_texture_enable(bvr_texture_t* texture, int unit);

/*
    Unbind textures
*/
void bvr_texture_disable(void);
void bvr_destroy_texture(bvr_texture_t* texture);

/* ATLAS TEXTURE */
int bvr_create_texture_atlasf(bvr_texture_atlas_t* atlas, FILE* file, uint32_t tile_width, uint32_t tile_height, int filter, int wrap);
static inline int bvr_create_texture_atlas(bvr_texture_atlas_t* atlas, const char* path, uint32_t tile_width, uint32_t tile_height, int filter, int wrap){
    FILE* file = fopen(path, "rb");
    int success = bvr_create_texture_atlasf(atlas, file, tile_width, tile_height, filter, wrap);
    fclose(file);
    return success;
}

void bvr_texture_atlas_enablei(bvr_texture_atlas_t* atlas, int unit);
void bvr_texture_atlas_disable(void);
void bvr_destroy_texture_atlas(bvr_texture_atlas_t* atlas);

/* LAYERED TEXTURE */
int bvr_create_layered_texturef(bvr_layered_texture_t* texture, FILE* file, int filter, int wrap);
static inline int bvr_create_layered_texture(bvr_layered_texture_t* texture, const char* path, int filter, int wrap){
    FILE* file = fopen(path, "rb");
    int success = bvr_create_layered_texturef(texture, file, filter, wrap);
    fclose(file);
    return success;
}

void bvr_layered_texture_enable(bvr_layered_texture_t* texture, int unit);
void bvr_layered_texture_disable(void);

void bvr_destroy_layered_texture(bvr_layered_texture_t* texture);