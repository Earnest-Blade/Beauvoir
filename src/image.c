#include <bvr/image.h>
#include <bvr/utils.h>
#include <BVR/file.h>

#include <bvr/shader.h>

#include <malloc.h>
#include <memory.h>
#include <math.h>

#include <glad/glad.h>

#ifndef BVR_NO_PNG

#include <png.h>

#define BVR_PNG_HEADER_LENGTH 8

static int bvri_is_png(FILE* __file) {
    fseek(__file, 0, SEEK_SET);
    uint8_t header[BVR_PNG_HEADER_LENGTH];
    fread(header, 1, BVR_PNG_HEADER_LENGTH, __file);
    return png_sig_cmp(header, 0, BVR_PNG_HEADER_LENGTH) == 0;
}

static void bvri_png_error(png_structp sptr, png_const_charp cc){
    BVR_PRINT(cc);
    BVR_ASSERT(0);
}

static int bvri_load_png(bvr_image_t* image, FILE* file){
    png_structp pngldr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, bvri_png_error, NULL);
    BVR_ASSERT(pngldr);

    png_infop pnginfo = png_create_info_struct(pngldr);
    BVR_ASSERT(pnginfo);

    if(setjmp(png_jmpbuf(pngldr))){
        png_destroy_read_struct(&pngldr, &pnginfo, NULL);
        return BVR_FAILED;
    }

    fseek(file, BVR_PNG_HEADER_LENGTH, SEEK_SET);

    png_init_io(pngldr, file);
    png_set_sig_bytes(pngldr, BVR_PNG_HEADER_LENGTH);
    png_read_info(pngldr, pnginfo);

    image->width = png_get_image_width(pngldr, pnginfo);
    image->height = png_get_image_height(pngldr, pnginfo);
    image->depth = png_get_bit_depth(pngldr, pnginfo);
    int color_type = png_get_color_type(pngldr, pnginfo);

    if(color_type == PNG_COLOR_TYPE_PALETTE){
        png_set_palette_to_rgb(pngldr);
    }
    
    if(color_type == PNG_COLOR_TYPE_GRAY && image->depth < 8){
        png_set_expand_gray_1_2_4_to_8(pngldr);
    }

    if(png_get_valid(pngldr, pnginfo, PNG_INFO_tRNS)){
        png_set_tRNS_to_alpha(pngldr);
    }

    if(image->depth == 16){
        png_set_strip_16(pngldr);
    }
    else if(image->depth < 8){
        png_set_packing(pngldr);
    }

    png_read_update_info(pngldr, pnginfo);
    color_type = png_get_color_type(pngldr, pnginfo);

    switch (color_type)
    {
    case PNG_COLOR_TYPE_RGB:
        image->format = BVR_RGB;
        image->channels = 3;
        break;
    case PNG_COLOR_TYPE_RGBA:
        image->format = BVR_RGBA;
        image->channels = 4;
        break;
    default:
        BVR_PRINTF("color type %x is not supported!", color_type);
        png_destroy_read_struct(&pngldr, &pnginfo, NULL);
        break;
    }

    size_t rowbytes = png_get_rowbytes(pngldr, pnginfo);
    image->pixels = malloc(image->width * image->height * image->channels * sizeof(uint8_t));
    BVR_ASSERT(image->pixels);

    uint8_t** rowp = malloc(image->height * sizeof(uint8_t*));
    BVR_ASSERT(rowp);

    for (size_t i = 0; i < image->height; i++)
    {
        rowp[image->height - i - 1] = image->pixels + i * rowbytes;
    }

    png_read_image(pngldr, rowp);

    free(rowp);
    png_destroy_read_struct(&pngldr, &pnginfo, NULL);
        
    return image->pixels == NULL;
}

#endif

#ifndef BVR_NO_BMP

/*
    https://en.wikipedia.org/wiki/BMP_file_format
*/

static int bvri_is_bmp(FILE* file){
    fseek(file, 0, SEEK_SET);

    int size;
    if(bvr_freadu8(file) != 'B') return 0;
    if(bvr_freadu8(file) != 'M') return 0;
    bvr_freadu32(file); // fsize
    bvr_freadu16(file); // res1
    bvr_freadu16(file); // res2
    bvr_freadu32(file); // offset
    size = bvr_freadu32(file);
    return (size == 12 || size == 40 || size == 56
        || size == 108 || size == 124);
}

static int bvri_load_bmp(bvr_image_t* image, FILE* file){
    image->format = BVR_BGR;
    image->channels = 3;

    // re-read the bitmap header
    fseek(file, 0, SEEK_SET);

    bvr_freadu16(file); // type
    int file_size = bvr_freadu32(file); // size
    bvr_freadu16(file); // res 1
    bvr_freadu16(file); // res 2
    uint32_t offset = bvr_freadu32(file); // offset
    uint32_t size = bvr_freadu32(file); // size

    image->width = bvr_fread32(file); // width
    image->height = bvr_fread32(file); // height

    bvr_freadu16(file); // color plane count
    image->depth = bvr_freadu16(file); // bit per pixels
    uint32_t compression = bvr_freadu32(file); // compression
    uint32_t buffer_size = bvr_freadu32(file); // image size

    bvr_fread32(file); // vertical res
    bvr_fread32(file); // horizontal res
    uint32_t color_type = bvr_freadu32(file);
    bvr_freadu32(file); // number of color used

    // sometimes the buffer_size can be = 0, just recalculate it
    if(buffer_size == 0){
        buffer_size = image->width * image->height * image->channels;
    }

    image->pixels = (uint8_t*) malloc(buffer_size * sizeof(uint8_t));
    BVR_ASSERT(image->pixels);

    if(offset == 0){
        offset = 64;
    }
    
    // seek to the start of data's block.
    fseek(file, offset, SEEK_SET);

    if(compression == 0){
        fread(image->pixels, sizeof(uint8_t), buffer_size, file);
    }
    else {
        BVR_ASSERT(0 && "unmanaged BMP compression type");
    }

    return image->pixels != NULL;
}

