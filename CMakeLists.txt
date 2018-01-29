cmake_minimum_required (VERSION 2.8)
project(mc2)

include(CMakeToolsHelpers OPTIONAL)

# Set a default build type for single-configuration
# CMake generators if no build type is set.
#IF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
#   SET(CMAKE_BUILD_TYPE RelWithDebInfo)
#ENDIF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

message("CMAKE library architecture: ${CMAKE_LIBRARY_ARCHITECTURE}")

if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
	message(STATUS "Target is 64 bits")
	set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS TRUE)
else("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
	message(STATUS "Target is 32 bits")
endif("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -g")

if(NOT MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedefs -Werror=array-bounds -Wno-switch -Wno-unknown-warning-option -Wunused-variable -fsanitize=address -fno-omit-frame-pointer")
endif()
# -Wno-unused-local-typedefs - to disable "typedef was ignored on this declaration"

if(MSVC)
	add_definitions(-DPLATFORM_WINDOWS)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	message("Compiling on M$ Windows")
endif()

# to support windows sln file generator as well
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_ARMOR -D_DEBUG -DBUGLOG -DLAB_ONLY")
#set(CMAKE_CXX_FLAGS_RELWITHDEBINFO" ${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DSOME_STUFF")
if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()


add_definitions(-DUSE_ASSEMBLER_CODE=0)
add_definitions(-DLINUX_BUILD)

find_path(GLEW_INCLUDE "GL/glew.h")

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

include_directories(${SDL_INCLUDES})
include_directories(${GLEW_INCLUDE})

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
    "code/mc2movie.cpp"
    )

find_library(SDL2_mixer NAMES "SDL2_mixer")
find_library(GLEW NAMES "GLEW" "glew32")
find_library(ZLIB NAMES "z" "zlib")

# TODO: get rid of gl library by not using old style opengl
find_package(OpenGL REQUIRED)

message("OPENGL library found in ${OPENGL}")
message("GLEW library found in ${GLEW}")
message("Zlib library found in ${ZLIB}")

include_directories("./GameOS/include" "./mclib" "./gui" "./code" "./netlib")

message("OpenGL library linking: ${OPENGL_gl_LIBRARY}")
message("OpenGL library include: ${OPENGL_INCLUDE_DIR}")

#if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
if(NOT WIN32)
	set(ADDITIONAL_LIBS dl)
	message("Not win32 system")
else()
	# winmm for timeGetTime, maybe switch to GetTickCount to remove this dependency
	set(ADDITIONAL_LIBS winmm)
endif()

message("sdl2 prefix: ${SDL2_PREFIX}")


add_executable(mc2 ${SOURCES})
target_link_libraries(mc2 mclib gosfx mlr stuff gui gameos windows ${ZLIB} ${SDL_LIBS} ${GLEW} ${SDL2_mixer} ${ADDITIONAL_LIBS} ${OPENGL_gl_LIBRARY})

add_subdirectory("./Viewer" "./out/Viewer")
