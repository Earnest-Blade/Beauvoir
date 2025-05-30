#include <BVR/editor/renderer.h>

#include <BVR/config.h>
#include <BVR/utils.h>
#include <BVR/math.h> 

#include <GLAD/glad.h>

int bvri_create_editor_render_buffers(uint32_t* array_buffer, uint32_t* vertex_buffer, size_t vertex_size){
    BVR_ASSERT(vertex_buffer);

    glGenVertexArrays(1, array_buffer);
    glGenBuffers(1, vertex_buffer);

    glBindVertexArray(*array_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, *vertex_buffer);

    glBufferData(GL_ARRAY_BUFFER, vertex_size * sizeof(float), NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
    glDisableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return BVR_OK;
}

void bvri_bind_editor_buffers(uint32_t array_buffer, uint32_t vertex_buffer){
    glBindVertexArray(array_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
}

void bvri_set_editor_buffers(float* vertices, uint32_t vertices_count){
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices_count * sizeof(vec3), vertices);
}

void bvri_draw_editor_buffer(int drawmode, uint32_t element_offset, uint32_t element_count){
    glEnableVertexAttribArray(0);
    glDrawArrays(drawmode, element_offset, element_count);
    glDisableVertexAttribArray(0);
}

void bvri_destroy_editor_render_buffers(uint32_t* array_buffer, uint32_t* vertex_buffer){
    glDeleteVertexArrays(1, array_buffer);
    glDeleteBuffers(1, vertex_buffer);
}