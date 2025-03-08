#include <BVR/actors.h>

#include <BVR/file.h>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

void bvr_model_draw(bvr_model_t* model, int drawmode){
    bvr_shader_enable(&model->shader);

    /* calculate model's matrix */
    bvr_identity_mat4(model->transform.matrix);
    model->transform.matrix[3][0] = model->transform.position[0];
    model->transform.matrix[3][1] = model->transform.position[1];
    model->transform.matrix[3][2] = model->transform.position[2];
    
    bvr_shader_use_uniform(&model->shader.uniforms[0], &model->transform.matrix);
    bvr_mesh_draw(&model->mesh, drawmode);
    bvr_shader_disable();
}