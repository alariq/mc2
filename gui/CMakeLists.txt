cmake_minimum_required (VERSION 2.8)
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

include_directories("../GameOS/include" "../mclib" "../Code")

add_library(gui ${SOURCES})
