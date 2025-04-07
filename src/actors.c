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
static void bvri_create_dynamic_actor(bvr_dynamic_model_t* actor, int flags){
    BVR_ASSERT(actor);

    bvr_create_collider(&actor->collider, NULL, 0);

    BVR_PRINTF("created a new collider %x %x", actor, &actor->collider);

    if(BVR_HAS_FLAG(flags, BVR_DYNACTOR_AGGRESSIVE)){
        actor->collider.body.mode = 1;
    }

    if(BVR_HAS_FLAG(flags, BVR_DYNACTOR_CREATE_COLLIDER_FROM_VERTICES)){
        if(actor->mesh.vertex_buffer){
            float* vertices_ptr;
            float vertices[4];

            glBindBuffer(GL_ARRAY_BUFFER, actor->mesh.vertex_buffer);
            vertices_ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, actor->mesh.vertex_count, GL_MAP_READ_BIT);
            BVR_ASSERT(vertices_ptr);
            BVR_ASSERT(actor->mesh.vertex_count == 16);

            vertices[0] = 0.0f; // top
            vertices[1] = 0.0f;  // left
            vertices[2] = -vertices_ptr[0]; // bottom   // height
            vertices[3] = vertices_ptr[1];  // right    // width

            actor->collider.geometry.elemsize = sizeof(float);
            actor->collider.geometry.size = 4 * sizeof(float);
            actor->collider.geometry.data = malloc(actor->collider.geometry.size);
            BVR_ASSERT(actor->collider.geometry.data);

            memcpy(actor->collider.geometry.data, vertices, actor->collider.geometry.size);

            glUnmapBuffer(GL_ARRAY_BUFFER);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        else {
            BVR_PRINT("failed to copy vertices data!");
        }
    }

    actor->collider.transform = &actor->object.transform;
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
    
    case BVR_STATIC_ACTOR:
        break;
    
    case BVR_DYNAMIC_ACTOR:
        bvri_create_dynamic_actor((bvr_dynamic_model_t*)actor, flags);
        break;
    default:
        break;
    }
}

void bvr_destroy_actor(struct bvr_actor_s* actor){
    bvr_destroy_string(&actor->name);
}

void bvr_draw_static_model(bvr_static_model_t* actor, int drawmode){
    bvri_update_transform((struct bvr_actor_s*)actor);

    bvr_shader_enable(&actor->shader);
    bvr_shader_use_uniform(&actor->shader.uniforms[0], &actor->object.transform.matrix[0][0]);

    bvr_mesh_draw(&actor->mesh, drawmode);
}