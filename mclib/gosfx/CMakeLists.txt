cmake_minimum_required (VERSION 2.8)
project(gosfx)

set(SOURCES ${SOURCES}
    effect.cpp
    effectlibrary.cpp
    fcurve.cpp
    cardcloud.cpp
    debriscloud.cpp
    effectcloud.cpp
    particlecloud.cpp
    pertcloud.cpp
    pointcloud.cpp
    shapecloud.cpp
    shardcloud.cpp
    spinningcloud.cpp
    card.cpp
    shape.cpp
    singleton.cpp
    tube.cpp
    pointlight.cpp
    gosfx.cpp
    gosfxheaders.cpp
)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -ggdb -Wno-unused-local-typedefs")
# -Wno-unused-local-typedefs - to disable "typedef was ignored on this declaration"

add_definitions(-D_DEBUG)
add_definitions(-D_ARMOR)
add_definitions(-DUSE_ASSEMBLER_CODE=0)
add_definitions(-DLINUX_BUILD)

#PreprocessorDefinitions="_ARMOR;LAB_ONLY;WIN32;_DEBUG;_WINDOWS"
#PreprocessorDefinitions="NDEBUG;LAB_ONLY;_WINDOWS;WIN32;NOMINMAX"

#include_directories(${mc2_SOURCE_DIR}/LibProject/inc)
include_directories("../../GameOS/include" "../../mclib")

#DisableSpecificWarnings="4996;4537;4731;4430"

add_library(gosfx ${SOURCES})

# cat gosFX.vcproj | grep ".cpp" | perl -pe 's/.+\"(\w+\.cpp)\".+/\1/g' >> CMakeLists.txt


