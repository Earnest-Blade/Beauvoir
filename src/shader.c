#include <bvr/shader.h>
#include <BVR/file.h>

#include <string.h>
#include <memory.h>
#include <malloc.h>

#include <GLAD/glad.h>

static int bvri_compile_shader(uint32_t* shader, bvr_string_t* values, int type){
    *shader = glCreateShader(type);

    const char* string_ptr = bvr_string_get(values);
    glShaderSource(*shader, 1, &string_ptr, NULL);
    glCompileShader(*shader);

    int s;
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &s);
    if(!s){
        char buffer[BVR_BUFFER_SIZE];
        glGetShaderInfoLog(*shader, BVR_BUFFER_SIZE, NULL, buffer);
        BVR_PRINT(buffer);

        return BVR_FAILED;
    }

    return BVR_OK;
}

static int bvri_link_shader(uint32_t program) {
    glLinkProgram(program);

    int s;
    glGetProgramiv(program, GL_LINK_STATUS, &s);
    if(!s){
        char buffer[BVR_BUFFER_SIZE];
        glGetProgramInfoLog(program, BVR_BUFFER_SIZE, NULL, buffer);
        BVR_PRINT(buffer);

        return BVR_FAILED;
    }

    return BVR_OK;
}

int bvr_create_shaderf(bvr_shader_t* shader, FILE* file){
    BVR_ASSERT(shader);
    BVR_ASSERT(file);

    shader->program = glCreateProgram();
    shader->shaders.elemsize = sizeof(uint32_t);
    shader->uniforms.elemsize = sizeof(bvr_shader_uniform_t);
    shader->shaders.size = 0;
    shader->uniforms.size = 0;
    shader->shaders.data = NULL;
    shader->uniforms.data = NULL;
    shader->transform.type = BVR_MAT4;
    shader->transform.name = NULL;
    shader->transform.memory.data = NULL;
    shader->transform.memory.size = sizeof(4 * 4 * sizeof(float));
    shader->transform.memory.elemsize = shader->transform.memory.size;
    
    bvr_create_string(&shader->file_string, NULL);
    BVR_ASSERT(bvr_read_file(&shader->file_string, file));

    // get header size
    fseek(file, 0, SEEK_SET);
    int c = 0;
    do
    {
        c = getc(file);
        shader->header_length++;
    } while (c != EOF && c != '\n');
    
    fseek(file, 0, SEEK_SET);
    
    return BVR_OK;
}

void bvr_shader_register_sub_shader(bvr_shader_t* shader, int type, const char* name){
    BVR_ASSERT(shader);
    BVR_ASSERT(name);

    bvr_string_t shader_str, shader_header;
    bvr_string_create_and_copy(&shader_str, &shader->file_string);
    bvr_create_string(&shader_header, "#define ");
    bvr_string_concat(&shader_header, name);

    bvr_string_insert(&shader_str, shader->header_length, shader_header.data);

    uint32_t target;
    if(bvri_compile_shader(&target, &shader_str, type)) {
        glAttachShader(shader->program, target);

        shader->shaders.size += shader->shaders.elemsize;
        shader->shaders.data = realloc(shader->shaders.data, shader->shaders.size);
        shader->shaders.data[shader->shaders.size - shader->shaders.elemsize] = target;
    }
    else {
        BVR_PRINT("failed to compile shader!");
    }

    bvr_destroy_string(&shader_str);
    bvr_destroy_string(&shader_header);
}

void bvr_shader_register_uniform(bvr_shader_t* shader, int type, int count, const char* name){
    BVR_ASSERT(shader);
    BVR_ASSERT(name);

    int location = glGetUniformLocation(shader->program, name);
    if(location != -1){
        shader->uniforms.size += shader->uniforms.elemsize;
        shader->uniforms.data = realloc(shader->uniforms.data, shader->uniforms.size);
        BVR_ASSERT(shader->uniforms.data);
        
        bvr_shader_uniform_t* uniform = (bvr_shader_uniform_t*)(shader->uniforms.data + shader->uniforms.size - shader->uniforms.elemsize);
        BVR_ASSERT(uniform);

        uniform->location = location;
        uniform->type = type;
        uniform->name = malloc(strlen(name) + 1);
        strncpy((char*) uniform->name, name, strlen(name));

        uniform->memory.elemsize = bvr_sizeof(type);
        uniform->memory.size = count * uniform->memory.elemsize;
        uniform->memory.data = malloc(uniform->memory.size);
        memset(uniform->memory.data, 0, uniform->memory.size);
    }
    else {
        BVR_PRINTF("cannot find uniform '%s'!", name);
    }
}

