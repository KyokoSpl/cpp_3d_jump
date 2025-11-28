# Processing 3D to C++ Conversion

This is a C++ conversion of the Processing 3D grid navigation sketch.

## Dependencies

- OpenGL
- GLEW (OpenGL Extension Wrangler Library)
- GLFW3 (Graphics Library Framework)
- CMake (build system)

## Installation of Dependencies

### Ubuntu/Debian:
```bash
sudo apt-get install libglew-dev libglfw3-dev cmake build-essential
```

### Fedora:
```bash
sudo dnf install glew-devel glfw-devel cmake gcc-c++
```

### Arch Linux:
```bash
sudo pacman -S glew glfw-x11 cmake base-devel
```

### macOS (with Homebrew):
```bash
brew install glew glfw cmake
```

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Running

```bash
./processing3d
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

The popup notification sound can be customized in `Menu.cpp` in the `generatePopupSound()` function:

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

- The code uses legacy OpenGL (immediate mode) for simplicity, similar to Processing's approach
- For production code, consider using modern OpenGL with shaders and VBOs
- Camera is implemented using manual matrix transformations to match Processing's `camera()` function
