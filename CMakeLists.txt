cmake_minimum_required(VERSION 3.16.3)

project(bvr)

set(BVR_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(BVR_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(BVR_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(BVR_BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(BVR_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(BVR_DEMO_DIR ${CMAKE_SOURCE_DIR}/demo)

if (NOT DEFINED BVR_MAIN_FILE)
    set(BVR_MAIN_FILE "${BVR_DEMO_DIR}/collider_test.c")
endif()

set(BVR_SDL_LIB ${BVR_LIB_DIR}/libSDL3.dll.a)
if(NOT EXISTS ${BVR_SDL_LIB})
    message(FATAL_ERROR "Cannot find SDL3 '${BVR_SDL_LIB}'")
endif()

set(BVR_PORT_AUDIO_LIB ${BVR_LIB_DIR}/libportaudio.dll.a)
if(NOT EXISTS ${BVR_PORT_AUDIO_LIB})
    message(FATAL_ERROR "Cannot find PortAudio '${BVR_PORT_AUDIO_LIB}'")
endif()

set(BVR_Z_LIB ${BVR_LIB_DIR}/libzlib.dll.a)
if(NOT EXISTS ${BVR_Z_LIB})
    message(FATAL_ERROR "Cannot find Zlib '${BVR_Z_LIB}'")
endif()

set(BVR_PNG_LIB ${BVR_LIB_DIR}/libpng16.dll.a)
if(NOT EXISTS ${BVR_PNG_LIB})
    message(FATAL_ERROR "Cannot find lPng '${BVR_PNG_LIB}'")
endif()

include_directories(${BVR_INCLUDE_DIR})

add_compile_options(-g -w)
add_executable(bvr ${BVR_MAIN_FILE})

find_package(OpenGL REQUIRED)

target_link_libraries(bvr PUBLIC
    ${OPENGL_LIBRARIES}
    ${BVR_PORT_AUDIO_LIB}
    ${BVR_SDL_LIB}
    ${BVR_Z_LIB}
    ${BVR_PNG_LIB}
)
target_include_directories(bvr PUBLIC ${BVR_SOURCE_DIR})

file(GLOB_RECURSE SOURCES "${BVR_SOURCE_DIR}/*.c")
target_sources(bvr PRIVATE ${SOURCES})

set_target_properties(bvr PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${BVR_BIN_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${BVR_BUILD_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${BVR_BUILD_DIR}"
)