#pragma once

#include <stdint.h>

#define BVR_KEY_UNKNOWN       0 
#define BVR_KEY_SPACE         44    //SDL_SCANCODE_SPACE
#define BVR_KEY_MENU          118   //SDL_SCANCODE_MENU
#define BVR_KEY_APOSTROPHE    52    //SDL_SCANCODE_APOSTROPHE
#define BVR_KEY_COMMA         54    //SDL_SCANCODE_COMMA
#define BVR_KEY_MINUS         45    //SDL_SCANCODE_MINUS
#define BVR_KEY_PERIOD        55    //SDL_SCANCODE_PERIOD
#define BVR_KEY_SLASH         22    //SDL_SCANCODE_SLASH
#define BVR_KEY_0             39    //SDL_SCANCODE_0
#define BVR_KEY_1             30    //SDL_SCANCODE_1
#define BVR_KEY_2             31    //SDL_SCANCODE_2
#define BVR_KEY_3             32    //SDL_SCANCODE_3
#define BVR_KEY_4             33    //SDL_SCANCODE_4
#define BVR_KEY_5             34    //SDL_SCANCODE_5
#define BVR_KEY_6             35    //SDL_SCANCODE_6
#define BVR_KEY_7             36    //SDL_SCANCODE_7
#define BVR_KEY_8             37    //SDL_SCANCODE_8
#define BVR_KEY_9             38    //SDL_SCANCODE_9
#define BVR_KEY_SEMICOLON     51    //SDL_SCANCODE_SEMICOLON
#define BVR_KEY_EQUAL         46    //SDL_SCANCODE_EQUALS
#define BVR_KEY_A             4     //SDL_SCANCODE_A
#define BVR_KEY_B             5     //SDL_SCANCODE_B
#define BVR_KEY_C             6     //SDL_SCANCODE_C
#define BVR_KEY_D             7     //SDL_SCANCODE_D
#define BVR_KEY_E             8     //SDL_SCANCODE_E
#define BVR_KEY_F             9     //SDL_SCANCODE_F
#define BVR_KEY_G             10    //SDL_SCANCODE_G
#define BVR_KEY_H             11    //SDL_SCANCODE_H
#define BVR_KEY_I             12    //SDL_SCANCODE_I
#define BVR_KEY_J             13    //SDL_SCANCODE_J
#define BVR_KEY_K             16    //SDL_SCANCODE_K
#define BVR_KEY_L             17    //SDL_SCANCODE_L
#define BVR_KEY_M             18    //SDL_SCANCODE_M
#define BVR_KEY_N             19    //SDL_SCANCODE_N
#define BVR_KEY_O             18    //SDL_SCANCODE_O
#define BVR_KEY_P             19    //SDL_SCANCODE_P
#define BVR_KEY_Q             20    //SDL_SCANCODE_Q
#define BVR_KEY_R             21    //SDL_SCANCODE_R
#define BVR_KEY_S             22    //SDL_SCANCODE_S
#define BVR_KEY_T             23    //SDL_SCANCODE_T
#define BVR_KEY_U             24    //SDL_SCANCODE_U
#define BVR_KEY_V             25    //SDL_SCANCODE_V
#define BVR_KEY_W             26    //SDL_SCANCODE_W
#define BVR_KEY_X             27    //SDL_SCANCODE_X
#define BVR_KEY_Y             28    //SDL_SCANCODE_Y
#define BVR_KEY_Z             29    //SDL_SCANCODE_Z
#define BVR_KEY_LEFT_BRACKET  47    //SDL_SCANCODE_LEFTBRACKET
#define BVR_KEY_BACKSLASH     49    //SDL_SCANCODE_BACKSLASH
#define BVR_KEY_RIGHT_BRACKET 48    //SDL_SCANCODE_RIGHTBRACKET
#define BVR_KEY_GRAVE_ACCENT  53    //SDL_SCANCODE_GRAVE
//#define BVR_KEY_WORLD_1       0
//#define BVR_KEY_WORLD_2       0
#define BVR_KEY_ESCAPE        41    //SDL_SCANCODE_ESCAPE
#define BVR_KEY_ENTER         88    //SDL_SCANCODE_KP_ENTER
#define BVR_KEY_TAB           43    //SDL_SCANCODE_TAB
#define BVR_KEY_BACKSPACE     42    //SDL_SCANCODE_BACKSPACE
#define BVR_KEY_INSERT        73    //SDL_SCANCODE_INSERT
#define BVR_KEY_DELETE        76    //SDL_SCANCODE_DELETE
#define BVR_KEY_RIGHT         79    //SDL_SCANCODE_RIGHT
#define BVR_KEY_LEFT          80    //SDL_SCANCODE_LEFT
#define BVR_KEY_DOWN          81    //SDL_SCANCODE_DOWN
#define BVR_KEY_UP            82    //SDL_SCANCODE_UP
#define BVR_KEY_PAGE_UP       75    //SDL_SCANCODE_PAGEUP
#define BVR_KEY_PAGE_DOWN     78    //SDL_SCANCODE_PAGEDOWN
#define BVR_KEY_HOME          71    //SDL_SCANCODE_HOME
#define BVR_KEY_END           77    //SDL_SCANCODE_END
#define BVR_KEY_CAPS_LOCK     6     //SDL_SCANCODE_CAPSLOCK
#define BVR_KEY_SCROLL_LOCK   71
#define BVR_KEY_NUM_LOCK      83
#define BVR_KEY_PRINT_SCREEN  70
#define BVR_KEY_PAUSE         72
#define BVR_KEY_F1            58    //SDL_SCANCODE_F1 
#define BVR_KEY_F2            59    //SDL_SCANCODE_F2 
#define BVR_KEY_F3            60    //SDL_SCANCODE_F3 
#define BVR_KEY_F4            61    //SDL_SCANCODE_F4 
#define BVR_KEY_F5            62    //SDL_SCANCODE_F5 
#define BVR_KEY_F6            63    //SDL_SCANCODE_F6 
#define BVR_KEY_F7            64    //SDL_SCANCODE_F7 
#define BVR_KEY_F8            65    //SDL_SCANCODE_F8 
#define BVR_KEY_F9            66    //SDL_SCANCODE_F9 
#define BVR_KEY_F10           67    //SDL_SCANCODE_F10
#define BVR_KEY_F11           68    //SDL_SCANCODE_F11
#define BVR_KEY_F12           69    //SDL_SCANCODE_F12
#define BVR_KEY_KP_0          98    //SDL_SCANCODE_KP_0
#define BVR_KEY_KP_1          89    //SDL_SCANCODE_KP_1
#define BVR_KEY_KP_2          90    //SDL_SCANCODE_KP_2
#define BVR_KEY_KP_3          91    //SDL_SCANCODE_KP_3
#define BVR_KEY_KP_4          92    //SDL_SCANCODE_KP_4
#define BVR_KEY_KP_5          93    //SDL_SCANCODE_KP_5
#define BVR_KEY_KP_6          94    //SDL_SCANCODE_KP_6
#define BVR_KEY_KP_7          95    //SDL_SCANCODE_KP_7
#define BVR_KEY_KP_8          96    //SDL_SCANCODE_KP_8
#define BVR_KEY_KP_9          97    //SDL_SCANCODE_KP_9
#define BVR_KEY_KP_DECIMAL    220   //SDL_SCANCODE_KP_DECIMAL
#define BVR_KEY_KP_DIVIDE     84    //SDL_SCANCODE_KP_DIVIDE
#define BVR_KEY_KP_MULTIPLY   85    //SDL_SCANCODE_KP_MULTIPLY
#define BVR_KEY_KP_SUBTRACT   212   //SDL_SCANCODE_KP_MEMSUBTRACT
#define BVR_KEY_KP_ADD        211   //SDL_SCANCODE_KP_MEMADD
#define BVR_KEY_KP_ENTER      88    //SDL_SCANCODE_KP_ENTER
#define BVR_KEY_KP_EQUAL      103