#endif

#ifndef BVR_NO_TIF

struct bvri_tififd_s {
    short count;
    struct bvri_tiftag_s {
        short id;
        short data_type;
        int data_count;
        int data_offset;
    }* tags;
    int next;
};

struct bvri_tifframe {
    uint32_t width;
    uint32_t height;
    uint16_t compression;

    uint32_t rows_per_strip;
    uint32_t* strip_offsets;
    uint32_t* strip_byte_counts;
    uint32_t strip_count;
    uint16_t samples_per_pixel;
    uint32_t bits_per_sample;
    uint32_t bit_count;
    uint16_t planar_configuration;
    uint16_t sample_format;
    uint32_t image_length;
    uint8_t orientation;
    uint8_t fill_order;
    uint32_t photometric_interpretation;
    uint8_t is_tiled;

    /*uint64_t photoshop_infos_count;
    uint8_t* photoshop_infos;*/
};

static int bvri_is_tif(FILE* file){
    fseek(file, 0, SEEK_SET);
    char sig1 = bvr_freadu8(file);
    char sig2 = bvr_freadu8(file);

    uint16_t version = bvr_fread16(file);
    uint32_t offset = bvr_fread32(file);

    return (sig1 == 'I' || sig1 == 'M') 
        && (sig2 == 'I' || sig2 == 'M') 
        && version == 42;
}

/*
    Copy TIF data from a buffer into a pointer.
*/
static void bvri_tif_copy_data(FILE* f, int offset, int size, void* data){
    size_t prev = ftell(f);
    fseek(f, offset, SEEK_SET);
    fread(data, sizeof(char), size, f);
    fseek(f, prev, SEEK_SET);
}

/*
// https://stackoverflow.com/questions/36035074/how-can-i-find-an-overlap-between-two-given-ran
static int bvri_tif_do_ranges_overlap(uint64_t xstart, uint64_t xend, uint64_t ystart, uint64_t yend,
    uint64_t* overlap_start, uint64_t* overlap_end){
    
    size_t range = fmax(xend, yend) - fmin(xstart, ystart);
    size_t sum = (xend - xstart) + (yend - ystart);

    if(sum > range){
        if(overlap_end) {
            *overlap_end = fmin(xend, yend);
        }
        if(overlap_start){
            *overlap_start = fmax(xstart, ystart);
        }
        return BVR_OK;
    }

    return BVR_FAILED;
}
*/

/*
    Return the size of each tag's types
*/
static uint32_t bvri_tif_sizeof(uint32_t size){
    switch (size)
    {
    case 1: case 2:
        return sizeof(char);
    case 3: return sizeof(short);
    case 4: return sizeof(int);
    
    default:
        return 0;
    }
}

