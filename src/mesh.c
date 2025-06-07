#include <bvr/mesh.h>
#include <bvr/math.h>

#include <malloc.h>
#include <string.h>
#include <memory.h>

#include <GLAD/glad.h>

static int bvri_create_mesh_buffers(bvr_mesh_t* mesh, uint64 vertices_size, uint64 element_size, int vertex_type, int element_type, bvr_mesh_array_attrib_t attrib);

#ifndef BVR_NO_OBJ

#include <ctype.h>

static int bvri_objreadline(char* buffer, FILE* file){
    int c;
    while (c = getc(file), c != EOF && c != '\n') {
        *buffer++ = c;
    }

    *buffer = '\0';

    return c != EOF;
}

static char* bvri_objparseint(char* buffer, int* v){
    int sig = + 1;
    int num = 0;

    if(*buffer == '-'){
        sig = -1;
        buffer++;
    }

    while (isdigit(*buffer))
    {
        num = 10 * num + (*buffer++ - '0');
    }
    
    *v = sig * num;

    return buffer;
}

static char* bvri_objparsefloat(char* buffer, float* v){
    double sig = +1, p = +1;
    double num = 0.0;
    double fra = 0.0;
    double div = 1.0;
    uint32 e = 0;

    if(*buffer == '-'){
        sig = -1;
        buffer++;
    }
    else if(*buffer == '+'){
        sig = +1;
        buffer++;
    }

    while (isdigit(*buffer))
    {
        num = 10 * num + (double)(*buffer++ - '0');
    }
    
    if(*buffer == '.'){
        buffer++;
    }

    while (isdigit(*buffer))
    {
        fra = 10.0 * fra + (double)(*buffer++ - '0');
        div *= 10.0;
    }

    num += fra / div;
    
    BVR_ASSERT(*buffer == 'E' || *buffer == 'e' || "powers aren't supported :(");

    *v = (float)sig * num;

    return buffer;
}

static int bvri_is_obj(FILE* file){
    fseek(file, 0, SEEK_SET);

    char sig[32];

    // check 5 first lines
    for (uint64 i = 0; i < 5; i++)
    {
        bvri_objreadline(sig, file);
        if(sig[0] == '#'){
            continue;
        }

        if(!strncmp(sig, "mtllib ", 7)){
            return BVR_OK;
        }

        if(sig[0] == 'o' && sig[1] == ' '){
            return BVR_OK;
        }
    }
    
    return BVR_FAILED;
}

/*
    Push back a new vertex group as the targetted group
*/
static bvr_vertex_group_t* bvri_objpushgrp(struct bvr_buffer_s* group, bvr_string_t* name){
    bvr_vertex_group_t* vertex_group;

    if(group->data){
        group->data = realloc(group->data, group->size + group->elemsize);
        BVR_ASSERT(group->data);

        vertex_group = (bvr_vertex_group_t*)group->data + group->size;
        group->size += group->elemsize;
    }
    else {
        group->data = malloc(group->elemsize);
        group->size += group->elemsize;

        vertex_group = (bvr_vertex_group_t*)group->data;
    }

    bvr_string_create_and_copy(&vertex_group->name, name);
    vertex_group->element_count = 0;
    return vertex_group;
}

struct bvri_objobject_s {
    bvr_string_t name;
    bvr_string_t material;

    vec3 vertex[BVR_BUFFER_SIZE];
    vec2 uvs[BVR_BUFFER_SIZE];
    vec3 normals[BVR_BUFFER_SIZE];
    struct {
        uint32 vertex[4];
        uint32 uv[4];
        uint32 normal[4];

        uint8 edges;
    } faces[BVR_BUFFER_SIZE];

    bvr_mesh_buffer_t vertices;
    bvr_mesh_buffer_t elements;

    struct bvr_buffer_s vertex_group;
    bvr_vertex_group_t* group;

    uint32 vertex_count;
    uint32 uv_count;
    uint32 normal_count;
    uint32 face_count;
};

