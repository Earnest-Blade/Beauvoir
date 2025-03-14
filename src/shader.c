#include <BVR/shader.h>
#include <BVR/math.h>
#include <BVR/file.h>

#include <string.h>
#include <memory.h>
#include <malloc.h>

#include <GLAD/glad.h>

#define BVR_MAX_GLSL_HEADER_SIZE 100

struct bvri_texture_uniform_s {
    int id;
    int layer; 

    int layer_location;
    int unit;
};

static int bvri_compile_shader(uint32_t* shader, bvr_string_t* content, int type){
    *shader = glCreateShader(type);

    glShaderSource(*shader, 1, (const char**)&content->data, NULL);
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

static void bvri_enable_uniforms(bvr_shader_t* shader){
    // start by 1 because we skip transformation uniform
    for (size_t uniform = 1; uniform < shader->uniform_count; uniform++)
    {
        bvr_shader_use_uniform(&shader->uniforms[uniform], NULL);
    }
}

static int bvri_register_shader_state(bvr_shader_t* program, bvr_shader_stage_t* shader, bvr_string_t* content, 
    const char* header, const char* name, int type){
    
    BVR_ASSERT(shader);
    BVR_ASSERT(content);
    BVR_ASSERT(header);
    BVR_ASSERT(name);

    char shader_header_str[BVR_SMALL_BUFFER_SIZE];
    memset(shader_header_str, 0, BVR_SMALL_BUFFER_SIZE);

    bvr_string_t shader_str;

    strncpy(shader_header_str, header, strnlen(header, 100));
    strncat(shader_header_str, "#define \0", 10);
    strncat(shader_header_str, name, strnlen(name, 100));
    strncat(shader_header_str, "\n", 1);

    bvr_create_string(&shader_str, shader_header_str);
    bvr_string_concat(&shader_str, content->data);

    if (type && shader_str.length) {
        if (bvri_compile_shader(&shader->shader, &shader_str, type)) {
            glAttachShader(program->program, shader->shader);
            shader->type = type;
        }
        else {
            BVR_PRINTF("failed to compile shader '%s'!", name);
        }
    }

    bvr_destroy_string(&shader_str);
}

int bvr_create_shaderf(bvr_shader_t* shader, FILE* file, int flags){
    BVR_ASSERT(shader);
    BVR_ASSERT(file);

    int version_offset = 0;
    char version_header_content[BVR_MAX_GLSL_HEADER_SIZE];
    bvr_string_t file_content;


    { // retrieve the end offset of the #version header
        fseek(file, 0, SEEK_SET);
        do
        {
            version_header_content[version_offset] = getc(file);
            version_offset++;

        } while(
            version_header_content[version_offset - 1] != EOF  &&
            version_header_content[version_offset - 1] != '\n' &&
            version_offset < BVR_MAX_GLSL_HEADER_SIZE - 1
        );
        version_header_content[version_offset] = '\0';
    }

    // read file
    bvr_create_string(&file_content, NULL);
    BVR_ASSERT(bvr_read_file(&file_content, file));

    // create shader's program
    shader->program = glCreateProgram();
    shader->flags = flags;
    shader->shader_count = 0;

    // by default there is:
    // - camera block
    //
    shader->block_count = 1;

    // by default there is
    // - transformation uniform
    //
    shader->uniform_count = 1;

    // check if it contains a vertex shader and create vertex shader stage.
    if (BVR_HAS_FLAG(flags, BVR_VERTEX_SHADER)) {
        bvri_register_shader_state(shader,
            &shader->shaders[shader->shader_count++], &file_content,
            version_header_content, "_VERTEX_", GL_VERTEX_SHADER
        );
    }
    else {
        BVR_PRINT("missing vertex shader!");
    }

    // check if it contains a fragment shader and create fragment shader stage.
    if (BVR_HAS_FLAG(flags, BVR_FRAGMENT_SHADER)) {
        bvri_register_shader_state(shader,
            &shader->shaders[shader->shader_count++], &file_content,
            version_header_content, "_FRAGMENT_", GL_FRAGMENT_SHADER
        );
    }
    else {
        BVR_PRINT("missing fragment shader!");
    }

    // try to compile shader
    if (!bvri_link_shader(shader->program)) {
        BVR_PRINT("failed to compile shader!");
    }

    // create default blocks
    
    // create camera block
    shader->blocks[0].type = BVR_MAT4;
    shader->blocks[0].count = 2;
    shader->blocks[0].location = glGetUniformBlockIndex(shader->program, BVR_UNIFORM_CAMERA_NAME);
    if (shader->blocks[0].location == -1) {
        BVR_PRINT("cannot find camera block uniform!");
    }
    else {
        glUniformBlockBinding(shader->program, shader->blocks[0].location, BVR_UNIFORM_BLOCK_CAMERA);
    }

    // create transform uniform
    shader->uniforms[0].location = glGetUniformLocation(shader->program, BVR_UNIFORM_TRANSFORM_NAME);
    shader->uniforms[0].memory.data = NULL;
    shader->uniforms[0].memory.size = 0;
    shader->uniforms[0].memory.elemsize = sizeof(bvr_mat4);
    shader->uniforms[0].name.data = NULL;
    shader->uniforms[0].name.length = 0;
    shader->uniforms[0].type = BVR_MAT4;
    if (shader->blocks[0].location == -1) {
        BVR_PRINT("cannot find transform uniform!");
    }
        
    bvr_destroy_string(&file_content);

    return BVR_OK;
}

bvr_shader_uniform_t* bvr_shader_register_uniform(bvr_shader_t* shader, int type, int count, const char* name){
    BVR_ASSERT(shader);
    BVR_ASSERT(name);

    if (shader->uniform_count + 1 >= BVR_MAX_UNIFORM_COUNT) {
        BVR_PRINTF("uniform maximum capacity reached for shader '%i'!", shader->program);
        return NULL;
    }

    int location = glGetUniformLocation(shader->program, name);
    if(location != -1){
        shader->uniforms[shader->uniform_count].location = location;
        shader->uniforms[shader->uniform_count].type = type;

        if(type == BVR_TEXTURE_2D || type == BVR_TEXTURE_2D_ARRAY){
            shader->uniforms[shader->uniform_count].memory.elemsize = sizeof(struct bvri_texture_uniform_s);
            shader->uniforms[shader->uniform_count].memory.size = sizeof(struct bvri_texture_uniform_s);
            shader->uniforms[shader->uniform_count].memory.data = malloc(shader->uniforms[shader->uniform_count].memory.size);
            BVR_ASSERT(shader->uniforms[shader->uniform_count].memory.data);
        }
        else{
            shader->uniforms[shader->uniform_count].memory.elemsize = bvr_sizeof(type);
            shader->uniforms[shader->uniform_count].memory.size = count * shader->uniforms[shader->uniform_count].memory.elemsize;
            shader->uniforms[shader->uniform_count].memory.data = calloc(shader->uniforms[shader->uniform_count].memory.size, 1);
            BVR_ASSERT(shader->uniforms[shader->uniform_count].memory.data);
        }
        
        bvr_create_string(&shader->uniforms[shader->uniform_count].name, name);

        shader->uniform_count++;
        return &shader->uniforms[shader->uniform_count - 1];
    }

    BVR_PRINTF("cannot find uniform '%s'!", name);
    return NULL;
}

bvr_shader_uniform_t* bvr_shader_register_texture(bvr_shader_t* shader, int type, int* id, int* layer, 
    const char* name, const char* layer_name)
{
    BVR_ASSERT(shader);
    if(type != BVR_TEXTURE_2D && type != BVR_TEXTURE_2D_ARRAY){
        BVR_PRINT("cannot link default uniform through texture registering!");
        return NULL;
    }

    bvr_shader_uniform_t* uniform = bvr_shader_register_uniform(shader, type, 0, name);
    if(uniform){
        struct bvri_texture_uniform_s texture_uniform;
        texture_uniform.id = 0;
        texture_uniform.layer = 0;
        texture_uniform.layer_location = 0;
        texture_uniform.unit = -1;

        if(id){
            texture_uniform.id = *id;
        }

        if(type == BVR_TEXTURE_2D_ARRAY){
            if(layer){
                texture_uniform.layer = *layer;
            }

            texture_uniform.layer_location = glGetUniformLocation(shader->program, layer_name);

            if(texture_uniform.layer_location == -1){
                BVR_PRINT("failed to find texture layer location!");
            }
        }

        for (size_t i = 0; i < shader->uniform_count; i++)
        {
            if(shader->uniforms[i].type == BVR_TEXTURE_2D || shader->uniforms[i].type == BVR_TEXTURE_2D_ARRAY){
                texture_uniform.unit++;
            }
        }
        
        memcpy(uniform->memory.data, &texture_uniform, sizeof(struct bvri_texture_uniform_s));

        return uniform;
    }
    else {
        BVR_PRINT("failed to create texture's uniform!");
    }

    return NULL;
}

void bvr_shader_set_uniformi(bvr_shader_uniform_t* uniform, void* data){

    if(data && uniform){
        memcpy(uniform->memory.data, data, uniform->memory.size);
    }
    else {
        BVR_PRINT("skipping uniform data updating, data is NULL");
    }
}

void bvr_shader_set_uniform(bvr_shader_t* shader, const char* name, void* data){
    BVR_ASSERT(shader);
    BVR_ASSERT(name);

    for (size_t i = 0; i < shader->uniform_count; i++)
    {
        if (strcmp(shader->uniforms[i].name.data, name) == 0) {
            bvr_shader_set_uniformi(&shader->uniforms[i], data);
        }
    }
}
void bvr_shader_set_texturei(bvr_shader_uniform_t* uniform, int* id, int* layer){
    if(uniform){
        struct bvri_texture_uniform_s texture;
        if(id){
            texture.id = *id;
        }

        if(layer){
            texture.layer = *layer;
        }

        memcpy(uniform->memory.data, &texture, sizeof(texture.id) + sizeof(texture.layer));
    }
}

void bvr_shader_set_texture(bvr_shader_t* shader, const char* name, int* id, int* layer){
    BVR_ASSERT(shader);
    BVR_ASSERT(name);

    for (size_t i = 0; i < shader->uniform_count; i++)
    {
        if (strcmp(shader->uniforms[i].name.data, name) == 0) {
            bvr_shader_set_texturei(&shader->uniforms[i], id, layer);
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

        case BVR_TEXTURE_2D:
            glUniform1i(uniform->location, ((struct bvri_texture_uniform_s*)uniform->memory.data)->unit);
            return;

        case BVR_TEXTURE_2D_ARRAY:
            {
                struct bvri_texture_uniform_s* texture = (struct bvri_texture_uniform_s*)uniform->memory.data;
                glUniform1i(uniform->location, texture->unit);
                glUniform1i(texture->layer_location, texture->layer);

                BVR_PRINTF("%i %i %i %i", uniform->location, texture->layer_location, texture->unit, texture->layer);
            }
            return;
        default:
            break;
        }
    }
}

void bvr_shader_enable(bvr_shader_t* shader){
    bvri_enable_uniforms(shader);
    
    glUseProgram(shader->program);
}

void bvr_shader_disable(void){
    glUseProgram(0);
}

void bvr_destroy_shader(bvr_shader_t* shader){
    BVR_ASSERT(shader);

    for (size_t shader_i = 0; shader_i < shader->shader_count; shader_i++)
    {
        glDeleteShader(shader->shaders[shader_i].shader);
    }

    for (size_t uniform = 0; uniform < shader->uniform_count; uniform++)
    {
        bvr_destroy_string(&shader->uniforms[uniform].name);

        free(shader->uniforms[uniform].memory.data);
        shader->uniforms[uniform].memory.data = NULL;
    }

    glDeleteProgram(shader->program);
}