/*
    Sources :
    https://github.com/jkriege2/TinyTIFF/blob/master/src/tinytiffreader.c
    https://www.fileformat.info/format/tiff/egff.htm
*/
static int bvri_load_tif(bvr_image_t* image, FILE* file){
    fseek(file, 0, SEEK_SET);
    bvr_fread32(file); // id & version
    int idf_offset = bvr_fread32(file);

    struct bvri_tififd_s idf;
    struct bvri_tifframe frame;
    idf.count = 0;
    idf.tags = NULL;
    idf.next = idf_offset;

    // while we got a next image header
    while (idf.next)
    {
        // clear frame's data.
        memset(&frame, 0, sizeof(struct bvri_tifframe));
        
        // seek to the first bit
        fseek(file, idf.next, SEEK_SET);
        uint16_t tag_count = bvr_fread16(file); // number of tags
        idf.tags = malloc(sizeof(struct bvri_tiftag_s) * tag_count);
        BVR_ASSERT(idf.tags);
        
        // read tags data from file.
        fread(idf.tags, sizeof(struct bvri_tiftag_s), tag_count, file);

        // find each tags
        for (size_t tagi = 0; tagi < tag_count; tagi++)
        {
            int integer_id;
            memcpy(&integer_id, &idf.tags[tagi].id, sizeof(short));

            switch (integer_id)
            {
            case 257:{ // height
                    frame.height = idf.tags[tagi].data_offset;
                    frame.image_length = frame.height;
                }
                break;
            case 256:{ // width
                    frame.width = idf.tags[tagi].data_offset;
                }
                break;
            case 258: { // bit per sample
                    frame.bit_count = idf.tags[tagi].data_count;
                    // we get each component sizes and add them together
                    for (size_t ii = 0; ii < idf.tags[tagi].data_count; ii++)
                    {
                        short bpp;
                        bvri_tif_copy_data(file, idf.tags[tagi].data_offset, sizeof(short), &bpp);
                        frame.bits_per_sample += bpp;
                    }
                }
                break;
            case 259:{ // compression
                    frame.compression = idf.tags[tagi].data_offset;
                }
                break;
            case 262: { // PhotometricInterpretation
                    frame.photometric_interpretation = idf.tags[tagi].data_offset;
                }
                break;
            case 273: { // strip offsets
                    if(!frame.strip_offsets){
                        frame.strip_count = idf.tags[tagi].data_count;
                        frame.strip_offsets = calloc(frame.strip_count, sizeof(uint32_t));
                        if(frame.strip_offsets){
                            bvri_tif_copy_data(file, idf.tags[tagi].data_offset, 
                                bvri_tif_sizeof(idf.tags[tagi].data_type) * idf.tags[tagi].data_count, 
                                frame.strip_offsets
                            );
                        }
                        else {BVR_ASSERT(0 || "failed to allocate strip offset!");}
                    }
                }
                break;
            case 277: { // sample per pixel
                    frame.samples_per_pixel = idf.tags[tagi].data_offset;
                }
                break;
            case 278: { // row per strip
                    frame.rows_per_strip = idf.tags[tagi].data_offset;
                }
                break;
            case 339: { // sample format
                    frame.sample_format = idf.tags[tagi].data_offset;
                }
                break;
            case 279: {
                    if(!frame.strip_byte_counts){
                        frame.strip_count = idf.tags[tagi].data_count;
                        frame.strip_byte_counts = calloc(frame.strip_count, sizeof(uint32_t));
                        if(frame.strip_byte_counts){
                            bvri_tif_copy_data(file, idf.tags[tagi].data_offset, 
                                bvri_tif_sizeof(idf.tags[tagi].data_type) * idf.tags[tagi].data_count, 
                                frame.strip_byte_counts
                            );
                        }
                        else {BVR_ASSERT(0 || "failed to allocate strip byte offset!");}
                    }
                }
                break;
            case 284: { // planar config
                    frame.planar_configuration = idf.tags[tagi].data_offset;
                }
                break;
            case 274: { // image orientation
                    frame.orientation = idf.tags[tagi].data_offset;
                }
                break;
            case 266: { // fill order
                    frame.fill_order = idf.tags[tagi].data_offset;
                }
                break;
            case 325:
            case 323:
            case 324:
            case 322: { // all of those tags means that it's tiled-based
                    frame.is_tiled = 1;
                }
                break;
            /*case 37724: { // TODO: handle photoshop's tags
                    if(!frame.photoshop_infos){
                        frame.photoshop_infos_count = idf.tags[tagi].data_count;
                        frame.photoshop_infos = calloc(idf.tags[tagi].data_count, sizeof(uint8_t));
                        BVR_PRINTF("photoshop offset %i", idf.tags[tagi].data_offset);
                        
                        if(frame.photoshop_infos){
                            bvri_tif_copy_data(file, idf.tags[tagi].data_offset,
                                bvri_tif_sizeof(idf.tags[tagi].data_type) * idf.tags[tagi].data_count,
                                frame.photoshop_infos 
                            );
                        }
                        else {BVR_ASSERT(0 || "failed to allocate photoshop informations!");}
                    }
                }*/
            default:
                break;
            }
        }

        BVR_ASSERT(frame.compression == 1); // other compressions are not supported
        BVR_ASSERT(frame.is_tiled == 0); // tilling is not supported
        BVR_ASSERT(frame.orientation == 1); // other orientations are not supported
        BVR_ASSERT(frame.photometric_interpretation != 3); // palettes are not supported
        BVR_ASSERT(frame.width > 0 && frame.height > 0);
        BVR_ASSERT(frame.bits_per_sample == 8 
                    || frame.bits_per_sample == 16
                    || frame.bits_per_sample == 24
                    || frame.bits_per_sample == 32);
        
        image->width = frame.width;
        image->height = frame.height;
        image->layers.size += sizeof(bvr_layer_t);
        image->depth = frame.samples_per_pixel;

        if(frame.planar_configuration == 1){
            /*BVR_PRINTF("strip count %i", frame.strip_count);
            for (size_t strip = 0; strip < frame.strip_count; strip++)
            {
                BVR_PRINTF("start %i end %i", frame.strip_offsets[strip], frame.strip_byte_counts[strip]);    
            }*/

            BVR_ASSERT(0 || "configuration not supported!");
        }
        else if(frame.planar_configuration == 2) {
            /*
                Each color layers are stored in a different strip
                strip 1 -> R 
                strip 2 -> G 
                strip 3 -> B 
                strip 4 -> A
            */
            if(image->pixels){
                // free previous allocated memory
                free(image->pixels);
            }

            image->channels = frame.strip_count;
            image->pixels = malloc(image->width * image->height * image->channels);
            BVR_ASSERT(image->pixels);

            for (size_t strip = 0; strip < frame.strip_count; strip++)
            {
                uint8_t* strip_buffer = calloc(frame.strip_byte_counts[strip], sizeof(uint8_t));
                BVR_ASSERT(strip_buffer);

                // read the entire strip into a buffer
                fseek(file, frame.strip_offsets[strip], SEEK_SET);
                fread(strip_buffer, sizeof(uint8_t), frame.strip_byte_counts[strip], file);

                size_t image_index = strip;
                for (size_t strip_index = 0; strip_index < frame.strip_byte_counts[strip]; strip_index++)
                {
                    // copy each pixels into the final image buffer
                    image->pixels[image_index] = strip_buffer[strip_index];
                    image_index += image->channels;
                }
                
                free(strip_buffer);
            }

        }
        else {
            BVR_ASSERT(0 || "configuration is not supported!");
        }

        switch (image->channels)
        {
        case 1: image->format = BVR_RED; break;
        case 2: image->format = BVR_RG; break;
        case 3: image->format = BVR_RGB; break;
        case 4: image->format = BVR_RGBA; break;
        
        default:
            break;
        }

        // free ressources
        free(idf.tags);
        free(frame.strip_offsets);
        free(frame.strip_byte_counts);
        //free(frame.photoshop_infos);

        // seek at the end of the image descriptor header
        fseek(file, idf.next + 2 + sizeof(struct bvri_tiftag_s) * tag_count, SEEK_SET);

        // define next image descriptor header
        idf.next = bvr_fread32(file);
        if(idf.next){
            BVR_PRINT("using multiple framed TIF files might overwrite previous data!");
        }
    }

    return BVR_OK;
}

