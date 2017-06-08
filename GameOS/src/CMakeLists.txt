cmake_minimum_required (VERSION 2.8)
project(windows)

set(SOURCES ${SOURCES}
    platform_winbase.cpp
    platform_winnls.cpp
    platform_winuser.cpp
    platform_stdlib.cpp
    platform_tchar.cpp
    platform_mbstring.cpp
    platform_eh.cpp
    platform_io.cpp
    platform_mmsystem.cpp
    platform_str.cpp
)

include_directories("../include" )

add_library(windows ${SOURCES})

