add_definitions(-DUSE_ASSEMBLER_CODE=0)
add_definitions(-DLINUX_BUILD)


set(TEXT_TOOL_SOURCES "main.cpp")

add_executable(text_tool ${TEXT_TOOL_SOURCES})
target_link_libraries(text_tool gameos windows SDL2::SDL2 SDL2::SDL2main ${SDL2_ttf} GLEW::GLEW ${ADDITIONAL_LIBS} OpenGL::GL)