#endif

#ifndef BVR_NO_PSD

struct bvri_psdheader_s {
    char sig[4];
    short version;
    char res[6];
    short channels;

    uint32_t rows;
    uint32_t columns;
    short depth;

    // Bitmap = 0; Grayscale = 1; Indexed = 2; RGB = 3; CMYK = 4;
    // Multichannel = 7; Duotone = 8; Lab = 9.
    short mode; 
};

struct bvri_psdlayer_s {
    // top, left, bottom, right
    uint32_t bounds[4];
    short channel_count;
    struct bvri_psdlayerchannel_s {
        short id;
        uint32_t size;
        size_t position;
    }* channels;
    
    char sig[5];
    char blend_mode[5];
    char opacity;
    char clipping;
    char flags;

    bvr_string_t name;
};

struct bvri_psdressource_s {
    char sig[5];                // ressource block signature
    short id;                   // ressource id
    bvr_string_t name;          // name
    struct bvr_buffer_s data;   // pointer to the data
};

static int bvri_is_psd(FILE* file){
    uint8_t sig[5];
    short version;

    fseek(file, 0, SEEK_SET);
    fread(sig, sizeof(char), 4, file);
    sig[4] = '\0';

    version = bvr_freadu16_be(file);
    return strcmp(sig, "8BPS") == 0 //8BPS -> psd's magic number 
            && version == 1; 
}

static void bvri_psd_read_pascal_string(bvr_string_t* string, FILE* file){
    string->data = NULL;
    string->length = (size_t)bvr_freadu8(file) + 1;

    if(string->length - 1){
        string->data = malloc(string->length);
        BVR_ASSERT(string->data);

        fread(string->data, sizeof(char), string->length - 1, file);
        string->data[string->length - 1] = '\0';
    }
}

