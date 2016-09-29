cmake_minimum_required (VERSION 2.8)
project(gameos)

set(SOURCES ${SOURCES}
    gameos.cpp
    gameosmain.cpp
    gameos_graphics.cpp
    )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -ggdb -Wno-unused-local-typedefs -std=c++0x")
# -Wno-unused-local-typedefs - to disable "typedef was ignored on this declaration"

add_definitions(-D_DEBUG)
add_definitions(-D_ARMOR)
add_definitions(-DUSE_ASSEMBLER_CODE=0)
add_definitions(-DLINUX_BUILD)

include_directories("../include"  ".")

add_library(gameos ${SOURCES})

