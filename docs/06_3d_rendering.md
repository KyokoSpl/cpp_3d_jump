# 6. 3D Rendering

This document explains how the game draws the 3D world, the player character (mannequin), and obstacles.

## Rendering Pipeline Overview

Every frame:

```
1. Clear screen (remove last frame)
2. Set up camera (projection + view matrices)
3. Draw world objects (obstacles, grid)
4. Draw player (mannequin)
5. Draw UI (menus, HUD) in 2D
6. Present frame (swap buffers)
```

## Setting Up the Camera

### Projection Matrix

The projection matrix defines how 3D coordinates map to 2D screen:

```cpp
void UserInput::update(int windowWidth, int windowHeight, ...) {
    // Set up projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    float aspect = (float)windowWidth / (float)windowHeight;
    float fovRad = fov * M_PI / 180.0f;  // Convert degrees to radians
    float nearPlane = 0.1f;
    float farPlane = renderDistance;
    
    // Create perspective projection matrix
    float f = 1.0f / tan(fovRad / 2.0f);
    float rangeInv = 1.0f / (nearPlane - farPlane);
    
    float matrix[16] = {
        f / aspect, 0, 0, 0,
        0, f, 0, 0,
        0, 0, (nearPlane + farPlane) * rangeInv, -1,
        0, 0, nearPlane * farPlane * rangeInv * 2.0f, 0
    };
    glMultMatrixf(matrix);
}
```

### View Matrix (Camera Transform)

The view matrix positions the camera in the world:

```cpp
    // Calculate camera position (behind player)
    Vector3 viewDir = getViewVector();
    
    float cameraX = playerX - viewDir.x * cameraDistance;
    float cameraY = playerY + playerHeight * 0.5f - viewDir.y * cameraDistance;
    float cameraZ = playerZ - viewDir.z * cameraDistance;
    
    // Calculate look-at target (player)
    float lookAtX = playerX;
    float lookAtY = playerY + playerHeight * 0.5f;
    float lookAtZ = playerZ;
    
    // Build view matrix using look-at formula
    Vector3 forward(lookAtX - cameraX, lookAtY - cameraY, lookAtZ - cameraZ);
    forward.normalize();
    
    Vector3 up(0, 1, 0);
    Vector3 side = forward.cross(up);
    side.normalize();
    
    Vector3 upVec = side.cross(forward);
    
    float viewMatrix[16] = {
        side.x, upVec.x, -forward.x, 0,
        side.y, upVec.y, -forward.y, 0,
        side.z, upVec.z, -forward.z, 0,
        /* translation components */
    };
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMultMatrixf(viewMatrix);
```

## Drawing the Player (3D Mannequin)

The player is rendered as a mannequin made of 3D capsules:

### Capsule Structure

A capsule is a cylinder with spherical caps:

```
    ____
   /    \    <- Sphere cap (top)
  |      |
  |      |   <- Cylinder body
  |      |
   \____/    <- Sphere cap (bottom)
```

### Drawing a Capsule

```cpp
void UserInput::drawCapsule(float x1, float y1, float z1,   // Start point
                            float x2, float y2, float z2,   // End point
                            float radius) {
    // Calculate capsule direction
    float dx = x2 - x1;
    float dy = y2 - y1;
    float dz = z2 - z1;
    float length = sqrt(dx*dx + dy*dy + dz*dz);
    
    if (length < 0.001f) return;
    
    // Normalize direction
    dx /= length;
    dy /= length;
    dz /= length;
    
    // Find perpendicular vectors for drawing circles
    Vector3 perp1, perp2;
    // ... calculate perpendiculars
    
    // Draw cylinder body as quad strips
    int segments = 12;  // How smooth the circle is
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= segments; i++) {
        float angle = 2 * M_PI * i / segments;
        float cosA = cos(angle);
        float sinA = sin(angle);
        
        // Calculate point on circle
        float nx = perp1.x * cosA + perp2.x * sinA;
        float ny = perp1.y * cosA + perp2.y * sinA;
        float nz = perp1.z * cosA + perp2.z * sinA;
        
        // Bottom circle point
        glVertex3f(x1 + nx * radius, y1 + ny * radius, z1 + nz * radius);
        // Top circle point
        glVertex3f(x2 + nx * radius, y2 + ny * radius, z2 + nz * radius);
    }
    glEnd();
    
    // Draw sphere caps at each end
    drawSphere(x1, y1, z1, radius, 8, 8);
    drawSphere(x2, y2, z2, radius, 8, 8);
}
```