static int bvri_load_obj(bvr_mesh_t* mesh, FILE* file){
    BVR_ASSERT(mesh);

    struct bvri_objobject_s object;

    object.vertex_count = 0;
    object.uv_count = 0;
    object.normal_count = 0;
    object.face_count = 0;
    object.group = NULL;

    object.vertex_group.data = NULL;
    object.vertex_group.elemsize = sizeof(bvr_vertex_group_t);
    object.vertex_group.size = 0;

    object.elements.data = NULL;
    object.vertices.data = NULL;
    object.elements.count = 0;
    object.vertices.count = 0;
    object.elements.type = BVR_UNSIGNED_INT32;
    object.vertices.type = BVR_FLOAT;

    mesh->stride = 3;
    if(mesh->attrib >= BVR_MESH_ATTRIB_V2UV2) mesh->stride += 2;
    if(mesh->attrib == BVR_MESH_ATTRIB_V3UV2N3) mesh->stride += 3;

    bvr_create_string(&object.name, NULL);
    bvr_create_string(&object.material, NULL);

    char* cursor;
    char buffer[256];
    while(bvri_objreadline(buffer, file)){
        switch (buffer[0])
        {
        case '#':
            /* skip */
            break;
        
        case 'm':
            bvr_create_string(&object.material, &buffer[7]);
            break;    

        case 'o':
            bvr_create_string(&object.name, &buffer[2]);
            object.group = bvri_objpushgrp(&object.vertex_group, &object.name);
            object.group->element_offset = object.elements.count;
            break;

        case 'v':
            {
                cursor = NULL;

                if(buffer[1] == 'n' && mesh->stride >= 8){
                    BVR_ASSERT(object.normal_count < BVR_BUFFER_SIZE);

                    cursor = bvri_objparsefloat(&buffer[3], &object.normals[object.normal_count][0]);
                    cursor = bvri_objparsefloat(++cursor, &object.normals[object.normal_count][1]);
                    cursor = bvri_objparsefloat(++cursor, &object.normals[object.normal_count][2]);
                    object.normal_count++;
                }
                else if(buffer[1] == 't' && mesh->stride >= 5){
                    BVR_ASSERT(object.uv_count < BVR_BUFFER_SIZE);
                    
                    cursor = bvri_objparsefloat(&buffer[3], &object.uvs[object.uv_count][0]);
                    cursor = bvri_objparsefloat(++cursor, &object.uvs[object.uv_count][1]);
                    object.uv_count++;
                }
                else if(buffer[1] == ' '){
                    BVR_ASSERT(object.vertex_count < BVR_BUFFER_SIZE);

                    cursor = bvri_objparsefloat(&buffer[2], &object.vertex[object.vertex_count][0]);
                    cursor = bvri_objparsefloat(++cursor, &object.vertex[object.vertex_count][1]);
                    cursor = bvri_objparsefloat(++cursor, &object.vertex[object.vertex_count][2]);
                    object.vertex_count++;
                }
            }   
            break;
        
        case 'f': 
            {
                BVR_ASSERT(object.face_count < BVR_BUFFER_SIZE);

                cursor = &buffer[1];
                object.faces[object.face_count].edges = 0;
                
                while (*cursor != '\0')
                {
                    cursor = bvri_objparseint(++cursor, &object.faces[object.face_count].vertex[object.faces[object.face_count].edges]);    
                    cursor = bvri_objparseint(++cursor, &object.faces[object.face_count].uv[object.faces[object.face_count].edges]);    
                    cursor = bvri_objparseint(++cursor, &object.faces[object.face_count].normal[object.faces[object.face_count].edges]);    

                    object.faces[object.face_count].edges++;
                    object.vertices.count += mesh->stride;
                }
                
                if(object.faces[object.face_count].edges == 4){
                    object.elements.count += 6;
                    object.group->element_count += 6;
                }
                else {
                    object.elements.count += 3;
                    object.group->element_count += 3;
                }

                object.face_count++;
            }
            break;

        default:
            break;
        }
    }

    if(!object.elements.count || !object.vertices.count){
        BVR_PRINT("failed to load mesh :(");
        goto bvr_objfailed;
    }

    if(bvri_create_mesh_buffers(mesh, 
        object.vertices.count * sizeof(float), 
        object.elements.count * sizeof(uint32),
        object.vertices.type, object.elements.type, mesh->attrib) == BVR_FAILED){

        BVR_PRINT("failed to create object buffers");
        goto bvr_objfailed;
    }

    mesh->vertex_groups.size = object.vertex_group.size;
    mesh->vertex_groups.data = object.vertex_group.data;

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->element_buffer);

    uint64 vertex = 0, element = 0;
    for (uint64 face = 0; face < object.face_count; face++)
    {
        BVR_ASSERT(object.faces[face].edges == 3 || object.faces[face].edges == 4);

        if(object.faces[face].edges == 4){
            BVR_ASSERT(0 || "not supported");
        }
        else {
            BVR_ASSERT(element + 3 <= object.elements.count);

            const uint32 elements_values[3] = {element, element + 1, element + 2};
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, element * sizeof(uint32), sizeof(elements_values), elements_values);
            element += 3;
        }

        for (uint64 i = 0; i < object.faces[face].edges; i++)
        {
            BVR_ASSERT(vertex + 8 <= object.vertices.count);

            glBufferSubData(GL_ARRAY_BUFFER, (vertex + 0) * sizeof(float), sizeof(vec3), object.vertex[object.faces[face].vertex[i] - 1]);
            glBufferSubData(GL_ARRAY_BUFFER, (vertex + 3) * sizeof(float), sizeof(vec2), object.uvs[object.faces[face].uv[i] - 1]);
            glBufferSubData(GL_ARRAY_BUFFER, (vertex + 5) * sizeof(float), sizeof(vec3), object.normals[object.faces[face].normal[i] - 1]);
            vertex += 8;
        }
        
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    bvr_destroy_string(&object.name);
    bvr_destroy_string(&object.material);

    free(object.vertices.data);
    free(object.elements.data);

    return BVR_OK;

