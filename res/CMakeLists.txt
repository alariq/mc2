cmake_minimum_required (VERSION 2.8)
project(mc2res)

set(SOURCES ${SOURCES}
    libmain.cpp
    libmainc.cpp
    )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -ggdb -std=c++0x")
if(NOT MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedefs -Werror=array-bounds -Wno-switch -Wno-unknown-warning-option -Wunused-variable")
endif()

if(WIN32)
    add_definitions(-DPLATFORM_WINDOWS)
endif()


add_definitions(-D_DEBUG)
add_definitions(-DLINUX_BUILD)

include_directories(".")

add_library(mc2res SHARED ${SOURCES})
