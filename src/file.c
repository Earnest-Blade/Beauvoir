#include <BVR/file.h>
#include <BVR/utils.h>

#include <malloc.h>
#include <memory.h>

size_t bvr_get_file_size(FILE* file){
    size_t currp = ftell(file);
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, currp, SEEK_SET);
    return size;
}

int bvr_read_file(bvr_string_t* string, FILE* file){
    BVR_ASSERT(string);
    BVR_ASSERT(file);

    size_t file_size = bvr_get_file_size(file);
    size_t string_p = string->length;

    if(string->data){
        BVR_ASSERT(0 || "cannot copy on a previously allocated string :(");
    }
    else {
        // TODO: check if size is correct
        string->length = file_size + 1;
        string->data = malloc(string->length);
        BVR_ASSERT(string->data);

        size_t final_size = fread(string->data, sizeof(char), file_size, file);
        string->data[string->length - 1] = '\0';
    }

    return BVR_OK;
}

short bvr_fread16(FILE* file){
    uint8_t a, b;
    a = bvr_freadu8(file);
    b = bvr_freadu8(file);
    return (short)((b << 8) | a);
}

int bvr_fread32(FILE* file){
    uint8_t a, b, c, d;
    a = bvr_freadu8(file);
    b = bvr_freadu8(file);
    c = bvr_freadu8(file);
    d = bvr_freadu8(file);
    return (int)((((d << 8) | c) << 8 | b) << 8 | a);
}

uint8_t bvr_freadu8(FILE* file){
    int v = getc(file);
    if(v == EOF){
        BVR_PRINTF("failed to read character %i", errno);
        return 0;
    }

    return (uint8_t)v;
}

void bvr_freadstr(char* string, size_t size, FILE* file){
    if(string){
        fread(string, sizeof(uint8_t), size - 1, file);
        string[size - 1] = '\0';
    }
}

uint16_t bvr_freadu16(FILE* file){
    uint8_t a, b;
    a = bvr_freadu8(file);
    b = bvr_freadu8(file);
    return (uint16_t)((b << 8) | a);
}

uint32_t bvr_freadu32(FILE* file){
    uint8_t a, b, c, d;
    a = bvr_freadu8(file);
    b = bvr_freadu8(file);
    c = bvr_freadu8(file);
    d = bvr_freadu8(file);
    return (uint32_t)((((d << 8) | c) << 8 | b) << 8 | a);
}