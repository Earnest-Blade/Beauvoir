#include <BVR/graphics.h>

#include <GLAD/glad.h>

#include <BVR/math.h>
#include <BVR/utils.h>

#include <memory.h>
#include <malloc.h>

void bvr_pipeline_state_enable(struct bvr_pipeline_state_s* state){
    BVR_ASSERT(state);

    // blending
    if(BVR_HAS_FLAG(state->blending, BVR_BLEND_ENABLE)){
        glEnable(GL_BLEND);

        if(BVR_HAS_FLAG(state->blending, BVR_BLEND_FUNC_ALPHA_ONE_MINUS)){
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        else if(BVR_HAS_FLAG(state->blending, BVR_BLEND_FUNC_ALPHA_ADD)){
            glBlendFunc(GL_ONE, GL_ONE);
        }
        else if(BVR_HAS_FLAG(state->blending, BVR_BLEND_FUNC_ALPHA_MULT)){
            glBlendFunc(GL_ONE, GL_SRC_COLOR); // ????
        }
    }
    else {
        glDisable(GL_BLEND);
    }

    // depth testing
    if(BVR_HAS_FLAG(state->depth, BVR_DEPTH_TEST_ENABLE)){
        glEnable(GL_DEPTH_TEST);
        
        if(BVR_HAS_FLAG(state->depth, BVR_DEPTH_FUNC_NEVER)){
            glDepthFunc(GL_NEVER);
        }
        else if(BVR_HAS_FLAG(state->depth, BVR_DEPTH_FUNC_ALWAYS)){
            glDepthFunc(GL_ALWAYS);
        }
        else if(BVR_HAS_FLAG(state->depth, BVR_DEPTH_FUNC_LESS)){
            glDepthFunc(GL_LESS);
        }
        else if(BVR_HAS_FLAG(state->depth, BVR_DEPTH_FUNC_GREATER)){
            glDepthFunc(GL_GREATER);
        }
        else if(BVR_HAS_FLAG(state->depth, BVR_DEPTH_FUNC_LEQUAL)){
            glDepthFunc(GL_LEQUAL);
        }
        else if(BVR_HAS_FLAG(state->depth, BVR_DEPTH_FUNC_GEQUAL)){
            glDepthFunc(GL_GEQUAL);
        }
        else if(BVR_HAS_FLAG(state->depth, BVR_DEPTH_FUNC_NOTEQUAL)){
            glDepthFunc(GL_NOTEQUAL);
        }
        else if(BVR_HAS_FLAG(state->depth, BVR_DEPTH_FUNC_EQUAL)){
            glDepthFunc(GL_EQUAL);
        }
    }
    else {
        glDisable(GL_DEPTH_TEST);
    }
}

int bvr_create_framebuffer(bvr_framebuffer_t* framebuffer, int width, int height, const char* shader){
    BVR_ASSERT(framebuffer);
    BVR_ASSERT(width > 0 && height > 0);
    BVR_ASSERT(shader);

    framebuffer->width = width;
    framebuffer->height = height;
    framebuffer->prev_width = width;
    framebuffer->prev_height = height;

    {
        const float quad[24] = {
            -1.0f,  1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f,  -1.0f, 1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f, 1.0f,
            1.0f,  -1.0f, 1.0f, 0.0f,
            1.0f,   1.0f, 1.0f, 1.0f,
        };

        glGenVertexArrays(1, &framebuffer->vertex_buffer);
        glBindVertexArray(framebuffer->vertex_buffer);

        glGenBuffers(1, &framebuffer->array_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, framebuffer->array_buffer);

        glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), &quad, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)8);
    }

    bvr_create_shader(&framebuffer->shader, shader, BVR_FRAMEBUFFER_SHADER);

    glGenFramebuffers(1, &framebuffer->buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->buffer);

    glGenTextures(1, &framebuffer->color_buffer);
    glBindTexture(GL_TEXTURE_2D, framebuffer->color_buffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer->color_buffer, 0);

    glGenRenderbuffers(1, &framebuffer->depth_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, framebuffer->depth_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, framebuffer->depth_buffer);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        BVR_PRINT("failed to create a new framebuffer!");
        return BVR_FAILED;
    }

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return BVR_OK;
}

void bvr_framebuffer_enable(bvr_framebuffer_t* framebuffer){
    int viewport[4];

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->buffer);
    glGetIntegerv(GL_VIEWPORT, viewport);
    framebuffer->prev_width = viewport[2];
    framebuffer->prev_height = viewport[3];

    glViewport(0, 0, framebuffer->width, framebuffer->height);
}

void bvr_framebuffer_disable(bvr_framebuffer_t* framebuffer){
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void bvr_framebuffer_clear(bvr_framebuffer_t* framebuffer, vec3 color){
    glClearColor(0, 0, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void bvr_framebuffer_blit(bvr_framebuffer_t* framebuffer){
    bvr_shader_enable(&framebuffer->shader);

    glBindVertexArray(framebuffer->vertex_buffer);
    glBindTexture(GL_TEXTURE_2D, framebuffer->color_buffer);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    bvr_shader_disable();
}

void bvr_destroy_framebuffer(bvr_framebuffer_t* framebuffer){
    bvr_destroy_shader(&framebuffer->shader);

    glDeleteVertexArrays(1, &framebuffer->vertex_buffer);
    glDeleteBuffers(1, &framebuffer->array_buffer);
    glDeleteTextures(1, &framebuffer->color_buffer);
    glDeleteRenderbuffers(1, &framebuffer->depth_buffer);
    glDeleteFramebuffers(1, &framebuffer->buffer);
}