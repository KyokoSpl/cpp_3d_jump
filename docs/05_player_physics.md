# 5. Player Movement & Physics

This document explains how player movement, jumping, and physics work in detail.

## Player State Variables

```cpp
// Position in 3D space
float playerX, playerY, playerZ;

// Spawn/respawn position
float spawnX, spawnY, spawnZ;

// Camera angles (radians)
float pitch;  // Up/down angle
float yaw;    // Left/right angle

// Movement
float SPEED;           // Units per frame at 60 FPS
float cameraDistance;  // Third person camera distance

// Physics
float yVel;            // Vertical velocity (positive = up)
float gravity;         // Gravity acceleration (negative)
float jumpForce;       // Initial jump velocity
bool grounded;         // Is player on ground?
float coyoteTimer;     // Grace period after leaving edge

// Player dimensions
float playerHeight;    // Current height (changes when crouching)
float crouchHeight;    // Height when crouching
float normalHeight;    // Height when standing
bool isCrouching;
float collisionRadius; // Horizontal collision size
```

## The View Vector

The **view vector** is a direction vector showing where the player is looking:

```cpp
Vector3 UserInput::getViewVector() {
    Vector3 v;
    
    // Convert yaw and pitch angles to a direction vector
    v.x = cos(yaw) * cos(pitch);
    v.y = sin(pitch);
    v.z = sin(yaw) * cos(pitch);
    
    return v;
}
```

### Understanding the Math

Imagine looking around in a sphere:
- `yaw` rotates you horizontally (0 = +X, π/2 = +Z, π = -X, 3π/2 = -Z)
- `pitch` tilts you up/down (-π/2 = straight down, 0 = horizontal, +π/2 = straight up)

The formulas convert these angles to a unit vector (length = 1):

```
x = cos(yaw) * cos(pitch)  // X component
y = sin(pitch)              // Y component (vertical)
z = sin(yaw) * cos(pitch)   // Z component
```

## Movement System

### Getting Movement Directions

```cpp
void UserInput::move(bool forward, bool backward, bool left, bool right, 
                     ObstacleCourse* course, float deltaTime) {
    // Get view direction
    Vector3 v = getViewVector();
    
    // Forward vector (horizontal only, Y=0)
    Vector3 forwardV(v.x, 0, v.z);
    forwardV.normalize();  // Make length = 1
    
    // Right vector = cross product of forward and up
    Vector3 rightV = forwardV.cross(Vector3(0, 1, 0));
    rightV.normalize();
```

### Cross Product

The **cross product** gives a vector perpendicular to two input vectors:

```cpp
Vector3 Vector3::cross(const Vector3& other) const {
    return Vector3(
        y * other.z - z * other.y,   // X component
        z * other.x - x * other.z,   // Y component  
        x * other.y - y * other.x    // Z component
    );
}
```

For our use: `forward × up = right`

```
  up (0,1,0)
     |
     |
     +----→ right (result)
    /
   /
  forward
```

### Applying Movement

```cpp
    // Frame-rate independent speed
    float frameSpeed = SPEED * deltaTime * 60.0f;
    
    float moveX = 0;
    float moveZ = 0;
    
    // Build movement vector from input
    if (forward) {
        moveX += forwardV.x * frameSpeed;
        moveZ += forwardV.z * frameSpeed;
    }
    if (backward) {
        moveX -= forwardV.x * frameSpeed;
        moveZ -= forwardV.z * frameSpeed;
    }
    if (right) {
        moveX += rightV.x * frameSpeed;
        moveZ += rightV.z * frameSpeed;
    }
    if (left) {
        moveX -= rightV.x * frameSpeed;
        moveZ -= rightV.z * frameSpeed;
    }
```

### Collision-Tested Movement

```cpp
    // Try X movement first
    float newX = playerX + moveX;
    if (!course || !course->checkCollision(newX, playerY, playerZ, collisionRadius)) {
        playerX = newX;  // No collision, allow movement
    }
    // If collision, playerX stays the same (slide along wall)
    
    // Try Z movement separately
    float newZ = playerZ + moveZ;
    if (!course || !course->checkCollision(playerX, playerY, newZ, collisionRadius)) {
        playerZ = newZ;
    }
}
```

By testing X and Z separately, the player can **slide along walls** instead of stopping completely.

## Gravity and Jumping

### The Physics Update

```cpp
void UserInput::update(..., float deltaTime) {
    float timeScale = deltaTime * 60.0f;  // Normalized to 60 FPS
    
    // Apply gravity to vertical velocity
    yVel += gravity * timeScale;  // gravity is negative, so yVel decreases
    
    // Calculate new Y position
    float newY = playerY + yVel * timeScale;
```

