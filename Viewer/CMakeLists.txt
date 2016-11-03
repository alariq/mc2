cmake_minimum_required (VERSION 2.8)
project(viewer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -ggdb -Wno-unused-local-typedefs")
# -Wno-unused-local-typedefs - to disable "typedef was ignored on this declaration"

#PreprocessorDefinitions="_ARMOR;LAB_ONLY;WIN32;_DEBUG;_WINDOWS"
#PreprocessorDefinitions="NDEBUG;LAB_ONLY;_WINDOWS;WIN32;NOMINMAX"

#include_directories(${mc2_SOURCE_DIR}/LibProject/inc)

#DisableSpecificWarnings="4996;4537;4731;4430"

#AdditionalDependencies="..\mclib\$(ConfigurationName)\mclib.lib ..\gameos\lib\$(ConfigurationName)\gameos.lib ..\gameos\lib\$(ConfigurationName)\gameplatform.lib ..\mclib\gosfx\$(ConfigurationName)\gosfx.lib ..\mclib\mlr\$(ConfigurationName)\mlr.lib ..\mclib\stuff\$(ConfigurationName)\stuff.lib ..\gui\$(ConfigurationName)\gui.lib ..\gameos\lib\$(ConfigurationName)\zlib.lib"

#mclib.lib
#gameos.lib
#gameplatform.lib
#gosfx.lib
#mlr.lib
#stuff.lib
#gui.lib
#zlib.lib

add_subdirectory("../mclib/" "./out/mclib")
add_subdirectory("../mclib/mlr" "./out/mclib/mlr")
add_subdirectory("../mclib/gosfx" "./out/mclib/gosfx")
add_subdirectory("../mclib/stuff" "./out/mclib/stuff")
add_subdirectory("../gui" "./out/gui")
add_subdirectory("../GameOS/gameos" "./out/GameOS/gameos")

add_subdirectory("../GameOS/src" "./out/windows")

# must be after all add_subdirectory(ies) because we use same array name (SOURCES) for all projects
set(SOURCES 
    "../gui/aanimobject.cpp"
    "../code/gamesound.cpp"
    "../code/logisticscomponent.cpp"
    "../code/logisticsdata.cpp"
    "../code/logisticsmech.cpp"
    "../code/logisticsmissioninfo.cpp"
    "../code/logisticspilot.cpp"
    "../code/logisticsvariant.cpp"
    "../code/mechlopedia.cpp"
    "../code/mission2.cpp"
    "../code/prefs.cpp"
    "../code/radio.cpp"
    "../code/simplecamera.cpp"
    "../code/simplecomponentlistbox.cpp"
    #    "../strings.res.cpp"
    "View.cpp"
    )

add_definitions(-DVIEWER=1)
add_definitions(-D_ARMOR)
add_definitions(-D_DEBUG)
add_definitions(-DUSE_ASSEMBLER_CODE=0)
add_definitions(-DLINUX_BUILD)

find_library(SDL2 NAMES "SDL2")
find_library(SDL2main NAMES "SDL2main")
find_library(OPENGL NAMES "opengl32" "GL")
find_library(GLEW NAMES "GLEW" "glew32")

if(NOT GLEW)
    message("${GLEW} library not found")
else(NOT GLEW)
    message("GLEW library found in ${GLEW}")
endif(NOT GLEW)

message("OPENCL library found in ${OPENCL}")
message("OPENGL library found in ${OPENGL}")
message("GLEW library found in ${GLEW}")

include_directories("./window" "../GameOS/include" "../mclib" "../gui" "../code" "../netlib")

add_executable(viewer ${SOURCES})
target_link_libraries(viewer mclib gosfx mlr stuff gui gameos windows z ${SDL2} ${SDL2main} ${OPENGL} ${GLEW} dl)
