#include <BVR/buffer.h>

#include <BVR/utils.h>

int main(){
    bvr_string_t text;
    bvr_create_string(&text, "hello <3");

    BVR_PRINT(bvr_string_get(&text));

    return 0;
}