bvr_objfailed:
    bvr_destroy_string(&object.name);
    bvr_destroy_string(&object.material);

    for (uint64 i = 0; i < BVR_BUFFER_COUNT(object.vertex_group); i++)
    {
        bvr_destroy_string(&((bvr_vertex_group_t*)object.vertex_group.data)[0].name);
    }
    
    free(object.vertex_group.data);
    free(object.vertices.data);
    free(object.elements.data);

    return BVR_FAILED;
}

#endif

int bvr_create_meshf(bvr_mesh_t* mesh, FILE* file, bvr_mesh_array_attrib_t attrib){
    BVR_ASSERT(mesh);
    BVR_ASSERT(file);

    int status = 0;

    mesh->array_buffer = 0;
    mesh->vertex_buffer = 0;
    mesh->element_buffer = 0;
    mesh->vertex_count = 0;
    mesh->element_count = 0;
    mesh->element_type = 0;
    mesh->attrib_count = 0;
    mesh->stride = 0;
    mesh->attrib = attrib;

    mesh->vertex_groups.size = 0;
    mesh->vertex_groups.elemsize = sizeof(bvr_vertex_group_t);
    mesh->vertex_groups.data = NULL;

#ifndef BVR_NO_OBJ
    if(bvri_is_obj(file)){
        status = bvri_load_obj(mesh, file);
    }
#endif

    if(!status){
        BVR_PRINT("failed to load model");
    }

    return status;
}

int bvr_create_meshv(bvr_mesh_t* mesh, bvr_mesh_buffer_t* vertices, bvr_mesh_buffer_t* elements, bvr_mesh_array_attrib_t attrib){
    BVR_ASSERT(mesh);
    BVR_ASSERT(vertices);
    BVR_ASSERT(elements);

    int status = 0;

    mesh->array_buffer = 0;
    mesh->vertex_buffer = 0;
    mesh->element_buffer = 0;
    mesh->vertex_count = 0;
    mesh->element_count = 0;
    mesh->element_type = 0;
    mesh->attrib_count = 0;
    mesh->stride = 0;
    mesh->attrib = attrib;

    mesh->vertex_groups.size = 0;
    mesh->vertex_groups.elemsize = sizeof(bvr_vertex_group_t);
    mesh->vertex_groups.data = NULL;

    status = bvri_create_mesh_buffers(mesh, 
        vertices->count * bvr_sizeof(vertices->type),
        elements->count * bvr_sizeof(elements->type),
        vertices->type, elements->type, attrib
    );

    // if cannot create buffers
    if(!status){
        return BVR_FAILED;
    }

    // allocate a single vertex group 
    mesh->vertex_groups.size = sizeof(bvr_vertex_group_t);
    mesh->vertex_groups.data = malloc(mesh->vertex_groups.size);
    BVR_ASSERT(mesh->vertex_groups.data);

    ((bvr_vertex_group_t*)mesh->vertex_groups.data)[0].name.length = 0;
    ((bvr_vertex_group_t*)mesh->vertex_groups.data)[0].name.string = NULL;
    ((bvr_vertex_group_t*)mesh->vertex_groups.data)[0].element_offset = 0;
    ((bvr_vertex_group_t*)mesh->vertex_groups.data)[0].element_count = elements->count;

    // copy vertex values over buffers
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->element_buffer);

    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices->count * bvr_sizeof(vertices->type), vertices->data);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, elements->count * bvr_sizeof(elements->type), elements->data);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return BVR_OK;
}

