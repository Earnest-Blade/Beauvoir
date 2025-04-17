#include <BVR/scene.h>
#include <BVR/math.h>

#include <string.h>
#include <memory.h>

#include <malloc.h>

int bvr_create_book(bvr_book_t* book){
    BVR_ASSERT(book);

    memset(&book->audio, 0, sizeof(bvr_audio_stream_t));
    memset(&book->window, 0, sizeof(bvr_window_t));
    memset(&book->page, 0, sizeof(bvr_page_t));

    book->delta_time = 0.0f;
    book->prev_time = 0.0f;
    book->current_time = 0.0f;

    book->pipeline.rendering_pass.blending = BVR_BLEND_ENABLE | BVR_BLEND_FUNC_ALPHA_ONE_MINUS;
    book->pipeline.rendering_pass.depth = BVR_DEPTH_TEST_ENABLE | BVR_DEPTH_FUNC_ALWAYS;
    book->pipeline.rendering_pass.flags = 0;

    book->pipeline.swap_pass.blending = BVR_BLEND_DISABLE;
    book->pipeline.swap_pass.depth = BVR_DEPTH_TEST_DISABLE;
    book->pipeline.swap_pass.flags = 0;

    book->pipeline.clear_color[0] = 0.0f;
    book->pipeline.clear_color[1] = 0.0f;
    book->pipeline.clear_color[2] = 0.0f;

    return BVR_OK;
}

void bvr_new_frame(bvr_book_t* book){
    bvr_window_poll_events(&book->window);

    book->current_time = bvr_frames();
    book->delta_time = (book->current_time - book->prev_time) / 1000.0f;

    // reset opengl states
    bvr_framebuffer_enable(&book->window.framebuffer);
    bvr_framebuffer_clear(&book->window.framebuffer, book->pipeline.clear_color);

    bvr_pipeline_state_enable(&book->pipeline.rendering_pass);

    /* calculate camera matrices */
    mat4x4 projection, view;
    bvr_camera_t* camera = &book->page.camera;
    BVR_IDENTITY_MAT4(projection);
    BVR_IDENTITY_MAT4(view);

    if(camera->mode == BVR_CAMERA_ORTHOGRAPHIC){
        float width = 1.0f / camera->framebuffer->width * camera->field_of_view.scale;
        float height = 1.0f / camera->framebuffer->height * camera->field_of_view.scale;
        float farnear = 1.0f / (camera->far - camera->near);

        projection[0][0] = width;
        projection[1][1] = height;
        projection[2][2] = farnear;
        projection[3][0] = -width;
        projection[3][1] = -height;
        projection[3][2] = -camera->near * farnear;
        projection[3][3] =  1.0f;
    }

    view[3][0] = camera->transform.position[0];
    view[3][1] = camera->transform.position[1];
    view[3][2] = camera->transform.position[2];

    bvr_enable_uniform_buffer(book->page.camera.buffer);
    bvr_uniform_buffer_set(book->page.camera.buffer, 0, sizeof(mat4x4), &projection[0][0]);
    bvr_uniform_buffer_set(book->page.camera.buffer, sizeof(mat4x4), sizeof(mat4x4), &view[0][0]);
    bvr_enable_uniform_buffer(0);
}

void bvr_update(bvr_book_t* book){
    bvr_collider_t* collider;
    bvr_collider_t* other;

    BVR_POOL_FOR_EACH(collider, book->page.colliders){        
        if(!collider){
            break;
        }
        
        // collision are disabled
        if(BVR_HAS_FLAG(collider->body.mode, BVR_COLLISION_DISABLE)){
            bvr_body_apply_motion(&collider->body, collider->transform);
            
            continue;
        }

        // if this actor is aggressive
        if(BVR_HAS_FLAG(collider->body.mode, 0x4)){

            struct bvr_collision_result_s result;

            BVR_POOL_FOR_EACH(other, book->page.colliders){
                if(!other){
                    break;
                }
    
                bvr_compare_colliders(collider, other, &result);
            }

            if(!result.collide){
                bvr_body_apply_motion(&collider->body, collider->transform);
            }
        }
    }
}

