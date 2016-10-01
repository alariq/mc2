cmake_minimum_required (VERSION 2.8)
project(gui)

set(SOURCES ${SOURCES}
    libmain.cpp
    libmainc.cpp
    )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -ggdb -Wno-unused-local-typedefs")

add_definitions(-D_DEBUG)
add_definitions(-DLINUX_BUILD)

include_directories(".")

add_library(mc2res SHARED ${SOURCES})
