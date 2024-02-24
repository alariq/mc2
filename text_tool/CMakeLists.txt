cmake_minimum_required (VERSION 3.14)
project(text_tool)

add_definitions(-DUSE_ASSEMBLER_CODE=0)
add_definitions(-DLINUX_BUILD)


set(TEXT_TOOL_SOURCES "main.cpp")

add_executable(${PROJECT_NAME} ${TEXT_TOOL_SOURCES})
target_link_libraries(${PROJECT_NAME} gameos windows SDL2::Main SDL2::TTF GLEW::GLEW ${ADDITIONAL_LIBS} OpenGL::GL)

add_custom_command(
        TARGET ${PROJECT_NAME}  POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/text_tool
        ${MC2_DATA_PATH}/build_scripts/)
