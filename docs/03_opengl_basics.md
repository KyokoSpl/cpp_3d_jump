# 3. OpenGL Basics

This document explains OpenGL concepts used in the game. We use "legacy" OpenGL (immediate mode) which is simpler to understand, though modern games use more advanced techniques.

## What is OpenGL?

OpenGL is an API for rendering 2D and 3D graphics. It talks to your graphics card (GPU) to draw shapes, textures, and more.

## Coordinate System

OpenGL uses a 3D coordinate system:

```
        Y (up)
        |
        |
        |_______ X (right)
       /
      /
     Z (towards you)
```

- **X-axis**: Left (-) to Right (+)
- **Y-axis**: Down (-) to Up (+)
- **Z-axis**: Away (-) to Towards (+)

## Drawing Primitives

### Points, Lines, Triangles

```cpp
// Drawing a triangle
glBegin(GL_TRIANGLES);
    glVertex3f(0, 0, 0);    // Vertex 1 (x, y, z)
    glVertex3f(1, 0, 0);    // Vertex 2
    glVertex3f(0.5, 1, 0);  // Vertex 3
glEnd();

// Drawing a line
glBegin(GL_LINES);
    glVertex3f(0, 0, 0);    // Start point
    glVertex3f(1, 1, 0);    // End point
glEnd();

// Drawing a quadrilateral (4 vertices)
glBegin(GL_QUADS);
    glVertex3f(0, 0, 0);    // Bottom-left
    glVertex3f(1, 0, 0);    // Bottom-right
    glVertex3f(1, 1, 0);    // Top-right
    glVertex3f(0, 1, 0);    // Top-left
glEnd();
```

### Primitive Types

| Type | Description |
|------|-------------|
| `GL_POINTS` | Individual points |
| `GL_LINES` | Pairs of vertices form lines |
| `GL_LINE_STRIP` | Connected lines |
| `GL_LINE_LOOP` | Connected lines, last connects to first |
| `GL_TRIANGLES` | Every 3 vertices form a triangle |
| `GL_QUADS` | Every 4 vertices form a quadrilateral |

## Colors

### Setting Colors

```cpp
// RGB color (values 0.0 to 1.0)
glColor3f(1.0f, 0.0f, 0.0f);  // Red
glColor3f(0.0f, 1.0f, 0.0f);  // Green
glColor3f(0.0f, 0.0f, 1.0f);  // Blue
glColor3f(1.0f, 1.0f, 1.0f);  // White
glColor3f(0.0f, 0.0f, 0.0f);  // Black
glColor3f(0.5f, 0.5f, 0.5f);  // Gray

// RGBA color (with alpha/transparency)
glColor4f(1.0f, 0.0f, 0.0f, 0.5f);  // 50% transparent red
```

Colors "stick" until you change them:

```cpp
glColor3f(1, 0, 0);  // Set to red
glBegin(GL_TRIANGLES);
    glVertex3f(...);  // Red vertex
    glVertex3f(...);  // Red vertex
    glColor3f(0, 1, 0);  // Change to green
    glVertex3f(...);  // Green vertex (creates gradient!)
glEnd();
```

## Drawing a 3D Box

From `Obstacle.cpp`:

```cpp
void ObstacleCourse::drawBox(const Box& box) {
    glColor3f(box.r, box.g, box.b);
    
    // Calculate corners
    float x1 = box.x - box.width / 2;   // Left
    float x2 = box.x + box.width / 2;   // Right
    float y1 = box.y;                    // Bottom
    float y2 = box.y + box.height;       // Top
    float z1 = box.z - box.depth / 2;   // Back
    float z2 = box.z + box.depth / 2;   // Front
    
    glBegin(GL_QUADS);
    
    // Front face (z = z2)
    glVertex3f(x1, y1, z2);
    glVertex3f(x2, y1, z2);
    glVertex3f(x2, y2, z2);
    glVertex3f(x1, y2, z2);
    
    // Back face (z = z1)
    glVertex3f(x2, y1, z1);
    glVertex3f(x1, y1, z1);
    glVertex3f(x1, y2, z1);
    glVertex3f(x2, y2, z1);
    
    // Top face (y = y2)
    glVertex3f(x1, y2, z2);
    glVertex3f(x2, y2, z2);
    glVertex3f(x2, y2, z1);
    glVertex3f(x1, y2, z1);
    
    // Bottom face (y = y1)
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y1, z1);
    glVertex3f(x2, y1, z2);
    glVertex3f(x1, y1, z2);
    
    // Right face (x = x2)
    glVertex3f(x2, y1, z2);
    glVertex3f(x2, y1, z1);
    glVertex3f(x2, y2, z1);
    glVertex3f(x2, y2, z2);
    
    // Left face (x = x1)
    glVertex3f(x1, y1, z1);
    glVertex3f(x1, y1, z2);
    glVertex3f(x1, y2, z2);
    glVertex3f(x1, y2, z1);
    
    glEnd();
}
```

## Matrix Transformations

OpenGL uses matrices to transform coordinates. There are two main matrix modes:

### Projection Matrix

Controls how 3D world maps to 2D screen (perspective, field of view):

