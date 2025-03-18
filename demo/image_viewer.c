#define BVR_GEOMETRY_IMPLEMENTATION
#define BVR_INCLUDE_NUKLEAR

#include <BVR/scene.h>
#include <BVR/gui.h>

#include <BVR/actors.h>

#define NK_INCLUDE_FIXED_TYPES 
#include <nuklear.h>
#include <malloc.h>

#ifdef _WIN32
    #include <Windows.h>
    #include <commdlg.h>
#endif

#define CAMERA_SLIDER_MAX 1000.0f

static bvr_book_t game;
static bvr_nuklear_t gui;

static struct {
    bvr_model_t model;

    bvr_layered_texture_t texture;
    bvr_shader_uniform_t* texture_uniform;
    char path[256];

    uint8_t* enabled_layers;
} texture_infos;

static void load_texture(const char* path){
    if(texture_infos.texture.id){
        bvr_destroy_layered_texture(&texture_infos.texture);
        
        free(texture_infos.enabled_layers);
        texture_infos.enabled_layers = NULL;
    }

    if(path){
        memcpy(texture_infos.path, path, sizeof(texture_infos.path));

        bvr_create_layered_texture(&texture_infos.texture, path, BVR_TEXTURE_FILTER_NEAREST, BVR_TEXTURE_WRAP_REPEAT);
        bvr_shader_set_texturei(texture_infos.texture_uniform, &texture_infos.texture.id, NULL);
    
        texture_infos.enabled_layers = calloc(BVR_BUFFER_COUNT(texture_infos.texture.image.layers), sizeof(uint8_t));   
        memset(texture_infos.enabled_layers, 1, BVR_BUFFER_COUNT(texture_infos.texture.image.layers)); 
    }
}

static void open_file(void){
#ifdef _WIN32

    OPENFILENAME ofn;
    char szfile[256] = {0};

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "All Files\0*.*\0Text Files\0*.TXT\0";
    ofn.lpstrFile = szfile;
    ofn.nMaxFile = 256;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    ofn.lpstrTitle = "Select a file";

    if(GetOpenFileName(&ofn)){
        strncpy(texture_infos.path, szfile, sizeof(texture_infos.path));
    }

#else
    /*
    */
    BVR_ASSERT(0);
#endif
}

