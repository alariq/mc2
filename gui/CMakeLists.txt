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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -ggdb -Wno-unused-local-typedefs")
# -Wno-unused-local-typedefs - to disable "typedef was ignored on this declaration"

add_definitions(-D_DEBUG)
add_definitions(-D_ARMOR)
add_definitions(-DUSE_ASSEMBLER_CODE=0)
add_definitions(-DLINUX_BUILD)

#PreprocessorDefinitions="_ARMOR;LAB_ONLY;WIN32;_DEBUG;_WINDOWS"
#PreprocessorDefinitions="NDEBUG;LAB_ONLY;_WINDOWS;WIN32;NOMINMAX"

#include_directories(${mc2_SOURCE_DIR}/LibProject/inc)
include_directories("../GameOS/include" "../mclib" "../Code")

# AVdditionalIncludeDirectories="mlr;..\GameOS\include;..\mclib;..\code"
#DisableSpecificWarnings="4996;4537;4430"

add_library(gui ${SOURCES})