### The Mannequin Body Parts

```cpp
void UserInput::drawStickFigure() {
    float baseY = playerY - playerHeight;  // Feet position
    float headTop = playerY;               // Top of head
    float scale = playerHeight / 100.0f;   // Scale factor
    
    // === TORSO ===
    float torsoBottom = baseY + 45 * scale;
    float torsoTop = baseY + 80 * scale;
    drawCapsule(playerX, torsoBottom, playerZ,
                playerX, torsoTop, playerZ, 
                12 * scale);  // Torso radius
    
    // === HEAD ===
    float headY = baseY + 90 * scale;
    drawSphere(playerX, headY, playerZ, 10 * scale, 12, 12);
    
    // === ARMS ===
    float shoulderY = torsoTop - 5 * scale;
    float shoulderOffset = 15 * scale;
    float elbowOffset = 25 * scale;
    float handOffset = 20 * scale;
    
    // Upper arm (shoulder to elbow)
    drawCapsule(playerX - shoulderOffset, shoulderY, playerZ,
                playerX - shoulderOffset - elbowOffset, shoulderY - 15*scale, playerZ,
                4 * scale);
                
    // Lower arm (elbow to hand)
    drawCapsule(playerX - shoulderOffset - elbowOffset, shoulderY - 15*scale, playerZ,
                playerX - shoulderOffset - elbowOffset - handOffset, shoulderY - 30*scale, playerZ,
                3.5f * scale);
    
    // Repeat for right arm...
    
    // === LEGS ===
    float hipY = torsoBottom;
    float hipOffset = 8 * scale;
    float kneeY = baseY + 25 * scale;
    float ankleY = baseY + 3 * scale;
    
    // Upper leg (hip to knee)
    drawCapsule(playerX - hipOffset, hipY, playerZ,
                playerX - hipOffset, kneeY, playerZ,
                5 * scale);
                
    // Lower leg (knee to ankle)
    drawCapsule(playerX - hipOffset, kneeY, playerZ,
                playerX - hipOffset, ankleY, playerZ,
                4.5f * scale);
    
    // Repeat for right leg...
}
```

### Mannequin Color

```cpp
// Skin color
glColor3f(0.9f, 0.75f, 0.6f);  // Peachy skin tone

// Draw all body parts...
```

## Drawing Obstacles

### Simple Box

```cpp
void ObstacleCourse::drawBox(const Box& box) {
    glColor3f(box.r, box.g, box.b);
    
    // Calculate 8 corners from center + dimensions
    float x1 = box.x - box.width / 2;   // Left
    float x2 = box.x + box.width / 2;   // Right
    float y1 = box.y;                    // Bottom
    float y2 = box.y + box.height;       // Top
    float z1 = box.z - box.depth / 2;   // Back
    float z2 = box.z + box.depth / 2;   // Front
    
    glBegin(GL_QUADS);
    
    // Front face (z = z2, facing +Z)
    glVertex3f(x1, y1, z2);  // Bottom-left
    glVertex3f(x2, y1, z2);  // Bottom-right
    glVertex3f(x2, y2, z2);  // Top-right
    glVertex3f(x1, y2, z2);  // Top-left
    
    // Back face (z = z1, facing -Z)
    glVertex3f(x2, y1, z1);
    glVertex3f(x1, y1, z1);
    glVertex3f(x1, y2, z1);
    glVertex3f(x2, y2, z1);
    
    // ... 4 more faces (top, bottom, left, right)
    
    glEnd();
}
```

### Glowing Checkpoints

```cpp
void ObstacleCourse::drawGlowingBox(const Box& box, float glow) {
    // Pulse brightness based on glow (0.0 to 1.0)
    float brightness = 0.6f + 0.4f * glow;
    glColor3f(box.r * brightness, box.g * brightness, box.b * brightness);
    
    // Draw the box...
    
    // Draw glowing border on top
    glColor3f(0.3f + 0.7f * glow, 1.0f, 0.4f + 0.3f * glow);  // Bright green
    float borderY = y2 + 0.5f;  // Slightly above surface
    
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glVertex3f(x1 + 2, borderY, z1 + 2);
    glVertex3f(x2 - 2, borderY, z1 + 2);
    glVertex3f(x2 - 2, borderY, z2 - 2);
    glVertex3f(x1 + 2, borderY, z2 - 2);
    glEnd();
    glLineWidth(1.0f);
}
```

### Death Zone Spikes

