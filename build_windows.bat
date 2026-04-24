@echo off
setlocal enabledelayedexpansion

echo =====================================
echo MechCommander 2 - Windows Build Script
echo =====================================
echo.

REM Check if we're in the correct directory
if not exist "CMakeLists.txt" (
    echo ERROR: CMakeLists.txt not found. Please run this script from the mc2 root directory.
    exit /b 1
)

REM Check if 3rdparty folder exists
if not exist "3rdparty" (
    echo ERROR: 3rdparty folder not found. Please extract 3rdparty.zip first.
    exit /b 1
)

REM Get the current directory for absolute path
set "MC2_ROOT=%CD%"
set "THIRDPARTY_PATH=%MC2_ROOT%\3rdparty"

echo Using 3rdparty path: %THIRDPARTY_PATH%
echo.

REM Step 1: Build main application
echo Step 1: Building main application...
if not exist "build64" mkdir build64
cd build64

echo Configuring CMake...
cmake.exe -G "Visual Studio 17 2022" -DCMAKE_PREFIX_PATH="%THIRDPARTY_PATH%" -DCMAKE_LIBRARY_ARCHITECTURE=x64 ..
if errorlevel 1 (
    echo ERROR: CMake configuration failed
    exit /b 1
)

echo Building main application...
cmake --build . --config Release
if errorlevel 1 (
    echo ERROR: Main application build failed
    exit /b 1
)

echo Main application built successfully!
echo.

REM Step 2: Build resource DLL
echo Step 2: Building resource DLL...
cd "%MC2_ROOT%\res"
if not exist "build64" mkdir build64
cd build64

echo Configuring resource DLL...
cmake.exe -G "Visual Studio 17 2022" -DCMAKE_LIBRARY_ARCHITECTURE=x64 ..
if errorlevel 1 (
    echo ERROR: Resource DLL CMake configuration failed
    exit /b 1
)

echo Building resource DLL...
cmake --build . --config Release
if errorlevel 1 (
    echo ERROR: Resource DLL build failed
    exit /b 1
)

echo Resource DLL built successfully!
echo.

REM Step 3: Copy string resources (if not already present)
echo Step 3: Setting up string resources...
cd "%MC2_ROOT%\res"
if not exist "strings.res.cpp" (
    echo Copying strings.res.cpp...
    copy "%MC2_ROOT%\test_scripts\res_conv\strings.res.cpp" "."
)
if not exist "strings.res.h" (
    echo Copying strings.res.h...
    copy "%MC2_ROOT%\test_scripts\res_conv\strings.res.h" "."
)

echo.
echo =====================================
echo BUILD COMPLETED SUCCESSFULLY!
echo =====================================
echo.
echo Build outputs:
echo - Main executable: build64\Release\mc2.exe
echo - Resource DLL: build64\out\res\Release\mc2res_64.dll
echo - Data tools: build64\out\data_tools\Release\
echo - Text tool: build64\out\text_tool\Release\text_tool.exe
echo - Viewer: build64\out\Viewer\Release\viewer.exe
echo.
echo To run the game, you'll need to:
echo 1. Copy mc2.exe and mc2res_64.dll to the same directory
echo 2. Copy required DLLs from 3rdparty\lib\x64\
echo 3. Build game data using mc2srcdata repository
echo.
echo Next steps for data building:
echo 1. Clone https://github.com/alariq/mc2srcdata
echo 2. Copy tools from build64\out\data_tools\Release\ to mc2srcdata\build_scripts\
echo 3. Copy x64 DLLs from 3rdparty\lib\x64\ to mc2srcdata\build_scripts\
echo 4. Run 'make all' in mc2srcdata\build_scripts\
echo.

cd "%MC2_ROOT%"
pause