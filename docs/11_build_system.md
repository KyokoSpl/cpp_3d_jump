# 11. Build System (CMake)

This document explains how the project is compiled using CMake.

## What is CMake?

CMake is a **build system generator**. It doesn't compile code directly - it generates build files for other tools:

- On Linux: Generates Makefiles → `make` compiles the code
- On Windows: Generates Visual Studio projects → VS compiles
- On macOS: Can generate Makefiles or Xcode projects

This lets one CMakeLists.txt work on all platforms.

## Project CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.10)
project(processing3d)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

### Breaking It Down

```cmake
cmake_minimum_required(VERSION 3.10)
```
- Sets minimum CMake version required
- Prevents users with old CMake from confusing errors

```cmake
project(processing3d)
```
- Names the project
- The executable will be called `processing3d`

```cmake
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```
- Use C++11 features (lambdas, auto, nullptr, etc.)
- `REQUIRED ON` means fail if compiler doesn't support C++11

## Finding Libraries

```cmake
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(Freetype REQUIRED)
```

### What find_package Does

1. Searches standard locations for the library
2. Sets variables like `OpenGL_FOUND`, `OPENGL_LIBRARIES`
3. Sets include paths and library paths
4. `REQUIRED` = stop with error if not found

### Common Find Locations

```
Linux:
  /usr/lib/cmake/
  /usr/local/lib/cmake/
  /usr/share/cmake/Modules/

macOS:
  /usr/local/lib/cmake/  (Homebrew)
  /opt/homebrew/lib/cmake/  (Apple Silicon)

Windows:
  C:/Program Files/cmake/share/cmake/Modules/
```

## Defining the Executable

```cmake
add_executable(processing3d
    main.cpp
    UserInput.cpp
    Obstacle.cpp
    Grid.cpp
    Menu.cpp
    Projectile.cpp
)
```

Lists all source files that make up the program.

### Header Files

Headers (.h) are NOT listed - they're included automatically via `#include`.
CMake tracks dependencies through includes.

## Include Directories

```cmake
target_include_directories(processing3d PRIVATE
    ${OPENGL_INCLUDE_DIR}
    ${FREETYPE_INCLUDE_DIRS}
)
```

Tells the compiler where to find header files:
- `-I/usr/include/freetype2` (example)

### PUBLIC vs PRIVATE

- `PRIVATE`: Only this target uses these includes
- `PUBLIC`: This target AND targets that link to it
- `INTERFACE`: Only targets that link to it

For executables, use `PRIVATE`.

## Linking Libraries

```cmake
target_link_libraries(processing3d PRIVATE
    ${OPENGL_LIBRARIES}
    glfw
    ${FREETYPE_LIBRARIES}
    m       # Math library
    dl      # Dynamic loading
    pthread # Threading
)
```

Links compiled libraries to the executable.

### Common Link Errors

```
undefined reference to `glClear'
```
→ Missing `-lGL` (OpenGL library)

```
undefined reference to `glfwInit'
```
→ Missing `-lglfw` (GLFW library)

```
undefined reference to `sin'
```
→ Missing `-lm` (math library)

## Export Compile Commands

```cmake
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
```

Creates `compile_commands.json` in build directory.
Used by editors/IDEs for:
- Code completion
- Error checking
- Go to definition

## Asset Copying

```cmake
# Create asset directory in build folder
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/asset)

