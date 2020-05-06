set(MAKEFST_SOURCES "makefst.cpp")
set(PAK_SOURCES "pak.cpp" "common.hpp")
set(ASECONV_SOURCES "aseconv.cpp" "common.hpp")
set(MAKERSP_SOURCES "makersp.cpp")

add_compile_definitions(DISABLE_GAMEOS_MAIN)

add_executable(makefst ${MAKEFST_SOURCES})
target_link_libraries(makefst mclib stuff gameos windows ZLIB::ZLIB SDL2::SDL2 SDL2::SDL2main ${ADDITIONAL_LIBS})

add_executable(pak ${PAK_SOURCES})
target_link_libraries(pak mclib stuff gameos windows ZLIB::ZLIB SDL2::SDL2 SDL2::SDL2main ${ADDITIONAL_LIBS})

add_executable(aseconv ${ASECONV_SOURCES})
target_link_libraries(aseconv mclib gosfx mlr stuff gameos windows ZLIB::ZLIB SDL2::SDL2 SDL2::SDL2main GLEW::GLEW ${SDL2_mixer} ${ADDITIONAL_LIBS} OpenGL::GL)

add_executable(makersp ${MAKERSP_SOURCES})
target_link_libraries(makersp mclib stuff gameos windows ZLIB::ZLIB SDL2::SDL2 SDL2::SDL2main ${ADDITIONAL_LIBS})

