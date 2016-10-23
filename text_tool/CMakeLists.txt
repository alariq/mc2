cmake_minimum_required (VERSION 2.8)
project(viewer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -ggdb")

add_subdirectory("../GameOS/gameos" "./out/GameOS/gameos")

# must be after all add_subdirectory(ies) because we use same array name (SOURCES) for all projects
set(SOURCES 
    "main.cpp"
    )

add_definitions(-DLINUX_BUILD)

find_library(SDL2 NAMES "SDL2")
find_library(SDL2main NAMES "SDL2main")
find_library(SDL2_ttf NAMES "SDL2_ttf")
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
message("AntTweakBar library found in ${AntTweakBar}")

include_directories("./" "../GameOS/gameos/" "../GameOS/include")

add_executable(text_tool ${SOURCES})
target_link_libraries(text_tool gameos ${SDL2} ${SDL2main} ${SDL2_ttf} ${OPENGL} ${GLEW})