### Gravity Explained

With `gravity = -0.8`:

```
Frame 1: yVel = 0,    playerY = 100
Frame 2: yVel = -0.8, playerY = 99.2
Frame 3: yVel = -1.6, playerY = 97.6
Frame 4: yVel = -2.4, playerY = 95.2
...
```

Velocity keeps increasing (more negative) = acceleration!

### Ground Detection

```cpp
    // Get floor height at player position
    float obstacleFloorY = course->getFloorHeight(playerX, playerZ, playerY);
    
    // Determine effective floor
    float floorY = -1000.0f;  // Default: no floor (will fall)
    
    bool onGrid = grid && !grid->isOutOfBounds(playerX, playerZ);
    if (onGrid) {
        floorY = std::max(0.0f, obstacleFloorY);
    } else if (obstacleFloorY > 0) {
        floorY = obstacleFloorY;  // On platform outside grid
    }
    
    // Ground collision
    if (playerY <= floorY + playerHeight) {
        playerY = floorY + playerHeight;  // Snap to ground
        yVel = 0;                          // Stop falling
        grounded = true;
        coyoteTimer = 0.1f;                // Reset coyote time
    } else {
        // In air
        if (coyoteTimer > 0) {
            coyoteTimer -= deltaTime;
            grounded = true;  // Still "grounded" for jumping
        } else {
            grounded = false;
        }
    }
```

### Coyote Time

**Coyote time** gives players a brief window to jump after walking off an edge. Named after Wile E. Coyote who runs off cliffs and floats for a moment!

```cpp
if (coyoteTimer > 0) {
    coyoteTimer -= deltaTime;
    grounded = true;  // Can still jump!
}
```

This makes the game feel more forgiving and responsive.

## Jumping

### Basic Jump

```cpp
void UserInput::jump() {
    if (grounded && !isCrouching) {
        yVel = jumpForce;  // Set upward velocity
        grounded = false;  // No longer on ground
    }
}
```

With `jumpForce = 15`:
```
Frame 0: yVel = 15, going UP
Frame 1: yVel = 15 - 0.8 = 14.2, still UP
...
Frame 19: yVel ≈ 0, at peak
Frame 20: yVel = -0.8, starting to fall
...
```

### Crouch Jump

Lower but faster jump while crouching:

```cpp
void UserInput::crouchJump() {
    if (grounded && isCrouching) {
        yVel = jumpForce * 0.6f;  // 60% of normal jump
        grounded = false;
    }
}
```

## Wall Running

### Wall Detection

```cpp
// Check for walls on left and right
Vector3 forwardV(v.x, 0, v.z);
forwardV.normalize();
Vector3 rightV = forwardV.cross(Vector3(0, 1, 0));
rightV.normalize();

float wallCheckDist = collisionRadius + 15.0f;

// Check left side
bool leftWall = course->checkCollision(
    playerX - rightV.x * wallCheckDist, playerY,
    playerZ - rightV.z * wallCheckDist, 5.0f);

// Check right side    
bool rightWall = course->checkCollision(
    playerX + rightV.x * wallCheckDist, playerY,
    playerZ + rightV.z * wallCheckDist, 5.0f);
```

### Wall Running Physics

```cpp
if ((leftWall || rightWall) && wallRunTimer < maxWallRunTime && yVel <= 0) {
    if (!isWallRunning) {
        isWallRunning = true;
        wallRunSide = rightWall ? 1 : -1;
    }
    wallRunTimer += deltaTime;
    
    // Slow descent while wall running
    yVel = -2.0f;  // Controlled fall speed
    
    // Move forward along wall
    float wallRunSpeed = SPEED * 1.2f * timeScale;
    playerX += forwardV.x * wallRunSpeed;
    playerZ += forwardV.z * wallRunSpeed;
}
```

### Wall Jump

```cpp
void UserInput::jump() {
    if (isWallRunning) {
        // Jump away from wall
        yVel = jumpForce * 0.9f;
        isWallRunning = false;
        wallRunTimer = 0.0f;
        grounded = false;
        return;
    }
    // ... normal jump
}
```

## Crouching

### Height Transition

```cpp
void UserInput::update(...) {
    // Smoothly interpolate height when crouching
    float targetHeight = isCrouching ? crouchHeight : normalHeight;
    
    // Frame-rate independent lerp
    float lerpFactor = 1.0f - std::pow(1.0f - 0.2f, timeScale);
    playerHeight += (targetHeight - playerHeight) * lerpFactor;
}
```

