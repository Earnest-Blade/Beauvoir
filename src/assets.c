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

#pragma region asset db

bvr_asset_t* bvr_register_asset(bvr_book_t* book, const char* path, char open_mode){
    BVR_ASSERT(book);
    BVR_ASSERT(path);

    // if the file does not exists
    if(access(path, F_OK) != 0){
        BVR_PRINTF("%s does not exist!", path);
        return NULL;
    }   

    bvr_asset_t asset;

    asset.open_mode = open_mode;

    bvr_create_uuid(asset.id);
    bvr_create_string(&asset.path, path);

    bvr_memstream_seek(&book->asset_stream, 0, SEEK_NEXT);
    bvr_memstream_write(&book->asset_stream, asset.id, sizeof(bvr_uuid_t));
    bvr_memstream_write(&book->asset_stream, &asset.path.length, sizeof(unsigned short));
    bvr_memstream_write(&book->asset_stream, asset.path.string, asset.path.length);
    bvr_memstream_write(&book->asset_stream, &asset.open_mode, sizeof(char));

    bvr_destroy_string(&asset.path);
}

// TODO: improve with an hash map
int bvr_find_asset(bvr_book_t* book, const char* path, bvr_asset_t* asset){
    BVR_ASSERT(book);
    BVR_ASSERT(path);

    uint16_t string_length;
    const char* prev_cursor = book->asset_stream.cursor;

    bvr_memstream_seek(&book->asset_stream, 0, SEEK_SET);
    while (book->asset_stream.cursor < prev_cursor)
    {
        // skip uuid
        book->asset_stream.cursor += sizeof(bvr_uuid_t);

        string_length = *((uint16_t*)book->asset_stream.cursor);

        if(!strncmp(book->asset_stream.cursor + sizeof(unsigned short), path, string_length)){
            memcpy(&asset->id, book->asset_stream.cursor - sizeof(bvr_uuid_t), sizeof(bvr_uuid_t));
            asset->path.length = string_length;
            asset->path.string = book->asset_stream.cursor + sizeof(unsigned short);
            
            book->asset_stream.cursor += string_length;
            asset->open_mode = *book->asset_stream.cursor;
            return BVR_OK;
        }

        book->asset_stream.cursor += string_length;

        // go to the end
        bvr_memstream_seek(&book->asset_stream, sizeof(uint16_t) + sizeof(char), SEEK_CUR);
    }
    
    return BVR_FAILED;
}

#pragma endregion

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

    uint32_t bin_offset = 0;
    // write asset informaions
    {
        uint32_t stream_size = book->asset_stream.next - (char*)book->asset_stream.data;
        bin_offset = 0;

        fwrite(&stream_size, sizeof(uint32_t), 1, file);
        fwrite(&bin_offset, sizeof(uint32_t), 1, file);
        fwrite(book->asset_stream.data, stream_size, 1, file);
    }

    // write page informations
    {
        bvr_page_t* page = &book->page;

        struct {
            float near, far, scale;
            bvr_transform_t transform;
        } camera;

        struct {
            uint32_t size;
            uint32_t offset;

            bvr_string_t name;
            uint16_t type;
            bvr_uuid_t id;
            int flags;

            bvr_transform_t transform;
        } actor;

        camera.near = page->camera.near;
        camera.far = page->camera.far;
        camera.scale = page->camera.field_of_view.scale;
        memcpy(&camera.transform, &page->camera.transform, sizeof(bvr_transform_t));

        bvri_write_string(file, &page->name);
        bvri_write_chunk_data(file, sizeof(camera), BVR_EDITOR_CAMERA, &camera);
    }

    // binary chunk
    {
 
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

    // read the header
    struct bvri_header_data_s header;
    fread(&header.sig, sizeof(char), 4, file);
    header.size = bvr_fread32_le(file);

    // check for BRVB signature
    BVR_ASSERT(
        header.sig[0] == 'B' && 
        header.sig[1] == 'V' && 
        header.sig[2] == 'R' && 
        header.sig[3] == 'B'
    );

    // read asset informations
    {
        uint32_t section_size = bvr_fread32_le(file);
        uint32_t asset_offset = bvr_fread32_le(file);

        // detroy current asset stream
        if(!book->asset_stream.data || book->asset_stream.size < section_size){
            bvr_destroy_memstream(&book->asset_stream);
            bvr_create_memstream(&book->asset_stream, section_size);
        }
        else {
            bvr_memstream_clear(&book->asset_stream);
        }

        // copy previously saved asset data stream into the asset stream
        fread(book->asset_stream.data, sizeof(char), section_size, file);
    }

    // read page informations
    {
        bvr_page_t* page = &book->page;
        struct bvri_chunk_data_s camera;

        // get scene name
        bvri_read_string(file, &page->name);

        // get camera component
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

    // binary chunk
    {

    }
}

#pragma endregion