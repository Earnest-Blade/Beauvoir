#include <BVR/scene.h>
#include <BVR/math.h>

#include <string.h>
#include <memory.h>

#include <malloc.h>

#include <GLAD/glad.h>

int bvr_create_book(bvr_book_t* book){
    BVR_ASSERT(book);

    memset(&book->audio, 0, sizeof(bvr_audio_stream_t));
    memset(&book->window, 0, sizeof(bvr_window_t));
    memset(&book->page, 0, sizeof(bvr_page_t));

    book->delta_time = 0.0f;
    book->prev_time = 0.0f;
    book->current_time = 0.0f;

    return BVR_OK;
}

void bvr_new_frame(bvr_book_t* book){
    bvr_window_poll_events(&book->window);

    book->current_time = bvr_frames();
    book->delta_time = (book->current_time - book->prev_time) / 1000.0f;

    // reset opengl states
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

    /* calculate camera matrices */
    bvr_mat4 projection, view;
    bvr_camera_t* camera = &book->page.camera;
    bvr_identity_mat4(projection);
    bvr_identity_mat4(view);

    if(camera->mode == BVR_CAMERA_ORTHOGRAPHIC){
        float width = 1.0f / camera->framebuffer->width * camera->field_of_view.scale;
        float height = 1.0f / camera->framebuffer->height * camera->field_of_view.scale;
        float farnear = 1.0f / (camera->far - camera->near);

        projection[0][0] = 1.0f * width;
        projection[1][1] = 1.0f * height;
        projection[2][2] = farnear;
        projection[3][0] = -width;
        projection[3][1] = -height;
        projection[3][2] = -camera->near * farnear;
        projection[3][3] =  1.0f;
    }

    view[3][0] = camera->transform.position[0];
    view[3][1] = camera->transform.position[1];
    view[3][2] = camera->transform.position[2];

    glBindBuffer(GL_UNIFORM_BUFFER, book->page.camera.buffer);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(bvr_mat4), &projection[0][0]);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(bvr_mat4), sizeof(bvr_mat4), &view[0][0]);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void bvr_render(bvr_book_t* book){
    bvr_window_push_buffers(&book->window);

#ifndef BVR_NO_FPS_CAP
    // wait for next frame. 
    if(book->prev_time + BVR_FRAMERATE > book->current_time){
        bvr_delay(book->current_time - book->prev_time + BVR_FRAMERATE);
    }
#endif

    book->prev_time = book->current_time;
}

void bvr_destroy_book(bvr_book_t* book){
    if(book->window.context){
        bvr_destroy_window(&book->window);
    }
    if(book->audio.stream){
        bvr_destroy_audio_stream(&book->audio);
    }

    bvr_destroy_page(&book->page);
}

int bvr_create_page(bvr_page_t* page){
    BVR_ASSERT(page);

    page->actors.size = 0;
    page->actors.data = NULL;
    page->actors.elemsize = sizeof(struct bvr_actor_s);

    return BVR_OK;
}

bvr_camera_t* bvr_add_orthographic_camera(bvr_page_t* page, bvr_framebuffer_t* framebuffer, float near, float far, float scale){
    BVR_ASSERT(page);

    page->camera.mode = BVR_CAMERA_ORTHOGRAPHIC;
    page->camera.framebuffer = framebuffer;
    page->camera.near = near;
    page->camera.far = far;
    page->camera.field_of_view.scale = scale;

    glGenBuffers(1, &page->camera.buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, page->camera.buffer);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(bvr_mat4), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, page->camera.buffer, 0, 2 * sizeof(bvr_mat4));

    return &page->camera;
}

void bvr_destroy_page(bvr_page_t* page){
    BVR_ASSERT(page);

    
    if(page->actors.data){
        
        for (size_t i = 0; i < page->actors.size / page->actors.elemsize; i++)
        {
            // process to destroy actor
        }

        free(page->actors.data);
        page->actors.data = NULL;
    }
}