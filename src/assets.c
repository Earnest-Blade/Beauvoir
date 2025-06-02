#include <BVR/assets.h>

#include <BVR/editor/flags.h>

#include <BVR/file.h>

#include <string.h>
#include <malloc.h>
#include <unistd.h>

struct bvri_chunk_data_s {
    uint32_t length;
    uint16_t flag;
    void* buffer;
};

struct bvri_header_data_s {
    char sig[4];
    uint32_t size;
};

#pragma region write

static void bvri_clear_file(FILE* file){
    BVR_ASSERT(file);

    fflush(file);
    fseek(file, 0, SEEK_SET);
    ftruncate(fileno(file), 0);
}

static void bvri_write_string(FILE* file, bvr_string_t* string){
    fwrite(&string->length, sizeof(uint16_t), 1, file);
    if(string->string){
        fwrite(string->string, sizeof(char), string->length, file);
    }
}

static void bvri_write_chunk(FILE* file, struct bvri_chunk_data_s* data){
    fwrite(&data->length, sizeof(uint32_t), 1, file);
    fwrite(&data->flag, sizeof(uint16_t), 1, file);
    if(data->buffer){
        fwrite(data->buffer, sizeof(char), data->length, file);
    }
}

static void bvri_write_chunk_data(FILE* file, uint32_t length, uint16_t flag, void* buffer){
    fwrite(&length, sizeof(uint32_t), 1, file);
    fwrite(&flag, sizeof(uint16_t), 1, file);
    if(buffer){
        fwrite(buffer, sizeof(char), length, file);
    }
}

void bvr_write_book_dataf(FILE* file, bvr_book_t* book){
    BVR_ASSERT(file);
    BVR_ASSERT(book);

    struct bvri_header_data_s header;
    header.sig[0] = 'B';  // 0x005FFD44
    header.sig[1] = 'V';  
    header.sig[2] = 'R';  
    header.sig[3] = 'B';
    header.size = 0;

    bvri_clear_file(file);

    fseek(file, 0, SEEK_SET);
    fwrite(&header, sizeof(struct bvri_header_data_s), 1, file);

    // write page informations
    {
        bvr_page_t* page = &book->page;

        struct {
            float near, far, scale;
            bvr_transform_t transform;
        } camera;

        camera.near = page->camera.near;
        camera.far = page->camera.far;
        camera.scale = page->camera.field_of_view.scale;
        memcpy(&camera.transform, &page->camera.transform, sizeof(bvr_transform_t));

        bvri_write_string(file, &page->name);
        bvri_write_chunk_data(file, sizeof(camera), BVR_EDITOR_CAMERA, &camera);
    }

    fflush(file);
}

#pragma endregion

#pragma region open

static void bvri_read_string(FILE* file, bvr_string_t* string){
    string->length = bvr_freadu16_le(file);
    if(string->length){
        string->string = malloc(string->length);
        BVR_ASSERT(string->string);

        fread(string->string, sizeof(char), string->length - 1, file);
        string->string[string->length - 1] = '\0';
    }
}

static void bvri_read_chunk(FILE* file, struct bvri_chunk_data_s* chunk, void* object){
    BVR_ASSERT(object);
    BVR_ASSERT(chunk);

    chunk->length = bvr_freadu32_le(file);
    chunk->flag = bvr_freadu16_le(file);
    chunk->buffer = object;

    if(chunk->length){
        fread(object, sizeof(char), chunk->length, file);
    }
}

void bvr_open_book_dataf(FILE* file, bvr_book_t* book){
    BVR_ASSERT(file);
    BVR_ASSERT(book);

    fseek(file, 0, SEEK_SET);

    struct bvri_header_data_s header;
    fread(&header.sig, sizeof(char), 4, file);
    header.size = bvr_fread32_le(file);

    BVR_ASSERT(((int)header.sig) == 0x005FFD44);

    // read page informations
    {
        bvr_page_t* page = &book->page;

        struct bvri_chunk_data_s camera;
        BVR_PRINTF("ftell %x", ftell(file));

        bvri_read_string(file, &page->name);

        BVR_PRINTF("ftell %x", ftell(file));

        if(bvr_fread32_le(file)){
            BVR_ASSERT(bvr_freadu16_le(file) == BVR_EDITOR_CAMERA);

            float near = bvr_freadf(file);
            float far = bvr_freadf(file);
            float scale = bvr_freadf(file);

            bvr_create_orthographic_camera(page, 
                &book->window.framebuffer, near, far, scale
            );

            // copy transform
            fread(&page->camera.transform, sizeof(bvr_transform_t), 1, file);            
        }
    }
}

#pragma endregion