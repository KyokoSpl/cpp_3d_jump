# 10. Collision System

This document explains how collision detection works in the game.

## Collision Overview

The game uses Axis-Aligned Bounding Box (AABB) collision detection. This means:
- All collision boxes have edges parallel to the X, Y, Z axes
- No rotated boxes (simpler math)
- Fast to compute

## AABB Structure

Each obstacle has a bounding box defined by minimum and maximum corners:

```cpp
struct BoundingBox {
    float minX, maxX;
    float minY, maxY;
    float minZ, maxZ;
};
```

Example platform at (100, 0, 50) with size (50, 5, 20):
```
minX = 100 - 50/2 = 75
maxX = 100 + 50/2 = 125
minY = 0 - 5/2 = -2.5
maxY = 0 + 5/2 = 2.5
minZ = 50 - 20/2 = 40
maxZ = 50 + 20/2 = 60
```

## AABB Overlap Test

Two boxes overlap if they overlap on ALL three axes:

```cpp
bool boxesOverlap(const BoundingBox& a, const BoundingBox& b) {
    // Check each axis
    bool overlapX = (a.minX < b.maxX) && (a.maxX > b.minX);
    bool overlapY = (a.minY < b.maxY) && (a.maxY > b.minY);
    bool overlapZ = (a.minZ < b.maxZ) && (a.maxZ > b.minZ);
    
    // Must overlap on all three axes
    return overlapX && overlapY && overlapZ;
}
```

### Visual Explanation

```
X-axis overlap:
    a.min        a.max
      |------------|
           |------------|
        b.min        b.max
    
    Overlap exists because a.max > b.min AND a.min < b.max

No X-axis overlap:
    a.min   a.max
      |------|
                  |------|
               b.min   b.max
```

## Player Collision Box

The player is represented as a capsule approximated by a box:

```cpp
// In UserInput.h
float playerHeight = 1.8f;      // Total height
float playerRadius = 0.3f;      // Body radius
bool isCrouching = false;

// Calculate player bounds
float getPlayerMinY() {
    return positionY;  // Feet position
}

float getPlayerMaxY() {
    if (isCrouching) {
        return positionY + playerHeight * 0.6f;  // 60% when crouching
    }
    return positionY + playerHeight;
}

float getPlayerMinX() { return positionX - playerRadius; }
float getPlayerMaxX() { return positionX + playerRadius; }
float getPlayerMinZ() { return positionZ - playerRadius; }
float getPlayerMaxZ() { return positionZ + playerRadius; }
```

## Ground Collision

Detecting if player is on ground:

```cpp
bool checkGroundCollision(float playerX, float playerY, float playerZ) {
    float playerMinX = playerX - playerRadius;
    float playerMaxX = playerX + playerRadius;
    float playerMinZ = playerZ - playerRadius;
    float playerMaxZ = playerZ + playerRadius;
    float playerFeet = playerY;
    
    for (const auto& platform : platforms) {
        // Check horizontal overlap
        bool overlapX = (playerMinX < platform.maxX) && (playerMaxX > platform.minX);
        bool overlapZ = (playerMinZ < platform.maxZ) && (playerMaxZ > platform.minZ);
        
        if (overlapX && overlapZ) {
            // Check if player feet are near platform top
            float platformTop = platform.maxY;
            float tolerance = 0.1f;  // Small margin
            
            if (playerFeet >= platformTop - tolerance && 
                playerFeet <= platformTop + tolerance) {
                return true;  // On ground!
            }
        }
    }
    return false;
}
```

### Ground Detection with Velocity

```cpp
bool shouldLandOnPlatform(float oldY, float newY, float platformTop) {
    // Check if player crossed the platform surface
    // (was above, now at or below)
    return (oldY > platformTop) && (newY <= platformTop);
}
```

## Wall Collision

Preventing player from walking through walls:

