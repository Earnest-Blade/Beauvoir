/*
    This file contains the fundation of every Beauvoir projects.
*/

/* include all Beauvoir's headers */
#define BVR_GEOMETRY_IMPLEMENTATION
#include <BVR/bvr.h>

#define CAMERA_SLIDER_MAX 100.0f

/* game's context */
static bvr_book_t book;
static bvr_nuklear_t gui;

static bvr_static_model_t player;

void draw_nk(){
#ifdef BVR_INCLUDE_NUKLEAR
    {
        bvr_nuklear_handle(&gui);
        if (nk_begin(gui.context, "Book informations", nk_rect(50, 50, 350, 200),
            NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
            NK_WINDOW_CLOSABLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
        {
            nk_layout_row_dynamic(gui.context, 15, 1);
            
            bvr_nuklear_vec3_label(&gui, "Position", book.page.camera.transform.position);
            
            nk_layout_row_dynamic(gui.context, 15, 2);
            {
                nk_label(gui.context, "X", NK_TEXT_ALIGN_LEFT);
                nk_slider_float(gui.context, -CAMERA_SLIDER_MAX, &book.page.camera.transform.position[0], CAMERA_SLIDER_MAX, 1.0f);
            }
            {
                nk_label(gui.context, "Y", NK_TEXT_ALIGN_LEFT);
                nk_slider_float(gui.context, -CAMERA_SLIDER_MAX, &book.page.camera.transform.position[1], CAMERA_SLIDER_MAX, 1.0f);
            }
            {
                nk_label(gui.context, "Scale", NK_TEXT_ALIGN_LEFT);
                nk_slider_float(gui.context, 1.0f, &book.page.camera.field_of_view.scale, 10.0f, 0.1f);
            }
            nk_layout_row_dynamic(gui.context, 15, 1);
            
            for (size_t i = 0; i < 1; i++)
            {
                bvr_nuklear_actor_label(&gui, *(struct bvr_actor_s**)(bvr_pool_try_get(&book.page.actors, i)));
            }
            
        }
        nk_end(gui.context);
        bvr_nuklear_render(&gui);
    }
#endif
}

int main(){
    /* create initial game's context */
    bvr_create_book(&book);
    bvr_create_page(&book.page);

    /* create the window */
    bvr_create_window(&book.window, 800, 800, "Window", 0);
    
    bvr_create_nuklear(&gui, &book.window);

    /* create an audio stream */
    bvr_create_audio_stream(&book.audio, BVR_DEFAULT_SAMPLE_RATE, BVR_DEFAULT_AUDIO_BUFFER_SIZE);

    bvr_add_orthographic_camera(&book.page, &book.window.framebuffer, 0.1f, 100.0f, 1.0f);

    bvr_create_actor(&player.object, "player", BVR_DYNAMIC_ACTOR);
    bvr_add_actor(&book.page, &player.object);

    bvr_create_2d_square_mesh(&player.mesh, 10.0f, 10.0f);
    bvr_create_shader(&player.shader, "res/monochrome.glsl", BVR_VERTEX_SHADER | BVR_FRAGMENT_SHADER);
    bvr_shader_register_uniform(&player.shader, BVR_VEC3, 1, "bvr_color");
    
    {
        bvr_vec3 color = {1.0f, 0.0f, 1.0f};
        bvr_shader_set_uniform(&player.shader, "bvr_color", &color);
    }

    /* main loop */
    while (1)
    {
        /* ask Beauvoir to prepare a new frame */
        bvr_new_frame(&book);

        /* quit the main loop if Beauvoir is not running */
        if(!bvr_is_awake(&book) || bvr_key_down(&book.window, BVR_KEY_ESCAPE)){
            break;
        }

        float x, y;
        bvr_mouse_position(&book.window, &x, &y);
        player.object.transform.position[0] = (x - (book.window.framebuffer.width / 2)) * 2;
        player.object.transform.position[1] = (-y + (book.window.framebuffer.height / 2)) * 2;

        //bvr_model_draw(&player, BVR_DRAWMODE_TRIANGLES);
        bvr_draw_static_model(&player, BVR_DRAWMODE_TRIANGLES);

        draw_nk();

        /* push Beauvoir's graphics to the window */
        bvr_render(&book);
    }
    
    /* free */
    bvr_destroy_shader(&player.shader);
    bvr_destroy_mesh(&player.mesh);
    bvr_destroy_actor(&player.object);
    bvr_destroy_book(&book);

    return 0;
}