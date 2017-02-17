cmake_minimum_required (VERSION 2.8)
project(mclib)

set(SOURCES ${SOURCES}
    alphapalette.cpp
    bitflag.cpp
    cident.cpp
    cmponent.cpp
    color.cpp
    debugging.cpp
    err.cpp
    floathelp.cpp
    heap.cpp
    lzcomp.cpp
    lzdecomp.cpp
    mathfunc.cpp
    mouse.cpp
    msl.cpp
    paths.cpp
    pqueue.cpp
    routines.cpp
    scale.cpp
    sortlist.cpp
    tgainfo.cpp
    tgl.cpp
    timing.cpp
    txmmgr.cpp
    userinput.cpp
    vport.cpp
    weaponfx.cpp
    csvfile.cpp
    fastfile.cpp
    ffile.cpp
    file.cpp
    gamelog.cpp
    inifile.cpp
    packet.cpp
    celement.cpp
    celine.cpp
    cellip.cpp
    cepane.cpp
    cevfx.cpp
    vfx_ellipse.cpp
    vfx_map_polygon.cpp
    vfx_transform.cpp
    vfx_translatedraw.cpp
    vfxtile.cpp
    abldbug.cpp
    abldecl.cpp
    ablenv.cpp
    ablerr.cpp
    ablexec.cpp
    ablexpr.cpp
    ablrtn.cpp
    ablscan.cpp
    ablstd.cpp
    ablstmt.cpp
    ablsymt.cpp
    ablxexpr.cpp
    ablxstd.cpp
    ablxstmt.cpp
    camera.cpp
    clouds.cpp
    crater.cpp
    mapdata.cpp
    quad.cpp
    terrain.cpp
    terrtxm.cpp
    terrtxm2.cpp
    txmconv.cpp
    appear.cpp
    apprtype.cpp
    bdactor.cpp
    genactor.cpp
    gvactor.cpp
    mech3d.cpp
    soundsys.cpp
    move.cpp
    echarstring.cpp
    estring.cpp
    ewcharstring.cpp
    tacmap.cpp
    utilities.cpp
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
include_directories("../GameOS/include" "./mclib" "../Editor" "../DirectX8/include" "./")

#DisableSpecificWarnings="4996;4537;4731;4430"

add_library(mclib ${SOURCES})

# cat MCLib.vcproj | grep ".cpp" | perl -pe 's/.+\"(\w+\.cpp)\".+/\1/g' >> CMakeLists.txt

