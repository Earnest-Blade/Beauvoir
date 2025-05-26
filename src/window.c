#include <BVR/window.h>

#include <BVR/utils.h>

#include <string.h>
#include <memory.h>

#include <SDL3/SDL.h>
#include <glad/glad.h>

int bvr_create_window(bvr_window_t* window, int width, int height, const char* title, int flags){
    BVR_ASSERT(window);
    BVR_ASSERT(width > 0 && height > 0);

    // doesn't init SDL_INIT_VIDEO, otherwise Nuklear doesn't work :/
    BVR_ASSERT(SDL_Init(SDL_INIT_EVENTS) == 1);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    window->handle = NULL;
    window->context = NULL;
    
    int window_flags = SDL_WINDOW_OPENGL;
    window_flags |= SDL_WINDOW_RESIZABLE;

    // create a new window
    window->handle = SDL_CreateWindow(title, width, height, window_flags);
    BVR_ASSERT(window->handle);

    // create a new context
    window->context = SDL_GL_CreateContext(window->handle);
    BVR_ASSERT(window->context);
    SDL_GL_MakeCurrent(window->handle, window->context);

    memset(window->inputs.keys, 0, BVR_KEYBOARD_SIZE * sizeof(char));
    memset(window->inputs.buttons, 0, BVR_MOUSE_SIZE * sizeof(char));
    window->inputs.sensivity = 1.0f;
    window->inputs.scroll = 0.0f;
    window->inputs.grab = 0;

    SDL_GetMouseState(&window->inputs.mouse[0], &window->inputs.mouse[1]);

    // initialize GLAD
    BVR_ASSERT(gladLoadGLES2Loader((GLADloadproc)SDL_GL_GetProcAddress));
    BVR_PRINT(glGetString(GL_VERSION));

    // create framebuffer
    if(BVR_HAS_FLAG(flags, BVR_WINDOW_USER_FRAMEBUFFER)){
        bvr_create_framebuffer(&window->framebuffer, width, height, BVR_WINDOW_FRAMEBUFFER_PATH);
    }
    else {
        bvr_create_framebuffer(&window->framebuffer, width, height, NULL);
    }

    window->awake = 1;
}

void bvr_window_poll_events(bvr_window_t* window){
    SDL_Event event;

    SDL_StartTextInput(window->handle);

    window->events = 0;
    while(SDL_PollEvent(&event)){
        window->events |= event.type;

        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            window->awake = 0;
            break;
        
        case SDL_EVENT_KEY_UP:
        case SDL_EVENT_KEY_DOWN:
            {
                int down = (event.type == SDL_EVENT_KEY_DOWN) + BVR_KEY_UP;
                int kevent = down;
                if(!event.key.repeat && down - BVR_KEY_UP){
                    kevent = BVR_KEY_PRESSED;
                }
                
                if(event.key.mod){
                    switch (event.key.mod)
                    {
                    case SDLK_LSHIFT:
                        window->inputs.keys[BVR_KEY_LEFT_SHIFT] = down;
                        break;
                    case SDLK_RSHIFT:
                        window->inputs.keys[BVR_KEY_RIGHT_SHIFT] = down;
                        break;
                    case SDLK_LCTRL:
                        window->inputs.keys[BVR_KEY_LEFT_CONTROL] = down;
                        break;
                    case SDLK_RCTRL:
                        window->inputs.keys[BVR_KEY_RIGHT_CONTROL] = down;
                        break;
                    case SDLK_LALT:
                        window->inputs.keys[BVR_KEY_LEFT_ALT] = down;
                        break;
                    case SDLK_RALT:
                        window->inputs.keys[BVR_KEY_RIGHT_ALT] = down;
                        break;
                    default: break;
                    }
                }
                
                window->inputs.keys[event.key.scancode] = kevent;
            }
            break;
        case SDL_EVENT_MOUSE_BUTTON_UP:
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                int down = event.type == SDL_EVENT_MOUSE_BUTTON_DOWN;
                window->inputs.buttons[event.button.button] = down + 1;
                window->inputs.buttons[BVR_MOUSE_BUTTON_DOUBLE] = (event.button.clicks > 1);
            }
            break;
        case SDL_EVENT_MOUSE_MOTION:
            {
                window->inputs.prev_motion[0] = window->inputs.motion[0];
                window->inputs.prev_motion[1] = window->inputs.motion[1];
                window->inputs.relative_motion[0] = event.motion.xrel;
                window->inputs.relative_motion[1] = event.motion.yrel;
                window->inputs.motion[0] = event.motion.x;
                window->inputs.motion[1] = event.motion.y;
                
                window->inputs.mouse[0] += window->inputs.relative_motion[0];
                window->inputs.mouse[1] += window->inputs.relative_motion[1];
            }
            break;
        case SDL_EVENT_MOUSE_WHEEL:
            {
                window->inputs.scroll = event.wheel.y;
            }
            break;
        case SDL_EVENT_WINDOW_RESIZED:
            {
                if(event.display.data1 & event.display.data2){
                    window->framebuffer.width = event.display.data1;
                    window->framebuffer.height = event.display.data2;
                }
            }
            break;
        case SDL_EVENT_TEXT_INPUT:
            {
                for (size_t i = 0; i < sizeof(window->inputs.text_input); i++)
                {
                    window->inputs.text_input[i] = event.text.text[i];
                    if(event.text.text[i] == '\0') {
                        break;
                    }
                }
            }
        default:
            break;
        }
    }

    SDL_StopTextInput(window->handle);

    glViewport(0, 0, window->framebuffer.width, window->framebuffer.height);
}

void bvr_window_push_buffers(bvr_window_t* window){
    SDL_GL_SwapWindow(window->handle);
}

void bvr_destroy_window(bvr_window_t* window){
    bvr_destroy_framebuffer(&window->framebuffer);

    SDL_DestroyWindow(window->handle);
    SDL_Quit();

    window->context = NULL;
    window->handle = NULL;
}

int bvr_key_presssed(bvr_window_t* window, uint16_t key){
    return window->inputs.keys[key] == BVR_KEY_PRESSED;
}

int bvr_key_down(bvr_window_t* window, uint16_t key){
    return window->inputs.keys[key] == BVR_KEY_DOWN || window->inputs.keys[key] == BVR_KEY_PRESSED;
}

int bvr_button_down(bvr_window_t* window, uint16_t button){
    return window->inputs.buttons[button] == BVR_KEY_DOWN;
}

void bvr_mouse_position(bvr_window_t* window, float* x, float* y){
    SDL_GetMouseState(x, y);
}

void bvr_mouse_relative_position(bvr_window_t* window, float* x, float *y){
    *x = window->inputs.relative_motion[0];
    *y = window->inputs.relative_motion[1];
}

uint64_t bvr_frames(){
    return SDL_GetTicks();
}

void bvr_delay(uint64_t ms){
    SDL_Delay(ms);
}