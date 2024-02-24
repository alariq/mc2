cmake_minimum_required (VERSION 3.14)
project(mc2res)

set(RES_SOURCES libmain.cpp libmainc.cpp)

if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
	message(STATUS "Target is 64 bits")
	set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS TRUE)
	set(res_suffix 64)
else("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
	message(STATUS "Target is 32 bits")
	set(res_suffix 32)
endif("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")

include_directories("." "../test_scripts/res_conv")
add_library(${PROJECT_NAME} SHARED ${RES_SOURCES})

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/libmc2res.so
        ${CMAKE_CURRENT_BINARY_DIR}/../..)
