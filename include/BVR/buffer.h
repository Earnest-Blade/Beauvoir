#pragma once

#include <BVR/config.h>

#include <stdio.h>

#define BVR_BUFFER_COUNT(buffer) ((buffer.data != NULL) * ((unsigned long long)(buffer.size / buffer.elemsize)))

#ifndef BVR_BUFFER_SIZE
    #define BVR_BUFFER_SIZE 1024
#endif

#ifndef BVR_POOL_SIZE
    #define BVR_POOL_SIZE 2048
#endif

#define SEEK_NEXT 3

/*
    This macro creates a for loop that interates through a pool.
    It will define each `a` as the current used value.
*/

// GCC specific macro
#ifdef __GNUC__
    #define BVR_POOL_FOR_EACH(a, pool)    \
        struct bvr_pool_block_s (first ## ##a) = {0};   \
        struct bvr_pool_block_s* (block ## ##a) = &(first ## ##a); \
        while(                                                     \
            pool.data && \
            ((int)(((block ## ##a) = (struct bvr_pool_block_s*)(pool.data + ((block ## ##a)->next * (pool.elemsize + sizeof(struct bvr_pool_block_s))))) \
            && (a = *(typeof(a)*)((block ## ##a) + sizeof(struct bvr_pool_block_s)))))*0 \
            || ((block ## ##a)->next || (block ## ##a) == &(first ## ##a)) \
        )     
// Clang specific macro
#elif defined(__clang__) || defined(_MSC_VER)
    #define BVR_POOL_FOR_EACH(a, pool)                                        \
        struct bvr_pool_block_s first_##a = {0};                                    \
        struct bvr_pool_block_s* block_##a = &first_##a;                            \
        while (  \
            pool.data &&                                                                   \
            ((block_##a = (struct bvr_pool_block_s*)(                              \
                (pool).data + (block_##a->next * ((pool).elemsize + sizeof(struct bvr_pool_block_s))))), \
            (a = *((char*)block_##a + sizeof(struct bvr_pool_block_s))),     \
            (block_##a->next || block_##a == &first_##a))                           \
            
        )
#else
    #define BVR_POOL_FOR_EACH(a, pool) while(0)                                 
#endif

/*
    Generic data pointer
*/
struct bvr_buffer_s {
    void* data;
    unsigned long size;
    unsigned int elemsize;
} __attribute__((packed));

typedef struct bvr_memstream_s {
    void* data;
    unsigned long long size;

    char* cursor;
    char* next;
} bvr_memstream_t;

/*
    pascal typed string
*/
typedef struct bvr_string_s  { 
    unsigned short length;
    char* string;
} __attribute__ ((packed)) bvr_string_t;

typedef struct bvr_pool_s {
    char* data;

    /*
        Pool's data is structured as such :

        | next data block id |
        |        ----        |
        |        data        |
    

        everything is aligned to this pattern.
    */
    struct bvr_pool_block_s {
        unsigned char next;
    }* next;

    unsigned int capacity;
    unsigned int count;
    unsigned int elemsize;
} bvr_pool_t;

void bvr_create_memstream(bvr_memstream_t* stream, const uint64 size);

void bvr_memstream_write(bvr_memstream_t* stream, const void* data, const uint64 size);
void bvr_memstream_read(bvr_memstream_t* stream, void* dest, const uint64 size);
void bvr_memstream_seek(bvr_memstream_t* stream, uint64 position, int mode);
void bvr_memstream_clear(bvr_memstream_t* stream);

BVR_H_FUNC int bvr_memstream_eof(bvr_memstream_t* stream){
    return stream->cursor - (char*)stream->data >= stream->size;
}

void bvr_destroy_memstream(bvr_memstream_t* stream);

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
void bvr_string_insert(bvr_string_t* string, const uint64 offset, const char* value);

/*
    Return a constant pointer to string's char array.
*/
BVR_H_FUNC const char* bvr_string_get(bvr_string_t* string){
    if(string){
        return string->string;
    }
    return NULL;
}

/*
    Free the string.
*/
void bvr_destroy_string(bvr_string_t* string);

void bvr_create_pool(bvr_pool_t* pool, uint64 size, uint64 count);

/*
    Get a pointer to the next writable slot.
*/
void* bvr_pool_alloc(bvr_pool_t* pool);

/* 
    Try to get a pointer to a writable slot by using an index .
*/
void* bvr_pool_try_get(bvr_pool_t* pool, int index);

/*
    Deallocate a memory block.
*/
void bvr_pool_free(bvr_pool_t* pool, void* ptr);
void bvr_destroy_pool(bvr_pool_t* pool);