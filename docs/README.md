# 3D Parkour Game - Code Documentation

Welcome to the detailed documentation for this 3D parkour game built with C++ and OpenGL. This documentation is designed for C++ beginners to understand how the project works and learn from it.

## Table of Contents

1. [Project Overview](./01_project_overview.md) - Game features, file structure, how it all fits together
2. [C++ Fundamentals Used](./02_cpp_fundamentals.md) - Classes, structs, pointers, memory management
3. [OpenGL Basics](./03_opengl_basics.md) - Coordinate system, immediate mode, matrices
4. [Game Architecture](./04_game_architecture.md) - Class relationships, game loop, state management
5. [Player Movement & Physics](./05_player_physics.md) - Movement, gravity, collision, wall running
6. [3D Rendering](./06_3d_rendering.md) - Camera setup, drawing 3D shapes, mannequin
7. [Menu System & UI](./07_menu_system.md) - State machine, FreeType text, buttons
8. [Settings & File I/O](./08_settings_file_io.md) - Saving/loading settings, parsing
9. [Audio System](./09_audio_system.md) - miniaudio library, sound generation
10. [Collision System](./10_collision_system.md) - AABB detection, resolution, optimization
11. [Build System (CMake)](./11_build_system.md) - CMakeLists.txt explained, compilation
12. [Future Improvements](./12_future_improvements.md) - Feature ideas, resources, planning guide

## Quick Start

To build and run the game:

> **Note:** The first build requires an internet connection to download [miniaudio.h](https://github.com/mackron/miniaudio) (~1MB). This is done automatically by CMake. Subsequent builds work offline.

```bash
cd build
cmake ..
make -j4
./cpp_3d_jump
```

## Project Structure

```
cpp_3d_jump/
├── src/                       # All source code
│   ├── main.cpp               # Entry point, game loop, input handling
│   ├── UserInput.cpp/h        # Player movement, physics, camera
│   ├── Obstacle.cpp/h         # Parkour course, collision detection
│   ├── Grid.cpp/h             # Ground grid rendering
│   ├── Projectile.cpp/h       # Projectile system (optional feature)
│   └── menus/                 # Menu system (split into modules)
│       ├── Menu.cpp/h         # Core menu functionality
│       ├── MenuRender.cpp     # All rendering logic (830 lines)
│       ├── MenuInput.cpp      # Input handling (552 lines)
│       ├── MenuAudio.cpp/h    # Audio system (151 lines)
│       ├── Settings.cpp/h     # Settings management (145 lines)
│       └── Leaderboard.cpp/h  # Leaderboard data (153 lines)
├── CMakeLists.txt             # Build configuration (auto-downloads miniaudio.h)
├── asset/                     # Game assets (fonts, sounds)
├── build/                     # Build output (includes leaderboard.json)
└── docs/                      # This documentation
```

## Key Concepts You'll Learn

- **Object-Oriented Programming**: Classes, encapsulation, member functions
- **3D Math**: Vectors, matrices, trigonometry for 3D graphics
- **Game Loop**: Fixed timestep, delta time, frame-rate independence
- **OpenGL**: Legacy immediate mode rendering, transformations
- **Physics**: Gravity, collision detection, player movement
- **Input Handling**: Keyboard, mouse, event callbacks
- **File I/O**: Saving and loading settings, JSON leaderboard
- **Audio**: Cross-platform sound playback

## Prerequisites

- Basic C++ knowledge (variables, functions, loops, conditionals)
- Understanding of basic math (coordinates, angles)
- Familiarity with the command line

Let's dive in! Start with [Project Overview](./01_project_overview.md).
