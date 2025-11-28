# 1. Project Overview

## What is This Game?

This is a 3D parkour platformer game where you control a stick figure character navigating through an obstacle course. The game features:

- **Third-person camera** that follows the player
- **Physics-based movement** with gravity, jumping, and crouching
- **Parkour obstacles** like barriers, tunnels, and platforms
- **Checkpoints** and **death zones**
- **Timer system** for speedrunning
- **Settings menu** with customizable controls

## How Games Work (The Game Loop)

Every game follows a pattern called the **game loop**. It's like a heartbeat that runs continuously:

```
while (game is running) {
    1. Handle Input     - Check what keys/mouse the player pressed
    2. Update State     - Move objects, apply physics, check collisions
    3. Render           - Draw everything to the screen
    4. Wait             - Control frame rate
}
```

In our code (`main.cpp`), this looks like:

```cpp
while (!glfwWindowShouldClose(window)) {
    // 1. Handle Input (done through callbacks)
    glfwPollEvents();
    
    // Calculate time since last frame
    float deltaTime = currentTime - lastTime;
    
    // 2. Update State
    userInput->update(windowWidth, windowHeight, obstacles, grid, deltaTime);
    userInput->move(w, s, a, d, obstacles, deltaTime);
    
    // 3. Render
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    userInput->render();
    obstacles->render(deltaTime);
    grid->render();
    
    // 4. Swap buffers (show what we drew)
    glfwSwapBuffers(window);
}
```

## What is Delta Time?

**Delta time** (`deltaTime`) is crucial for smooth gameplay. It measures how much time passed since the last frame.

### Why Do We Need It?

Without delta time, game speed depends on frame rate:
- Fast computer (120 FPS) → Character moves 120 times per second
- Slow computer (30 FPS) → Character moves 30 times per second

With delta time, we multiply movement by how much time passed:

```cpp
// BAD: Position depends on frame rate
playerX += 5;  // Moves 5 units every frame

// GOOD: Position independent of frame rate
playerX += 5 * deltaTime * 60;  // Moves 5 units per 1/60th of a second
```

At 60 FPS: `deltaTime = 1/60 = 0.0167`, so movement = `5 * 0.0167 * 60 = 5`
At 30 FPS: `deltaTime = 1/30 = 0.0333`, so movement = `5 * 0.0333 * 60 = 10` per frame, but only 30 frames = same total

## Files and Their Purposes

### main.cpp
The entry point of the program. Contains:
- Window creation with GLFW
- Game loop
- Input callbacks (keyboard, mouse)
- Initialization of all game systems

### UserInput.h / UserInput.cpp
Handles everything about the player:
- Position (X, Y, Z coordinates)
- Movement (walking, jumping, crouching)
- Physics (gravity, velocity)
- Camera (view angle, zoom)
- Stats (timer, death count)

### Obstacle.h / Obstacle.cpp
Manages the parkour course:
- Box structures (platforms, barriers)
- Collision detection
- Checkpoints and death zones
- Rendering obstacles

### Menu.h / Menu.cpp
The pause menu and settings:
- UI rendering (buttons, sliders, checkboxes)
- Settings management (controls, graphics)
- Font rendering with FreeType
- Settings file I/O

### Grid.h / Grid.cpp
The ground plane:
- Renders a grid pattern
- Defines play area boundaries

## Libraries Used

### GLFW (Graphics Library Framework)
Creates windows and handles input. Think of it as the "container" for our game.

```cpp
// Create a window
GLFWwindow* window = glfwCreateWindow(800, 600, "Game", NULL, NULL);

// Check if a key is pressed
if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    // W key is pressed
}
```

### OpenGL (Open Graphics Library)
Draws everything on screen. We use the "legacy" immediate mode which is simpler to understand.

```cpp
// Draw a triangle
glBegin(GL_TRIANGLES);
glVertex3f(0, 0, 0);   // Point 1
glVertex3f(1, 0, 0);   // Point 2
glVertex3f(0.5, 1, 0); // Point 3
glEnd();
```

### FreeType
Renders text using TrueType fonts (.ttf files).

### miniaudio
Single-header library for playing sounds. Works on Windows, Mac, and Linux.

## Next Steps

Continue to [C++ Fundamentals Used](./02_cpp_fundamentals.md) to learn about the C++ features used in this project.