#define BVR_KEY_LEFT_SHIFT    340   //CUSTOM KEYCODES   
#define BVR_KEY_LEFT_CONTROL  341   //CUSTOM KEYCODES
#define BVR_KEY_LEFT_ALT      342   //CUSTOM KEYCODES
#define BVR_KEY_LEFT_SUPER    343   //CUSTOM KEYCODES
#define BVR_KEY_RIGHT_SHIFT   344   //CUSTOM KEYCODES
#define BVR_KEY_RIGHT_CONTROL 345   //CUSTOM KEYCODES
#define BVR_KEY_RIGHT_ALT     346   //CUSTOM KEYCODES
#define BVR_KEY_RIGHT_SUPER   347   //CUSTOM KEYCODES

#define BVR_MOUSE_BUTTON_1 1
#define BVR_MOUSE_BUTTON_2 2
#define BVR_MOUSE_BUTTON_3 3
#define BVR_MOUSE_BUTTON_4 4
#define BVR_MOUSE_BUTTON_5 5
#define BVR_MOUSE_BUTTON_6 6
#define BVR_MOUSE_BUTTON_7 7
#define BVR_MOUSE_BUTTON_8 8
#define BVR_MOUSE_BUTTON_DOUBLE 9

#define BVR_MOUSE_BUTTON_LAST BVR_MOUSE_BUTTON_8
#define BVR_MOUSE_BUTTON_LEFT BVR_MOUSE_BUTTON_1
#define BVR_MOUSE_BUTTON_RIGHT BVR_MOUSE_BUTTON_2
#define BVR_MOUSE_BUTTON_MIDDLE BVR_MOUSE_BUTTON_3

#define BVR_KEYBOARD_SIZE 512
#define BVR_MOUSE_SIZE 16

#define BVR_PRESSED 1
#define BVR_RELEASE 2

typedef struct bvr_framebuffer_s {
    int width;
    int height;
    int id;
} bvr_framebuffer_t;

typedef struct bvr_window_s {
    void* handle;
    void* context;

    bvr_framebuffer_t framebuffer;
    struct {
        char keys[BVR_KEYBOARD_SIZE];
        char buttons[BVR_MOUSE_SIZE];
        char text_input[4];
        float sensivity;
        float scroll;

        float motion[2]; // mouse motion
        float rel_motion[2]; // relative mouse motion
        float prev_motion[2]; // previous mouse motion

        int grab;
    } inputs;

    int events;
    int awake;
} bvr_window_t;

int bvr_create_window(bvr_window_t* window, int width, int height, const char* title, int flags);

void bvr_window_poll_events(bvr_window_t* window);
void bvr_window_push_buffers(bvr_window_t* window);

void bvr_destroy_window(bvr_window_t* window);

int bvr_key_down(bvr_window_t* window, uint16_t key);
int bvr_button_down(bvr_window_t* window, uint16_t button);
void bvr_mouse_position(bvr_window_t* window, float* x, float* y);

/*
    Returns the number of milliseconds since SDL has started.
*/
uint64_t bvr_frames();

/*
    Wait a specified number of milliseconds before returning.
*/
void bvr_delay(uint64_t ms);