```cpp
void ObstacleCourse::drawSpikes(const Box& box) {
    float topY = box.y + box.height;
    
    // Spike parameters
    float spikeHeight = 12.0f;
    float spikeSpacing = 10.0f;
    float baseSize = 3.5f;  // Base radius
    
    // Dark metal color
    glColor3f(0.25f, 0.25f, 0.28f);
    
    // Grid of spikes
    for (float sx = x1 + spikeSpacing/2; sx < x2; sx += spikeSpacing) {
        for (float sz = z1 + spikeSpacing/2; sz < z2; sz += spikeSpacing) {
            
            // Each spike is a pyramid (4 triangular faces)
            glBegin(GL_TRIANGLES);
            
            // Front face
            glVertex3f(sx, topY + spikeHeight, sz);      // Tip
            glVertex3f(sx - baseSize, topY, sz + baseSize);  // Base corner
            glVertex3f(sx + baseSize, topY, sz + baseSize);  // Base corner
            
            // Right face
            glVertex3f(sx, topY + spikeHeight, sz);
            glVertex3f(sx + baseSize, topY, sz + baseSize);
            glVertex3f(sx + baseSize, topY, sz - baseSize);
            
            // Back face
            glVertex3f(sx, topY + spikeHeight, sz);
            glVertex3f(sx + baseSize, topY, sz - baseSize);
            glVertex3f(sx - baseSize, topY, sz - baseSize);
            
            // Left face
            glVertex3f(sx, topY + spikeHeight, sz);
            glVertex3f(sx - baseSize, topY, sz - baseSize);
            glVertex3f(sx - baseSize, topY, sz + baseSize);
            
            glEnd();
        }
    }
}
```

## Drawing the Grid

```cpp
void Grid::render() {
    glColor3f(0.3f, 0.3f, 0.3f);  // Dark gray
    
    glBegin(GL_LINES);
    
    // Draw lines along X axis
    for (int z = -gridDepth/2; z <= gridDepth/2; z += spacing) {
        glVertex3f(-gridWidth/2, 0, z);
        glVertex3f(gridWidth/2, 0, z);
    }
    
    // Draw lines along Z axis
    for (int x = -gridWidth/2; x <= gridWidth/2; x += spacing) {
        glVertex3f(x, 0, -gridDepth/2);
        glVertex3f(x, 0, gridDepth/2);
    }
    
    glEnd();
}
```

## Animation: Glow Effect

```cpp
void ObstacleCourse::render(float deltaTime) {
    // Update animation phase (0 to 2π)
    glowPhase += deltaTime * 3.0f;
    if (glowPhase > 2.0f * M_PI) glowPhase -= 2.0f * M_PI;
    
    // Calculate glow intensity (oscillates between 0 and 1)
    float glow = 0.5f + 0.5f * sin(glowPhase);
    
    // Render checkpoints with animated glow
    for (const auto& cp : checkpoints) {
        drawGlowingBox(cp, glow);
    }
}
```

The sine wave creates smooth oscillation:
```
sin(0) = 0      → glow = 0.5
sin(π/2) = 1   → glow = 1.0 (brightest)
sin(π) = 0     → glow = 0.5
sin(3π/2) = -1 → glow = 0.0 (dimmest)
```

## Player Shadow

```cpp
void UserInput::drawShadow() {
    // Project shadow onto floor (Y = 0)
    float shadowY = 0.01f;  // Slightly above ground to avoid z-fighting
    float shadowAlpha = 0.3f;  // Semi-transparent
    
    glEnable(GL_BLEND);
    glColor4f(0, 0, 0, shadowAlpha);  // Black with transparency
    
    // Draw oval shape at player XZ position
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(playerX, shadowY, playerZ);  // Center
    for (int i = 0; i <= 20; i++) {
        float angle = 2 * M_PI * i / 20;
        float rx = 15 * cos(angle);  // X radius
        float rz = 10 * sin(angle);  // Z radius (ellipse)
        glVertex3f(playerX + rx, shadowY, playerZ + rz);
    }
    glEnd();
    
    glDisable(GL_BLEND);
}
```

## Depth Buffer (Z-Buffer)

Without depth testing, objects drawn later appear on top regardless of position:

```cpp
// Enable depth testing
glEnable(GL_DEPTH_TEST);

// Each frame, clear the depth buffer
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
```

The depth buffer stores the Z coordinate of each pixel. When drawing a new pixel, OpenGL checks:
- If new pixel is closer → draw it and update depth buffer
- If new pixel is farther → discard it (hidden behind something)

## Next Steps

Continue to [Menu System & UI](./07_menu_system.md) to learn about the user interface rendering.
