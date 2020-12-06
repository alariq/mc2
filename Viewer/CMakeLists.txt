set(VIEWER_SOURCES 
    "../gui/aanimobject.cpp"
    "../code/gamesound.cpp"
    "../code/logisticscomponent.cpp"
    "../code/logisticsdata.cpp"
    "../code/logisticsmech.cpp"
    "../code/logisticsmissioninfo.cpp"
    "../code/logisticspilot.cpp"
    "../code/logisticsvariant.cpp"
    "../code/mechlopedia.cpp"
    "../code/mission2.cpp"
    "../code/prefs.cpp"
    "../code/radio.cpp"
    "../code/simplecamera.cpp"
    "../code/simplecomponentlistbox.cpp"
    #    "../strings.res.cpp"
    "View.cpp"
    )

add_definitions(-DVIEWER=1)
#add_definitions(-D_ARMOR)
#add_definitions(-D_DEBUG)

include_directories("./window" ${COMMON_INCLUDE_DIRS} "../mclib" "../gui" "../code" "../netlib")

add_executable(viewer ${VIEWER_SOURCES})
target_link_libraries(viewer mclib gosfx mlr stuff gui gameos gameos_main windows ZLIB::ZLIB SDL2::Main GLEW::GLEW SDL2::Mixer ${ADDITIONAL_LIBS} OpenGL::GL)

