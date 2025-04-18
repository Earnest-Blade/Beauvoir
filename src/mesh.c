#include <bvr/mesh.h>

#include <memory.h>
#include <malloc.h>

#include <GLAD/glad.h>

int bvr_create_mesh(bvr_mesh_t* mesh, bvr_mesh_buffer_t* vertices, bvr_mesh_buffer_t* elements, bvr_mesh_array_attrib_t attrib){
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
    glBufferData(GL_ARRAY_BUFFER, vertices->size, vertices->data, GL_STATIC_DRAW); //WARNING: Change once debug is finished

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->element_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements->size, elements->data, GL_STATIC_DRAW);

    mesh->attrib = attrib;
    mesh->element_type = elements->elemsize;
    mesh->vertex_count = vertices->size / bvr_sizeof(vertices->elemsize);
    mesh->element_count = elements->size / bvr_sizeof(elements->elemsize);
    
    switch (attrib)
    {
    case BVR_MESH_ATTRIB_V2:
        {
            mesh->attrib_count = 1;
            mesh->stride = 2 * bvr_sizeof(vertices->elemsize);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, vertices->elemsize, GL_FALSE, mesh->stride, (void*)0L);
        }
        break;
    
    case BVR_MESH_ATTRIB_V3:
        {
            mesh->attrib_count = 1;
            mesh->stride = 3 * bvr_sizeof(vertices->elemsize);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, vertices->elemsize, GL_FALSE, mesh->stride, (void*)0L);
        }
        break;

    case BVR_MESH_ATTRIB_V2UV2:
        {
            mesh->attrib_count = 2;
            mesh->stride = (2 + 2) * bvr_sizeof(vertices->elemsize);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, vertices->elemsize, GL_FALSE, mesh->stride, (void*)0L);
            
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, vertices->elemsize, GL_FALSE, mesh->stride, (void*)(2 * bvr_sizeof(vertices->elemsize)));
        }
        break;
    
    case BVR_MESH_ATTRIB_V3UV2:
        {
            mesh->attrib_count = 2;
            mesh->stride = (3 + 2) * bvr_sizeof(vertices->elemsize);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, vertices->elemsize, GL_FALSE, mesh->stride, (void*)0L);
            
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, vertices->elemsize, GL_FALSE, mesh->stride, (void*)(3 * bvr_sizeof(vertices->elemsize)));
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