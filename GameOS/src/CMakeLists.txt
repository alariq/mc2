cmake_minimum_required (VERSION 2.8)
project(windows)

set(SOURCES ${SOURCES}
    winbase.cpp
    winnls.cpp
    winuser.cpp
    string_win.cpp
    stdlib_win.cpp
    tchar.cpp
    mbstring.cpp
    eh.cpp
    io.cpp
    )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -ggdb -Wno-unused-local-typedefs")
# -Wno-unused-local-typedefs - to disable "typedef was ignored on this declaration"

add_definitions(-D_DEBUG)
add_definitions(-DUSE_ASSEMBLER_CODE=0)
add_definitions(-DLINUX_BUILD)

include_directories("../include" )

add_library(windows ${SOURCES})