# Copy assets after build
add_custom_command(TARGET processing3d POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/asset
        ${CMAKE_BINARY_DIR}/asset
)
```

### Variables

- `CMAKE_SOURCE_DIR` - Where CMakeLists.txt is
- `CMAKE_BINARY_DIR` - Where you're building (build/)

### POST_BUILD Command

Runs after the executable is compiled:
1. Creates `build/asset/` directory
2. Copies `asset/*` to `build/asset/`

This ensures fonts and sounds are available at runtime.

## Complete CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.10)
project(processing3d)

# C++11 standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate compile_commands.json for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(Freetype REQUIRED)

# Define executable and source files
add_executable(processing3d
    main.cpp
    UserInput.cpp
    Obstacle.cpp
    Grid.cpp
    Menu.cpp
    Projectile.cpp
)

# Include directories for headers
target_include_directories(processing3d PRIVATE
    ${OPENGL_INCLUDE_DIR}
    ${FREETYPE_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(processing3d PRIVATE
    ${OPENGL_LIBRARIES}
    glfw
    ${FREETYPE_LIBRARIES}
    m
    dl
    pthread
)

# Copy assets to build directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/asset)
add_custom_command(TARGET processing3d POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/asset
        ${CMAKE_BINARY_DIR}/asset
)
```

## Building the Project

### First-Time Setup

```bash
# Create build directory
mkdir build
cd build

# Run CMake to generate Makefiles
cmake ..

# Compile
make
```

### Subsequent Builds

```bash
cd build
make
```

CMake only needs to be re-run if CMakeLists.txt changes.

### run.sh Script

```bash
#!/bin/bash
cd build
cmake ..
make -j$(nproc)  # Parallel compilation
./processing3d   # Run the game
```

`-j$(nproc)` uses all CPU cores for faster compilation.

## Build Types

```bash
# Debug build (slow, has debug symbols)
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release build (optimized)
cmake -DCMAKE_BUILD_TYPE=Release ..

# Release with debug info
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
```

### What Changes

Debug:
- `-g` flag (debug symbols)
- `-O0` (no optimization)
- Can use gdb/lldb debugger

Release:
- `-O3` (max optimization)
- No debug symbols
- Much faster runtime

## Compiler Flags

```cmake
# Add custom compiler flags
target_compile_options(processing3d PRIVATE
    -Wall        # Enable warnings
    -Wextra      # Extra warnings
    -Wpedantic   # Strict standard compliance
)

# For debug builds only
target_compile_options(processing3d PRIVATE
    $<$<CONFIG:Debug>:-fsanitize=address>
)
```

### Common Flags

- `-Wall` - Enable common warnings
- `-Wextra` - Additional warnings
- `-Werror` - Treat warnings as errors
- `-O2` - Optimization level 2
- `-g` - Include debug information
- `-fsanitize=address` - Memory error detection

## Understanding the Build Process

```
Source Files → Compiler → Object Files → Linker → Executable

main.cpp → g++ -c → main.o ─┐
UserInput.cpp → g++ -c → UserInput.o ─┤
Menu.cpp → g++ -c → Menu.o ─┼→ g++ → processing3d
Obstacle.cpp → g++ -c → Obstacle.o ─┤
Grid.cpp → g++ -c → Grid.o ─┘
```

### Compilation Steps

1. **Preprocessing**: `#include` files are inserted, `#define` macros expanded
2. **Compilation**: C++ code → assembly → object file (.o)
3. **Linking**: Object files + libraries → executable

## Generated Files

After running CMake:

```
build/
├── CMakeCache.txt       # CMake configuration cache
├── CMakeFiles/          # CMake internal files
├── Makefile             # Generated build instructions
├── compile_commands.json # For IDE support
├── processing3d         # The executable (after make)
├── settings.cfg         # Runtime settings
└── asset/
    └── BoldPixels.ttf   # Copied font
```

## Clean Build

```bash
# Remove build directory
rm -rf build

# Or clean build artifacts only
cd build
make clean
```

## Adding New Source Files

If you add a new `.cpp` file:

1. Edit CMakeLists.txt:
```cmake
add_executable(processing3d
    main.cpp
    UserInput.cpp
    Obstacle.cpp
    Grid.cpp
    Menu.cpp
    Projectile.cpp
    NewFile.cpp        # Add here
)
```

2. Re-run cmake:
```bash
cd build
cmake ..
make
```

## Common CMake Issues

### Package Not Found

```
-- Could NOT find OpenGL (missing: OPENGL_gl_LIBRARY)
```

Install the missing library:
```bash
# Ubuntu/Debian
sudo apt install libgl1-mesa-dev
sudo apt install libglfw3-dev
sudo apt install libfreetype-dev

# Arch Linux
sudo pacman -S mesa glfw-x11 freetype2

# macOS
brew install glfw freetype
```

### Old CMake Version

```
CMake 3.10 or higher is required.  You are running version 3.5
```

Update CMake:
```bash
# Ubuntu
sudo apt install cmake

# Or download from cmake.org
```

### Compiler Not Found

```
-- The CXX compiler identification is unknown
```

Install a compiler:
```bash
sudo apt install g++
# or
sudo apt install clang++
```

## Next Steps

You now understand the complete project! Try:
1. Modifying existing features
2. Adding new parkour elements
3. Creating new settings options
4. Experimenting with the physics

Happy coding! 🎮