void bvr_render(bvr_book_t* book){
    bvr_framebuffer_disable(&book->window.framebuffer);

    bvr_pipeline_state_enable(&book->pipeline.swap_pass);
    bvr_framebuffer_blit(&book->window.framebuffer);
    
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

    bvr_create_pool(&page->actors, sizeof(struct bvr_actor_s*), BVR_MAX_SCENE_ACTOR_COUNT);
    bvr_create_pool(&page->colliders, sizeof(bvr_collider_t*), BVR_COLLIDER_COLLECTION_SIZE);

    return BVR_OK;
}

bvr_camera_t* bvr_add_orthographic_camera(bvr_page_t* page, bvr_framebuffer_t* framebuffer, float near, float far, float scale){
    BVR_ASSERT(page);

    page->camera.mode = BVR_CAMERA_ORTHOGRAPHIC;
    page->camera.framebuffer = framebuffer;
    page->camera.near = near;
    page->camera.far = far;
    page->camera.field_of_view.scale = scale;

    bvr_create_uniform_buffer(&page->camera.buffer, 2 * sizeof(mat4x4));

    return &page->camera;
}

void bvr_screen_to_world_coords(bvr_book_t* book, vec3 coords){
    BVR_ASSERT(book);

    if(!coords){
        return;
    }

    if(book->page.camera.mode == BVR_CAMERA_ORTHOGRAPHIC){
        vec4 viewport, result;        
        mat4x4 projection, inv;
        BVR_IDENTITY_MAT4(projection);
        BVR_IDENTITY_MAT4(inv);

        float width =    1.0f / book->page.camera.framebuffer->width * book->page.camera.field_of_view.scale;
        float height =  -1.0f / book->page.camera.framebuffer->height * book->page.camera.field_of_view.scale;
        float farnear = -1.0f / (book->page.camera.far - book->page.camera.near);

        projection[0][0] = 1.0f * width;
        projection[1][1] = 1.0f * height;
        projection[2][2] = farnear;
        projection[3][0] = width;
        projection[3][1] = height;
        projection[3][2] = book->page.camera.near * farnear;
        projection[3][3] =  1.0f;

        mat4x4_invert(inv, projection);
        viewport[0] = coords[0] * width; 
        viewport[1] = coords[1] * height;
        viewport[2] = coords[2] * farnear;
        viewport[3] = 0.0f;

        vec4_sub(result, result, viewport);
        vec4_scale(viewport, result, 2);

        mat4x4_mul_vec4(result, inv, viewport);
        coords[0] = result[0];
        coords[1] = result[1];
        coords[2] = result[2];

        BVR_PRINT_VEC3("", viewport);
    }
}

struct bvr_actor_s* bvr_link_actor_to_page(bvr_page_t* page, struct bvr_actor_s* actor){
    BVR_ASSERT(page);
    
    if(actor){
        struct bvr_actor_s** aptr = (struct bvr_actor_s**) bvr_pool_alloc(&page->actors);
        *aptr = actor;

        switch (actor->type)
        {
        case BVR_DYNAMIC_ACTOR:
            bvr_link_collider_to_page(page, &((bvr_dynamic_actor_t*)actor)->collider);
            break;
        
        default:
            break;
        }

        return *aptr;
    }

    return NULL;
}

bvr_collider_t* bvr_link_collider_to_page(bvr_page_t* page, bvr_collider_t* collider){
    BVR_ASSERT(page);

    if(collider){
        bvr_collider_t** cptr = (bvr_collider_t**) bvr_pool_alloc(&page->colliders);
        *cptr = collider;
        return *cptr;
    }

    return NULL;
}

void bvr_destroy_page(bvr_page_t* page){
    BVR_ASSERT(page);

    struct bvr_actor_s* actor;
    BVR_POOL_FOR_EACH(actor, page->actors){
        if(!actor) break;

        bvr_destroy_actor(actor);
    }

    bvr_collider_t* collider;
    BVR_POOL_FOR_EACH(collider, page->colliders){
        if(!collider) break;

        collider = NULL;
    }

    bvr_destroy_uniform_buffer(&page->camera.buffer);

    bvr_destroy_pool(&page->actors);
    bvr_destroy_pool(&page->colliders);
}