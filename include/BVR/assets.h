#pragma once

#include <BVR/config.h>
#include <BVR/utils.h>
#include <BVR/scene.h>

#define BVR_OPEN_READ  1
#define BVR_OPEN_WRITE 2

typedef struct bvr_asset_s {
    bvr_uuid_t id;

    bvr_string_t path;
    char open_mode;
} bvr_asset_t;

bvr_asset_t* bvr_register_asset(bvr_book_t* book, const char* path, char open_mode);
int bvr_find_asset(bvr_book_t* book, const char* path, bvr_asset_t* asset);

BVR_H_FUNC FILE* bvr_open_asset(bvr_asset_t* asset){
    char open_mode[2] = "rb";
    if(asset->open_mode == BVR_OPEN_WRITE) {
        open_mode[0] = 'w';
    }

    BVR_PRINTF("found %s", asset->path.string);
    return fopen(asset->path.string, open_mode);
}

void bvr_write_book_dataf(FILE* file, bvr_book_t* book);
void bvr_open_book_dataf(FILE* file, bvr_book_t* book);

BVR_H_FUNC void bvr_write_book(const char* path, bvr_book_t* book){
    FILE* file = fopen(path, "wb");
    bvr_write_book_dataf(file, book);
    fclose(file);
}

BVR_H_FUNC void bvr_open_book(const char* path, bvr_book_t* book){
    FILE* file = fopen(path, "rb");
    bvr_open_book_dataf(file, book);
    fclose(file);
}