#include <BVR/actors.h>

#include <BVR/file.h>
#include <BVR/graphics.h>

#include <stdlib.h>
#include <math.h>
#include <memory.h>

#include <GLAD/glad.h>

/*
    calculate actor's transformation matrix
*/
static void bvri_update_transform(struct bvr_actor_s* actor){
    BVR_ASSERT(actor);

    mat4x4 rotation_mat;

    BVR_IDENTITY_MAT4(rotation_mat);
    BVR_IDENTITY_MAT4(actor->transform.matrix);

    mat4_rotate(rotation_mat, actor->transform.rotation);

    // copy translation to the translate matrix
    actor->transform.matrix[3][0] = actor->transform.position[0];
    actor->transform.matrix[3][1] = actor->transform.position[1];
    actor->transform.matrix[3][2] = actor->transform.position[2];

    // scale matrix
    actor->transform.matrix[0][0] = actor->transform.scale[0];
    actor->transform.matrix[1][1] = actor->transform.scale[0];
    actor->transform.matrix[2][2] = actor->transform.scale[0];

    mat4_mul(actor->transform.matrix, actor->transform.matrix, rotation_mat);
}

/*
    Generic contructor for dynamics actors
*/
static void bvri_create_generic_dynactor(bvr_dynamic_actor_t* actor, int flags){
    bvr_create_collider(&actor->collider, NULL, 0);
    actor->collider.transform = &actor->object.transform;

    actor->collider.body.mode = BVR_COLLISION_DISABLE;
    actor->collider.shape = BVR_COLLIDER_EMPTY;
    
    if(BVR_HAS_FLAG(flags, BVR_COLLISION_ENABLE)){
        actor->collider.body.mode |= BVR_COLLISION_ENABLE;
    }

    if(BVR_HAS_FLAG(flags, BVR_DYNACTOR_AGGRESSIVE)){
        actor->collider.body.mode |= BVR_COLLISION_AGRESSIVE; // aggressive
    }
    else {
        actor->collider.body.mode |= BVR_COLLISION_PASSIVE;
    }
}