```cpp
void resolveWallCollision(float& newX, float& newZ, float playerY) {
    float playerMinY = playerY;
    float playerMaxY = playerY + playerHeight;
    
    for (const auto& wall : obstacles) {
        // Check vertical overlap first
        bool overlapY = (playerMinY < wall.maxY) && (playerMaxY > wall.minY);
        if (!overlapY) continue;  // No collision possible
        
        // Calculate player bounds at new position
        float playerMinX = newX - playerRadius;
        float playerMaxX = newX + playerRadius;
        float playerMinZ = newZ - playerRadius;
        float playerMaxZ = newZ + playerRadius;
        
        // Check for overlap
        if (playerMinX < wall.maxX && playerMaxX > wall.minX &&
            playerMinZ < wall.maxZ && playerMaxZ > wall.minZ) {
            
            // Find smallest penetration
            float penX1 = wall.maxX - playerMinX;  // Push right
            float penX2 = playerMaxX - wall.minX;  // Push left
            float penZ1 = wall.maxZ - playerMinZ;  // Push forward
            float penZ2 = playerMaxZ - wall.minZ;  // Push back
            
            // Find minimum penetration
            float minPen = std::min({penX1, penX2, penZ1, penZ2});
            
            // Resolve by pushing out along smallest axis
            if (minPen == penX1) {
                newX = wall.maxX + playerRadius;
            } else if (minPen == penX2) {
                newX = wall.minX - playerRadius;
            } else if (minPen == penZ1) {
                newZ = wall.maxZ + playerRadius;
            } else {
                newZ = wall.minZ - playerRadius;
            }
        }
    }
}
```

## Ceiling Collision

Detecting head bumps:

```cpp
bool checkCeilingCollision(float playerX, float playerY, float playerZ) {
    float playerHead = playerY + playerHeight;
    float playerMinX = playerX - playerRadius;
    float playerMaxX = playerX + playerRadius;
    float playerMinZ = playerZ - playerRadius;
    float playerMaxZ = playerZ + playerRadius;
    
    for (const auto& ceiling : obstacles) {
        // Check horizontal overlap
        bool overlapX = (playerMinX < ceiling.maxX) && (playerMaxX > ceiling.minX);
        bool overlapZ = (playerMinZ < ceiling.maxZ) && (playerMaxZ > ceiling.minZ);
        
        if (overlapX && overlapZ) {
            // Check if head hits ceiling bottom
            if (playerHead > ceiling.minY && playerY < ceiling.maxY) {
                return true;  // Hit ceiling!
            }
        }
    }
    return false;
}

// Usage:
if (checkCeilingCollision(posX, posY, posZ)) {
    velocityY = 0;  // Stop upward movement
}
```

## Death Zone Collision

```cpp
bool checkDeathZone(float playerX, float playerY, float playerZ) {
    float playerMinX = playerX - playerRadius;
    float playerMaxX = playerX + playerRadius;
    float playerMinY = playerY;
    float playerMaxY = playerY + playerHeight;
    float playerMinZ = playerZ - playerRadius;
    float playerMaxZ = playerZ + playerRadius;
    
    for (const auto& zone : deathZones) {
        // Simple AABB overlap
        bool overlapX = (playerMinX < zone.maxX) && (playerMaxX > zone.minX);
        bool overlapY = (playerMinY < zone.maxY) && (playerMaxY > zone.minY);
        bool overlapZ = (playerMinZ < zone.maxZ) && (playerMaxZ > zone.minZ);
        
        if (overlapX && overlapY && overlapZ) {
            return true;  // Dead!
        }
    }
    return false;
}
```

## Checkpoint Collision

```cpp
int checkCheckpoint(float playerX, float playerY, float playerZ) {
    for (int i = 0; i < checkpoints.size(); i++) {
        const auto& cp = checkpoints[i];
        
        // Checkpoints use larger detection area
        float range = 2.0f;
        
        if (fabsf(playerX - cp.x) < range &&
            fabsf(playerY - cp.y) < playerHeight &&
            fabsf(playerZ - cp.z) < range) {
            return i;  // Return checkpoint index
        }
    }
    return -1;  // No checkpoint reached
}
```

