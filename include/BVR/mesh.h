#pragma once

#include <stdint.h>

#include <BVR/utils.h>
#include <BVR/buffer.h>

#define BVR_DRAWMODE_TRIANGLES 0x0004

/*
    !! MESH BUFFERS DONT WORK LIKE OTHERS BUFFERS !!
    char* data; -> contains data 
    unsigned long long size; -> count of vertices
    unsigned int elemsize; -> type 
*/
typedef struct bvr_buffer_s bvr_mesh_buffer_t;

typedef enum bvr_mesh_array_attrib_e {
    /*
        vertices    -> vec2
    */
    BVR_MESH_ATTRIB_V2,
    
    /*
        vertices    -> vec3
    */
    BVR_MESH_ATTRIB_V3,

    /*
        vertices    -> vec2
        uvs         -> vec2
    */
    BVR_MESH_ATTRIB_V2UV2,
    
    /*
        vertices    -> vec3
        uvs         -> vec2
    */
    BVR_MESH_ATTRIB_V3UV2
} bvr_mesh_array_attrib_t;

typedef struct bvr_mesh_s {
    uint32_t array_buffer;
    uint32_t vertex_buffer;
    uint32_t element_buffer;

    int element_type;
    bvr_mesh_array_attrib_t attrib;
    uint32_t element_count, vertex_count;
    uint32_t attrib_count, stride;
} bvr_mesh_t;

int bvr_create_mesh(bvr_mesh_t* mesh, bvr_mesh_buffer_t* vertices, bvr_mesh_buffer_t* elements, bvr_mesh_array_attrib_t attrib);
void bvr_mesh_draw(bvr_mesh_t* mesh, int drawmode);
void bvr_destroy_mesh(bvr_mesh_t* mesh);

#ifdef BVR_GEOMETRY_IMPLEMENTATION

static inline void bvr_create_2d_square_mesh(bvr_mesh_t* mesh, float width, float height){
    float vertices[16] = {
        -width,  height, 0, 1,
        -width, -height, 0, 0,
         width, -height, 1, 0,
         width,  height, 1, 1
    };

    uint32_t indices[] = {0, 1, 2, 0, 2, 3};

    bvr_mesh_buffer_t vertices_buffer;
    vertices_buffer.data = (char*) vertices;
    vertices_buffer.elemsize = BVR_FLOAT;
    vertices_buffer.size = sizeof(vertices);

    bvr_mesh_buffer_t element_buffer;
    element_buffer.data = (char*) indices;
    element_buffer.elemsize = BVR_UNSIGNED_INT32;
    element_buffer.size = sizeof(indices);

    bvr_create_mesh(mesh, &vertices_buffer, &element_buffer, BVR_MESH_ATTRIB_V2UV2);
}


static inline void bvr_create_3d_square_mesh(bvr_mesh_t* mesh, float width, float height){
    float vertices[] = {
        -width, 0,  height, 0, 1,
        -width, 0, -height, 0, 0,
         width, 0, -height, 1, 0,
         width, 0,  height, 1, 1
    };

    uint32_t indices[] = {0, 1, 2, 0, 2, 3};

    bvr_mesh_buffer_t vertices_buffer;
    vertices_buffer.data = (char*) vertices;
    vertices_buffer.elemsize = BVR_FLOAT;
    vertices_buffer.size = sizeof(vertices);

    bvr_mesh_buffer_t element_buffer;
    element_buffer.data = (char*) indices;
    element_buffer.elemsize = BVR_UNSIGNED_INT32;
    element_buffer.size = sizeof(indices);

    bvr_create_mesh(mesh, &vertices_buffer, &element_buffer, BVR_MESH_ATTRIB_V3UV2);
}

#endif