cmake_minimum_required (VERSION 2.8)
project(gameos)

set(SOURCES ${SOURCES}
    gameos.cpp
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
    utils/timing.cpp
    utils/string_utils.cpp
    utils/file_utils.cpp
    )

set(MAIN_SRC gameosmain.cpp)

#find_package(SDL2 REQUIRED PATHS "/opt" NO_DEFAULT_PATH)
#include_directories("${SDL2_PREFIX}/include")

include_directories("../include"  ".")

add_library(gameos ${SOURCES})
add_library(gameos_main ${MAIN_SRC})

