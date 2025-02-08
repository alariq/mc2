cmake_minimum_required (VERSION 3.10)
project(gui)

set(SOURCES ${SOURCES}
    aanim.cpp
    abutton.cpp
    aedit.cpp
    afont.cpp
    alistbox.cpp
    ascroll.cpp
    asystem.cpp
    logisticsscreen.cpp
    )

include_directories(${COMMON_INCLUDE_DIRS} "../mclib" "../Code")
include_directories(${THIRDPARTY_INCLUDE_DIRS})

add_library(gui ${SOURCES})
