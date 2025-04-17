#include <BVR/actors.h>

#include <BVR/file.h>

#include <stdlib.h>

#include <GLAD/glad.h>

/*
    
*/
static void bvri_update_transform(struct bvr_actor_s* actor){
    BVR_ASSERT(actor);

    BVR_IDENTITY_MAT4(actor->transform.matrix);

    // copy translation to the translate matrix
    actor->transform.matrix[3][0] = actor->transform.position[0];
    actor->transform.matrix[3][1] = actor->transform.position[1];
    actor->transform.matrix[3][2] = actor->transform.position[2];
}

/*
    Constructor for any dynamic actor.
*/
static void bvri_create_dynamic_actor(bvr_dynamic_actor_t* actor, int flags){
    BVR_ASSERT(actor);

    bvr_create_collider(&actor->collider, NULL, 0);
    actor->collider.transform = &actor->object.transform;

    actor->collider.body.mode = BVR_COLLISION_DISABLE | BVR_COLLISION_AABB;
    if(BVR_HAS_FLAG(flags, BVR_COLLISION_ENABLE)){
        actor->collider.body.mode = BVR_COLLISION_ENABLE;
    }

    if(BVR_HAS_FLAG(flags, BVR_DYNACTOR_AGGRESSIVE)){
        actor->collider.body.mode |= 0x04; // aggressive
    }

    // generate bounding boxes by using mesh's vertices
    if(BVR_HAS_FLAG(flags, BVR_DYNACTOR_CREATE_COLLIDER_FROM_VERTICES)){
        if(actor->mesh.attrib == BVR_MESH_ATTRIB_V3 || 
            actor->mesh.attrib == BVR_MESH_ATTRIB_V3UV2){
            
            BVR_PRINT("cannot generate bounding box for 3d meshes!");
        }
        else {
            if(actor->mesh.vertex_buffer){
                float* vertices_ptr;
                struct bvr_bounds_s bounds;
    
                // get a pointer to mesh's vertices
                glBindBuffer(GL_ARRAY_BUFFER, actor->mesh.vertex_buffer);
                vertices_ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, actor->mesh.vertex_count, GL_MAP_READ_BIT);
                BVR_ASSERT(vertices_ptr);
    
                bounds.coords[0] = 0.0f;
                bounds.coords[1] = 0.0f;
                bounds.width = 0;
                bounds.height = 0;
    
                // get max bounds
                for (size_t i = 0; i < actor->mesh.vertex_count; i += actor->mesh.stride)
                {
                    if(abs(vertices_ptr[i + 0] * 2) > bounds.width){
                        bounds.width = abs(vertices_ptr[i + 0] * 2);
                    }
                    if(abs(vertices_ptr[i + 1] * 2) > bounds.height){
                        bounds.height = abs(vertices_ptr[i + 1] * 2);
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
    }
}

static void bvri_create_bitmap_layer(bvr_bitmap_layer_t* layer, int flags){
    BVR_ASSERT(layer);

    bvr_create_collider(&layer->collider, NULL, 0);
    layer->collider.transform = &layer->object.transform;

    layer->collider.body.mode = BVR_COLLISION_DISABLE | BVR_COLLISION_AABB;
    if(BVR_HAS_FLAG(flags, BVR_COLLISION_ENABLE)){
        layer->collider.body.mode = BVR_COLLISION_ENABLE;
    }

    if(BVR_HAS_FLAG(flags, BVR_DYNACTOR_AGGRESSIVE)){
        layer->collider.body.mode |= 0x04; // aggressive
    }

    /*
        Here, we're trying to bind collision boxes by using a bit map
    */
    if(BVR_HAS_FLAG(flags, BVR_BITMAP_CREATE_COLLIDER)){
        BVR_ASSERT(layer->bitmap.image.pixels);

        struct bvr_bounds_s rects[BVR_BUFFER_SIZE / 2];
        uint8_t* pixels = malloc(layer->bitmap.image.width * layer->bitmap.image.height);
        memcpy(pixels, layer->bitmap.image.pixels, layer->bitmap.image.width * layer->bitmap.image.height);

        int x, y;
        int rect_width, rect_height, rc;
        int rect_count = 0;

        for (size_t i = 0; i < layer->bitmap.image.width * layer->bitmap.image.height; i++)
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
                for (size_t dx = 0; dx < rect_width; dx++)
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
            for (size_t dy = 0; dy < rect_height; dy++)
            {
                for (size_t dx = 0; dx < rect_width; dx++)
                {
                    pixels[(y + dy) * layer->bitmap.image.width + x + dx] = 0;
                }
                
            }
            
            // set bounds
            rects[rect_count].coords[0] = (float)x;
            rects[rect_count].coords[1] = (float)y;
            rects[rect_count].width = rect_width - 1;
            rects[rect_count].height = rect_height - 2;
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
    actor->id = (uint32_t)actor;
    actor->flags = flags;
    
    BVR_IDENTITY_VEC3(actor->transform.position);
    BVR_IDENTITY_VEC3(actor->transform.rotation);
    BVR_SCALE_VEC3(actor->transform.scale, 1.0f);

    BVR_IDENTITY_MAT4(actor->transform.matrix);
    bvr_create_string(&actor->name, name);

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

    if(actor->name.data){
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
}

void bvr_draw_actor(bvr_static_actor_t* actor, int drawmode){
    bvri_update_transform((struct bvr_actor_s*)actor);

    bvr_shader_enable(&actor->shader);
    bvr_shader_use_uniform(&actor->shader.uniforms[0], &actor->object.transform.matrix[0][0]);

    bvr_mesh_draw(&actor->mesh, drawmode);
}