# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

### Prerequisites
- **Visual Studio 2022** with C++ build tools
- **CMake** (3.10 or higher)
- **Windows SDK** 10.0.22621.0 or similar
- **Git** (for cloning the repository)

### Windows Build Process (TESTED & WORKING)

**Step 1: Extract 3rdparty Dependencies**
The repository includes a `3rdparty.zip` file that contains all necessary libraries. This needs to be extracted to a temporary `3rdparty/` folder (this folder is not committed to git):
```
mc2/
├── 3rdparty.zip          (committed)
├── 3rdparty/             (temporary, extract from zip)
│   ├── cmake/
│   ├── include/
│   └── lib/
│       ├── x64/
│       └── x86/
```

**Step 2: Extract Dependencies (if not using setup script)**
```bash
# Extract 3rdparty.zip to create temporary 3rdparty/ folder
powershell -Command "Expand-Archive -Path '3rdparty.zip' -DestinationPath '.' -Force"
```

**Step 3: Main Application Build**
```bash
# Navigate to project root
cd G:/games source code/games/mc2

# Create build directory
mkdir build64
cd build64

# Configure with CMake - use ABSOLUTE path to 3rdparty
cmake.exe -G "Visual Studio 17 2022" -DCMAKE_PREFIX_PATH="G:/games source code/games/mc2/3rdparty" -DCMAKE_LIBRARY_ARCHITECTURE=x64 ..

# Build the project
cmake --build . --config Release
```

**Step 4: Resource DLL Build**
```bash
# Navigate to res directory
cd G:/games source code/games/mc2/res

# Create build directory
mkdir build64
cd build64

# Configure resource DLL build
cmake.exe -G "Visual Studio 17 2022" -DCMAKE_LIBRARY_ARCHITECTURE=x64 ..

# Build the DLL
cmake --build . --config Release
```

**Step 5: Prepare String Resources**
```bash
# Copy the pre-generated string resources to res directory
copy "test_scripts\res_conv\strings.res.cpp" "res\"
copy "test_scripts\res_conv\strings.res.h" "res\"
```

### Build Outputs
After successful build, you'll find:
- **Main executable**: `build64/Release/mc2.exe`
- **Resource DLL**: `build64/out/res/Release/mc2res_64.dll`
- **Data tools**: `build64/out/data_tools/Release/` (aseconv.exe, makefst.exe, makersp.exe, pak.exe)
- **Text tool**: `build64/out/text_tool/Release/text_tool.exe`
- **Viewer**: `build64/out/Viewer/Release/viewer.exe`

### Build Notes
- The build generates many warnings about type conversions (size_t to int, etc.) but these are expected for this legacy codebase
- The CMake configuration automatically detects x64 vs x86 architecture
- SDL2, GLEW, and zlib are all properly detected from the 3rdparty folder
- The resource DLL is built as mc2res_64.dll for 64-bit builds

### Data Building
Game data is built separately using the mc2srcdata repository. Use the tools from the build output:
- Copy all tools from `build64/out/data_tools/Release/` to `mc2srcdata/build_scripts/`
- Copy required DLLs from `3rdparty/lib/x64/` to the same folder
- Run `make all` in the build_scripts folder

### Quick Start Scripts
Two convenience scripts are provided:
- **`setup_build_environment.bat`** - Extracts 3rdparty.zip and checks build tools
- **`build_windows.bat`** - Complete build process for Windows

**Note:** The scripts automatically extract `3rdparty.zip` to a temporary `3rdparty/` folder. This folder is not committed to git and should be treated as a build artifact. The original `3rdparty.zip` remains in the repository for convenience.

### Common Issues and Solutions

**CMake can't find libraries:**
- Ensure you're using the ABSOLUTE path to the 3rdparty folder
- Verify that 3rdparty.zip was extracted properly
- Check that you're using the correct architecture (x64)

**Visual Studio not found:**
- Install Visual Studio 2022 with C++ Desktop Development workload
- Run builds from Visual Studio Developer Command Prompt
- Ensure Windows SDK is installed

**Build warnings:**
- The build generates many warnings about type conversions (size_t to int, etc.)
- These are expected for this legacy 2001 codebase and don't affect functionality

**Missing DLLs at runtime:**
- Copy all DLLs from `3rdparty/lib/x64/` to your executable directory
- Required DLLs: SDL2.dll, SDL2_mixer.dll, SDL2_ttf.dll, glew32.dll, zlib1.dll

**String resource errors:**
- Copy strings.res.cpp and strings.res.h from `test_scripts/res_conv/` to `res/`
- These are pre-generated and don't need to be rebuilt

## Architecture Overview

### Core Architecture
This is MechCommander 2, a real-time strategy game engine with these major components:

**GameOS Layer** (`GameOS/`):
- Cross-platform abstraction layer for graphics, input, sound, and file I/O
- Contains the main game loop and window management
- Handles OpenGL rendering, SDL integration

**MCLib** (`mclib/`):
- Core game engine library with utilities and low-level systems
- Contains key subsystems:
  - **MLR** (`mclib/mlr/`): 3D rendering and mesh system
  - **gosfx** (`mclib/gosfx/`): Special effects and particle system  
  - **stuff** (`mclib/stuff/`): Math utilities (vectors, matrices, etc.)
- File I/O, memory management, terrain rendering

**GUI System** (`gui/`):
- Custom GUI framework for game menus and interfaces
- Animation system, buttons, listboxes, text editing
- Logistics screens for mech configuration

**Game Logic** (`code/`):
- Core game entities: mechs, buildings, weapons, terrain objects
- Mission system, AI, physics, collision detection
- Campaign progression, pilot management
- Multiplayer networking support

### Key Systems

**Rendering Pipeline:**
- Uses OpenGL with custom shader system (`shaders/`)
- Terrain rendering with texture management
- 3D model loading and animation
- Particle effects and special effects

**Game Objects:**
- Hierarchical object system with base GameObject class
- Specialized types: Mech, Building, Vehicle, Turret, etc.
- Component-based approach for weapons, sensors, etc.

**Mission System:**
- ABL (A Behavior Language) scripting for AI and missions
- Trigger system for mission events
- Save/load game state management

**Audio System:**
- Sound effects, music, and voice-over support
- 3D positional audio (partially implemented)

### Build Configuration
- Debug builds use `-D_ARMOR -D_DEBUG -DBUGLOG -DLAB_ONLY` flags
- Cross-platform compilation with Windows and Linux support
- Uses CMake with separate configurations for x86/x64

### Testing
No automated test framework is present. Testing is manual gameplay validation.

### Data Files
Game uses custom file formats (.fst, .pak, .tga) built from source assets in separate mc2srcdata repository.