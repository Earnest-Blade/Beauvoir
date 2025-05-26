#include <BVR/buffer.h>
#include <BVR/utils.h>

#include <malloc.h>
#include <string.h>

void bvr_create_string(bvr_string_t* string, const char* value){
    BVR_ASSERT(string);

    string->length = 0;
    string->string = NULL;
    if(value){
        string->length = strlen(value) + 1;
        string->string = malloc(string->length);
        BVR_ASSERT(string->string);

        memcpy(string->string, value, string->length - 1);
        string->string[string->length - 1] = '\0';
    }
}

void bvr_string_concat(bvr_string_t* string, const char* other){
    BVR_ASSERT(string);
    
    if(other) {
        
        // string is already allocated
        if(string->string){
            unsigned int size = string->length;

            // new size = size - 1 (EOF) + strlen(other) + 1 (new EOF) 
            string->length = string->length + strlen(other);
            string->string = realloc(string->string, string->length);
            BVR_ASSERT(string->string);

            strcat(string->string, other);
            string->string[string->length - 1] = '\0';
        }
        else {
            string->length = strlen(other) + 1;
            string->string = malloc(string->length);
            
            memcpy(string->string, other, string->length - 1);
            string->string[string->length - 1] = '\0';
        }
    }
}

void bvr_string_create_and_copy(bvr_string_t* dest, bvr_string_t* source){
    BVR_ASSERT(dest);

    if(source) {
        dest->length = source->length;
        dest->string = malloc(dest->length);
        BVR_ASSERT(dest->string);

        memcpy(dest->string, source->string, dest->length);
        dest->string[dest->length - 1] = '\0';
    }
}

void bvr_string_insert(bvr_string_t* string, const size_t offset, const char* value){
    BVR_ASSERT(string);
    BVR_ASSERT(string->string);
    BVR_ASSERT(value);
    
    bvr_string_t prev;
    bvr_string_create_and_copy(&prev, string);

    if(value) {
        const size_t vlen = strlen(value);
        //BVR_PRINTF("string %x ; string size %i", string->data, string->length);

        string->length += vlen;
        string->string = realloc(string->string, string->length);
        BVR_ASSERT(string->string);

        memset(string->string, 0, string->length);

        strncpy(string->string, prev.string, offset);
        string->string[offset] = '\0';
        strncat(string->string, value, vlen);
        strncat(string->string, &prev.string[offset], prev.length - offset);
    }

    bvr_destroy_string(&prev);
}

const char* bvr_string_get(bvr_string_t* string){
    BVR_ASSERT(string);
    return (const char*)string->string;
}

void bvr_destroy_string(bvr_string_t* string){
    BVR_ASSERT(string);
    free(string->string);
    string->string = NULL;
}

void bvr_create_pool(bvr_pool_t* pool, size_t size, size_t count){
    BVR_ASSERT(pool);
    BVR_ASSERT(size < 255);

    pool->data = NULL;
    pool->next = NULL;
    pool->count = 0;
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
        
        if(block->next >= pool->capacity){
            BVR_PRINT("pool is full");
            return NULL;
        }

        pool->count++;
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
    while (block->next || counter > 0)
    {
        if(index == pool->capacity - counter) {
            return (void*)(block + sizeof(struct bvr_pool_block_s));
        }

        block = (struct bvr_pool_block_s*)(pool->data + block->next * (pool->elemsize + sizeof(struct bvr_pool_block_s)));

        counter--;
    }
    
    return NULL;
}

void bvr_pool_free(bvr_pool_t* pool, void* ptr){
    BVR_ASSERT(pool);

    if(ptr){
        pool->count--;

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