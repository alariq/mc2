cmake_minimum_required(VERSION 3.10)
# Select GLVND versus legacy OpenGL libraries
cmake_policy(SET CMP0072 NEW) # cmake --help-policy CMP0072
project(mc2)

include(CMakeToolsHelpers OPTIONAL)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set a default build type for single-configuration
#IF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
#   SET(CMAKE_BUILD_TYPE RelWithDebInfo)
#ENDIF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

# Define third-party paths
set(THIRDPARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty")
set(THIRDPARTY_INCLUDE_DIRS "${THIRDPARTY_DIR}/include")
if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
    message(STATUS "Target is 64 bits")
    set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS TRUE)
    set(THIRDPARTY_LIB_DIR "${THIRDPARTY_DIR}/lib/x64")
else()
    message(STATUS "Target is 32 bits")
    set(THIRDPARTY_LIB_DIR "${THIRDPARTY_DIR}/lib/x86")
endif()

# Add third-party include and library directories
include_directories(${THIRDPARTY_INCLUDE_DIRS})
link_directories(${THIRDPARTY_LIB_DIR})

# FFmpeg paths (already specific to FFmpeg)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/ffmpeg/include")
set(FFMPEG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/ffmpeg")
include_directories("${FFMPEG_DIR}/include")
link_directories("${FFMPEG_DIR}/lib")

set(FFMPEG_LIBS
    avcodec
    avformat
    avutil
    swscale
    swresample
)

message("CMAKE library architecture: ${CMAKE_LIBRARY_ARCHITECTURE}")

set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")

# Set CMAKE_PREFIX_PATH to include the 3rdparty directory
list(APPEND CMAKE_PREFIX_PATH "${THIRDPARTY_DIR}")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2")

# Set SDL2_DIR to the directory containing SDL2Config.cmake
set(SDL2_DIR "${THIRDPARTY_DIR}/cmake")

if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -ggdb")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedefs -Werror=array-bounds -Wno-switch -Wwrite-strings -Wno-unknown-warning-option -Wunused-variable -fpermissive") # -fsanitize=address -fno-omit-frame-pointer")
endif()
# -Wno-unused-local-typedefs - to disable "typedef was ignored on this declaration"

if(MSVC)
    add_definitions(-DPLATFORM_WINDOWS)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    message("Compiling on M$ Windows")
endif()

add_definitions(-DUSE_ASSEMBLER_CODE=0)
add_definitions(-DLINUX_BUILD)

# Find packages
find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2)
find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)

# GLEW
find_package(GLEW)
if(NOT GLEW_FOUND)
    message(STATUS "GLEW not found by find_package, setting manually")
    set(GLEW_FOUND TRUE)
    set(GLEW_INCLUDE_DIRS "${THIRDPARTY_INCLUDE_DIRS}")
    set(GLEW_LIBRARIES "${THIRDPARTY_LIB_DIR}/glew32.lib")
    # Create imported target GLEW::GLEW
    add_library(GLEW::GLEW UNKNOWN IMPORTED)
    set_target_properties(GLEW::GLEW PROPERTIES
        IMPORTED_LOCATION "${GLEW_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${GLEW_INCLUDE_DIRS}"
    )
endif()
message(STATUS "GLEW_FOUND: ${GLEW_FOUND}")
message(STATUS "GLEW_INCLUDE_DIRS: ${GLEW_INCLUDE_DIRS}")
message(STATUS "GLEW_LIBRARIES: ${GLEW_LIBRARIES}")

find_package(OpenGL REQUIRED)

# Zlib
find_package(ZLIB)
if(NOT ZLIB_FOUND)
    message(STATUS "Zlib not found by find_package, setting manually")
    set(ZLIB_FOUND TRUE)
    set(ZLIB_INCLUDE_DIRS "${THIRDPARTY_INCLUDE_DIRS}")
    set(ZLIB_LIBRARIES "${THIRDPARTY_LIB_DIR}/zlib.lib")
    # Create imported target ZLIB::ZLIB
    add_library(ZLIB::ZLIB UNKNOWN IMPORTED)
    set_target_properties(ZLIB::ZLIB PROPERTIES
        IMPORTED_LOCATION "${ZLIB_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${ZLIB_INCLUDE_DIRS}"
    )
endif()
message(STATUS "ZLIB_FOUND: ${ZLIB_FOUND}")
message(STATUS "ZLIB_INCLUDE_DIRS: ${ZLIB_INCLUDE_DIRS}")
message(STATUS "ZLIB_LIBRARIES: ${ZLIB_LIBRARIES}")

message("Found SDL2 package ${SDL2_LIBRARIES}")
message("SDL2 prefix: ${SDL2_PREFIX}")
message("OPENGL library found in ${OPENGL}")
message("GLEW library found in ${GLEW}")
message("Zlib library found in ${ZLIB}")

message("OpenGL library linking: ${OPENGL_gl_LIBRARY}")
message("OpenGL library include: ${OPENGL_INCLUDE_DIR}")

get_filename_component(COM_PATH1 "./GameOS/include" ABSOLUTE)
get_filename_component(COM_PATH2 "./GameOS/gameos" ABSOLUTE)

set(COMMON_INCLUDE_DIRS ${COM_PATH1} ${COM_PATH2})

add_subdirectory("./mclib/" "./out/mclib")
add_subdirectory("./mclib/mlr" "./out/mclib/mlr")
add_subdirectory("./mclib/gosfx" "./out/mclib/gosfx")
add_subdirectory("./mclib/stuff" "./out/mclib/stuff")
add_subdirectory("./gui" "./out/gui")
add_subdirectory("./GameOS/gameos" "./out/GameOS/gameos")

add_subdirectory("./GameOS/src" "./out/windows")

