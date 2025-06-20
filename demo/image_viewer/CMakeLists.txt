cmake_minimum_required(VERSION 3.16.3)

project(bvr_image_viewer)

set(BVR_TARGET_SHARED ON)

set(BVR_CURRENT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(BVR_DEMO_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(BVR_DEMO_DIRECTORY_BIN ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(BVR_DEMO_DIRECTORY_BUILD ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(BVR_DEMO_DIRECTORY_INCLUDE ${BVR_CURRENT_DIR}/include)

set(BVR_MAIN_FILE "image_viewer.c")

add_subdirectory(${BVR_DEMO_DIRECTORY} ${BVR_DEMO_DIRECTORY_BIN} EXCLUDE_FROM_ALL)

include_directories(${BVR_DEMO_DIRECTORY_INCLUDE})
message("${BVR_DEMO_DIRECTORY_INCLUDE}")
add_executable(bvr_image_viewer ${BVR_MAIN_FILE})

target_link_libraries(bvr_image_viewer Beauvoir)
target_include_directories(bvr_image_viewer PRIVATE ${BVR_DEMO_DIRECTORY_INCLUDE})

set_target_properties(bvr_image_viewer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${BVR_DEMO_DIRECTORY_BIN}"
    ARCHIVE_OUTPUT_DIRECTORY "${BVR_DEMO_DIRECTORY_BUILD}"
    LIBRARY_OUTPUT_DIRECTORY "${BVR_DEMO_DIRECTORY_BUILD}"
)