#include <BVR/buffer.h>
#include <BVR/graphics.h>

#include <BVR/utils.h>

int main(){
    typedef struct client_s {
        int age;
        int department;
        int id;
    } client_t;

    bvr_pool_t pool;
    bvr_create_pool(&pool, sizeof(client_t), 5);
    
    client_t* c0 = bvr_pool_alloc(&pool);
    client_t* c1 = bvr_pool_alloc(&pool);

    c0->age = 55;
    c0->department = 54;
    c0->id = 1902;

    c1->age = 555;
    c1->department = 554;
    c1->id = 19502;

    BVR_PRINTF("c0 %i %i %i", c0->age, c0->department, c0->id);
    BVR_PRINTF("c1 %i %i %i", c1->age, c1->department, c1->id);

    bvr_pool_free(&pool, c0);
    c0 = bvr_pool_alloc(&pool);

    c0->age = 555;
    c0->department = 554;
    c0->id = 19502;

    BVR_PRINTF("c0 %i %i %i", c0->age, c0->department, c0->id);

    bvr_destroy_pool(&pool);

    return 0;
}