```cpp
glMatrixMode(GL_PROJECTION);
glLoadIdentity();  // Reset to identity matrix

// Create perspective projection
// fov: field of view angle
// aspect: width/height ratio
// near, far: clipping planes
float f = 1.0f / tan(fovRadians / 2.0f);
float rangeInv = 1.0f / (near - far);

float matrix[16] = {
    f / aspect, 0, 0, 0,
    0, f, 0, 0,
    0, 0, (near + far) * rangeInv, -1,
    0, 0, near * far * rangeInv * 2.0f, 0
};
glMultMatrixf(matrix);
```

### Modelview Matrix

Controls camera position and object transformations:

```cpp
glMatrixMode(GL_MODELVIEW);
glLoadIdentity();

// Look at a target from a position
// Camera at (camX, camY, camZ), looking at (targetX, targetY, targetZ)
```

### Push/Pop Matrix

Save and restore matrix state:

```cpp
glPushMatrix();    // Save current transformation

// Apply transformations for one object
glTranslatef(10, 0, 0);
drawObject();

glPopMatrix();     // Restore saved state

// Next object isn't affected by the translate
drawAnotherObject();
```

## Depth Testing

Without depth testing, objects drawn later appear on top:

```cpp
// Enable depth testing
glEnable(GL_DEPTH_TEST);

// Clear both color and depth buffers each frame
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
```

The **depth buffer** (Z-buffer) stores the depth of each pixel. When drawing, OpenGL only updates pixels if they're closer to the camera.

## Blending (Transparency)

For transparent objects:

```cpp
// Enable blending
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

// Draw semi-transparent overlay
glColor4f(0.0f, 0.0f, 0.0f, 0.7f);  // 70% black
glBegin(GL_QUADS);
// ... draw quad covering screen
glEnd();

// Disable when done
glDisable(GL_BLEND);
```

## 2D Rendering (UI/HUD)

For menus and HUD, we switch to orthographic (2D) projection:

```cpp
// Save 3D state
glMatrixMode(GL_PROJECTION);
glPushMatrix();
glLoadIdentity();

// Set up 2D coordinates (0,0 at bottom-left)
glOrtho(0, windowWidth, 0, windowHeight, -1, 1);

glMatrixMode(GL_MODELVIEW);
glPushMatrix();
glLoadIdentity();

// Disable depth test for 2D
glDisable(GL_DEPTH_TEST);

// Draw 2D elements
glColor3f(1, 1, 1);
drawText(20, windowHeight - 50, "Score: 100");

// Restore 3D state
glEnable(GL_DEPTH_TEST);

glMatrixMode(GL_PROJECTION);
glPopMatrix();
glMatrixMode(GL_MODELVIEW);
glPopMatrix();
```

## Line Width

Control line thickness:

```cpp
glLineWidth(2.0f);  // 2 pixel wide lines

glBegin(GL_LINES);
// ... draw lines
glEnd();

glLineWidth(1.0f);  // Reset to default
```

## Clearing the Screen

Every frame, clear the previous frame:

```cpp
// Set clear color (background)
glClearColor(0.1f, 0.15f, 0.2f, 1.0f);  // Dark blue-gray

// Clear color buffer and depth buffer
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
```

## GLFW Window Management

### Creating a Window

```cpp
// Initialize GLFW
if (!glfwInit()) {
    return -1;
}

// Create window
GLFWwindow* window = glfwCreateWindow(800, 600, "3D Game", NULL, NULL);
if (!window) {
    glfwTerminate();
    return -1;
}

// Make OpenGL context current
glfwMakeContextCurrent(window);

// Enable VSync (limit to monitor refresh rate)
glfwSwapInterval(1);
```

### Swap Buffers (Double Buffering)

Games use **double buffering**: draw to a hidden buffer, then swap it to the screen:

```cpp
// Game loop
while (!glfwWindowShouldClose(window)) {
    // Draw to back buffer
    glClear(...);
    drawEverything();
    
    // Swap back buffer to front (show what we drew)
    glfwSwapBuffers(window);
    
    // Process input events
    glfwPollEvents();
}
```

Without double buffering, you'd see the drawing in progress (flickering).

## Common OpenGL Functions Used

| Function | Purpose |
|----------|---------|
| `glBegin(mode)` | Start drawing primitives |
| `glEnd()` | Finish drawing primitives |
| `glVertex3f(x,y,z)` | Specify a vertex |
| `glColor3f(r,g,b)` | Set current color |
| `glColor4f(r,g,b,a)` | Set color with alpha |
| `glEnable(cap)` | Enable a capability |
| `glDisable(cap)` | Disable a capability |
| `glMatrixMode(mode)` | Set matrix to modify |
| `glLoadIdentity()` | Reset matrix |
| `glPushMatrix()` | Save current matrix |
| `glPopMatrix()` | Restore saved matrix |
| `glTranslatef(x,y,z)` | Move origin |
| `glRotatef(angle,x,y,z)` | Rotate around axis |
| `glScalef(x,y,z)` | Scale objects |
| `glClear(mask)` | Clear buffers |
| `glClearColor(r,g,b,a)` | Set clear color |
| `glLineWidth(w)` | Set line thickness |
| `glOrtho(...)` | 2D projection |

## Next Steps

Continue to [Game Architecture](./04_game_architecture.md) to see how the game systems work together.