/*
    Sources :
    https://docs.fileformat.com/image/psd/
    https://www.fileformat.info/format/psd/egff.htm
    https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/#50577409_pgfId-1030196
    https://en.wikipedia.org/wiki/PackBits

    https://github.dev/MolecularMatters/psd_sdk/tree/master/src/Psd
*/
static int bvri_load_psd(bvr_image_t* image, FILE* file){
    struct bvri_psdheader_s header;
    
    struct {
        uint32_t size;
        uint8_t* data;
    } color_mode_section;

    struct {
        uint32_t size; // ressource's data full size
        uint32_t end_position;
        uint32_t count; //
        struct bvri_psdressource_s block;
    } ressources_section;

    struct {
        size_t size;
        size_t end_position;

        size_t layer_size;
        size_t mask_size;

        uint8_t next_alpha_channel_is_global;
        short layer_count;
        struct bvri_psdlayer_s* layers;
    } layer_section;

    struct {
        short compression;
        short target_channel;
        uint32_t rows;
        uint32_t columns;
        uint32_t unpacked_length;
        uint32_t packed_length;
        uint8_t* unpacked_buffer;
        uint8_t* packed_buffer;
        uint16_t* rle_pack_lengths;
    } image_data_section;

    // reading psd's header
    // skip sig header
    fseek(file, 4, SEEK_SET);
    header.version = bvr_freadu16_be(file);
    fseek(file, 6, SEEK_CUR); // skip reserved
    header.channels = bvr_freadu16_be(file);
    header.rows = bvr_freadu32_be(file);
    header.columns = bvr_freadu32_be(file);
    header.depth = bvr_freadu16_be(file);
    header.mode = bvr_freadu16_be(file);

    // check for color mode section (if the size == 0, no section)
    color_mode_section.size = bvr_freadu32_be(file);
    color_mode_section.data = NULL;
    if(color_mode_section.size){
        BVR_PRINTF("color mode %i, should read full data", color_mode_section.size);
        BVR_ASSERT(0 || "not supported!");
    }

    // ressource section parsing
    ressources_section.size = bvr_freadu32_be(file);
    ressources_section.end_position = ftell(file) + ressources_section.size;
    
    if(ressources_section.size){
        while (ftell(file) < ressources_section.end_position)
        {
            bvr_freadstr(ressources_section.block.sig, sizeof(ressources_section.block.sig), file);
            
            // check for signature
            BVR_ASSERT(strcmp(ressources_section.block.sig, "8BIM") == 0);

            ressources_section.block.id = bvr_freadu16_be(file);

            bvri_psd_read_pascal_string(&ressources_section.block.name, file);
            bvr_freadu8(file); // read an empty character used as padding

            ressources_section.block.data.size = bvr_freadu32_be(file);
            ressources_section.block.data.elemsize = ressources_section.block.data.size;
            ressources_section.block.data.data = NULL;

            // seek to the end of the section. Each section's size must be even. 
            fseek(file, (ressources_section.block.data.size + 1) & ~1, SEEK_CUR);
            bvr_destroy_string(&ressources_section.block.name);
            
            free(ressources_section.block.data.data);
            ressources_section.block.data.data = NULL;
        }
        
        ressources_section.count++;

        fseek(file, ressources_section.end_position, SEEK_SET);
    }

    layer_section.size = bvr_freadu32_be(file);
    layer_section.end_position = ftell(file) + layer_section.size;
    {
        size_t start_of_the_header = ftell(file);

        layer_section.next_alpha_channel_is_global = 0;
        layer_section.layer_size = bvr_freadu32_be(file);
        layer_section.layer_count = bvr_freadu16_be(file);

        if(layer_section.layer_count < 0){
            layer_section.next_alpha_channel_is_global = 1;
            layer_section.layer_count = -layer_section.layer_count;
        }

        layer_section.layers = NULL;
        layer_section.layers = calloc(layer_section.layer_count, sizeof(struct bvri_psdlayer_s));
        BVR_ASSERT(layer_section.layers);
        
        struct bvri_psdlayer_s* layer;
        for (size_t layer_id = 0; layer_id < layer_section.layer_count; layer_id++)
        {
            size_t end_of_header;

            layer = &layer_section.layers[layer_id];

            layer->bounds[0] = bvr_freadu32_be(file);
            layer->bounds[1] = bvr_freadu32_be(file);
            layer->bounds[2] = bvr_freadu32_be(file);
            layer->bounds[3] = bvr_freadu32_be(file);
            layer->channel_count = bvr_freadu16_be(file);

            BVR_PRINTF("bounds %i %i %i %i channels %i", layer->bounds[0], layer->bounds[1], layer->bounds[2], layer->bounds[3], layer->channel_count);

            // skip channel info???
            layer->channels = calloc(layer->channel_count, sizeof(struct bvri_psdlayerchannel_s));
            for (size_t channel = 0; channel < layer->channel_count; channel++)
            {
                layer->channels[channel].id = bvr_freadu16_be(file);
                layer->channels[channel].position = 0;
                layer->channels[channel].size = bvr_freadu32_be(file);
            }
            
            BVR_PRINTF("before sig ftell %x", ftell(file));
            bvr_freadstr(layer->sig, 5, file);
            bvr_freadstr(layer->blend_mode, 5, file);

            BVR_ASSERT(strcmp(layer->sig, "8BIM") == 0);
            // TODO: define blend mode

            layer->opacity = bvr_freadu8_be(file);
            layer->clipping = bvr_freadu8_be(file);
            layer->flags = bvr_freadu8_be(file);
            bvr_freadu8_be(file); // filler bit

            end_of_header = ftell(file) + bvr_freadu32_be(file) + 4U; 

            fseek(file, bvr_freadu32_be(file), SEEK_CUR); // skip Layer mask / adjustment layer data
            fseek(file, bvr_freadu32_be(file), SEEK_CUR); // skip Layer blending ranges data

            BVR_PRINTF("informations : %s opa%i chan%i", layer->blend_mode, layer->opacity, layer->channel_count);
            
            bvri_psd_read_pascal_string(&layer->name, file);

            // pascal string padding.
            fseek(file, (layer->name.length - 1) - (((layer->name.length - 1) / 4) * 4) + 3, SEEK_CUR);

            int has_next_additional_data = 1;
            while(has_next_additional_data) {
                char additional_data_sig[5];
                char additional_data_tag[5];

                bvr_freadstr(additional_data_sig, sizeof(additional_data_sig), file);

                if(strcmp(additional_data_sig, "8BIM") == 0 || strcmp(additional_data_sig, "8B64") == 0){
                    size_t data_size;
                    
                    bvr_freadstr(additional_data_tag, sizeof(additional_data_tag), file);
                    // TODO: check tags

                    data_size = (bvr_freadu32_be(file) + 1) & ~1;
                    fseek(file, data_size, SEEK_CUR);

                    //BVR_PRINTF("add data infos %s %s size%i", additional_data_sig, additional_data_tag, data_size);
                }
                else {
                    has_next_additional_data = 0;
                }
            }

            fseek(file, end_of_header, SEEK_SET);
            BVR_PRINTF("layer name %s (%i)", layer->name.data, layer->name.length);
            BVR_PRINTF("end of header %x", ftell(file));
        }

        // calculate layers offsets
        {
            size_t prev_stream_position = ftell(file);

            for (size_t layer = 0; layer < layer_section.layer_count; layer++)
            {
                for (size_t channel = 0; channel < header.channels; channel++)
                {
                    layer_section.layers[layer].channels[channel].position = ftell(file);
                    fseek(file, layer_section.layers[layer].channels[channel].size, SEEK_CUR);
                }
                
            }
            
            fseek(file, prev_stream_position, SEEK_SET);
        }
    }

    image->channels = header.channels;
    image->width = header.columns;
    image->height = header.rows;
    image->depth = header.depth;

    switch (image->channels)
    {
    case 0: image->format = BVR_RED; break; // monochrome
    case 1: image->format = BVR_RED; break; // gray-scale
    case 2: image->format = BVR_RG; break;
    case 3: image->format = BVR_RGB; break; // rgb
    case 4: image->format = BVR_RGBA; break;
    default:
        BVR_ASSERT(0 || "image format not supported!");
        break;
    }

    BVR_PRINTF("psd format %i", image->format);

    image->pixels = malloc(image->width * image->height * image->channels * layer_section.layer_count);
    BVR_ASSERT(image->pixels);

    memset(image->pixels, 0, image->width * image->height * image->channels * layer_section.layer_count);
    
    image->layers.size = layer_section.layer_count * image->layers.elemsize;
    image->layers.data = calloc(layer_section.layer_count, image->layers.elemsize);
    BVR_ASSERT(image->layers.data);

    for (size_t layer = 0; layer < layer_section.layer_count; layer++)
    {
        bvr_string_create_and_copy(&((bvr_layer_t*)image->layers.data)[layer].name, &layer_section.layers[layer].name);
        ((bvr_layer_t*)image->layers.data)[layer].flags = 0;
        ((bvr_layer_t*)image->layers.data)[layer].blend_mode = 0;
        ((bvr_layer_t*)image->layers.data)[layer].width = layer_section.layers[layer].bounds[3] - layer_section.layers[layer].bounds[1];
        ((bvr_layer_t*)image->layers.data)[layer].height = layer_section.layers[layer].bounds[2] - layer_section.layers[layer].bounds[0];
        ((bvr_layer_t*)image->layers.data)[layer].anchor_x = layer_section.layers[layer].bounds[1];
        ((bvr_layer_t*)image->layers.data)[layer].anchor_y = layer_section.layers[layer].bounds[0];
    }

    image_data_section.unpacked_buffer = NULL;
    image_data_section.packed_buffer = NULL;
    image_data_section.rle_pack_lengths = NULL;
    image_data_section.compression = bvr_freadu16_be(file);
    {
        if(image_data_section.compression == 0){
            // proceed to RAW uncompression 

            BVR_ASSERT(0 || "unsupported compression mode");
        }
        else if(image_data_section.compression == 1){
            // do RLE uncompression
            for (size_t layer = 0; layer < layer_section.layer_count; layer++)
            {
                int layer_width = layer_section.layers[layer].bounds[3] - layer_section.layers[layer].bounds[1];
                int layer_height = layer_section.layers[layer].bounds[2] - layer_section.layers[layer].bounds[0];
                int layer_anchor_x = layer_section.layers[layer].bounds[1];
                int layer_anchor_y = layer_section.layers[layer].bounds[0];
            
                image_data_section.rle_pack_lengths = calloc(layer_height, sizeof(uint16_t));
                BVR_ASSERT(image_data_section.rle_pack_lengths);

                for (size_t channel = 0; channel < image->channels; channel++)
                {
                    size_t readed_size;

                    fseek(file, layer_section.layers[layer].channels[channel].position, SEEK_SET);

                    image_data_section.target_channel = layer_section.layers[layer].channels[channel].id; 
                    switch (image_data_section.target_channel)
                    {
                    case -1: // transparency
                        {
                            image_data_section.columns = layer_width;
                            image_data_section.rows = layer_height;  
                            image_data_section.target_channel = 3;
                        }
                        break;
                    case -2: // layer or vector
                        {
                            image_data_section.columns = layer_width;
                            image_data_section.rows = layer_height;
                            image_data_section.target_channel = 0;
                        }
                        break;
                    case -3: // layer mask
                        {
                            image_data_section.columns = layer_width;
                            image_data_section.rows = layer_height;
                            image_data_section.target_channel = 0;
                        }
                        break;
                    
                    case 1: // red
                    case 2: // blue
                    case 3: // green
                    default: // color channel
                        {
                            image_data_section.columns = layer_width;
                            image_data_section.rows = layer_height;
                        }
                        break;
                    }

                    image_data_section.packed_length = 0;
                    image_data_section.unpacked_length = image_data_section.columns * image_data_section.rows;
                    for (size_t j = 0; j < image_data_section.rows; j++)
                    {
                        image_data_section.rle_pack_lengths[j] = bvr_freadu16_be(file);
                        image_data_section.packed_length += image_data_section.rle_pack_lengths[j];
                    }
                    BVR_PRINTF("packed length %i", image_data_section.packed_length);
                    
                    image_data_section.packed_buffer = calloc(image_data_section.packed_length, sizeof(uint8_t));
                    image_data_section.unpacked_buffer = calloc(image_data_section.unpacked_length, sizeof(uint8_t));
                    BVR_ASSERT(image_data_section.packed_buffer);
                    BVR_ASSERT(image_data_section.unpacked_buffer);

                    BVR_PRINTF("channel %i id %i ftell %x", channel, image_data_section.target_channel, ftell(file));

                    readed_size = fread(image_data_section.packed_buffer, sizeof(uint8_t), image_data_section.packed_length, file);
                    //BVR_ASSERT(readed_size == image_data_section.packed_length);
                    if(readed_size != image_data_section.packed_length){
                        BVR_PRINT("skipping layer");
                        continue;
                    }

                    char count = 0;
                    size_t offset = 0;
                    uint8_t count_dis = 0;
                    uint8_t character = 0;
                    size_t readed_bytes = 0;
                    while (readed_bytes < image_data_section.packed_length 
                            && offset < image_data_section.unpacked_length)
                    {
                        BVR_ASSERT(readed_bytes < image_data_section.packed_length);
                        BVR_ASSERT(offset < image_data_section.unpacked_length);

                        if(readed_bytes < image_data_section.packed_length){
                            count = (char)image_data_section.packed_buffer[readed_bytes++];
                            
                            if(count == -0x80){
                                // byte == -128
                                // no-op
                            }
                            else if((count) & 0x80){
                                // 0x81 < byte < 0xFF
                                count_dis = (uint8_t)(0x101 - count);

                                BVR_ASSERT(offset + count_dis < image_data_section.unpacked_length);

                                memset(&image_data_section.unpacked_buffer[offset], 
                                    image_data_section.packed_buffer[readed_bytes++],
                                    count_dis
                                );

                                offset += count_dis;
                            }
                            else {
                                // 0x00 < byte < 0x7F
                                count_dis = (uint8_t)(count + 1);

                                BVR_ASSERT(offset + count_dis < image_data_section.unpacked_length);

                                memcpy(&image_data_section.unpacked_buffer[offset],
                                    &image_data_section.packed_buffer[readed_bytes],
                                    count_dis
                                );

                                offset += count_dis;
                                readed_bytes += count_dis;
                            }
                        }
                        else {
                            memset(image_data_section.unpacked_buffer, 0, image_data_section.unpacked_length - offset);
                            offset = image_data_section.unpacked_length;
                        }
                    }

                    for (size_t strip = 0; strip < image_data_section.rows; strip++)
                    {
                        for (size_t column = 0; column < image_data_section.columns; column++)
                        {
                            image->pixels[
                                ((strip * image->width + column) * image->channels + image_data_section.target_channel) +
                                (image->width * image->height * image->channels * layer)
                            ] = image_data_section.unpacked_buffer[strip * image_data_section.columns + column];
                        }
                    }
                    
                    free(image_data_section.packed_buffer);
                    free(image_data_section.unpacked_buffer);
                    image_data_section.packed_buffer = NULL;
                    image_data_section.unpacked_buffer = NULL;
                }
                
                free(image_data_section.rle_pack_lengths);
                image_data_section.rle_pack_lengths = NULL;
            }
            
        }
        else {
            BVR_ASSERT(0 || "unsupported compression mode");
        }
    }

    // freeing data
    for (size_t i = 0; i < layer_section.layer_count; i++)
    {
        bvr_destroy_string(&layer_section.layers[i].name);
        
        free(layer_section.layers[i].channels);
        layer_section.layers[i].channels = NULL;
    }

    free(layer_section.layers);

    return BVR_OK;
}