**Lerp** (Linear Interpolation) smoothly moves from current to target:
```cpp
current += (target - current) * factor;
```

Factor = 0: no change
Factor = 1: instant change
Factor = 0.2: 20% of the way each frame (smooth)

## Collision Detection

### AABB Collision (Axis-Aligned Bounding Box)

```cpp
bool Box::checkCollision(float px, float py, float pz, float radius) const {
    // Box bounds
    float halfW = width / 2;
    float halfD = depth / 2;
    
    // Find closest point on box to sphere center
    float closestX = std::max(x - halfW, std::min(px, x + halfW));
    float closestY = std::max(y, std::min(py, y + height));
    float closestZ = std::max(z - halfD, std::min(pz, z + halfD));
    
    // Distance from closest point to player
    float dx = px - closestX;
    float dy = py - closestY;
    float dz = pz - closestZ;
    
    float distSquared = dx * dx + dy * dy + dz * dz;
    
    // Collision if distance < radius
    return distSquared < (radius * radius);
}
```

### Getting Floor Height

```cpp
float ObstacleCourse::getFloorHeight(float x, float z, float currentY) {
    float maxFloor = 0.0f;
    
    for (const auto& box : obstacles) {
        float halfW = box.width / 2 + 10.0f;  // Margin for standing
        float halfD = box.depth / 2 + 10.0f;
        
        // Is player within X-Z bounds of this box?
        if (x >= box.x - halfW && x <= box.x + halfW &&
            z >= box.z - halfD && z <= box.z + halfD) {
            
            float topY = box.y + box.height;
            
            // Is player above this surface?
            if (currentY >= topY && topY > maxFloor) {
                maxFloor = topY;  // This is the floor
            }
        }
    }
    
    return maxFloor;
}
```

## Camera System

### Rotation

```cpp
void UserInput::rotate(float dx, float dy) {
    yaw += dx * sensitivity;
    pitch -= dy * sensitivity;  // Inverted Y
    
    // Clamp pitch to prevent flipping
    float limit = M_PI / 2.0f - 0.01f;
    pitch = std::max(-limit, std::min(limit, pitch));
}
```

### Camera Position

```cpp
// Third person: camera behind and above player
Vector3 viewDir = getViewVector();

cameraX = playerX - viewDir.x * cameraDistance;
cameraY = playerY + playerHeight * 0.5f - viewDir.y * cameraDistance;
cameraZ = playerZ - viewDir.z * cameraDistance;

// Look at player center
lookAtX = playerX;
lookAtY = playerY + playerHeight * 0.5f;
lookAtZ = playerZ;
```

The camera sits behind the player (`- viewDir * distance`) and looks at them.

## Checkpoint System

```cpp
int ObstacleCourse::isOnCheckpoint(float x, float y, float z) {
    for (size_t i = 0; i < checkpoints.size(); i++) {
        const Box& cp = checkpoints[i];
        
        // Check if player is on this checkpoint
        if (/* player within bounds */) {
            return (int)i;  // Return checkpoint index
        }
    }
    return -1;  // Not on any checkpoint
}

// In UserInput::update()
int checkpoint = course->isOnCheckpoint(playerX, playerY, playerZ);
if (checkpoint != -1 && checkpoint > lastCheckpoint) {
    lastCheckpoint = checkpoint;
    checkpointPopupTimer = 2.0f;
    checkpointMessage = "Checkpoint " + std::to_string(checkpoint + 1) + " Reached!";
}
```

### Respawning

```cpp
void UserInput::respawn(ObstacleCourse* course) {
    if (lastCheckpoint >= 0 && course) {
        // Respawn at last checkpoint
        course->getCheckpointPosition(lastCheckpoint, playerX, playerY, playerZ);
    } else {
        // Respawn at start
        playerX = spawnX;
        playerY = spawnY;
        playerZ = spawnZ;
    }
    yVel = 0;
    grounded = false;
    deathCount++;
}
```

## Death Zones

```cpp
bool ObstacleCourse::isOnDeathZone(float x, float y, float z) {
    for (const auto& dz : deathZones) {
        // Check if player is touching this death zone
        if (/* player within bounds */) {
            return true;
        }
    }
    return false;
}

// In UserInput::update()
if (!devMode && course->isOnDeathZone(playerX, playerY, playerZ)) {
    respawn(course);
}
```

## Next Steps

Continue to [3D Rendering](./06_3d_rendering.md) to learn how the player and world are drawn.
