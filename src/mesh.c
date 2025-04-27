#include <bvr/mesh.h>
#include <bvr/math.h>

#include <memory.h>
#include <malloc.h>
#include <ctype.h>

#include <GLAD/glad.h>

#ifndef BVR_NO_OBJ

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
    uint32_t e = 0;

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
    for (size_t i = 0; i < 5; i++)
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

static int bvri_load_obj(bvr_mesh_t* mesh, FILE* file){
    fseek(file, 0, SEEK_SET);

    struct {
        bvr_string_t name;
        bvr_string_t material_path;

        vec3 vertices[BVR_BUFFER_SIZE];
        vec3 normals[BVR_BUFFER_SIZE];
        vec2 uvs[BVR_BUFFER_SIZE];

        struct
        {
            int vertex[4];
            int normal[4];
            int uv[4];

            char length;
        } faces[BVR_BUFFER_SIZE];

        int vertex_count;
        int normal_count;
        int uv_count;
        int face_count;

        int overtex_count;
        int indices_count;
    } object;

    bvr_mesh_buffer_t vertices;
    bvr_mesh_buffer_t indices;
    
    object.vertex_count = 0;
    object.normal_count = 0;
    object.uv_count = 0;
    object.face_count = 0;
    object.vertex_count = 0;
    object.indices_count = 0;
    mesh->stride = 3;

    if(mesh->attrib >= BVR_MESH_ATTRIB_V2UV2){
        mesh->stride += 2;
    }
    if(mesh->attrib == BVR_MESH_ATTRIB_V3UV2N3){
        mesh->stride += 3;
    }

    bvr_create_string(&object.name, NULL);
    bvr_create_string(&object.material_path, NULL);

    char buffer[128];
    while (bvri_objreadline(buffer, file))
    {
        if(buffer[0] == '#'){
            // skip comments
            continue;
        }

        if(!strncmp(buffer, "mtllib ", 7)){
            bvr_create_string(&object.material_path, &buffer[7]);
            continue;
        }

        if(buffer[0] == 'o'){
            // reset object state
            bvr_create_string(&object.name, &buffer[2]);
            continue;
        }

        if(buffer[0] == 'v'){
            if(buffer[1] == 'n' && mesh->attrib == BVR_MESH_ATTRIB_V3UV2N3){
                BVR_ASSERT(object.normal_count < BVR_BUFFER_SIZE);

                // parse normal
                char* bufferp;
                bufferp = bvri_objparsefloat(&buffer[3], &object.normals[object.normal_count][0]);
                bufferp = bvri_objparsefloat(++bufferp, &object.normals[object.normal_count][1]);
                bufferp = bvri_objparsefloat(++bufferp, &object.normals[object.normal_count][2]);
                object.normal_count++;
            }

            if(buffer[1] == 't' && mesh->attrib >= BVR_MESH_ATTRIB_V2UV2){
                BVR_ASSERT(object.uv_count < BVR_BUFFER_SIZE);
                
                // parse uvs
                char* bufferp;
                bufferp = bvri_objparsefloat(&buffer[3], &object.uvs[object.uv_count][0]);
                bufferp = bvri_objparsefloat(++bufferp, &object.uvs[object.uv_count][1]);
                object.uv_count++;
            }

            if(buffer[1] == ' ') {
                BVR_ASSERT(object.vertex_count < BVR_BUFFER_SIZE);

                // parse vertex
                char* bufferp;
                bufferp = bvri_objparsefloat(&buffer[2], &object.vertices[object.vertex_count][0]);
                bufferp = bvri_objparsefloat(++bufferp, &object.vertices[object.vertex_count][1]);
                bufferp = bvri_objparsefloat(++bufferp, &object.vertices[object.vertex_count][2]);
                object.vertex_count++;

            }

            continue;
        }

        if(buffer[0] == 's'){
            continue;
        }

        if(buffer[0] == 'f'){
            // parse face
            object.faces[object.face_count].length = 0;

            char* bufferp = &buffer[1];
            while (*bufferp != '\0')
            {
                bufferp++; // skip whitespace
                bufferp = bvri_objparseint(
                    bufferp, 
                    &object.faces[object.face_count].vertex[object.faces[object.face_count].length]
                );

                bufferp++;
                bufferp = bvri_objparseint(
                    bufferp, 
                    &object.faces[object.face_count].uv[object.faces[object.face_count].length]
                );

                bufferp++;
                bufferp = bvri_objparseint(
                    bufferp,
                    &object.faces[object.face_count].normal[object.faces[object.face_count].length]
                );

                object.faces[object.face_count].length++;

                object.overtex_count += mesh->stride;

                if(object.faces[object.face_count].length == 4){
                    object.indices_count += 6;
                }
                else {
                    object.indices_count += 3;
                }
            }

            object.face_count++;
        }
    }

    vertices.type = BVR_FLOAT;
    vertices.count = object.overtex_count;
    vertices.data = malloc(vertices.count * sizeof(float));
    
    indices.type = BVR_UNSIGNED_INT32;
    indices.count = object.indices_count;
    indices.data = malloc(indices.count * sizeof(uint32_t));
    
    for (
        size_t face = 0, indice = 0, vertex = 0; 
        face < object.face_count; face++)
    {
        BVR_ASSERT(object.faces[face].length == 3 || object.faces[face].length == 4);

        // triangulate quads
        if(object.faces[face].length == 4){
            ((uint32_t*)indices.data)[indice + 0] = indice + 0;
            ((uint32_t*)indices.data)[indice + 1] = indice + 1;
            ((uint32_t*)indices.data)[indice + 2] = indice + 2;
            ((uint32_t*)indices.data)[indice + 3] = indice + 0;
            ((uint32_t*)indices.data)[indice + 4] = indice + 2;
            ((uint32_t*)indices.data)[indice + 5] = indice + 3;
            indice += 6;
        }
        else {
            ((uint32_t*)indices.data)[indice + 0] = indice + 0;
            ((uint32_t*)indices.data)[indice + 1] = indice + 1;
            ((uint32_t*)indices.data)[indice + 2] = indice + 2;
            indice += 3;
        }

        for (size_t i = 0; i < object.faces[face].length; i++)
        {
            ((float*)vertices.data)[vertex + 0] = object.vertices[object.faces[face].vertex[i] - 1][0];
            ((float*)vertices.data)[vertex + 1] = object.vertices[object.faces[face].vertex[i] - 1][1];
            ((float*)vertices.data)[vertex + 2] = object.vertices[object.faces[face].vertex[i] - 1][2];
        
            ((float*)vertices.data)[vertex + 3] = object.uvs[object.faces[face].uv[i] - 1][0];
            ((float*)vertices.data)[vertex + 4] = object.uvs[object.faces[face].uv[i] - 1][1];

            ((float*)vertices.data)[vertex + 5] = object.normals[object.faces[face].normal[i] - 1][0];
            ((float*)vertices.data)[vertex + 6] = object.normals[object.faces[face].normal[i] - 1][1];
            ((float*)vertices.data)[vertex + 7] = object.normals[object.faces[face].normal[i] - 1][2];

            vertex += mesh->stride;
        }
    }
    
    bvr_create_meshv(mesh, &vertices, &indices, mesh->attrib);

    bvr_destroy_string(&object.name);
    bvr_destroy_string(&object.material_path);

    free(vertices.data);
    free(indices.data);

    return BVR_OK;
}

