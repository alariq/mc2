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

include_directories(${COMMON_INCLUDE_DIRS} "../../GameOS/include" "../../mclib")

add_library(mlr ${SOURCES})

# cat MCLib.vcproj | grep ".cpp" | perl -pe 's/.+\"(\w+\.cpp)\".+/\1/g' >> CMakeLists.txt

