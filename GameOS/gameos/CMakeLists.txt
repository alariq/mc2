cmake_minimum_required (VERSION 2.8)
project(gameos)

set(SOURCES ${SOURCES}
    gameos.cpp
    gameosmain.cpp
    gameos_graphics.cpp
    gameos_res.cpp
    gameos_fileio.cpp
    gameos_input.cpp
    gameos_debugging.cpp
    gameos_sound.cpp
    gos_render.cpp
    gos_font.cpp
    gos_input.cpp

    utils/stream.cpp
    utils/camera.cpp
    utils/gl_utils.cpp
    utils/gl_render_constants.cpp
    utils/Image.cpp
    utils/logging.cpp
    utils/shader_builder.cpp
    utils/matrix.cpp
    utils/vec.cpp
    )
# add -Wall sometime later
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -ggdb -Wno-unused-local-typedefs -Wunused-variable -std=c++0x -Wno-unknown-warning-option")
# -Wno-unused-local-typedefs - to disable "typedef was ignored on this declaration"

add_definitions(-D_DEBUG)
add_definitions(-D_ARMOR)
add_definitions(-DUSE_ASSEMBLER_CODE=0)
add_definitions(-DLINUX_BUILD)

include_directories("../include"  ".")

add_library(gameos ${SOURCES})