void bvr_shader_set_uniformi(bvr_shader_t* shader, const int id, void* data){
    BVR_ASSERT(shader);

    if(data){
        if(id >= 0 && id <= shader->uniforms.size / shader->uniforms.elemsize) {
            bvr_shader_uniform_t* uniform = (bvr_shader_uniform_t*)shader->uniforms.data + id * shader->uniforms.elemsize;
            BVR_PRINTF("update shader %s buffer size %i", uniform->name, uniform->memory.size);

            memcpy(uniform->memory.data, data, uniform->memory.size);
            
            float* idata = (float*)data;
            for (size_t i = 0; i < 16; i++)
            {
                BVR_PRINTF("mat%i : %f", i, (float)*(uniform->memory.data + i * uniform->memory.elemsize));
            }
        }
    }
    else {
        BVR_PRINT("skipping uniform data updating, data is NULL");
    }
}

void bvr_shader_set_uniform(bvr_shader_t* shader, const char* name, void* data){
    BVR_ASSERT(shader);
    BVR_ASSERT(name);

    bvr_shader_uniform_t* uniform;
    for (size_t i = 0; i < shader->uniforms.size / shader->uniforms.elemsize; i++)
    {
        uniform = (bvr_shader_uniform_t*)shader->uniforms.data + i * shader->uniforms.elemsize;
        if(strcmp(uniform->name, name) == 0){
            bvr_shader_set_uniformi(shader, i, data);
            return;
        }
    }
}

void bvr_shader_use_uniform(bvr_shader_uniform_t* uniform, void* data){
    BVR_ASSERT(uniform);

    if(uniform->location == -1) {
        return;
    }

    if(!data){
        data = uniform->memory.data;
    }

    if(data){
        switch (uniform->type)
        {
        case BVR_FLOAT: 
            glUniform1fv(uniform->location, uniform->memory.size / uniform->memory.elemsize, (float*)data); 
            return;

        case BVR_INT32: 
            glUniform1iv(uniform->location, uniform->memory.size / uniform->memory.elemsize, (int*)data); 
            return;

        case BVR_MAT4: 
            glUniformMatrix4fv(uniform->location, uniform->memory.size / uniform->memory.elemsize, GL_FALSE, (float*)data); 
            return;

        default:
            break;
        }
    }
}

void bvr_shader_compile(bvr_shader_t* shader){
    BVR_ASSERT(shader);

    if(!bvri_link_shader(shader->program)){
        BVR_PRINT("failed to compile shader");
    }

    bvr_destroy_string(&shader->file_string);
    shader->header_length = 0L;

    shader->camera.type = BVR_MAT4;
    shader->camera.count = 2;
    shader->camera.location = glGetUniformBlockIndex(shader->program, BVR_UNIFORM_CAMERA);
    if(shader->camera.location == -1){
        BVR_PRINT("failed to find camera block uniform!");
    }
    else {
        glUniformBlockBinding(shader->program, shader->camera.location, BVR_UNIFORM_BLOCK_CAMERA);
    }

    shader->transform.location = glGetUniformLocation(shader->program, BVR_UNIFORM_TRANSFORM);
    if(shader->transform.location == -1){
        BVR_PRINT("failed to find transform uniform!");
    }
}

void bvr_shader_enable(bvr_shader_t* shader){
    if(shader->uniforms.data){

        bvr_shader_uniform_t* uniform;
        for (size_t i = 0; i < shader->uniforms.size / shader->uniforms.elemsize; i++)
        {
            uniform = (bvr_shader_uniform_t*)&shader->uniforms.data[i * shader->uniforms.elemsize];
            bvr_shader_use_uniform(uniform, NULL);
        }
    }
    
    glUseProgram(shader->program);
}

void bvr_shader_disable(void){
    glUseProgram(0);
}

void bvr_destroy_shader(bvr_shader_t* shader){
    BVR_ASSERT(shader);

    for (size_t shaderid = 0; shaderid < shader->shaders.size / shader->shaders.elemsize; shaderid++)
    {
        glDetachShader((uint32_t)shader->program, shader->shaders.data[shaderid * shader->shaders.elemsize]);
        glDeleteShader((uint32_t)shader->shaders.data[shaderid * shader->shaders.elemsize]);
    }

    bvr_shader_uniform_t* uniform;
    for (size_t uniformid = 0; uniformid < shader->uniforms.size / shader->uniforms.elemsize; uniformid++)
    {
        uniform = (bvr_shader_uniform_t*)&shader->uniforms.data[uniformid * shader->uniforms.elemsize];
        free(uniform->name);
        free(uniform->memory.data);
    }
    
    glDeleteProgram(shader->program);

    free(shader->shaders.data);
    free(shader->uniforms.data);
    shader->shaders.data = NULL;
    shader->uniforms.data = NULL;
}