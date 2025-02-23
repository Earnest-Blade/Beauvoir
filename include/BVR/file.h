#pragma once

#include <BVR/buffer.h>

#include <stdint.h>
#include <stdio.h>

// https://stackoverflow.com/questions/2182002/how-to-convert-big-endian-to-little-endian-in-c-without-using-library-functions
#define BVR_BE_TO_LE8(x) (((x & 0x0000ff00) >> 8) | ((x & 0x000000ff) << 8))
#define BVR_BE_TO_LE16(x) ((x >> 8) | (x << 8))
#define BVR_BE_TO_LE32(x) (((x >>24) & 0xff) | ((x <<8) & 0xff0000) \
                         | ((x >>8)& 0xff00) | ((x <<24)& 0xff000000))

#define BVR_EOF(x) (BVR_ASSERT(feof(x) == 0))

/*
    Return size of a file.
*/
size_t bvr_get_file_size(FILE* file);

/*
    Read all the file and copy data into a string.
*/
int bvr_read_file(bvr_string_t* string, FILE* file);

/*
    Read a single signed short from a stream.
*/
short bvr_fread16(FILE* file);

/*
    Read a single signed int from a stream.
*/
int bvr_fread32(FILE* file);

/*
    Read a single unsigned char from a stream.
*/
uint8_t bvr_freadu8(FILE* file);

/*
    Read a null terminate string from a stream.
*/
void bvr_freadstr(char* string, size_t size, FILE* file);

/*
    Read a single unsigned short from a stream.
*/
uint16_t bvr_freadu16(FILE* file);

/*
    Read a single unsigned int from a stream.
*/
uint32_t bvr_freadu32(FILE* file);

/*
    Read a single unsigned char from a stream and translate big-endian to little-endian.
*/
static inline uint8_t bvr_freadu8_be(FILE* file){
    uint8_t value = bvr_freadu8(file);
    return BVR_BE_TO_LE8(value);
}

/*
    Read a single unsigned short from a stream and translate big-endian to little-endian.
*/
static inline uint16_t bvr_freadu16_be(FILE* file){
    uint16_t value = bvr_freadu16(file);
    return BVR_BE_TO_LE16(value);
}

/*
    Read a single unsigned int from a stream and translate big-endian to little-endian.
*/
static inline uint32_t bvr_freadu32_be(FILE* file){
    uint32_t value = bvr_freadu32(file);
    return BVR_BE_TO_LE32(value);
}