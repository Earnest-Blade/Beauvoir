#include <BVR/buffer.h>
#include <BVR/utils.h>

#include <malloc.h>
#include <string.h>

void bvr_create_string(bvr_string_t* string, const char* value){
    BVR_ASSERT(string);

    string->length = 0;
    string->data = NULL;
    if(value){
        string->length = strlen(value) + 1;
        string->data = malloc(string->length);
        BVR_ASSERT(string->data);

        memcpy(string->data, value, string->length - 1);
        string->data[string->length - 1] = '\0';
    }
}

void bvr_string_concat(bvr_string_t* string, const char* other){
    BVR_ASSERT(string);
    
    if(other) {
        
        // string is already allocated
        if(string->data){
            unsigned int size = string->length;

            // new size = size - 1 (EOF) + strlen(other) + 1 (new EOF) 
            string->length = string->length + strlen(other);
            string->data = realloc(string->data, string->length);
            BVR_ASSERT(string->data);

            strcat(string->data, other);
            string->data[string->length - 1] = '\0';
        }
        else {
            string->length = strlen(other) + 1;
            string->data = malloc(string->length);
            
            memcpy(string->data, other, string->length - 1);
            string->data[string->length - 1] = '\0';
        }
    }
}

void bvr_string_create_and_copy(bvr_string_t* dest, bvr_string_t* source){
    BVR_ASSERT(dest);

    if(source) {
        dest->length = source->length;
        dest->data = malloc(dest->length);
        BVR_ASSERT(dest->data);

        memcpy(dest->data, source->data, dest->length);
        dest->data[dest->length - 1] = '\0';
    }
}

void bvr_string_insert(bvr_string_t* string, const size_t offset, const char* value){
    BVR_ASSERT(string);
    BVR_ASSERT(string->data);
    BVR_ASSERT(value);
    
    bvr_string_t prev;
    bvr_string_create_and_copy(&prev, string);

    if(value) {
        const size_t vlen = strlen(value);
        BVR_PRINTF("string %x ; string size %i", string->data, string->length);

        string->length += vlen;
        string->data = realloc(string->data, string->length);
        BVR_ASSERT(string->data);

        memset(string->data, 0, string->length);

        strncpy(string->data, prev.data, offset);
        string->data[offset] = '\0';
        strncat(string->data, value, vlen);
        strncat(string->data, &prev.data[offset], prev.length - offset);
    }

    bvr_destroy_string(&prev);
}

const char* bvr_string_get(bvr_string_t* string){
    BVR_ASSERT(string);
    return (const char*)string->data;
}

void bvr_destroy_string(bvr_string_t* string){
    BVR_ASSERT(string);
    free(string->data);
    string->data = NULL;
}

void bvr_create_pool(bvr_pool_t* pool, size_t size, size_t count){
    BVR_ASSERT(pool);
    BVR_ASSERT(size < 255);

    pool->data = NULL;
    pool->next = NULL;
    pool->elemsize = size;
    pool->capacity = count;

    if(size && count){

        pool->data = calloc(pool->capacity, (pool->elemsize + sizeof(struct bvr_pool_block_s)));
        BVR_ASSERT(pool->data);
        pool->next = (struct bvr_pool_block_s*)pool->data;

        struct bvr_pool_block_s* block = (struct bvr_pool_block_s*)pool->data;
        for (size_t i = 0; i < pool->capacity - 1; i++)
        {
            block->next = i;
            block = (struct bvr_pool_block_s*)(pool->data + i * (pool->elemsize + sizeof(struct bvr_pool_block_s)));
        }
        
        block->next = 0;

    }
}

void* bvr_pool_alloc(bvr_pool_t* pool){
    BVR_ASSERT(pool);

    if(pool->next){
        struct bvr_pool_block_s* block = pool->next;
        
        pool->next = (struct bvr_pool_block_s*)(
            pool->data + block->next * (pool->elemsize + sizeof(struct bvr_pool_block_s))
        );
        return (void*)(block + sizeof(struct bvr_pool_block_s));
    }

    return NULL;
}

void* bvr_pool_try_get(bvr_pool_t* pool, int index){
    BVR_ASSERT(pool);
    
    int counter = pool->capacity;
    struct bvr_pool_block_s* block = (struct bvr_pool_block_s*)pool->data;
    while (block->next != NULL || counter > 0)
    {
        if(index == pool->capacity - counter) {
            return (void*)(block + sizeof(struct bvr_pool_block_s));
        }

        BVR_POOL_NEXT(pool->data, pool->elemsize, block);

        counter--;
    }
    
    return NULL;
}

void bvr_pool_free(bvr_pool_t* pool, void* ptr){
    BVR_ASSERT(pool);

    if(ptr){
        struct bvr_pool_block_s* prev = pool->next;
        pool->next = (struct bvr_pool_block_s*)((char*)ptr - sizeof(struct bvr_pool_block_s));
        pool->next->next = ((size_t)prev / (pool->elemsize + sizeof(struct bvr_pool_block_s))) - (size_t)pool->data;
    }
}

void bvr_destroy_pool(bvr_pool_t* pool){
    BVR_ASSERT(pool);

    free(pool->data);
    pool->data = NULL;
    pool->next = NULL;
}