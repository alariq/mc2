# Mech Commander 2 open source engine + Linux port.
[website](https://alariq.github.io/mc2-website/)

 ## !NB: as russia conducts war in Ukraine I have no time to support this project until we will get rid of orcs. You are encouraged to help.
 

Disclaimer: I consider this project finished for now, there is a lot more to do for someone who wants to improve the game, but all functionality (except networking) is implemented and I've passed the game on my Linux box. Also found original game bugs and crashes are fixed.


This port is an open source implementation of a closed MC2 engine code using available interface (.h) files.
Currently game can be run on both Linux and Windows in 64bit mode.
Fixed a lot of bugs (including ones present in original game).
Sound system is not fully implemented (panning, doppler, etc. not supported yet)

## TODO: 
* fix remaining memory leaks (finish implementation of memory heaps)
* make nice data packs, so not only me can play the game :-) (in progress, see [data repo](https://github.com/alariq/mc2srcdata)
* ~~actually finish all missions in the game~~
* make sure no files are created outside of user directory
* reduce draw calls number
* reimplement/optimize priority queue
* finish moving lighting to shaders (move whole lighting there, not only shader-based drawing of CPU-prelit vertices like I do now)
* Update graphics to ~~2018~~ ~~2020~~ 2021
* Add network support?
* I am sure there is more

### Licensing
* Original game was released under Shared Source Limited Permission License (please refer to EULA.txt)
* My code is licenced under GPL v.3 (see license.txt)
* All third party libraries use their own licenses


Building on Windows
===================

**Updated detailed build manual for Windows can be found in `BUILD-WIN.md`**

To build on windows use CMake

Ensure, that you have all necessary dependencies: SDL2, SDL_mixer, SDL_ttf(for tools), zlib, glew
I recommend to get zlib sources and build them by hand (do not forget to copy zconf.h)
One can also use 3rdparty.zip package in the repo for simpler setup, it contains all needed 3rdparty libraries

Just for a reference here is how my 3rdparty tree may look like
```
|   
+---bin
+---cmake
    |    sdl2_mixer-config.cmake
    |    sdl2_ttf-config.cmake
    |    sdl2-config.cmake
    |    
+---include
|   |   zconf.h
|   |   zlib.h
|   |   
|   +---GL
|   |       eglew.h
|   |       glew.h
|   |       glxew.h
|   |       wglew.h
|   |       
|   \---SDL2
|           all sdl headers (SDL_mixer.h and SDL_ttf.h should be there as well)
|           
\---lib
    +---x64
    |       glew32.lib
    |       glew32s.lib
    |       SDL2.dll
    |       SDL2.lib
    |       SDL2main.lib
    |       SDL2test.lib
    |       SDL2_mixer.dll
    |       SDL2_mixer.lib
    |       SDL2_ttf.lib
    |       SDL2_ttf.dll
    |       zlib.dll
    |       zlib.lib
    |       zlibstatic.lib
    |       
    \---x86
            glew32.lib
            glew32s.lib
            SDL2.dll
            SDL2.lib
            SDL2main.lib
            SDL2test.lib
            SDL2_mixer.dll
            SDL2_mixer.lib
            SDL2_mixer.dll
            zlib.dll
            zlib.lib
            zlibstatic.lib
```

You may already have your dependencies installed in other place(s), so just make sure CMake knows where to find them.

Now:
```
git clone https://github.com/alariq/mc2.git
cd mc2
md build64
cd build64
cmake.exe -G "Visual Studio 15 2017 Win64" -DCMAKE_PREFIX_PATH=c:/path_to_your_dependencies/ -DCMAKE_LIBRARY_ARCHITECTURE=x64 ..
```
(to generate project for VS1027 for 64bit build)
(prefer absolute path)

or for 32bit build:

`cmake.exe -G "Visual Studio 15 2017 Win64" -DCMAKE_PREFIX_PATH=c:/path_to_your_dependencies/ -DCMAKE_LIBRARY_ARCHITECTURE=x86 ..`
(prefer absolute path)

for 6bit build and VS2022:

`cmake.exe -G "Visual Studio 17 2022" -DCMAKE_PREFIX_PATH=c:/path_to_your_dependencies/ -DCMAKE_LIBRARY_ARCHITECTURE=x64 ..`
(prefer absolute path)

Now run generated solution and try to build it!


Now build string resources
--------------------------

First use script in `test_scripts/res_conv/res_conv.pl` to generate `strings.res.cpp` and  `strings.res.h` files or just take them from that folder
Then copy them to `./res` folder
Then:

```
cd res
md build64
cd build 64
cmake.exe -G "Visual Studio 15 2017 Win64" -DCMAKE_LIBRARY_ARCHITECTURE=x64 ..
```
put resulting dll along with executable file

Building data
--------------------------

Data files are located in a separate [repository](https://github.com/alariq/mc2srcdata)
Building data is just a matter of executing ```make``` command. You can find instruction on how to do it there.
But first one needs to build all necessary tools. There are 2 projects which have to be built:

* data_tools 
* text_tool

All steps are same as for the main application. As a result you'll have next binaries: ```aseconv, makefst, makersp, mpak, text_tool```. Copy those to the ```build_scripts``` folder in ```mc2srcdata``` repository.

Once everything in place, you can launch build scripts as described in corresponding ```README.txt``` file.


Building on Linux
=================

You, probably already know hot to do it. If not, please, see windows building section, the process is quite similar.