#endif

int bvr_create_meshf(bvr_mesh_t* mesh, FILE* file, bvr_mesh_array_attrib_t attrib){
    BVR_ASSERT(mesh);
    BVR_ASSERT(file);

    int status = 0;

    mesh->array_buffer = 0;
    mesh->vertex_buffer = 0;
    mesh->element_buffer = 0;
    mesh->element_type = 0;
    mesh->element_count = 0;
    mesh->vertex_count = 0;
    mesh->attrib_count = 0;
    mesh->stride = 0;
    mesh->attrib = attrib;

#ifndef BVR_NO_OBJ
    if(bvri_is_obj(file)){
        status = bvri_load_obj(mesh, file);
    }
#endif

    if(!status){
        BVR_PRINT("failed to load model");
    }
}

int bvr_create_meshv(bvr_mesh_t* mesh, bvr_mesh_buffer_t* vertices, bvr_mesh_buffer_t* elements, bvr_mesh_array_attrib_t attrib){
    BVR_ASSERT(mesh);
    BVR_ASSERT(vertices);
    BVR_ASSERT(elements);

    memset(mesh, 0, sizeof(bvr_mesh_t));

    // creating vertex array
    glGenVertexArrays(1, &mesh->array_buffer);
    glBindVertexArray(mesh->array_buffer);

    // basic buffer bindings
    glGenBuffers(1, &mesh->vertex_buffer);
    glGenBuffers(1, &mesh->element_buffer);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, vertices->count * bvr_sizeof(vertices->type), vertices->data, GL_STATIC_DRAW); //WARNING: Change once debug is finished

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->element_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements->count * bvr_sizeof(elements->type), elements->data, GL_STATIC_DRAW);

    mesh->attrib = attrib;
    mesh->element_type = elements->type;
    mesh->vertex_count = vertices->count;
    mesh->element_count = elements->count;
    
    switch (attrib)
    {
    case BVR_MESH_ATTRIB_V2:
        {
            mesh->attrib_count = 1;
            mesh->stride = 2 * bvr_sizeof(vertices->type);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, vertices->type, GL_FALSE, mesh->stride, (void*)0);
        }
        break;
    
    case BVR_MESH_ATTRIB_V3:
        {
            mesh->attrib_count = 1;
            mesh->stride = 3 * bvr_sizeof(vertices->type);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, vertices->type, GL_FALSE, mesh->stride, (void*)0);
        }
        break;

    case BVR_MESH_ATTRIB_V2UV2:
        {
            mesh->attrib_count = 2;
            mesh->stride = (2 + 2) * bvr_sizeof(vertices->type);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, vertices->type, GL_FALSE, mesh->stride, (void*)0);
            
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, vertices->type, GL_FALSE, mesh->stride, (void*)(2 * bvr_sizeof(vertices->type)));
        }
        break;
    
    case BVR_MESH_ATTRIB_V3UV2:
        {
            mesh->attrib_count = 2;
            mesh->stride = (3 + 2) * bvr_sizeof(vertices->type);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, vertices->type, GL_FALSE, mesh->stride, (void*)0);
            
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, vertices->type, GL_FALSE, mesh->stride, (void*)(3 * bvr_sizeof(vertices->type)));
        }
        break;
    
    case BVR_MESH_ATTRIB_V3UV2N3:
        {
            mesh->attrib_count = 3;
            mesh->stride = (3 + 2 + 3) * bvr_sizeof(vertices->type);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, vertices->type, GL_FALSE, mesh->stride, (void*)0);
            
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, vertices->type, GL_FALSE, mesh->stride, (void*)(3 * bvr_sizeof(vertices->type)));

            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 3, vertices->type, GL_FALSE, mesh->stride, (void*)(5 * bvr_sizeof(vertices->type)));
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

    for (size_t i = 0; i < mesh->attrib_count; i++){ glDisableVertexAttribArray(i); }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return BVR_OK;
}


void bvr_mesh_draw(bvr_mesh_t* mesh, int drawmode){
    glBindVertexArray(mesh->array_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->element_buffer);

    for (size_t i = 0; i < mesh->attrib_count; i++){ 
        glEnableVertexAttribArray(i); 
    }

    glDrawElements(drawmode, mesh->element_count, mesh->element_type, NULL);

    for (size_t i = 0; i < mesh->attrib_count; i++){ 
        glDisableVertexAttribArray(i); 
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void bvr_destroy_mesh(bvr_mesh_t* mesh){
    BVR_ASSERT(mesh);

    glDeleteVertexArrays(1, &mesh->array_buffer);
    glDeleteBuffers(1, &mesh->vertex_buffer);
    glDeleteBuffers(1, &mesh->element_buffer);
}