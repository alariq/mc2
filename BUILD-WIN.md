Preparing 3rdparties:
====================

One can also use 3rdparty.zip package in the repo for simpler setup, it contains all needed 3rdparty libraries
If you select to do it then skip directly to **Compiling mc2**

zlib
----

1. Download zlib sources from here: 
https://gnuwin32.sourceforge.net/packages/zlib.htm
direct link: https://gnuwin32.sourceforge.net/downlinks/zlib-src-zip.php
2. Download unistd.h for windows here:
https://gist.githubusercontent.com/mbikovitsky/39224cf521bfea7eabe9/raw/69e4852c06452a368a174ca1f0f33ce87bb52985/unistd.h
2b. Open it and comment out: `#include <getopt.h>` also comment integer types typedefs at the end
3. put it where zlib sourse files are located then open zconf.h and change `#include <unistd.h>` for `#include "unistd.h"` (or see 4.)
4. (alternative to 3) Put unistd.h to place where your compiler system headers are.
5. Open x86 Native Tools command prompt for VS2022 and cd to zlib
6. `nmake -f win32\Makefile.msc`
7. copy resulting .dll & .lib files to your 3rdparty folder: e.g. 3rdparty\lib\x86\
8. delete compilation files because we will now do same steps starting from step 5 but in x64 Native Tools (copy them to 3rdparty\lib\x64
9. Copy that unistd.h file to 3rdparty include folder

SDL
---

1. Download SLD x864 & x64 here: https://github.com/libsdl-org/SDL/releases/tag/release-2.30.11
direct link: https://github.com/libsdl-org/SDL/releases/download/release-2.30.11/SDL2-devel-2.30.11-VC.zip
2. copy libraries to corresponding x86 and x64 folders 
3. copy headers to 3rdparty\include\SDL2 folder
4. copy files from cmake folder to 3rdparty\cmake

4. Download SLD_mixer here: https://github.com/libsdl-org/SDL_mixer/releases
direct link: https://github.com/libsdl-org/SDL_mixer/releases/download/release-2.8.0/SDL2_mixer-devel-2.8.0-VC.zip
5. do same with lib/dll and headers as with SDL (headers should also go to 3rdparty\include\SDL2\ folder)
6. copy files from cmake folder to 3rdparty\cmake

7 Download SDL2_ttf here: https://github.com/libsdl-org/SDL_ttf/releases/tag/release-2.24.0
direct link: https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.24.0/SDL2_ttf-devel-2.24.0-VC.zip
8. do same with lib/dll and headers as with SDL (headers should also go to 3rdparty\include\SDL2\ folder)
9. copy files from cmake folder to 3rdparty\cmake


glew
----
From sources:
1. Download glew here: https://sourceforge.net/projects/glew/files/glew/snapshots/glew-20190928.tgz/download
2. unzip and open build\vs12\glew.sln file
3. build for x64 and Win32

Binaries:
2. Or download prebuilt lib/dlls from this page: https://glew.sourceforge.net/ 
3. Put lib/dll/headers accordingly to x86/x64 (headers go into 3rdparty\include\GL)


Compiling mc2
=============
```
git clone https://github.com/alariq/mc2.git
cd mc2
md build64
cd build64
cmake.exe -G "Visual Studio 17 2022" -DCMAKE_PREFIX_PATH=c:/path_to_3rdparty_folder/ -DCMAKE_LIBRARY_ARCHITECTURE=x64 ..
```
(use absolute path to 3rdparty folder)
Copy mc2.exe to your executable folder of preference (say mc2exe)
```
cd res
md build64
cd build 64
cmake.exe -G "Visual Studio 17 2022" -DCMAKE_LIBRARY_ARCHITECTURE=x64 ..
```
Copy mc2res.dll/pdb to your executable folder of preference (say mc2exe)


Building data
=============
```
git clone https://github.com/alariq/mc2srcdata.git
cd mc2srcdata
```

1. Read `README.md` in `build_scripts` folder

If you did not here are the steps:
1. copy tools from the exe solution to build_scripts folder 
(better copy Release version of these to make things faster)
    aseconv
    makefst
    makersp
    pak
    text_tool
1a. Copy glew32.dll there as well (x86 or x64 depending on what version of tools you've built)
2. launch some console which has `make` in its path (needs GNUMake)
(you can install it from here: https://gnuwin32.sourceforge.net/packages/make.htm)
3. `cd build_scripts`
4. `make all` (or `>c:\path_to_gnumake\bin\make all`)
5. copy assets & data folder to your exe folder of preference
6. copy `*.cfg, *.fst, testtxm.tga` to your exe folder of preference


Final steps:
-----------
Copy all required dlls to your exe folder of preference

Run the game!

.. and, hopefully, enjoy

