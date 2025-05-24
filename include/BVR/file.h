#pragma once

#include <BVR/config.h>
#include <BVR/buffer.h>

#include <stdint.h>
#include <stdio.h>

#pragma endregion

#define BVR_BE_TO_LE_U16 __bswap_16
#define BVR_BE_TO_LE_U32 __bswap_32

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
short bvr_fread16_le(FILE* file);

/*
    Read a single signed int from a stream.
*/
int bvr_fread32_le(FILE* file);

/*
    Read a single unsigned char from a stream.
*/
uint8_t bvr_freadu8_le(FILE* file);

/*
    Read a single unsigned short from a stream.
*/
uint16_t bvr_freadu16_le(FILE* file);

/*
    Read a single unsigned int from a stream.
*/
uint32_t bvr_freadu32_le(FILE* file);

/*
    Read a null terminate string from a stream.
*/
void bvr_freadstr(char* string, size_t size, FILE* file);

/*
    Read a single unsigned char from a stream and translate big-endian to little-endian.
*/
static inline uint8_t bvr_freadu8_be(FILE* file){
    return bvr_freadu8_le(file);
}

/*
    Read a single unsigned short from a stream and translate big-endian to little-endian.
*/
static inline uint16_t bvr_freadu16_be(FILE* file){
    uint16_t value = bvr_freadu16_le(file);
    return BVR_BE_TO_LE_U16(value);
}

/*
    Read a single unsigned int from a stream and translate big-endian to little-endian.
*/
static inline uint32_t bvr_freadu32_be(FILE* file){
    uint32_t value = bvr_freadu32_le(file);
    return BVR_BE_TO_LE_U32(value);
}