int main(){
    memset(&texture_infos, 0, sizeof(texture_infos));

    /* Create initial game context */
    bvr_create_book(&game);
    bvr_create_page(&game.page);

    bvr_create_window(&game.window, 800, 800, "Window", 0);
    bvr_create_audio_stream(&game.audio, BVR_DEFAULT_SAMPLE_RATE, BVR_DEFAULT_AUDIO_BUFFER_SIZE);

    /* Initialize GUI */
    bvr_create_nuklear(&gui, &game.window);

    /* Create the camera */
    bvr_add_orthographic_camera(&game.page, &game.window.framebuffer, 0.0f, 100.0f, 1.0f);

    /* Create image's plane mesh */
    bvr_create_2d_square_mesh(&texture_infos.model.mesh, 480.0f, 270.0f);

    /* Create the shader */
    bvr_create_shader(&texture_infos.model.shader, "res/shader.glsl", BVR_VERTEX_SHADER | BVR_FRAGMENT_SHADER);
    texture_infos.texture_uniform = bvr_shader_register_texture(
        &texture_infos.model.shader, BVR_TEXTURE_2D_ARRAY, NULL, NULL, 
        "bvr_texture", "bvr_texture_layer"
    );

    load_texture("res/texture.psd");

    while (1)
    {
        /* prepare the engine to draw a new frame */
        bvr_new_frame(&game);

        /* exit the main loop conditions */
        if(!bvr_is_awake(&game) || bvr_key_down(&game.window, BVR_KEY_ESCAPE)) {
            break;
        }

        /* start drawing */
        bvr_shader_enable(&texture_infos.model.shader);
        for (int layer = 0; layer < BVR_BUFFER_COUNT(texture_infos.texture.image.layers); layer++)
        {
            if(texture_infos.enabled_layers[layer]){
                texture_infos.model.transform.position[1] = layer;
                
                /* enable texture unit */
                bvr_layered_texture_enable(&texture_infos.texture, BVR_TEXTURE_UNIT0);

                /* update texture's content and push it to the shader */
                bvr_shader_set_texturei(texture_infos.texture_uniform, NULL, &layer);
                bvr_shader_use_uniform(texture_infos.texture_uniform, NULL);
    
                /* draw the plane */
                bvr_mesh_draw(&texture_infos.model.mesh, BVR_DRAWMODE_TRIANGLES);
                
                /* disable texture unit */
                bvr_layered_texture_disable();
            }
        }
        
        /* disable shader */
        bvr_shader_disable();

#ifdef BVR_INCLUDE_NUKLEAR
        {
            bvr_nuklear_handle(&gui);

            if (nk_begin(gui.context, "Book informations", nk_rect(50, 50, 350, 200),
                NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
                NK_WINDOW_CLOSABLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
            {
                nk_layout_row_dynamic(gui.context, 15, 1);
                nk_label(gui.context, BVR_FORMAT("delta time %fms", game.delta_time * 1000.0f), NK_TEXT_ALIGN_LEFT);
                nk_label(gui.context, "-", NK_TEXT_ALIGN_CENTERED);
                
                bvr_nuklear_vec3_label(&gui, "Position", game.page.camera.transform.position);
                
                nk_layout_row_dynamic(gui.context, 15, 2);
                {
                    nk_label(gui.context, "X", NK_TEXT_ALIGN_LEFT);
                    nk_slider_float(gui.context, -CAMERA_SLIDER_MAX, &game.page.camera.transform.position[0], CAMERA_SLIDER_MAX, 1.0f);
                }
                {
                    nk_label(gui.context, "Y", NK_TEXT_ALIGN_LEFT);
                    nk_slider_float(gui.context, -CAMERA_SLIDER_MAX, &game.page.camera.transform.position[1], CAMERA_SLIDER_MAX, 1.0f);
                }
                {
                    nk_label(gui.context, "Scale", NK_TEXT_ALIGN_LEFT);
                    nk_slider_float(gui.context, 1.0f, &game.page.camera.field_of_view.scale, 10.0f, 0.1f);
                }

                nk_label(gui.context, "-", NK_TEXT_ALIGN_CENTERED);

                nk_layout_row_dynamic(gui.context, 30, 1);
                nk_label(gui.context, texture_infos.path, NK_TEXT_ALIGN_LEFT);
                nk_layout_row_dynamic(gui.context, 15, 1);
                
                for (size_t layer = 0; layer < BVR_BUFFER_COUNT(texture_infos.texture.image.layers); layer++)
                {
                    texture_infos.enabled_layers[layer] = nk_check_label(gui.context, 
                        ((bvr_layer_t*)texture_infos.texture.image.layers.data)[layer].name.data, 
                        texture_infos.enabled_layers[layer]
                    );
                }

                nk_layout_row_dynamic(gui.context, 15, 2);
                
                if(nk_button_label(gui.context, "Select File")){
                    open_file();
                    load_texture(texture_infos.path);
                }

                if(nk_button_label(gui.context, "Reload")){
                    load_texture(texture_infos.path);
                }
            }
            nk_end(gui.context);

            bvr_nuklear_render(&gui);
        }
#endif

        /* push buffers to the window */
        bvr_render(&game);
    }
    
    free(texture_infos.enabled_layers);

    bvr_destroy_nuklear(&gui);
    bvr_destroy_shader(&texture_infos.model.shader);
    bvr_destroy_mesh(&texture_infos.model.mesh);
    bvr_destroy_layered_texture(&texture_infos.texture);
    bvr_destroy_book(&game);

    return 0;
}