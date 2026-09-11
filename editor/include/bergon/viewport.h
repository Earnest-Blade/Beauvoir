#pragma once

#include <bvr/config.h>
#include <bvr/common.h>

#include <bvr/window.h>
#include <bvr/graphics.h>

struct bgs_viewport_s {
    void* context;
    void* widget;
};

void* bgs_create_viewport(struct bgs_viewport_s* viewport);
void bgs_destroy_viewport(struct bgs_viewport_s* viewport);