/*
    Constructor for any dynamic actor.
*/
static void bvri_create_dynamic_actor(bvr_dynamic_actor_t* actor, int flags){
    BVR_ASSERT(actor);

    bvri_create_generic_dynactor(actor, flags);

    // generate bounding boxes by using mesh's vertices
    if(BVR_HAS_FLAG(flags, BVR_DYNACTOR_CREATE_COLLIDER_FROM_BOUNDS)){
        if(actor->mesh.attrib == BVR_MESH_ATTRIB_V3 || 
            actor->mesh.attrib == BVR_MESH_ATTRIB_V3UV2){
            
            BVR_PRINT("cannot generate bounding box for 3d meshes!");
        }
        else if (actor->mesh.vertex_buffer) {
            actor->collider.shape = BVR_COLLIDER_BOX;

            float *vertices = NULL;
            struct bvr_bounds_s bounds;

            // get a pointer to mesh's vertices
            glBindBuffer(GL_ARRAY_BUFFER, actor->mesh.vertex_buffer);
            vertices = glMapBufferRange(GL_ARRAY_BUFFER, 0, actor->mesh.vertex_count, GL_MAP_READ_BIT);
            BVR_ASSERT(vertices);

            vec2_copy(bounds.coords, actor->object.transform.position);
            bounds.width = 0;
            bounds.height = 0;

            // get max bounds
            for (uint64 i = 0; i < actor->mesh.vertex_count; i += actor->mesh.stride)
            {
                if (abs(vertices[i + 0] * 2) > bounds.width)
                {
                    bounds.width = abs(vertices[i + 0] * 2);
                }
                if (abs(vertices[i + 1] * 2) > bounds.height)
                {
                    bounds.height = abs(vertices[i + 1] * 2);
                }
            }

            // allocate geometry
            actor->collider.geometry.elemsize = sizeof(struct bvr_bounds_s);
            actor->collider.geometry.size = 1 * sizeof(struct bvr_bounds_s);
            actor->collider.geometry.data = malloc(actor->collider.geometry.size);
            BVR_ASSERT(actor->collider.geometry.data);

            memcpy(actor->collider.geometry.data, &bounds, actor->collider.geometry.size);

            glUnmapBuffer(GL_ARRAY_BUFFER);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        else {
            BVR_PRINT("failed to copy vertices data!");
        }
    }

   
    if(BVR_HAS_FLAG(flags, BVR_DYNACTOR_TRIANGULATE_COLLIDER_FROM_VERTICES)){
        if(actor->mesh.attrib == BVR_MESH_ATTRIB_V3 || 
            actor->mesh.attrib == BVR_MESH_ATTRIB_V3UV2){
            
            BVR_PRINT("cannot triangulate mesh for 3d meshes!");
        }
        else if(actor->mesh.vertex_buffer) {
            actor->collider.shape = BVR_COLLIDER_TRIARRAY;
            char* vmap;

            // get raw data
            glBindBuffer(GL_ARRAY_BUFFER, actor->mesh.vertex_buffer);
            vmap = glMapBufferRange(GL_ARRAY_BUFFER, 0, actor->mesh.vertex_count, GL_MAP_READ_BIT);
            BVR_ASSERT(vmap);

            bvr_mesh_buffer_t sbuf, tbuf;
            sbuf.type = BVR_FLOAT;
            tbuf.type = BVR_FLOAT;
            tbuf.count = 0;
            sbuf.count = actor->mesh.vertex_count;
            tbuf.data = NULL;
            sbuf.data = vmap;

            bvr_triangulate(&sbuf, &tbuf, 2);
            
            glUnmapBuffer(GL_ARRAY_BUFFER);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            // allocate and copy geometry
            actor->collider.geometry.elemsize = sizeof(vec2) * 3;
            actor->collider.geometry.size = tbuf.count * sizeof(float);
            actor->collider.geometry.data = malloc(actor->collider.geometry.size);
            BVR_ASSERT(actor->collider.geometry.data);

            memcpy(actor->collider.geometry.data, tbuf.data, actor->collider.geometry.size);
        }
    }
}

static void bvri_create_bitmap_layer(bvr_bitmap_layer_t* layer, int flags){
    BVR_ASSERT(layer);

    bvri_create_generic_dynactor((bvr_dynamic_actor_t*)layer, flags);

    /*
        Here, we're trying to bind collision boxes by using a bit map
    */
    if(BVR_HAS_FLAG(flags, BVR_BITMAP_CREATE_COLLIDER)){
        BVR_ASSERT(layer->bitmap.image.pixels);

        struct bvr_bounds_s rects[BVR_BUFFER_SIZE / 2];
        uint8* pixels = malloc(layer->bitmap.image.width * layer->bitmap.image.height);
        memcpy(pixels, layer->bitmap.image.pixels, layer->bitmap.image.width * layer->bitmap.image.height);

        int x, y;
        int rect_width, rect_height, rc;
        int rect_count = 0;

        for (uint64 i = 0; i < layer->bitmap.image.width * layer->bitmap.image.height; i++)
        {
            // skip null pixels
            if(pixels[i] == 0){
                continue;
            }

            y = i / layer->bitmap.image.width;
            x = i % layer->bitmap.image.width;
            rc = 1;
            rect_width = 0;
            rect_height = 0;

            // scan horizontal line until there is a non-null pixel 
            while (x + rect_width < layer->bitmap.image.width && 
                pixels[y * layer->bitmap.image.width + x + rect_width] > 0)
            {
                rect_width++;
            }
            
            // then, scan vertical line from the end of the rectangle until it reaches a non-null pixel
            while (y + rect_height < layer->bitmap.image.height && rc)
            {
                // for each vertical line check if the width is still correct 
                for (uint64 dx = 0; dx < rect_width; dx++)
                {
                    // if not, it's the end of the rectangle
                    if(pixels[(y + rect_height) * layer->bitmap.image.width + x + dx] == 0){
                        rc = 0;
                        break;
                    }
                }
                
                rect_height++;
            }

            // clear the rectangle that we found 
            for (uint64 dy = 0; dy < rect_height; dy++)
            {
                for (uint64 dx = 0; dx < rect_width; dx++)
                {
                    pixels[(y + dy) * layer->bitmap.image.width + x + dx] = 0;
                }
                
            }
            
            // set bounds
            rects[rect_count].coords[0] = (float)x;
            rects[rect_count].coords[1] = (float)y;
            rects[rect_count].width = rect_width - 1;
            rects[rect_count].height = rect_height - 2;
            BVR_PRINTF("%f %f %i %i", rects[rect_count].coords[0], rects[rect_count].coords[1], rects[rect_count].width, rects[rect_count].height);
            rect_count++;


            if(rect_count >= BVR_BUFFER_SIZE / 2){
                BVR_PRINT("skipping colliders, there is too much sub rectangles...");
                break;
            }
        }

        layer->collider.geometry.elemsize = sizeof(struct bvr_bounds_s);
        layer->collider.geometry.size = rect_count * sizeof(struct bvr_bounds_s);
        layer->collider.geometry.data = malloc(layer->collider.geometry.size);
        BVR_ASSERT(layer->collider.geometry.data);
        
        memcpy(layer->collider.geometry.data, &rects, layer->collider.geometry.size);

        free(pixels);
    }
}

void bvr_create_actor(struct bvr_actor_s* actor, const char* name, bvr_actor_type_t type, int flags){
    BVR_ASSERT(actor);

    actor->type = type;
    actor->flags = flags;
    
    actor->active = 1;
    BVR_IDENTITY_VEC3(actor->transform.position);
    BVR_IDENTITY_VEC3(actor->transform.rotation);
    BVR_SCALE_VEC3(actor->transform.scale, 1.0f);

    BVR_IDENTITY_MAT4(actor->transform.matrix);
    bvr_create_string(&actor->name, name);
    bvr_create_uuid(actor->id);

    BVR_PRINTF("created a new actor %x", actor);

    switch (type)
    {
    case BVR_EMPTY_ACTOR:
        /* doing nothing */
        break;
    
    case BVR_LAYER_ACTOR:
        break;
    
    case BVR_BITMAP_ACTOR:
        bvri_create_bitmap_layer((bvr_bitmap_layer_t*)actor, flags);
        break;

    case BVR_STATIC_ACTOR:
        break;
    
    case BVR_DYNAMIC_ACTOR:
        bvri_create_dynamic_actor((bvr_dynamic_actor_t*)actor, flags);
        break;

    default:
        break;
    }
}

void bvr_destroy_actor(struct bvr_actor_s* actor){
    BVR_ASSERT(actor);

    if(actor->name.string){
        bvr_destroy_string(&actor->name);
    }

    switch (actor->type)
    {
    case BVR_EMPTY_ACTOR:
        /* code */
        break;
    case BVR_BITMAP_ACTOR:
        {
            bvr_destroy_mesh(&((bvr_bitmap_layer_t*)actor)->mesh);
            bvr_destroy_shader(&((bvr_bitmap_layer_t*)actor)->shader);
            bvr_destroy_collider(&((bvr_bitmap_layer_t*)actor)->collider);
            bvr_destroy_texture(&((bvr_bitmap_layer_t*)actor)->bitmap);
        }
        break;
    case BVR_LAYER_ACTOR:
        {
            bvr_destroy_mesh(&((bvr_layer_actor_t*)actor)->mesh);
            bvr_destroy_shader(&((bvr_layer_actor_t*)actor)->shader);
            bvr_destroy_layered_texture(&((bvr_layer_actor_t*)actor)->texture);
        }
        break;
    case BVR_STATIC_ACTOR:
        {
            bvr_destroy_mesh(&((bvr_static_actor_t*)actor)->mesh);
            bvr_destroy_shader(&((bvr_static_actor_t*)actor)->shader);
        }
    case BVR_DYNAMIC_ACTOR:
        {
            bvr_destroy_mesh(&((bvr_dynamic_actor_t*)actor)->mesh);
            bvr_destroy_shader(&((bvr_dynamic_actor_t*)actor)->shader);
            bvr_destroy_collider(&((bvr_dynamic_actor_t*)actor)->collider);
        }
        break;
    default:
        break;
    }

    actor->type = BVR_NULL_ACTOR;
    
    BVR_IDENTITY_VEC3(actor->transform.position);
    BVR_IDENTITY_VEC3(actor->transform.rotation);
    BVR_SCALE_VEC3(actor->transform.scale, 1.0f);

    BVR_IDENTITY_MAT4(actor->transform.matrix);
}

static void bvri_draw_layer_actor(bvr_layer_actor_t* actor){
    struct bvr_draw_command_s cmd;
    bvr_shader_uniform_t* texture;

    bvri_update_transform(&actor->object);
    bvr_shader_enable(&actor->shader);

    for (int layer = BVR_BUFFER_COUNT(actor->texture.image.layers); layer >= 0; layer--)
    {
        if(!(((bvr_layer_t*)actor->texture.image.layers.data)[layer]).opacity){
            continue;
        }

        texture = bvr_find_uniform(&actor->shader, "bvr_texture");

        bvr_shader_set_texturei(texture, NULL, &layer);

        bvr_shader_use_uniform(&actor->shader.uniforms[0], &actor->object.transform.matrix[0][0]);

        cmd.order = actor->object.order_in_layer + layer * 2;
        cmd.array_buffer = actor->mesh.array_buffer;
        cmd.vertex_buffer = actor->mesh.vertex_buffer;
        cmd.element_buffer = actor->mesh.element_buffer;
        cmd.attrib_count = actor->mesh.attrib_count;

        cmd.shader = &actor->shader;
        cmd.texture_type = BVR_TEXTURE_2D_ARRAY;
        cmd.texture = (bvr_texture_t*)&actor->texture;
        cmd.draw_mode = BVR_DRAWMODE_TRIANGLES;
        cmd.element_count = actor->mesh.element_count;
        cmd.element_offset = 0;

        cmd.user_data = malloc(sizeof(int));
        BVR_ASSERT(cmd.user_data);

        memcpy(cmd.user_data, &layer, sizeof(int));

        bvr_pipeline_add_draw_cmd(&cmd);
    }

    bvr_shader_disable();
}

void bvr_draw_actor(struct bvr_actor_s* actor, int drawmode){
    // skip actor if 
    if(!actor->active || bvr_is_actor_null(actor)){
        return;
    }

    // empty actors cannot be drawn
    if(actor->type == BVR_EMPTY_ACTOR){
        return;
    }

    // layered actors are drawn differentlty
    if(actor->type == BVR_LAYER_ACTOR){
        bvri_draw_layer_actor((bvr_layer_actor_t*)actor);
        return;
    }

    // update shaders transform
    bvri_update_transform(actor);

    bvr_static_actor_t* sactor = (bvr_static_actor_t*)actor;

    bvr_shader_enable(&sactor->shader);
    bvr_shader_use_uniform(&sactor->shader.uniforms[0], &actor->transform.matrix[0][0]);

    struct bvr_draw_command_s cmd;
    cmd.order = actor->order_in_layer;
    cmd.array_buffer = sactor->mesh.array_buffer;
    cmd.vertex_buffer = sactor->mesh.vertex_buffer;
    cmd.element_buffer = sactor->mesh.element_buffer;
    cmd.attrib_count = sactor->mesh.attrib_count;

    cmd.shader = &sactor->shader;
    cmd.texture = NULL;
    cmd.texture_type = 0;
    cmd.draw_mode = drawmode;
    cmd.user_data = NULL;

    for (uint64 i = 0; i < BVR_BUFFER_COUNT(sactor->mesh.vertex_groups); i++)
    {
        cmd.element_offset = ((bvr_vertex_group_t*)sactor->mesh.vertex_groups.data)[i].element_offset;
        cmd.element_count = ((bvr_vertex_group_t*)sactor->mesh.vertex_groups.data)[i].element_count;
        bvr_pipeline_add_draw_cmd(&cmd);
    }
    

    bvr_shader_disable();
}