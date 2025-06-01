#include <BVR/editor/editor.h>
#include <BVR/editor/renderer.h>

#include <BVR/buffer.h>
#include <BVR/utils.h>
#include <BVR/window.h>
#include <BVR/actors.h>

#include <BVR/editor/flags.h>
#include <BVR/editor/io.h>

#define NK_INCLUDE_FIXED_TYPES 
#include <nuklear.h>

#define BVR_EDITOR_VERTEX_BUFFER_SIZE 1000

#define BVR_TRANSFORM_MAX 100000.0f

#define BVR_HIERARCHY_RECT(width, height) (nk_rect(50, 50, width, height))
#define BVR_INSPECTOR_RECT(width, height) (nk_rect(350, 50, width, height))

static bvr_editor_t* __editor = NULL;

static void bvri_draw_editor_vec3(const char* text, vec3 value){
    nk_layout_row_dynamic(__editor->gui.context, 15, 4);
    nk_label_wrap(__editor->gui.context, text);
    nk_label_wrap(__editor->gui.context, BVR_FORMAT("x%f ", value[0]));
    nk_label_wrap(__editor->gui.context, BVR_FORMAT("y%f ", value[1]));
    nk_label_wrap(__editor->gui.context, BVR_FORMAT("z%f ", value[2]));
}

static void bvri_draw_editor_transform(bvr_transform_t* transform){
    nk_layout_row_dynamic(__editor->gui.context, 90, 1);

    nk_group_begin_titled(__editor->gui.context, BVR_MACRO_STR(__LINE__), "transform", 
        NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_TITLE);
    {
        nk_layout_row_dynamic(__editor->gui.context, 15, 1);
        nk_property_float(__editor->gui.context, "x", -BVR_TRANSFORM_MAX, &transform->position[0], BVR_TRANSFORM_MAX, 0.1f, 0.1f);
        nk_property_float(__editor->gui.context, "y", -BVR_TRANSFORM_MAX, &transform->position[1], BVR_TRANSFORM_MAX, 0.1f, 0.1f);
        nk_property_float(__editor->gui.context, "z", -BVR_TRANSFORM_MAX, &transform->position[2], BVR_TRANSFORM_MAX, 0.1f, 0.1f);
    }
    nk_group_end(__editor->gui.context);
}

static void bvri_draw_editor_body(struct bvr_body_s* body){
    nk_layout_row_dynamic(__editor->gui.context, 15, 1);

    nk_label(__editor->gui.context, BVR_FORMAT("acceleration %f", body->acceleration), NK_TEXT_ALIGN_LEFT);
    bvri_draw_editor_vec3("direction", body->direction);
}

static void bvri_draw_editor_mesh(bvr_mesh_t* mesh){

}

static void bvri_draw_hierarchy_button(const char* name, size_t type, void* object){
    if(nk_button_label(__editor->gui.context, name)){

        bvr_destroy_string(&__editor->inspector_command.name);
        bvr_create_string(&__editor->inspector_command.name, name);

        __editor->inspector_command.type = type;
        __editor->inspector_command.pointer = object;
    }
}

void bvr_create_editor(bvr_editor_t* editor, bvr_book_t* book){
    BVR_ASSERT(editor);
    BVR_ASSERT(book);
    
    if(__editor){
        BVR_PRINT("warning, override previous binded editor!");
        bvr_destroy_editor(__editor);
    }
    
    __editor = editor;

    editor->book = book;
    editor->state = BVR_EDITOR_STATE_HANDLE;
    editor->inspector_command.pointer = NULL;
    editor->inspector_command.type = 0;
    editor->draw_command.drawmode = 0;
    editor->draw_command.element_offset = 0;
    editor->draw_command.element_count = 0;

    {
        const char* vertex_shader = 
            "#version 400\n"
            "layout(location=0) in vec3 in_position;\n"
            "uniform mat4 bvr_transform;\n"
            "layout(std140) uniform bvr_camera {\n"
	        "mat4 bvr_projection;\n"
	        "mat4 bvr_view;\n"
            "};\n"
            "void main() {\n"
            "	gl_Position = bvr_projection * bvr_view * vec4(in_position, 1.0);\n"
            "}";

        const char* fragment_shader = 
            "#version 400\n"
            "uniform vec3 bvr_color;\n"
            "void main() {\n"
            	"gl_FragColor = vec4(bvr_color, 1.0);\n"
            "}";
        
        bvri_create_shader_vert_frag(&editor->device.shader, vertex_shader, fragment_shader);
        //BVR_ASSERT(bvr_shader_register_uniform(&editor->device.shader, BVR_MAT4, 1, "bvr_transform"));
        BVR_ASSERT(bvr_shader_register_uniform(&editor->device.shader, BVR_VEC3, 1, "bvr_color"));
        BVR_ASSERT(bvr_shader_register_block(&editor->device.shader, BVR_UNIFORM_CAMERA_NAME, BVR_MAT4, 2, BVR_UNIFORM_BLOCK_CAMERA));

        vec3 color = {0.0f, 1.0f, 0.0f};
        bvr_shader_set_uniformi(&editor->device.shader.uniforms[1], &color);
    }

    if(bvri_create_editor_render_buffers(
        &editor->device.array_buffer, 
        &editor->device.vertex_buffer, 
        BVR_EDITOR_VERTEX_BUFFER_SIZE
    )){
        BVR_ASSERT(0 || "failed to create editor buffers");
    }
    
    bvr_create_string(&editor->inspector_command.name, NULL);
    bvr_create_nuklear(&editor->gui, &book->window);
}

