cmake_minimum_required(VERSION 3.16.3)

project(bvr)

if(NOT DEFINED BVR_TARGET_SHARED)
    set(BVR_TARGET_SHARED OFF)
endif()

# Setting up Beauvoir paths
if(NOT DEFINED BVR_CURRENT_DIR)
    set(BVR_CURRENT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

set(BVR_SOURCE_DIR ${BVR_CURRENT_DIR}/src)
set(BVR_INCLUDE_DIR ${BVR_CURRENT_DIR}/include)
set(BVR_LIB_DIR ${BVR_CURRENT_DIR}/lib)
set(BVR_BUILD_DIR ${BVR_CURRENT_DIR}/build)
set(BVR_BINARY_DIR ${BVR_CURRENT_DIR}/bin)

set(BVR_DEMO_DIR ${BVR_CURRENT_DIR}/dev)

# Define main file
if(NOT ${BVR_TARGET_SHARED})
    if (NOT DEFINED BVR_MAIN_FILE)
        set(BVR_MAIN_FILE "${BVR_DEMO_DIR}/collider_test.c")
    endif()
endif()

## Check for modules
set(BVR_THIRD_PARTY_MODULES_NAMES "SDL" "PortAudio" "Zlib" "LPng")
foreach(BVR_THIRD_PARTY_MODULES_ITEM ${BVR_THIRD_PARTY_MODULES_NAMES})
    set(BVR_THIRD_PARTY_MODULES_ITEM_PATH "${BVR_CURRENT_DIR}/extern/${BVR_THIRD_PARTY_MODULES_ITEM}")

    if(EXISTS ${BVR_THIRD_PARTY_MODULES_ITEM_PATH})
        message("found ${BVR_THIRD_PARTY_MODULES_ITEM} (${BVR_THIRD_PARTY_MODULES_ITEM_PATH})")
    else()
        message(FATAL_ERROR "cannot find ${BVR_THIRD_PARTY_MODULES_ITEM} (${BVR_THIRD_PARTY_MODULES_ITEM_PATH})")
    endif()
endforeach()

## add OpenGL package
set(BVR_THIRD_PARTY_PACKAGES "OpenGL")
foreach(BVR_THIRD_PARTY_PACKAGES_ITEM ${BVR_THIRD_PARTY_PACKAGES})
    find_package(${BVR_THIRD_PARTY_PACKAGES_ITEM} REQUIRED)
endforeach()

## SETUP FOR STATIC LIB
#
set(BVR_SDL_LIB ${BVR_LIB_DIR}/SDL3.lib)
if(NOT EXISTS ${BVR_SDL_LIB})
    message(FATAL_ERROR "Cannot find SDL3 '${BVR_SDL_LIB}'")
endif()

set(BVR_PORT_AUDIO_LIB ${BVR_LIB_DIR}/portaudio.lib)
if(NOT EXISTS ${BVR_PORT_AUDIO_LIB})
    message(FATAL_ERROR "Cannot find PortAudio '${BVR_PORT_AUDIO_LIB}'")
endif()

set(BVR_Z_LIB ${BVR_LIB_DIR}/zsd.lib)
if(NOT EXISTS ${BVR_Z_LIB})
    message(FATAL_ERROR "Cannot find Zlib '${BVR_Z_LIB}'")
endif()

set(BVR_PNG_LIB ${BVR_LIB_DIR}/libpng16d.lib)
if(NOT EXISTS ${BVR_PNG_LIB})
    message(FATAL_ERROR "Cannot find lPng '${BVR_PNG_LIB}'")
endif()

include_directories(${BVR_INCLUDE_DIR})

add_compile_options(-g -w)

if(${BVR_TARGET_SHARED})
    add_library(bvr SHARED)
else()
    add_executable(bvr ${BVR_MAIN_FILE})
endif()


target_link_libraries(bvr PUBLIC
    ${OPENGL_LIBRARIES}
    ${BVR_SDL_LIB}
    ${BVR_PORT_AUDIO_LIB}
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