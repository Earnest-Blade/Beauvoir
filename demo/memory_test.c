#include <BVR/buffer.h>

#include <BVR/utils.h>

int main(){
    typedef struct client_s {
        int age;
        int department;
        int id;
    } client_t;

    bvr_pool_t pool;
    bvr_create_pool(&pool, sizeof(client_t*), 5);
    
    client_t clients[4];
    
    for (size_t i = 0; i < 4; i++)
    {
        clients[i].id = i;
        clients[i].age = i + 30;
        clients[i].department = i - 5;

        client_t** c = (client_t**)bvr_pool_alloc(&pool);
        *c = &clients[i];

        BVR_PRINT("alloc");
    }
    
    client_t* c;
    BVR_POOL_FOR_EACH(c, pool){
        BVR_PRINTF("client %x", c->id);
    }

    return 0;
}