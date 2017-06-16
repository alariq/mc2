cmake_minimum_required (VERSION 2.8)
project(text_tool)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -ggdb -std=c++0x")
if(NOT MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedefs -Werror=array-bounds -Wno-switch -Wno-unknown-warning-option -Wunused-variable")
endif()

find_path(SDL2_INCLUDE "SDL2/SDL.h")
find_path(GLEW_INCLUDE "gl/glew.h")

# on linux and others assune that required libs are located in system paths, so for now add them only for Windows
if(WIN32)
	include_directories(${SDL2_INCLUDE} ${GLEW_INCLUDE})
endif()
add_definitions(-DLINUX_BUILD)

if(MSVC)
	add_definitions(-DPLATFORM_WINDOWS)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	message("Compiling on M$ Windows")
endif()

add_subdirectory("../GameOS/gameos" "./out/GameOS/gameos")


# must be after all add_subdirectory(ies) because we use same array name (SOURCES) for all projects
set(SOURCES 
    "main.cpp"
    )

find_package(SDL2 QUIET)
if(SDL2_FOUND)
	set(SDL_LIBS ${SDL2_LIBRARIES})
    set(SDL_INCLUDES "${SDL2_PREFIX}/include")
else()
    message("sdl2 package was not found, looking for libs and headers separately")
    find_library(SDL2 NAMES "SDL2")
    find_library(SDL2main NAMES "SDL2main")
    find_path(SDL2_INCLUDE "SDL2/SDL.h")
    set(SDL_LIBS ${SDL2} ${SDL2main})
    set(SDL_INCLUDES ${SDL2_INCLUDE})
endif()
message("SDL libs are here: ${SDL_LIBS}")
find_library(SDL2_ttf NAMES "SDL2_ttf" PATHS /opt)


find_library(GLEW NAMES "GLEW" "glew32")
find_package(OpenGL REQUIRED)

message("GLEW library found in ${GLEW}")

include_directories(${SDL_INCLUDES})
include_directories("./" "../GameOS/gameos/" "../GameOS/include")

if(WIN32)
	set(ADDITIONAL_LIBS winmm) # timeGetTime used in gameos
	set(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${OPENGL_gl_LIBRARY} )
	message("OpenGL library linking: ${OPENGL_gl_LIBRARY}")
	message("OpenGL library include: ${OPENGL_INCLUDE_DIR}")
endif()


add_executable(text_tool ${SOURCES})
target_link_libraries(text_tool gameos ${SDL_LIBS} ${SDL2_ttf} ${GLEW} ${ADDITIONAL_LIBS} ${OPENGL})
