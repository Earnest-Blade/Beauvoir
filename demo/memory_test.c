#include <BVR/buffer.h>

#include <BVR/utils.h>

int main(){
    typedef struct client_s {
        int age;
        int department;
        int id;
    } client_t;

    bvr_pool_t pool;
    bvr_create_pool(&pool, sizeof(client_t*), 25);
    
    client_t clients[20];
    
    for (size_t i = 0; i < 20; i++)
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
        if(!c) break;
        
        BVR_PRINTF("client %i", c->age);
    }

    return 0;
}