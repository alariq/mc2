@echo off
setlocal enabledelayedexpansion

echo ================================================
echo MechCommander 2 - Build Environment Setup
echo ================================================
echo.

REM Check if we're in the correct directory
if not exist "CMakeLists.txt" (
    echo ERROR: CMakeLists.txt not found. Please run this script from the mc2 root directory.
    exit /b 1
)

REM Check if 3rdparty.zip exists
if not exist "3rdparty.zip" (
    echo ERROR: 3rdparty.zip not found in the current directory.
    echo Please ensure 3rdparty.zip is present in the mc2 root directory.
    exit /b 1
)

REM Extract 3rdparty.zip if 3rdparty folder doesn't exist
if not exist "3rdparty" (
    echo Extracting 3rdparty dependencies...
    powershell -Command "Expand-Archive -Path '3rdparty.zip' -DestinationPath '.' -Force"
    if errorlevel 1 (
        echo ERROR: Failed to extract 3rdparty.zip
        echo Please extract 3rdparty.zip manually to the mc2 root directory.
        exit /b 1
    )
    echo 3rdparty dependencies extracted successfully!
) else (
    echo 3rdparty folder already exists.
)

REM Verify 3rdparty structure
if not exist "3rdparty\lib\x64" (
    echo ERROR: 3rdparty\lib\x64 not found. The 3rdparty.zip may be corrupted.
    exit /b 1
)

if not exist "3rdparty\include" (
    echo ERROR: 3rdparty\include not found. The 3rdparty.zip may be corrupted.
    exit /b 1
)

if not exist "3rdparty\cmake" (
    echo ERROR: 3rdparty\cmake not found. The 3rdparty.zip may be corrupted.
    exit /b 1
)

echo.
echo ================================================
echo ENVIRONMENT SETUP COMPLETE!
echo ================================================
echo.
echo 3rdparty dependencies are ready.
echo You can now run 'build_windows.bat' to build the project.
echo.

REM Check for Visual Studio and CMake
echo Checking build tools...
cmake --version >nul 2>&1
if errorlevel 1 (
    echo WARNING: CMake not found in PATH. Please ensure CMake is installed and in your PATH.
) else (
    echo CMake: Found
)

where cl.exe >nul 2>&1
if errorlevel 1 (
    echo WARNING: Visual Studio compiler not found. Please ensure Visual Studio 2022 is installed.
    echo You may need to run this from a Visual Studio Developer Command Prompt.
) else (
    echo Visual Studio: Found
)

echo.
echo Ready to build!
echo Run 'build_windows.bat' to start the build process.
echo.

pause