## Wall Run Detection

```cpp
int checkWallContact(float playerX, float playerZ, float playerY) {
    float checkDistance = playerRadius + 0.1f;  // Slightly larger than player
    float playerMinY = playerY;
    float playerMaxY = playerY + playerHeight;
    
    for (const auto& wall : walls) {
        // Skip if no vertical overlap
        if (playerMaxY < wall.minY || playerMinY > wall.maxY) continue;
        
        // Check left side of wall
        if (fabsf(playerX - wall.minX) < checkDistance) {
            // Check Z overlap
            if (playerZ > wall.minZ && playerZ < wall.maxZ) {
                return -1;  // Wall on left
            }
        }
        
        // Check right side of wall
        if (fabsf(playerX - wall.maxX) < checkDistance) {
            if (playerZ > wall.minZ && playerZ < wall.maxZ) {
                return 1;  // Wall on right
            }
        }
        
        // Similarly for front/back walls using Z axis
    }
    return 0;  // No wall contact
}
```

## Collision Response

### Bouncy Collision

```cpp
void bounceOffWall(float& velocityX, float& velocityZ, 
                   float normalX, float normalZ, float bounciness) {
    // Reflect velocity around normal
    float dot = velocityX * normalX + velocityZ * normalZ;
    velocityX = (velocityX - 2 * dot * normalX) * bounciness;
    velocityZ = (velocityZ - 2 * dot * normalZ) * bounciness;
}
```

### Sliding Collision

```cpp
void slideAlongWall(float& velocityX, float& velocityZ,
                    float normalX, float normalZ) {
    // Remove velocity component along normal
    float dot = velocityX * normalX + velocityZ * normalZ;
    velocityX -= dot * normalX;
    velocityZ -= dot * normalZ;
}
```

## Swept Collision (Advanced)

For fast-moving objects, check collision along the path:

```cpp
bool sweptAABBCollision(
    float startX, float startY, float startZ,
    float endX, float endY, float endZ,
    const BoundingBox& obstacle,
    float& collisionTime)  // Output: 0-1, when collision occurs
{
    // Calculate entry and exit times for each axis
    float invVelX = 1.0f / (endX - startX);
    float invVelY = 1.0f / (endY - startY);
    float invVelZ = 1.0f / (endZ - startZ);
    
    float entryX, exitX, entryY, exitY, entryZ, exitZ;
    
    // X axis
    if (invVelX > 0) {
        entryX = (obstacle.minX - startX) * invVelX;
        exitX = (obstacle.maxX - startX) * invVelX;
    } else {
        entryX = (obstacle.maxX - startX) * invVelX;
        exitX = (obstacle.minX - startX) * invVelX;
    }
    
    // Similar for Y and Z...
    
    // Find latest entry and earliest exit
    float entryTime = std::max({entryX, entryY, entryZ});
    float exitTime = std::min({exitX, exitY, exitZ});
    
    // Check for collision
    if (entryTime > exitTime || entryTime < 0 || entryTime > 1) {
        return false;
    }
    
    collisionTime = entryTime;
    return true;
}
```

## Spatial Optimization

For many obstacles, check only nearby ones:

### Grid-Based Culling

```cpp
// Divide world into cells
const float CELL_SIZE = 50.0f;

std::vector<int> getNearbyCells(float playerX, float playerZ) {
    int cellX = (int)(playerX / CELL_SIZE);
    int cellZ = (int)(playerZ / CELL_SIZE);
    
    std::vector<int> cells;
    // Check 3x3 grid of cells around player
    for (int dx = -1; dx <= 1; dx++) {
        for (int dz = -1; dz <= 1; dz++) {
            int cell = (cellX + dx) + (cellZ + dz) * 1000;
            cells.push_back(cell);
        }
    }
    return cells;
}
```

