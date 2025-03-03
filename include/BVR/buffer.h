#pragma once

#include <stdio.h>
#include <string.h>

#define BVR_BUFFER_SIZE 1024
#define BVR_POOL_SIZE 2048

#define BVR_BUFFER_COUNT(buffer)((unsigned long long)(buffer.size / buffer.elemsize))

struct bvr_buffer_s {
    char* data;
    unsigned long long size;
    unsigned int elemsize;
};

/*
    pascal typed string
*/
typedef struct bvr_string_s { 
    unsigned short length;
    char* data;
} bvr_string_t;

typedef struct bvr_pool_s {
    char* data;

    /*
        Pool's data is structure as such :

        | next data block id |
        |        ----        |
        |        data        |
    

        after that, all informations are aligned to this pattern.
    */
    struct bvr_pool_block_s {
        unsigned char next;
    }* next;

    unsigned int count;
    unsigned int elemsize;
} bvr_pool_t;

void bvr_create_string(bvr_string_t* string, const char* value);

/*
    Concatenate a string.
    WARNING: function might be slow -> no growth factor :(
*/
void bvr_string_concat(bvr_string_t* string, const char* other);

/*
    Allocate a new string and copy other string's content.
*/
void bvr_string_create_and_copy(bvr_string_t* dest, bvr_string_t* source);

/*
    Insert a char array into a string.
*/
void bvr_string_insert(bvr_string_t* string, const size_t offset, const char* value);

/*
    Return a constant pointer to string's char array.
*/
const char* bvr_string_get(bvr_string_t* string);

/*
    Free the string.
*/
void bvr_destroy_string(bvr_string_t* string);

void bvr_create_pool(bvr_pool_t* pool, size_t size, size_t count);
void* bvr_pool_alloc(bvr_pool_t* pool);
void bvr_pool_free(bvr_pool_t* pool, void* ptr);
void bvr_destroy_pool(bvr_pool_t* pool);