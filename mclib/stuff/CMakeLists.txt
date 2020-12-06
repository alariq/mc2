cmake_minimum_required (VERSION 3.10)
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

include_directories(${COMMON_INCLUDE_DIRS})

add_library(stuff ${SOURCES})

# cat MCLib.vcproj | grep ".cpp" | perl -pe 's/.+\"(\w+\.cpp)\".+/\1/g' >> CMakeLists.txt

