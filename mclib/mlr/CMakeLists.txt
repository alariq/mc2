cmake_minimum_required (VERSION 2.8)
project(mlr)

set(SOURCES ${SOURCES}
    mlrindexedprimitivebase.cpp
    mlrprimitivebase.cpp
    mlrshape.cpp
    mlr_i_c_det_pmesh.cpp
    mlr_i_c_dt_pmesh.cpp
    mlr_i_c_pmesh.cpp
    mlr_i_det_pmesh.cpp
    mlr_i_dt_pmesh.cpp
    mlr_i_l_det_pmesh.cpp
    mlr_i_l_dt_pmesh.cpp
    mlr_i_l_pmesh.cpp
    mlr_i_mt_pmesh.cpp
    mlr_i_pmesh.cpp
    mlr_i_c_det_tmesh.cpp
    mlr_i_c_dt_tmesh.cpp
    mlr_i_c_tmesh.cpp
    mlr_i_det_tmesh.cpp
    mlr_i_dt_tmesh.cpp
    mlr_i_l_det_tmesh.cpp
    mlr_i_l_dt_tmesh.cpp
    mlr_i_l_tmesh.cpp
    mlr_i_tmesh.cpp
    mlr_terrain.cpp
    mlr_terrain2.cpp
    mlrambientlight.cpp
    mlrinfinitelight.cpp
    mlrinfinitelightwithfalloff.cpp
    mlrlight.cpp
    mlrlightmap.cpp
    mlrlookuplight.cpp
    mlrpointlight.cpp
    mlrspotlight.cpp
    mlrsortbyorder.cpp
    mlrsorter.cpp
    mlrstate.cpp
    mlrtexture.cpp
    mlrtexturepool.cpp
    gosimage.cpp
    gosimagepool.cpp
    gosvertex.cpp
    gosvertex2uv.cpp
    mlrclipper.cpp
    mlrclippingstate.cpp
    mlrcardcloud.cpp
    mlreffect.cpp
    mlrindexedtrianglecloud.cpp
    mlrlinecloud.cpp
    mlrngoncloud.cpp
    mlrpointcloud.cpp
    mlrtrianglecloud.cpp
    mlr.cpp
    mlrheaders.cpp
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
include_directories("../../GameOS/include" "../../mclib")

#DisableSpecificWarnings="4996;4537;4731;4430"

add_library(mlr ${SOURCES})

# cat MCLib.vcproj | grep ".cpp" | perl -pe 's/.+\"(\w+\.cpp)\".+/\1/g' >> CMakeLists.txt