void bvr_editor_handle(){
    BVR_ASSERT(__editor);

    bvr_nuklear_handle(&__editor->gui);
    
    if(bvr_key_down(&__editor->book->window, BVR_EDITOR_HIDDEN_INPUT)){
        __editor->state = BVR_EDITOR_STATE_HIDDEN;
    }
    if(bvr_key_down(&__editor->book->window, BVR_EDITOR_SHOW_INPUT)){
        __editor->state = BVR_EDITOR_STATE_HANDLE;
    }

    if(__editor->state != BVR_EDITOR_STATE_HIDDEN){
        __editor->state = BVR_EDITOR_STATE_HANDLE;
    }
}

void bvr_editor_draw_page_hierarchy(){
    if(__editor->state == BVR_EDITOR_STATE_HIDDEN) {
        return;
    }

    BVR_ASSERT(__editor->state == BVR_EDITOR_STATE_HANDLE);
    __editor->state = BVR_EDITOR_STATE_DRAWING;

    if(nk_begin(__editor->gui.context, BVR_FORMAT("scene '%s'", __editor->book->page.name.string), BVR_HIERARCHY_RECT(200, 450), 
        NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_TITLE)){

        nk_menubar_begin(__editor->gui.context);
        {
            nk_layout_row_begin(__editor->gui.context, NK_STATIC, 25, 1); 
            nk_layout_row_push(__editor->gui.context, 45);

            if(nk_menu_begin_label(__editor->gui.context, "file", NK_TEXT_ALIGN_LEFT, nk_vec2(100, 100))){
                nk_layout_row_dynamic(__editor->gui.context, 15, 1);

                if(nk_menu_item_label(__editor->gui.context, "save", NK_TEXT_ALIGN_LEFT)){
                    bvr_write_book("book.bin", __editor->book);
                }

                if(nk_menu_item_label(__editor->gui.context, "open", NK_TEXT_ALIGN_LEFT)){
                    bvr_open_book("book.bin", __editor->book);
                }

                if(nk_menu_item_label(__editor->gui.context, "exit", NK_TEXT_ALIGN_LEFT)){
                    __editor->book->window.awake = 0;
                }

                nk_menu_end(__editor->gui.context);
            }
        }
        nk_menubar_end(__editor->gui.context);

        // scene components
        nk_layout_row_dynamic(__editor->gui.context, 150, 1);
        nk_group_begin_titled(__editor->gui.context, BVR_MACRO_STR(__LINE__), "scene infos", NK_WINDOW_BORDER | NK_WINDOW_TITLE);
        {
            nk_layout_row_dynamic(__editor->gui.context, 15, 1);
            bvri_draw_hierarchy_button("camera", BVR_EDITOR_CAMERA, &__editor->book->page.camera);
        }
        nk_group_end(__editor->gui.context);

        // scene actors
        nk_layout_row_dynamic(__editor->gui.context, 50 + __editor->book->page.actors.count * 20, 1);
        nk_group_begin_titled(__editor->gui.context, BVR_MACRO_STR(__LINE__), "actors", NK_WINDOW_BORDER | NK_WINDOW_TITLE);
        {
            nk_layout_row_dynamic(__editor->gui.context, 15, 1);
            
            struct bvr_actor_s* actor;
            BVR_POOL_FOR_EACH(actor, __editor->book->page.actors){
                if(!actor){
                    break;
                }

                bvri_draw_hierarchy_button(actor->name.string, BVR_EDITOR_ACTOR, actor);
            }
        }
        nk_group_end(__editor->gui.context);

        nk_layout_row_dynamic(__editor->gui.context, 70 + __editor->book->page.colliders.count * 20, 1);
        nk_group_begin_titled(__editor->gui.context, BVR_MACRO_STR(__LINE__), "colliders", NK_WINDOW_BORDER | NK_WINDOW_TITLE);
        {
            nk_layout_row_dynamic(__editor->gui.context, 15, 1);
            
            struct bvr_collider_s* collider;
            BVR_POOL_FOR_EACH(collider, __editor->book->page.colliders){
                if(!collider){
                    break;
                }

                bvri_draw_hierarchy_button(
                    BVR_FORMAT("collider%x", (size_t)blockcollider - (size_t)__editor->book->page.colliders.data),
                    BVR_EDITOR_COLLIDER, collider
                );
            }

            if(nk_button_label(__editor->gui.context, "Add")){

            }
        }
        
        nk_group_end(__editor->gui.context);

        nk_end(__editor->gui.context);
    }
}

