cmake_minimum_required (VERSION 3.14)
project(data_tools)

set(MAKEFST_SOURCES "makefst.cpp")
set(PAK_SOURCES "pak.cpp" "common.hpp")
set(ASECONV_SOURCES "aseconv.cpp" "common.hpp")
set(MAKERSP_SOURCES "makersp.cpp")

add_compile_definitions(DISABLE_GAMEOS_MAIN)

add_executable(makefst ${MAKEFST_SOURCES})
target_link_libraries(makefst mclib stuff gameos windows ZLIB::ZLIB SDL2::Main ${ADDITIONAL_LIBS})

add_executable(pak ${PAK_SOURCES})
target_link_libraries(pak mclib stuff gameos windows ZLIB::ZLIB SDL2::Main ${ADDITIONAL_LIBS})

add_executable(aseconv ${ASECONV_SOURCES})
target_link_libraries(aseconv mclib gosfx mlr stuff gameos windows ZLIB::ZLIB SDL2::Main GLEW::GLEW ${SDL2_mixer} ${ADDITIONAL_LIBS} OpenGL::GL)

add_executable(makersp ${MAKERSP_SOURCES})
target_link_libraries(makersp mclib stuff gameos windows ZLIB::ZLIB SDL2::Main ${ADDITIONAL_LIBS})

add_custom_command(
        TARGET makefst  POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/makefst
        ${MC2_DATA_PATH}/build_scripts/)

add_custom_command(
        TARGET pak  POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/pak
        ${MC2_DATA_PATH}/build_scripts/)

add_custom_command(
        TARGET aseconv  POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/aseconv
        ${MC2_DATA_PATH}/build_scripts/)
        
add_custom_command(
        TARGET makersp  POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/makersp
        ${MC2_DATA_PATH}/build_scripts/)
        
                                                