### Distance Culling

```cpp
void checkCollisions(float playerX, float playerY, float playerZ) {
    float maxDistance = 100.0f;  // Only check nearby obstacles
    
    for (const auto& obstacle : obstacles) {
        // Quick distance check
        float dx = playerX - obstacle.centerX;
        float dz = playerZ - obstacle.centerZ;
        float distSq = dx * dx + dz * dz;
        
        if (distSq > maxDistance * maxDistance) {
            continue;  // Too far, skip detailed check
        }
        
        // Do full collision check
        if (boxesOverlap(playerBox, obstacleBox)) {
            handleCollision(obstacle);
        }
    }
}
```

## Debug Visualization

```cpp
void drawCollisionBox(const BoundingBox& box, float r, float g, float b) {
    glColor3f(r, g, b);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // Wireframe
    
    glBegin(GL_QUADS);
    // Bottom face
    glVertex3f(box.minX, box.minY, box.minZ);
    glVertex3f(box.maxX, box.minY, box.minZ);
    glVertex3f(box.maxX, box.minY, box.maxZ);
    glVertex3f(box.minX, box.minY, box.maxZ);
    
    // Top face
    glVertex3f(box.minX, box.maxY, box.minZ);
    glVertex3f(box.maxX, box.maxY, box.minZ);
    glVertex3f(box.maxX, box.maxY, box.maxZ);
    glVertex3f(box.minX, box.maxY, box.maxZ);
    
    // Side faces...
    glEnd();
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // Back to solid
}

// Usage in dev mode:
if (devMode) {
    for (const auto& obstacle : obstacles) {
        drawCollisionBox(obstacle, 0, 1, 0);  // Green wireframe
    }
    for (const auto& deathZone : deathZones) {
        drawCollisionBox(deathZone, 1, 0, 0);  // Red wireframe
    }
}
```

## Collision in Update Loop

```cpp
void UserInput::update(float deltaTime) {
    // Save old position
    float oldX = positionX;
    float oldY = positionY;
    float oldZ = positionZ;
    
    // Apply input
    float moveX = 0, moveZ = 0;
    if (moveForward) moveZ -= speed;
    if (moveBackward) moveZ += speed;
    if (moveLeft) moveX -= speed;
    if (moveRight) moveX += speed;
    
    // Transform by camera rotation
    float sinYaw = sinf(yaw);
    float cosYaw = cosf(yaw);
    float worldMoveX = moveX * cosYaw - moveZ * sinYaw;
    float worldMoveZ = moveX * sinYaw + moveZ * cosYaw;
    
    // Tentative new position
    float newX = positionX + worldMoveX * deltaTime;
    float newZ = positionZ + worldMoveZ * deltaTime;
    
    // Apply gravity
    velocityY += gravity * deltaTime;
    float newY = positionY + velocityY * deltaTime;
    
    // Resolve collisions
    resolveWallCollision(newX, newZ, newY);
    
    if (checkGroundCollision(newX, newY, newZ)) {
        newY = findGroundHeight(newX, newZ);
        velocityY = 0;
        onGround = true;
    } else {
        onGround = false;
    }
    
    if (checkCeilingCollision(newX, newY, newZ)) {
        velocityY = 0;
    }
    
    // Apply resolved position
    positionX = newX;
    positionY = newY;
    positionZ = newZ;
    
    // Check death and checkpoints
    if (checkDeathZone(positionX, positionY, positionZ)) {
        respawn();
    }
    
    int cp = checkCheckpoint(positionX, positionY, positionZ);
    if (cp > lastCheckpoint) {
        lastCheckpoint = cp;
        spawnX = checkpoints[cp].x;
        spawnY = checkpoints[cp].y;
        spawnZ = checkpoints[cp].z;
    }
}
```

## Next Steps

Continue to [Build System](./11_build_system.md) to learn about CMake and compilation.
