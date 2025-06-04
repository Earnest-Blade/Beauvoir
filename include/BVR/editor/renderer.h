#pragma once

#include <stdint.h>

/*
    buffer related functions
*/

int bvri_create_editor_render_buffers(uint32_t* array_buffer, uint32_t* vertex_buffer, size_t vertex_size);

void bvri_bind_editor_buffers(uint32_t array_buffer, uint32_t vertex_buffer);
void bvri_set_editor_buffers(float* vertices, uint32_t vertices_count);

void bvri_draw_editor_buffer(int drawmode, uint32_t element_offset, uint32_t element_count);

void bvri_destroy_editor_render_buffers(uint32_t* array_buffer, uint32_t* vertex_buffer);