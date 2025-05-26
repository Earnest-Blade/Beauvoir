#include <BVR/editor/editor.h>

#include <BVR/buffer.h>
#include <BVR/utils.h>
#include <BVR/window.h>
#include <BVR/actors.h>

#include <BVR/editor/flags.h>

#define NK_INCLUDE_FIXED_TYPES 
#include <nuklear.h>

#define BVR_TRANSFORM_MAX 100000.0f

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

    if(nk_begin(__editor->gui.context, BVR_FORMAT("scene '%s'", __editor->book->page.name.string), nk_rect(50, 50, 200, 450), 
        NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_TITLE)){

        // scene components
        nk_layout_row_dynamic(__editor->gui.context, 150, 1);
        nk_group_begin_titled(__editor->gui.context, BVR_MACRO_STR(__LINE__), "scene infos", NK_WINDOW_BORDER | NK_WINDOW_TITLE);
        {
            nk_layout_row_dynamic(__editor->gui.context, 15, 1);
            bvri_draw_hierarchy_button("camera", BVR_EDITOR_CAMERA, &__editor->book->page.camera);
        }
        nk_group_end(__editor->gui.context);

        // scene actors
        nk_layout_row_dynamic(__editor->gui.context, 40 + __editor->book->page.actors.count * 20, 1);
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

        nk_layout_row_dynamic(__editor->gui.context, 40 + __editor->book->page.colliders.count * 20, 1);
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
        }
        nk_group_end(__editor->gui.context);
    }
    nk_end(__editor->gui.context);
}

void bvr_editor_draw_inspector(){
    if(__editor->state == BVR_EDITOR_STATE_HIDDEN) {
        return;
    }

    BVR_ASSERT(__editor->state == BVR_EDITOR_STATE_DRAWING);

    if(nk_begin(__editor->gui.context, BVR_FORMAT("inspector '%s'", __editor->inspector_command.name.string), nk_rect(250, 50, 350, 200), 
        NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_TITLE)){
    
        if(!__editor->inspector_command.pointer){
            nk_end(__editor->gui.context);
            return;
        }

        nk_layout_row_dynamic(__editor->gui.context, 15, 1);
        switch (__editor->inspector_command.type)
        {
        case BVR_EDITOR_CAMERA:
            {
                bvr_camera_t* camera = (bvr_camera_t*)__editor->inspector_command.pointer;
                
                nk_layout_row_dynamic(__editor->gui.context, 20, 1);
                nk_group_begin(__editor->gui.context, BVR_FORMAT("framebuffer%x", &camera->framebuffer), NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR);
                {
                    nk_layout_row_dynamic(__editor->gui.context, 15, 2);
                    
                    nk_label(__editor->gui.context, BVR_FORMAT("width %i", camera->framebuffer->width), NK_TEXT_ALIGN_LEFT);
                    nk_label(__editor->gui.context, BVR_FORMAT("height %i", camera->framebuffer->height), NK_TEXT_ALIGN_LEFT);
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
                    }
                    break;
                default:
                    break;
                }

                nk_group_end(__editor->gui.context);
            }
            break;
        default:
            break;
        }
    }
    nk_end(__editor->gui.context);
}

void bvr_editor_render(){
    BVR_ASSERT(__editor);
    if(__editor->state == BVR_EDITOR_STATE_HIDDEN) {
        return;
    }

    BVR_ASSERT(__editor->state == BVR_EDITOR_STATE_DRAWING);
    __editor->state = BVR_EDITOR_STATE_RENDERING;

    bvr_nuklear_render(&__editor->gui);
}

void bvr_destroy_editor(bvr_editor_t* editor){
    bvr_destroy_string(&editor->inspector_command.name);
    bvr_destroy_nuklear(&editor->gui);
}