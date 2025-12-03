# C++ 3D Jump

A first-person 3D obstacle course game built with C++ and OpenGL.

## Dependencies

- OpenGL
- GLFW3 (Graphics Library Framework)
- FreeType (Font rendering)
- CMake (build system)

## Installation of Dependencies

### Ubuntu/Debian:

```bash
sudo apt-get install libglfw3-dev libfreetype-dev libgl1-mesa-dev libglu1-mesa-dev cmake build-essential
```

### Fedora:

```bash
sudo dnf install glfw-devel freetype-devel mesa-libGL-devel mesa-libGLU-devel cmake gcc-c++
```

### Arch Linux:

```bash
sudo pacman -S glfw freetype2 mesa glu cmake base-devel
```

### macOS (with Homebrew):

```bash
brew install glfw freetype cmake
```

## Building

> **Note:** The first build requires an internet connection to download [miniaudio.h](https://github.com/mackron/miniaudio) (~1MB). This is done automatically by CMake. Subsequent builds work offline.

```bash
mkdir build
cd build
cmake ..
make
```

## Running

```bash
./cpp_3d_jump
```

## Controls

- **W/A/S/D**: Move forward/left/backward/right
- **Mouse**: Look around
- **Space**: Jump
- **Shift**: Crouch (hold to duck under low obstacles)
- **ESC**: Exit

## Features

- 3D grid rendering
- First-person camera control
- Mouse look (captured cursor)
- WASD movement
- Jump mechanics with gravity
- Crouching (Shift key)
- Jump and run obstacle course with:
  - Barriers to jump over
  - Walls to navigate around
  - Low tunnels to crouch through
  - Narrow passages
- Collision detection
- Fullscreen window

## Sound Customization

The popup notification sound can be customized in `src/Menu.cpp` in the `generatePopupSound()` function:

- **frequency**: Any value in Hz works. Recommended range: 200-2000 Hz
  - 261.63 Hz = C4 (Middle C)
  - 440.00 Hz = A4 (Concert pitch)
  - 523.25 Hz = C5
  - 659.25 Hz = E5
  - 880.00 Hz = A5
  - 1046.50 Hz = C6
- **duration**: Length in milliseconds (50-200 recommended for a short blip)

The sound is generated as a sine wave with harmonics and an envelope for a pleasant "blob" effect.

## Notes

- The code uses legacy OpenGL (immediate mode) for simplicity
- Camera is implemented using manual matrix transformations
- Audio is handled by [miniaudio](https://github.com/mackron/miniaudio) (single-header library, downloaded automatically on first build)

## External Dependencies

This project uses the following external library that is downloaded automatically during the first build:

| Library | Purpose | License |
|---------|---------|---------|
| [miniaudio](https://github.com/mackron/miniaudio) | Cross-platform audio | Public Domain / MIT-0 |

## TODOs:

- [ ] implement server side time saving for leader board (database and api prob)
- [ ] fix I hate my self difficulty (just spam space and run forward to complete, no kind of difficulty if exploit is known)
- [ ] add crouching, wallriding and double jump obsticales that work
- [ ] sprint ability with storing velocity for further jumps if jumped while sprint
