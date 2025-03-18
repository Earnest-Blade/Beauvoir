# Beauvoir
> 'Beauvoir' just like Simone de Beauvoir

## Overview
Beauvoir is an OpenGL-based game framework that aims to provide a simple, yet performant and flexible way to create 2D games - but can still create 3D games - . I chose to create it in C to keep the code simple and flexible.

The main purpose is to create pre-rendered games using a layer system. You can either import images from Photoshop using ```.PSD``` files or stack images manually.

The framework is still in early development, but I'm doing my best to improve it!

## Getting Started
Beauvoir can be compiled through CMake into Makefiles and Visual Studio solutions. 

### Building for Windows
To generate Makefiles on Windows you can run 
```cmake -G="MinGW Makefiles" -B build```. 

To generate Visual Studio solutions, use 
```cmake -G="Visual Studio 17 2022" -B build```. Then, open the generated ```.sln``` file or build using ```cmake --build build --config Release```. Then, link the binaries inside Visual Studio.

### Binaries
Precompiled Windows x64 binaries can be found into the [Bin](/bin/) directory. 

```C
/* include all Beauvoir's headers */
#include <BVR/bvr.h>

/* game's context */
static bvr_book_t book;

int main(){
    /* create initial game's context */
    bvr_create_book(&book);
    bvr_create_page(&book.page);

    /* create the window */
    bvr_create_window(&book.window, 800, 800, "Window", 0);
    
    /* create an audio stream */
    bvr_create_audio_stream(&book.audio, BVR_DEFAULT_SAMPLE_RATE, BVR_DEFAULT_AUDIO_BUFFER_SIZE);

    /* main loop */
    while (1)
    {
        /* ask Beauvoir to prepare a new frame */
        bvr_new_frame(&book);

        /* quit the main loop if Beauvoir is not running */
        if(!bvr_is_awake(&book)){
            break;
            
        }

        /* push Beauvoir's graphics to the window */
        bvr_render(&book);
    }
    
    /* free */
    bvr_destroy_book(&book);

    return 0;
}
```
> You can find other demos in the [Demo](/demo/) folder.

## Third Party 
You can find submodules in the [Extern](/extern/) folder.
- [SDL](https://github.com/libsdl-org/SDL)
- [GLAD](https://glad.dav1d.de/)
- [PortAudio](https://github.com/PortAudio/portaudio)
- [Zlib](https://github.com/madler/zlib)
- [Nuklear](https://github.com/vurtun/nuklear)
- [Libpng16](https://github.com/pnggroup/libpng)