/*
    Generic buffer creation function
*/
static int bvri_create_mesh_buffers(bvr_mesh_t* mesh, uint64 vertices_size, uint64 element_size, 
    int vertex_type, int element_type, bvr_mesh_array_attrib_t attrib){

    BVR_ASSERT(mesh);
    BVR_ASSERT(vertices_size && element_size);

    // create vertex array 
    glGenVertexArrays(1, &mesh->array_buffer);
    glBindVertexArray(mesh->array_buffer);

    // create vertex and element buffers
    glGenBuffers(1, &mesh->vertex_buffer);
    glGenBuffers(1, &mesh->element_buffer);

    // allocate the whole buffers
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, vertices_size, NULL, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->element_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, element_size, NULL, GL_STATIC_DRAW);

    mesh->attrib = attrib;
    mesh->vertex_count = vertices_size / bvr_sizeof(vertex_type);
    mesh->element_count = element_size / bvr_sizeof(element_type);
    mesh->element_type = element_type;

    // define each attributes pointers depending on attribute's type
    switch (attrib)
    {
    case BVR_MESH_ATTRIB_V2:
        {
            mesh->attrib_count = 1;
            mesh->stride = 2 * bvr_sizeof(vertex_type);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, vertex_type, GL_FALSE, mesh->stride, (void*)0);
        }
        break;
    
    case BVR_MESH_ATTRIB_V3:
        {
            mesh->attrib_count = 1;
            mesh->stride = 3 * bvr_sizeof(vertex_type);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, vertex_type, GL_FALSE, mesh->stride, (void*)0);
        }
        break;

    case BVR_MESH_ATTRIB_V2UV2:
        {
            mesh->attrib_count = 2;
            mesh->stride = (2 + 2) * bvr_sizeof(vertex_type);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, vertex_type, GL_FALSE, mesh->stride, (void*)0);
            
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, vertex_type, GL_FALSE, mesh->stride, (void*)(2 * bvr_sizeof(vertex_type)));
        }
        break;
    
    case BVR_MESH_ATTRIB_V3UV2:
        {
            mesh->attrib_count = 2;
            mesh->stride = (3 + 2) * bvr_sizeof(vertex_type);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, vertex_type, GL_FALSE, mesh->stride, (void*)0);
            
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, vertex_type, GL_FALSE, mesh->stride, (void*)(3 * bvr_sizeof(vertex_type)));
        }
        break;
    
    case BVR_MESH_ATTRIB_V3UV2N3:
        {
            mesh->attrib_count = 3;
            mesh->stride = (3 + 2 + 3) * bvr_sizeof(vertex_type);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, vertex_type, GL_FALSE, mesh->stride, (void*)0);
            
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, vertex_type, GL_FALSE, mesh->stride, (void*)(3 * bvr_sizeof(vertex_type)));

            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 3, vertex_type, GL_FALSE, mesh->stride, (void*)(5 * bvr_sizeof(vertex_type)));
        }
        break;

    default:
        {
            BVR_PRINT("cannot recognize attribute type!");
            bvr_destroy_mesh(mesh);
        }
        return BVR_FAILED;
    }

    if(!mesh->stride){
        BVR_PRINT("cannot get vertex type size!");
        bvr_destroy_mesh(mesh);
        return BVR_FAILED;
    }

    for (uint64 i = 0; i < mesh->attrib_count; i++){ 
        glDisableVertexAttribArray(i); 
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return BVR_OK;
}

/*void bvr_mesh_draw(bvr_mesh_t* mesh, int drawmode){
    glBindVertexArray(mesh->array_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->element_buffer);

    for (size_t i = 0; i < mesh->attrib_count; i++){ 
        glEnableVertexAttribArray(i); 
    }

    // draw each vertex group
    for (size_t i = 0; i < BVR_BUFFER_COUNT(mesh->vertex_groups); i++)
    {
        glDrawElements(drawmode, 
            ((bvr_vertex_group_t*)mesh->vertex_groups.data)[i].element_count, 
            mesh->element_type, 
            (void*)((bvr_vertex_group_t*)mesh->vertex_groups.data)[i].element_offset
        );
    }
    
    for (size_t i = 0; i < mesh->attrib_count; i++){ 
        glDisableVertexAttribArray(i); 
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}*/

void bvr_destroy_mesh(bvr_mesh_t* mesh){
    BVR_ASSERT(mesh);

    for (uint64 i = 0; i < BVR_BUFFER_COUNT(mesh->vertex_groups); i++)
    {
        bvr_destroy_string(&((bvr_vertex_group_t*)mesh->vertex_groups.data)[i].name);
    }
    free(mesh->vertex_groups.data);

    glDeleteVertexArrays(1, &mesh->array_buffer);
    glDeleteBuffers(1, &mesh->vertex_buffer);
    glDeleteBuffers(1, &mesh->element_buffer);
}