void bvr_editor_draw_inspector(){
    if(__editor->state == BVR_EDITOR_STATE_HIDDEN) {
        return;
    }

    BVR_ASSERT(__editor->state == BVR_EDITOR_STATE_DRAWING);

    if(nk_begin(__editor->gui.context, BVR_FORMAT("inspector '%s'", __editor->inspector_command.name.string), BVR_INSPECTOR_RECT(350, 200), 
        NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_TITLE)){
    
        if(!__editor->inspector_command.pointer){
            nk_end(__editor->gui.context);
            return;
        }

        
        __editor->draw_command.drawmode = 0;
        __editor->draw_command.element_offset = 0;
        __editor->draw_command.element_count = 0;

        nk_layout_row_dynamic(__editor->gui.context, 15, 1);
        switch (__editor->inspector_command.type)
        {
        case BVR_EDITOR_CAMERA:
            {
                bvr_camera_t* camera = (bvr_camera_t*)__editor->inspector_command.pointer;
                
                nk_layout_row_dynamic(__editor->gui.context, 40, 1);
                nk_group_begin(__editor->gui.context, BVR_FORMAT("framebuffer%x", &camera->framebuffer), NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR);
                {
                    nk_layout_row_dynamic(__editor->gui.context, 15, 2);
                    
                    nk_label(__editor->gui.context, BVR_FORMAT("width %i", camera->framebuffer->width), NK_TEXT_ALIGN_LEFT);
                    nk_label(__editor->gui.context, BVR_FORMAT("height %i", camera->framebuffer->height), NK_TEXT_ALIGN_LEFT);
                    
                    nk_label(__editor->gui.context, BVR_FORMAT("far %f", camera->far), NK_TEXT_ALIGN_LEFT);
                    nk_label(__editor->gui.context, BVR_FORMAT("near %f", camera->near), NK_TEXT_ALIGN_LEFT);
                }
                nk_group_end(__editor->gui.context);

                nk_property_float(__editor->gui.context, "scale", 0.01f, &camera->field_of_view.scale, 20.0f, 0.1f, 0.1f);
                
                bvri_draw_editor_transform(&camera->transform);
            }
            break;
        
        case BVR_EDITOR_ACTOR: 
            {
                struct bvr_actor_s* actor = (struct bvr_actor_s*)__editor->inspector_command.pointer;
                
                nk_layout_row_dynamic(__editor->gui.context, 15, 1);
                nk_label(__editor->gui.context, BVR_FORMAT("id %i", actor->id), NK_TEXT_ALIGN_LEFT);
                nk_label(__editor->gui.context, BVR_FORMAT("flags %x", actor->flags), NK_TEXT_ALIGN_LEFT);

                bvri_draw_editor_transform(&actor->transform);

                nk_layout_row_dynamic(__editor->gui.context, 100, 1);
                nk_group_begin(__editor->gui.context, BVR_MACRO_STR(__LINE__), NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR);
                nk_layout_row_dynamic(__editor->gui.context, 15, 1);

                switch (actor->type)
                {
                case BVR_EMPTY_ACTOR:
                    nk_label(__editor->gui.context, "EMPTY ACTOR", NK_TEXT_ALIGN_CENTERED);
                    break;
                case BVR_BITMAP_ACTOR:
                    {
                        nk_label(__editor->gui.context, "BITMAP ACTOR", NK_TEXT_ALIGN_CENTERED);
                    }
                    break;
                case BVR_STATIC_ACTOR:
                    {
                        nk_label(__editor->gui.context, "STATIC ACTOR", NK_TEXT_ALIGN_CENTERED);

                    }
                case BVR_DYNAMIC_ACTOR:
                    {
                        nk_label(__editor->gui.context, "DYNAMIC ACTOR", NK_TEXT_ALIGN_CENTERED);
                        bvri_draw_editor_mesh(&((bvr_dynamic_actor_t*)actor)->mesh);
                        bvri_draw_editor_body(&((bvr_dynamic_actor_t*)actor)->collider.body);

                        nk_layout_row_dynamic(__editor->gui.context, 15, 1);
                        bvri_draw_hierarchy_button("go to collider", BVR_EDITOR_COLLIDER, &((bvr_dynamic_actor_t*)actor)->collider);
                    }
                    break;
                default:
                    break;
                }

                nk_group_end(__editor->gui.context);
            }
            break;

        case BVR_EDITOR_COLLIDER:
            {
                bvr_collider_t* collider = (bvr_collider_t*)__editor->inspector_command.pointer;

                nk_layout_row_dynamic(__editor->gui.context, 100, 1);
                nk_group_begin_titled(__editor->gui.context, BVR_FORMAT("collider%i", collider), "bounds", NK_WINDOW_BORDER | NK_WINDOW_TITLE);

                if(collider->shape = BVR_COLLIDER_BOX){
                    struct bvr_bounds_s* bounds = (struct bvr_bounds_s*)collider->geometry.data;

                    {
                        float vertices[24] = {
                            bounds->coords[0] + bounds->width * -0.5f, bounds->coords[1] + bounds->height * +0.5f, 0.1f,
                            bounds->coords[0] + bounds->width * +0.5f, bounds->coords[1] + bounds->height * +0.5f, 0.1f,
                            bounds->coords[0] + bounds->width * +0.5f, bounds->coords[1] + bounds->height * +0.5f, 0.1f,
                            bounds->coords[0] + bounds->width * +0.5f, bounds->coords[1] + bounds->height * -0.5f, 0.1f,
                            bounds->coords[0] + bounds->width * +0.5f, bounds->coords[1] + bounds->height * -0.5f, 0.1f,
                            bounds->coords[0] + bounds->width * -0.5f, bounds->coords[1] + bounds->height * -0.5f, 0.1f,
                            bounds->coords[0] + bounds->width * -0.5f, bounds->coords[1] + bounds->height * +0.5f, 0.1f,
                            bounds->coords[0] + bounds->width * -0.5f, bounds->coords[1] + bounds->height * -0.5f, 0.1f,
                        };

                        bvri_bind_editor_buffers(__editor->device.array_buffer, __editor->device.vertex_buffer);
                        bvri_set_editor_buffers(vertices, 8);
                        bvri_bind_editor_buffers(0, 0);

                        __editor->draw_command.drawmode = BVR_DRAWMODE_LINES;
                        __editor->draw_command.element_offset = 0;
                        __editor->draw_command.element_count = 8;
                    }
                    
                    nk_layout_row_dynamic(__editor->gui.context, 15, 3);
                    
                    nk_label_wrap(__editor->gui.context, "coords");
                    nk_label_wrap(__editor->gui.context, BVR_FORMAT("x %f ", bounds->coords[0]));
                    nk_label_wrap(__editor->gui.context, BVR_FORMAT("y %f ", bounds->coords[1]));

                    nk_label_wrap(__editor->gui.context, "size");
                    nk_label_wrap(__editor->gui.context, BVR_FORMAT("width %i ", bounds->width));
                    nk_label_wrap(__editor->gui.context, BVR_FORMAT("height %i ", bounds->height));
                }

                nk_group_end(__editor->gui.context);
            }
            break;
        default:
            break;
        }

        nk_end(__editor->gui.context);
    }
}

void bvr_editor_render(){
    BVR_ASSERT(__editor);
    if(__editor->state == BVR_EDITOR_STATE_HIDDEN) {
        return;
    }

    BVR_ASSERT(__editor->state == BVR_EDITOR_STATE_DRAWING);
    __editor->state = BVR_EDITOR_STATE_RENDERING;

    if(__editor->draw_command.drawmode){
        bvr_shader_enable(&__editor->device.shader);

        bvri_bind_editor_buffers(__editor->device.array_buffer, __editor->device.vertex_buffer);
        bvri_draw_editor_buffer(__editor->draw_command.drawmode, __editor->draw_command.element_offset, __editor->draw_command.element_count);
        bvri_bind_editor_buffers(0, 0);

        bvr_shader_disable();
    }

    bvr_nuklear_render(&__editor->gui);
}

void bvr_destroy_editor(bvr_editor_t* editor){
    bvr_destroy_string(&editor->inspector_command.name);
    bvr_destroy_nuklear(&editor->gui);
}