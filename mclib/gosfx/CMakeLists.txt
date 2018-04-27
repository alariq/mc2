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

include_directories(${COMMON_INCLUDE_DIRS} "../../mclib")

add_library(gosfx ${SOURCES})

# cat gosFX.vcproj | grep ".cpp" | perl -pe 's/.+\"(\w+\.cpp)\".+/\1/g' >> CMakeLists.txt


