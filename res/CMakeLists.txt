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
add_library(mc2res_${res_suffix} SHARED ${RES_SOURCES})
