cmake_minimum_required (VERSION 2.8)
project(data_tools)

if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
	message(STATUS "Target is 64 bits")
	set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS TRUE)
else("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
	message(STATUS "Target is 32 bits")
endif("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -std=c++0x -ggdb ")
if(NOT MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedefs -Wno-unknown-warning-option")
endif()

add_definitions(-D_ARMOR)
add_definitions(-D_DEBUG)
add_definitions(-DUSE_ASSEMBLER_CODE=0)
add_definitions(-DLINUX_BUILD)

if(MSVC)
	add_definitions(-DPLATFORM_WINDOWS)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	message("Compiling on M$ Windows")
endif()

# TODO: get rid of gl library by not using old style opengl
find_package(OpenGL REQUIRED)

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

find_library(SDL2_mixer NAMES "SDL2_mixer")
find_library(GLEW NAMES "GLEW" "glew32")
find_library(ZLIB NAMES "z" "zlib")

message("SDL includes: ${SDL_INCLUDES}")

include_directories(${SDL_INCLUDES})
include_directories("./window" "../GameOS/include" "../mclib" "../gui" "../code" "../netlib")

add_subdirectory("../mclib/" "./out/mclib")
add_subdirectory("../mclib/mlr" "./out/mclib/mlr")
add_subdirectory("../mclib/gosfx" "./out/mclib/gosfx")
add_subdirectory("../mclib/stuff" "./out/mclib/stuff")
#add_subdirectory("../gui" "./out/gui")
add_subdirectory("../GameOS/gameos" "./out/GameOS/gameos")

add_subdirectory("../GameOS/src" "./out/windows")

set(MAKEFST_SOURCES "makefst.cpp")

set(PAK_SOURCES "pak.cpp" "common.hpp")

set(ASECONV_SOURCES "aseconv.cpp" "common.hpp")

set(MAKERSP_SOURCES "makersp.cpp")

if(WIN32)
	set(ADDITIONAL_LIBS winmm) # timeGetTime used in gameos
	set(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${OPENGL_gl_LIBRARY} )
	message("OpenGL library linking: ${OPENGL_gl_LIBRARY}")
	message("OpenGL library include: ${OPENGL_INCLUDE_DIR}")
endif()

add_executable(makefst ${MAKEFST_SOURCES})
target_link_libraries(makefst mclib stuff gameos windows ${ZLIB} ${SDL_LIBS} ${ADDITIONAL_LIBS})

add_executable(pak ${PAK_SOURCES})
target_link_libraries(pak mclib stuff gameos windows ${ZLIB} ${SDL_LIBS} ${ADDITIONAL_LIBS})

add_executable(aseconv ${ASECONV_SOURCES})
target_link_libraries(aseconv mclib gosfx mlr stuff gameos windows ${ZLIB} ${SDL_LIBS} ${GLEW} ${SDL2_mixer} ${ADDITIONAL_LIBS} ${OPENGL_gl_LIBRARY})

add_executable(makersp ${MAKERSP_SOURCES})
target_link_libraries(makersp mclib stuff gameos windows ${ZLIB} ${SDL_LIBS} ${ADDITIONAL_LIBS})