#endif

static void bvri_create_default_layer(bvr_image_t* image){
    BVR_ASSERT(image);

    image->layers.data = malloc(image->layers.elemsize);
    image->layers.size = image->layers.elemsize;

    ((bvr_layer_t*)image->layers.data)[0].blend_mode = 0;
    ((bvr_layer_t*)image->layers.data)[0].flags = 0;
    ((bvr_layer_t*)image->layers.data)[0].width = image->width;
    ((bvr_layer_t*)image->layers.data)[0].height = image->height;
    ((bvr_layer_t*)image->layers.data)[0].anchor_x = 0;
    ((bvr_layer_t*)image->layers.data)[0].anchor_y = 0;
    
    bvr_create_string(&((bvr_layer_t*)image->layers.data)[0].name, "DefaultLayer");
}

int bvr_create_image(bvr_image_t* image, FILE* file){
    BVR_ASSERT(image);
    BVR_ASSERT(file);

    image->width = 0;
    image->height = 0;
    image->depth = 0;
    image->format = 0;
    image->channels = 0;
    image->pixels = NULL;
    image->layers.data = NULL;
    image->layers.size = 0;
    image->layers.elemsize = sizeof(bvr_layer_t);

    int status = 0;
#ifndef BVR_NO_PNG
    if(bvri_is_png(file)){ 
        status = bvri_load_png(image, file);
    }
#endif

#ifndef BVR_NO_BMP
    if(bvri_is_bmp(file) && !status){
        status = bvri_load_bmp(image, file);
    }
#endif

#ifndef BVR_NO_TIF
    if(bvri_is_tif(file) && !status){
        status = bvri_load_tif(image, file);
    }
#endif

#ifndef BVR_NO_PSD
    if(bvri_is_psd(file) && !status){
        status = bvri_load_psd(image, file);
    }
#endif

    if(image->pixels && !image->layers.data){
        bvri_create_default_layer(image);
    }

#ifndef BVR_NO_FLIP
    if(image->pixels && status){
        bvr_flip_image_vertically(image);
    }
#endif

    return status;
}

