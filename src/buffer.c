#include <BVR/buffer.h>
#include <BVR/utils.h>

#include <malloc.h>
#include <string.h>

#define BVR_GROWTH_FACTOR 2

static int bvri_grow_buffer(void* ptr, size_t* size){
    if(ptr && size){
        ptr = realloc(ptr, (*size *= BVR_GROWTH_FACTOR));
        return ptr != NULL;
    }

    return 0;
}

void bvr_create_memstream(bvr_memstream_t* stream, unsigned long long const size){
    BVR_ASSERT(stream);

    stream->data = NULL;
    stream->size = size;
    stream->cursor = NULL;

    if(size){
        stream->data = malloc(size);
        stream->cursor = stream->data;
    }
}

void bvr_memstream_write(bvr_memstream_t* stream, const void* data, const size_t size){
    BVR_ASSERT(stream && stream->data);
    BVR_ASSERT(data);

    if(stream->cursor - (char*)stream->data + size < stream->size){
        memcpy(stream->cursor, data, size);
        stream->cursor += size;
    }
    else {
        BVR_PRINT("out of bounds!");
    }
}

void bvr_memstream_read(bvr_memstream_t* stream, void* dest, const size_t size){
    BVR_ASSERT(stream && stream->data);
    BVR_ASSERT(dest);

    if(stream->cursor - (char*)stream->data + size < stream->size){
        memcpy(dest, stream->cursor, size);
        stream->cursor += size;
    }
    else {
        BVR_PRINT("out of bounds!");
    }
}

void bvr_memstream_seek(bvr_memstream_t* stream, size_t position, int mode){
    BVR_ASSERT(stream);

    switch (mode)
    {
    case SEEK_CUR:
        {
            if(stream->cursor - (char*)stream->data + position < stream->size){
                stream->cursor += position;
            } 
            else {
                BVR_PRINT("out of bounds!");
            }
        }
        break;

    case SEEK_SET:
        {
            if(position <= stream->size){
                stream->cursor = stream->data + position;
            }
            else {
                BVR_PRINT("out of bounds!");
            }
        }
        break;

    case SEEK_END:
        {
            if(position <= stream->size){
                stream->cursor = stream->data + (stream->size - position);
            }
            else {
                BVR_PRINT("out of bounds!");
            }
        }
        break;
    default:
        break;
    }
}

void bvr_destroy_memstream(bvr_memstream_t* stream){
    BVR_ASSERT(stream);

    free(stream->data);

    stream->size = 0;
    stream->cursor = NULL;
    stream->data = NULL;
}

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

void bvr_destroy_string(bvr_string_t* string){
    BVR_ASSERT(string);
    free(string->string);
    string->string = NULL;
    string->length = 0;
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