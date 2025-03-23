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

static struct {
    bvr_model_t model;
} player;

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

    bvr_create_2d_square_mesh(&player.model.mesh, 10.0f, 10.0f);
    bvr_create_shader(&player.model.shader, "res/monochrome.glsl", BVR_VERTEX_SHADER | BVR_FRAGMENT_SHADER);
    bvr_shader_register_uniform(&player.model.shader, BVR_VEC3, 1, "bvr_color");
    
    {
        bvr_vec3 color = {1.0f, 0.0f, 1.0f};
        bvr_shader_set_uniform(&player.model.shader, "bvr_color", &color);
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
        player.model.transform.position[0] = (x - (book.window.framebuffer.width / 2)) * 2;
        player.model.transform.position[1] = (-y + (book.window.framebuffer.height / 2)) * 2;

        //bvr_model_draw(&player.model, BVR_DRAWMODE_TRIANGLES);

        bvr_identity_mat4(player.model.transform.matrix);
        player.model.transform.matrix[3][0] = player.model.transform.position[0];
        player.model.transform.matrix[3][1] = player.model.transform.position[1];
        player.model.transform.matrix[3][2] = player.model.transform.position[2];

        bvr_shader_enable(&player.model.shader);
        bvr_shader_use_uniform(&player.model.shader.uniforms[0], &player.model.transform.matrix[0][0]);

        bvr_mesh_draw(&player.model.mesh, BVR_DRAWMODE_TRIANGLES);

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


                bvr_nuklear_vec3_label(&gui, "Player", player.model.transform.position);
            }
            nk_end(gui.context);

            bvr_nuklear_render(&gui);
        }
#endif

        /* push Beauvoir's graphics to the window */
        bvr_render(&book);
    }
    
    /* free */
    bvr_destroy_book(&book);

    return 0;
}