void bvr_flip_image_vertically(bvr_image_t* image){
    int row;
    size_t stride = image->width * image->channels;
    uint8_t buffer[BVR_BUFFER_SIZE];
    uint8_t* bytes = image->pixels;

    for (row = 0; row < (image->height>>1); row++)
    {
        uint8_t* row0 = bytes + row * stride;
        uint8_t* row1 = bytes + (image->height - row - 1) * stride;

        size_t bleft = stride;
        while (bleft)
        {
            size_t bcpy = (bleft < sizeof(buffer)) ? bleft : sizeof(buffer);
            memcpy(buffer, row0, bcpy);
            memcpy(row0, row1, bcpy);
            memcpy(row1, buffer, bcpy);
            row0 += bcpy;
            row1 += bcpy;
            bleft -= bcpy;
        }
    }
}

void bvr_destroy_image(bvr_image_t* image){
    BVR_ASSERT(image);

    for (size_t layer = 0; layer < BVR_BUFFER_COUNT(image->layers); layer++)
    {
        bvr_destroy_string(&((bvr_layer_t*)image->layers.data)[layer].name);
    }
    

    free(image->pixels);
    free(image->layers.data);
    image->pixels = NULL;
    image->layers.data = NULL;
}

static int bvri_sizeof_format(int format, int depth){
    if(depth == 16){
        switch (format)
        {
        case BVR_RED: return BVR_RED16;
        case BVR_RG: return BVR_RG16;
        case BVR_RGB: case BVR_BGR: return BVR_RGB16;
        case BVR_RGBA: case BVR_BGRA: return BVR_RGBA16;
        default:
            return BVR_RED16;
        }
    }

    switch (format)
    {
    case BVR_RED: return BVR_RED8;
    case BVR_RG: return BVR_RG8;
    case BVR_RGB: case BVR_BGR: return BVR_RGB8;
    case BVR_RGBA: case BVR_BGRA: return BVR_RGBA8;
    default:
        return BVR_RED8;
    }
}

int bvr_create_texturef(bvr_texture_t* texture, FILE* file, int filter, int wrap){
    BVR_ASSERT(texture);
    BVR_ASSERT(file);

    memset(texture, 0, sizeof(bvr_texture_t));

    texture->filter = filter;
    texture->wrap = wrap;
    texture->id = 0;

    bvr_create_image(&texture->image, file);
    if(!texture->image.pixels){
        BVR_PRINT("invalid image!");
        return BVR_FAILED;
    }

    if(texture->image.layers.size > sizeof(bvr_layer_t)){
        // TODO: compress images into one layer
    }

    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, texture->image.width);
    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, texture->image.height);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (int)texture->wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (int)texture->wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)texture->filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)texture->filter);

    int format = texture->image.format;
    int internal_format = bvri_sizeof_format(texture->image.format, texture->image.depth);
    glTexStorage2D(GL_TEXTURE_2D, 1, internal_format, texture->image.width, texture->image.height);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->image.width, texture->image.height, format, GL_UNSIGNED_BYTE, texture->image.pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    free(texture->image.pixels);
    texture->image.pixels = NULL;

    return BVR_OK;
}

