cmake_minimum_required (VERSION 2.8)
project(stuff)

set(SOURCES ${SOURCES}
    style.cpp
    trace.cpp
    affinematrix.cpp
    affinematrix_test.cpp
    angle.cpp
    angle_test.cpp
    color.cpp
    extentbox.cpp
    line.cpp
    linearmatrix.cpp
    linearmatrix_test.cpp
    matrix.cpp
    matrix_test.cpp
    motion.cpp
    normal.cpp
    normal_test.cpp
    obb.cpp
    origin.cpp
    origin_test.cpp
    plane.cpp
    point3d.cpp
    point3d_test.cpp
    polar.cpp
    random.cpp
    random_test.cpp
    ray.cpp
    ray_test.cpp
    rotation.cpp
    rotation_test.cpp
    scalar.cpp
    sphere.cpp
    unitvector.cpp
    unitvector_test.cpp
    vector3d.cpp
    vector3d_test.cpp
    vector4d.cpp
    vector4d_test.cpp
    matrixstack.cpp
    memoryblock.cpp
    memoryblock_test.cpp
    mstring.cpp
    mstring_test.cpp
    chain.cpp
    chain_test.cpp
    hash.cpp
    hash_test.cpp
    iterator.cpp
    link.cpp
    node.cpp
    plug.cpp
    registeredclass.cpp
    safechain.cpp
    safechain_test.cpp
    safesocket.cpp
    slot.cpp
    socket.cpp
    sortedchain.cpp
    sortedchain_test.cpp
    sortedsocket.cpp
    table.cpp
    table_test.cpp
    tree.cpp
    tree_test.cpp
    database.cpp
    filestream.cpp
    filestream_test.cpp
    filestreammanager.cpp
    memorystream.cpp
    memorystream_test.cpp
    namelist.cpp
    namelist_test.cpp
    notationfile.cpp
    notationfile_test.cpp
    note.cpp
    page.cpp
    stuff.cpp
    stuffheaders.cpp
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
include_directories("../../GameOS/include")

#DisableSpecificWarnings="4996;4537;4731;4430"

add_library(stuff ${SOURCES})

# cat MCLib.vcproj | grep ".cpp" | perl -pe 's/.+\"(\w+\.cpp)\".+/\1/g' >> CMakeLists.txt