set(SOURCES 
    "code/dw.cpp"
    "code/mechcmd2.cpp"
    "code/artlry.cpp"
    "code/bldng.cpp"
    "code/carnage.cpp"
    "code/collsn.cpp"
    "code/comndr.cpp"
    "code/contact.cpp"
    "code/gameobj.cpp"
    "code/gate.cpp"
    "code/goal.cpp"
    "code/group.cpp"
    "code/gvehicl.cpp"
    "code/light.cpp"
    "code/mech.cpp"
    "code/mover.cpp"
    "code/objmgr.cpp"
    "code/objtype.cpp"
    "code/mp4player.cpp"
    "code/mc2movie.cpp"
    "code/tacordr.cpp"
    "code/team.cpp"
    "code/terrobj.cpp"
    "code/turret.cpp"
    "code/warrior.cpp"
    "code/weaponbolt.cpp"
    "code/ablmc2.cpp"
    "code/gamecam.cpp"
    "code/mission.cpp"
    "code/mission2.cpp"
    "code/objective.cpp"
    "code/saveload.cpp"
    "code/trigger.cpp"
    "code/weather.cpp"
    "gui/aanimobject.cpp"
    "gui/aedit.cpp"
    "code/attributemeter.cpp"
    "code/controlgui.cpp"
    "code/forcegroupbar.cpp"
    "code/gametacmap.cpp"
    "code/infowindow.cpp"
    "code/keyboardref.cpp"
    "code/mechicon.cpp"
    "code/missiongui.cpp"
    "code/mpconnectiontype.cpp"
    "code/mpgamebrowser.cpp"
    "code/mphostgame.cpp"
    "code/mploadmap.cpp"
    "code/mpparameterscreen.cpp"
    "code/mpsetuparea.cpp"
    "code/optionsarea.cpp"
    "code/optionsscreenwrapper.cpp"
    "code/pausewindow.cpp"
    "code/prefs.cpp"
    "code/simplecamera.cpp"
    "code/componentlistbox.cpp"
    "code/loadscreen.cpp"
    "code/logistics.cpp"
    "code/logisticscomponent.cpp"
    "code/logisticsdata.cpp"
    "code/logisticsdialog.cpp"
    "code/logisticsmech.cpp"
    "code/logisticsmechdisplay.cpp"
    "code/logisticsmechicon.cpp"
    "code/logisticsmissioninfo.cpp"
    "code/logisticspilot.cpp"
    "code/logisticspilotlistbox.cpp"
    "gui/logisticsscreen.cpp"
    "code/logisticsvariant.cpp"
    "code/mainmenu.cpp"
    "code/mechbayscreen.cpp"
    "code/mechlabscreen.cpp"
    "code/mechlistbox.cpp"
    "code/mechlopedia.cpp"
    "code/mechpurchasescreen.cpp"
    "code/missionbegin.cpp"
    "code/missionbriefingscreen.cpp"
    "code/missionresults.cpp"
    "code/missionselectionscreen.cpp"
    "code/mpprefs.cpp"
    "code/pilotreadyscreen.cpp"
    "code/pilotreviewarea.cpp"
    "code/salvagemecharea.cpp"
    "code/simplecomponentlistbox.cpp"
    "code/gamesound.cpp"
    "code/radio.cpp"
    "code/movemgr.cpp"
    "code/chatwindow.cpp"
    "code/mpstats.cpp"
    "code/multplyr.cpp"
)

include_directories(${COMMON_INCLUDE_DIRS} "./mclib" "./gui" "./code" "./netlib")
include_directories(${THIRDPARTY_INCLUDE_DIRS})

if(NOT WIN32)
    set(ADDITIONAL_LIBS dl)
    message("Not win32 system")
else()
    # winmm for timeGetTime, maybe switch to GetTickCount to remove this dependency
    set(ADDITIONAL_LIBS winmm)
endif()

add_executable(mc2 ${SOURCES} ${MAIN_SRC})
target_link_libraries(
    mc2 
    mclib 
    gosfx 
    mlr 
    stuff 
    gui 
    gameos 
    gameos_main 
    windows 
    ZLIB::ZLIB 
    ${SDL2_LIBRARIES} 
    GLEW::GLEW 
    SDL2_mixer::SDL2_mixer 
    ${ADDITIONAL_LIBS} 
    OpenGL::GL 
    ${FFMPEG_LIBS}
    avcodec
    avformat
    avutil
    swscale
    swresample
)

# Clean standalone MP4 player — NO game engine dependencies
add_executable(clean_mp4_player
    code/clean_mp4_player.cpp
)

target_include_directories(clean_mp4_player PRIVATE
    ${THIRDPARTY_INCLUDE_DIRS}
)

target_link_libraries(clean_mp4_player
    ${SDL2_LIBRARIES}
    GLEW::GLEW
    OpenGL::GL
    ${FFMPEG_LIBS}
)

add_subdirectory("./res" "./out/res")
add_subdirectory("./data_tools" "./out/data_tools")
add_subdirectory("./text_tool" "./out/text_tool")
add_subdirectory("./Viewer" "./out/Viewer")

# Copy runtime DLLs
add_custom_command(TARGET mc2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${THIRDPARTY_LIB_DIR}/glew32.dll"
    $<TARGET_FILE_DIR:mc2>)
add_custom_command(TARGET mc2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${THIRDPARTY_LIB_DIR}/zlib1.dll"
    $<TARGET_FILE_DIR:mc2>)

# Copy mc2.exe to full_game directory after build so the user can launch
# the freshly built binary directly from full_game/ without a manual copy.
add_custom_command(TARGET mc2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:mc2>
    "${CMAKE_SOURCE_DIR}/full_game/mc2.exe"
    COMMENT "Copying mc2.exe to full_game/"
)