void bvr_texture_enable(bvr_texture_t* texture, int unit){
    glActiveTexture(unit);
    glBindTexture(GL_TEXTURE_2D, texture->id);
}

void bvr_texture_disable(void){
    glBindTexture(GL_TEXTURE_2D, 0);
}

void bvr_destroy_texture(bvr_texture_t* texture){
    BVR_ASSERT(texture);

    glDeleteTextures(1, &texture->id);
    
    bvr_destroy_image(&texture->image);
}


int bvr_create_texture_atlasf(bvr_texture_atlas_t* atlas, FILE* file, 
        uint32_t tile_width, uint32_t tile_height, int filter, int wrap){

    BVR_ASSERT(atlas);
    BVR_ASSERT(file);
    atlas->filter = filter;
    atlas->wrap = wrap;
    atlas->tile_width = tile_width;
    atlas->tile_height = tile_height;

    atlas->id = 0;
    
    bvr_create_image(&atlas->image, file);
    if(!atlas->image.pixels){
        BVR_PRINT("invalid image!");
        return BVR_FAILED;
    }

    if(atlas->image.layers.size > sizeof(bvr_layer_t)){
        // TODO: compress images into one layer
    }

    glGenTextures(1, &atlas->id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, atlas->id);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, atlas->image.width);
    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, atlas->image.height);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 1);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, (int)atlas->wrap);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, (int)atlas->wrap);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, (int)atlas->filter);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, (int)atlas->filter);

    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, bvri_sizeof_format(atlas->image.format, atlas->image.depth), 
        atlas->tile_width, atlas->tile_height, 
        (atlas->image.width / atlas->tile_width) * (atlas->image.height / atlas->tile_height)
    );

    for (size_t y = 0; y < atlas->image.height; y += atlas->tile_height)
    {
        for (size_t x = 0; x < atlas->image.width; x += atlas->tile_width)
        {
            glTexSubImage3D(
                GL_TEXTURE_2D_ARRAY, 0, 0, 0,
                y * (atlas->image.width / atlas->tile_width) + x,
                atlas->tile_width, atlas->tile_height, 1, atlas->image.format, GL_UNSIGNED_BYTE,
                atlas->image.pixels + ((y * atlas->tile_height * atlas->image.width + x * atlas->tile_width) * atlas->image.channels)
            );
        }
    }
    
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    free(atlas->image.pixels);
    atlas->image.pixels = NULL;

    return BVR_OK;
}

void bvr_texture_atlas_enablei(bvr_texture_atlas_t* atlas, int unit){
    glActiveTexture(unit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, atlas->id);
}

void bvr_texture_atlas_disable(void){
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void bvr_destroy_texture_atlas(bvr_texture_atlas_t* atlas){
    BVR_ASSERT(atlas);

    glDeleteTextures(1, &atlas->id);
    bvr_destroy_image(&atlas->image);
}

int bvr_create_layered_texturef(bvr_layered_texture_t* texture, FILE* file, int filter, int wrap){
    BVR_ASSERT(texture);
    BVR_ASSERT(file);
    texture->filter = filter;
    texture->wrap = wrap;

    texture->id = 0;

    bvr_create_image(&texture->image, file);
    if(!texture->image.pixels){
        BVR_PRINT("invalid image!");
        return BVR_FAILED;
    }

    if(texture->image.layers.size / sizeof(bvr_layer_t) < 1){
        BVR_PRINT("layered texture will load without layer info. Data might be lost.");
    }

    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture->id);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, texture->image.width);
    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, texture->image.height);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 1);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, (int)texture->wrap);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, (int)texture->wrap);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, (int)texture->filter);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, (int)texture->filter);

    BVR_PRINTF("format %i %i", texture->image.format, bvri_sizeof_format(texture->image.format, texture->image.depth));
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, bvri_sizeof_format(texture->image.format, texture->image.depth),
        texture->image.width, texture->image.height, 
        texture->image.layers.size / sizeof(bvr_layer_t)
    );

    for (size_t layer = 0; layer < texture->image.layers.size / sizeof(bvr_layer_t); layer++)
    {
        glTexSubImage3D(
            GL_TEXTURE_2D_ARRAY, 0, 
            ((bvr_layer_t*)texture->image.layers.data)[layer].anchor_x, 
            ((bvr_layer_t*)texture->image.layers.data)[layer].anchor_y,
            layer,
            ((bvr_layer_t*)texture->image.layers.data)[layer].width, 
            ((bvr_layer_t*)texture->image.layers.data)[layer].height, 
            1, texture->image.format, GL_UNSIGNED_BYTE,
            texture->image.pixels + texture->image.width * texture->image.height * texture->image.channels * layer
        );
    }
    
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    free(texture->image.pixels);
    texture->image.pixels = NULL;

    return BVR_OK;
}

void bvr_layered_texture_enable(bvr_layered_texture_t* texture, int unit){
    glActiveTexture(unit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture->id);
}

void bvr_layered_texture_disable(void){
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void bvr_destroy_layered_texture(bvr_layered_texture_t* texture){
    BVR_ASSERT(texture);

    glDeleteTextures(1, &texture->id);
    bvr_destroy_image(&texture->image);
}