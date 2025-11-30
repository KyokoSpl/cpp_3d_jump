# 12. Future Improvements & Learning Resources

This document provides ideas for extending the game, starting points for implementation, and helpful resources.

---

## Feature Ideas

### 🟢 Beginner Level

#### 1. Double Jump
**What**: Allow a second jump while in the air.

**Starting Point**:
```cpp
// In UserInput.h
int jumpsRemaining = 2;
int maxJumps = 2;

// In UserInput.cpp jump logic
void UserInput::jump() {
    if (jumpsRemaining > 0) {
        velocityY = jumpForce;
        jumpsRemaining--;
        isGrounded = false;
    }
}

// Reset when landing
void UserInput::land() {
    jumpsRemaining = maxJumps;
    isGrounded = true;
}
```

**Files to modify**: `src/UserInput.h`, `src/UserInput.cpp`

---

#### 2. Sprint Mechanic
**What**: Hold a key to move faster, with limited stamina.

**Starting Point**:
```cpp
// In UserInput.h
float stamina = 100.0f;
float maxStamina = 100.0f;
float staminaDrain = 30.0f;    // Per second while sprinting
float staminaRegen = 15.0f;    // Per second while not sprinting
bool isSprinting = false;
float sprintMultiplier = 1.6f;

// In update()
if (isSprinting && stamina > 0) {
    stamina -= staminaDrain * deltaTime;
    currentSpeed = baseSpeed * sprintMultiplier;
} else {
    stamina = std::min(maxStamina, stamina + staminaRegen * deltaTime);
    currentSpeed = baseSpeed;
}
```

**UI Addition**: Draw stamina bar in `Menu::renderHUD()`

---

#### 3. Moving Platforms
**What**: Platforms that move back and forth.

**Starting Point**:
```cpp
// New struct in Obstacle.h
struct MovingPlatform {
    float startX, startY, startZ;
    float endX, endY, endZ;
    float speed;
    float progress;  // 0 to 1
    bool movingForward;
    
    void update(float deltaTime) {
        if (movingForward) {
            progress += speed * deltaTime;
            if (progress >= 1.0f) {
                progress = 1.0f;
                movingForward = false;
            }
        } else {
            progress -= speed * deltaTime;
            if (progress <= 0.0f) {
                progress = 0.0f;
                movingForward = true;
            }
        }
    }
    
    float getCurrentX() { return startX + (endX - startX) * progress; }
    float getCurrentY() { return startY + (endY - startY) * progress; }
    float getCurrentZ() { return startZ + (endZ - startZ) * progress; }
};
```

**Challenge**: Player must move with platform when standing on it.

---

### 🟡 Intermediate Level

#### 5. Dash Ability
**What**: Quick burst of movement in look direction with cooldown.

**Starting Point**:
```cpp
// In UserInput.h
float dashCooldown = 0.0f;
float dashCooldownMax = 2.0f;
float dashDistance = 10.0f;
float dashDuration = 0.15f;
bool isDashing = false;
float dashTimer = 0.0f;

// Trigger dash
void UserInput::dash() {
    if (dashCooldown <= 0 && !isDashing) {
        isDashing = true;
        dashTimer = dashDuration;
        dashCooldown = dashCooldownMax;
        
        // Calculate dash direction from camera
        dashVelX = -sinf(yaw) * dashDistance / dashDuration;
        dashVelZ = -cosf(yaw) * dashDistance / dashDuration;
    }
}

// In update()
if (isDashing) {
    positionX += dashVelX * deltaTime;
    positionZ += dashVelZ * deltaTime;
    dashTimer -= deltaTime;
    if (dashTimer <= 0) isDashing = false;
}
dashCooldown -= deltaTime;
```

---

#### 6. Slide Mechanic
**What**: Slide under low obstacles while moving fast.

**Starting Point**:
```cpp
// In UserInput.h
bool isSliding = false;
float slideTimer = 0.0f;
float maxSlideTime = 0.8f;
float slideSpeed = 8.0f;
float slideHeight = 0.6f;  // Crouch height during slide

// Trigger slide (while moving + crouch key)
void UserInput::startSlide() {
    if (isGrounded && currentSpeed > baseSpeed * 0.8f && !isSliding) {
        isSliding = true;
        slideTimer = maxSlideTime;
        slideDirection = currentMoveDirection;
    }
}

// In update()
if (isSliding) {
    // Move in slide direction, ignore input
    positionX += slideDirection.x * slideSpeed * deltaTime;
    positionZ += slideDirection.z * slideSpeed * deltaTime;
    playerHeight = slideHeight;
    
    slideTimer -= deltaTime;
    if (slideTimer <= 0) {
        isSliding = false;
        playerHeight = normalHeight;
    }
}
```

---

#### 7. Grappling Hook
**What**: Shoot a hook to swing or pull toward a point.

**Starting Point**:
```cpp
// In UserInput.h
bool grappleActive = false;
float grappleX, grappleY, grappleZ;  // Hook point
float ropeLength;
float grappleSpeed = 50.0f;

// Shoot grapple (raycast to find hit point)
void UserInput::shootGrapple() {
    // Cast ray from camera
    float rayX = -sinf(yaw) * cosf(pitch);
    float rayY = sinf(pitch);
    float rayZ = -cosf(yaw) * cosf(pitch);
    
    // Check intersection with platforms
    // If hit, set grappleX/Y/Z and grappleActive = true
}

// In update() - simple pull version
if (grappleActive) {
    float dx = grappleX - positionX;
    float dy = grappleY - positionY;
    float dz = grappleZ - positionZ;
    float dist = sqrtf(dx*dx + dy*dy + dz*dz);
    
    if (dist > 1.0f) {
        velocityX += (dx / dist) * grappleSpeed * deltaTime;
        velocityY += (dy / dist) * grappleSpeed * deltaTime;
        velocityZ += (dz / dist) * grappleSpeed * deltaTime;
    } else {
        grappleActive = false;
    }
}
```

---

#### 8. Level Editor
**What**: In-game tool to place and modify platforms.

**Starting Point**:
```cpp
// Editor mode struct
struct EditorMode {
    bool active = false;
    int selectedPlatformType = 0;  // 0=platform, 1=wall, 2=death zone
    float gridSnap = 5.0f;
    
    // Ghost platform preview
    float previewX, previewY, previewZ;
    float previewSizeX = 10, previewSizeY = 2, previewSizeZ = 10;
    
    // Raycast from cursor to find placement position
    void updatePreview(float cameraX, float cameraY, float cameraZ,
                       float lookX, float lookY, float lookZ);
    
    // Place platform at preview location
    void placePlatform(std::vector<Platform>& platforms);
    
    // Save/load level
    void saveLevel(const std::string& filename);
    void loadLevel(const std::string& filename);
};
```

**UI**: Draw ghost platform, show type selector, save/load buttons.

---

### 🔴 Advanced Level

#### 9. Multiplayer (Local Split-Screen)
**What**: Two players on same screen.

**Concepts to Learn**:
- Viewport splitting with `glViewport()`
- Multiple camera instances
- Separate input handling per player
- Rendering scene twice

**Starting Point**:
```cpp
// Render left half for player 1
glViewport(0, 0, windowWidth/2, windowHeight);
renderScene(player1Camera);

// Render right half for player 2
glViewport(windowWidth/2, 0, windowWidth/2, windowHeight);
renderScene(player2Camera);
```

---

#### 10. Procedural Level Generation
**What**: Randomly generate parkour courses.

**Concepts to Learn**:
- Random number generation
- Platform placement rules
- Difficulty curves
- Ensuring course is completable

**Starting Point**:
```cpp
void generateLevel(int seed, int difficulty) {
    srand(seed);
    
    float currentX = 0;
    float currentY = 100;
    float currentZ = 0;
    
    for (int i = 0; i < 50; i++) {
        // Random gap (harder = larger gaps)
        float gap = 5.0f + (rand() % 10) * (difficulty * 0.5f);
        currentX += gap;
        
        // Random height change
        float heightChange = (rand() % 40) - 20;
        currentY += heightChange;
        currentY = std::max(50.0f, currentY);  // Minimum height
        
        // Random platform size
        float sizeX = 10 + rand() % 20;
        float sizeZ = 10 + rand() % 15;
        
        // Add platform
        addPlatform(currentX, currentY, currentZ, sizeX, 2, sizeZ);
        
        // Occasionally add obstacles
        if (rand() % 3 == 0) {
            addDeathZone(currentX + sizeX/2, currentY - 50, currentZ, 30, 40, 30);
        }
    }
}
```

---

#### 11. Replay System
**What**: Record and playback player movement.

**Concepts to Learn**:
- Data recording over time
- Interpolation
- Ghost rendering

**Starting Point**:
```cpp
struct ReplayFrame {
    float time;
    float posX, posY, posZ;
    float yaw, pitch;
    bool isCrouching;
};

class ReplaySystem {
    std::vector<ReplayFrame> frames;
    float recordInterval = 0.05f;  // 20 FPS recording
    
    void recordFrame(float time, const UserInput& player) {
        ReplayFrame frame;
        frame.time = time;
        frame.posX = player.getX();
        frame.posY = player.getY();
        frame.posZ = player.getZ();
        // ... etc
        frames.push_back(frame);
    }
    
    ReplayFrame getFrameAt(float time) {
        // Find frames before and after 'time'
        // Interpolate between them
    }
    
    void saveReplay(const std::string& filename);
    void loadReplay(const std::string& filename);
};
```

---

#### 12. Modern OpenGL Renderer
**What**: Replace immediate mode with shaders and VBOs.

**Concepts to Learn**:
- Vertex Buffer Objects (VBOs)
- Vertex Array Objects (VAOs)
- Shader programs (GLSL)
- Uniform variables

**Starting Point**:
```cpp
// Vertex shader (vertex.glsl)
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vertexColor;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vertexColor = aColor;
}

// Fragment shader (fragment.glsl)
#version 330 core
in vec3 vertexColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(vertexColor, 1.0);
}
```

**Resources**: LearnOpenGL.com has excellent modern OpenGL tutorials.

---

## Helpful Resources

### C++ Learning

| Resource | Description | URL |
|----------|-------------|-----|
| **LearnCpp.com** | Comprehensive C++ tutorials | https://www.learncpp.com/ |
| **cppreference.com** | C++ standard library reference | https://en.cppreference.com/ |
| **Compiler Explorer** | See assembly output | https://godbolt.org/ |
| **C++ Core Guidelines** | Best practices | https://isocpp.github.io/CppCoreGuidelines/ |

### OpenGL & Graphics

| Resource | Description | URL |
|----------|-------------|-----|
| **LearnOpenGL** | Modern OpenGL tutorials | https://learnopengl.com/ |
| **OpenGL Reference** | Official documentation | https://www.khronos.org/opengl/ |
| **GLFW Documentation** | Window/input library | https://www.glfw.org/documentation.html |
| **Scratchapixel** | Graphics programming from scratch | https://www.scratchapixel.com/ |

### Game Development

| Resource | Description | URL |
|----------|-------------|-----|
| **Game Programming Patterns** | Design patterns for games | https://gameprogrammingpatterns.com/ |
| **Red Blob Games** | Pathfinding, procedural gen | https://www.redblobgames.com/ |
| **Gaffer On Games** | Physics, networking | https://gafferongames.com/ |
| **Game Dev Stack Exchange** | Q&A community | https://gamedev.stackexchange.com/ |

### Math for Games

| Resource | Description | URL |
|----------|-------------|-----|
| **3Blue1Brown** | Visual linear algebra | https://www.3blue1brown.com/ |
| **Immersive Math** | Interactive linear algebra | https://immersivemath.com/ |
| **Essential Math for Games** | Game-focused math | https://www.essentialmath.com/ |

### Tools

| Tool | Purpose | URL |
|------|---------|-----|
| **Trello** | Project planning boards | https://trello.com/ |
| **Notion** | Notes and documentation | https://www.notion.so/ |
| **GitHub Projects** | Issue tracking | https://github.com/features/issues |
| **draw.io** | Diagrams and flowcharts | https://app.diagrams.net/ |

---

## Planning Your Feature

### Step-by-Step Process

1. **Define the Feature**
   - What does it do?
   - How does the player use it?
   - What does it look like?

2. **Break It Down**
   - What data do you need to store?
   - What functions do you need?
   - Which files need changes?

3. **Create a Checklist**
   ```
   [ ] Add variables to src/UserInput.h
   [ ] Implement logic in src/UserInput.cpp
   [ ] Add input handling in src/main.cpp
   [ ] Add UI feedback in src/Menu.cpp
   [ ] Test basic functionality
   [ ] Handle edge cases
   [ ] Polish and tune values
   ```

4. **Start Small**
   - Get the simplest version working first
   - Add complexity incrementally
   - Test after each change

5. **Use Version Control**
   ```bash
   # Before starting a feature
   git checkout -b feature/double-jump
   
   # Commit small changes often
   git add -A
   git commit -m "Add jump counter variable"
   
   # When done, merge back
   git checkout main
   git merge feature/double-jump
   ```

---

## Example: Adding Double Jump (Full Walkthrough)

### Step 1: Plan
```
Feature: Double Jump
- Player can jump once more while in the air
- Reset on landing
- Visual feedback (optional particle effect later)

Files to modify:
- src/UserInput.h: Add variables
- src/UserInput.cpp: Add logic
- src/main.cpp: No changes needed (uses existing jump key)
```

### Step 2: Add Variables (src/UserInput.h)
```cpp
// Find the jump-related variables and add:
int jumpsRemaining;
int maxJumps;
```

### Step 3: Initialize in Constructor (src/UserInput.cpp)
```cpp
UserInput::UserInput() {
    // ... existing code ...
    maxJumps = 2;
    jumpsRemaining = maxJumps;
}
```

### Step 4: Modify Jump Logic (src/UserInput.cpp)
```cpp
void UserInput::jump() {
    // Replace the old jump check
    if (jumpsRemaining > 0) {
        velocityY = jumpForce;
        jumpsRemaining--;
        isGrounded = false;
        
        // Optional: Second jump is weaker
        if (jumpsRemaining == 0) {
            velocityY = jumpForce * 0.8f;
        }
    }
}
```

### Step 5: Reset on Landing (src/UserInput.cpp)
```cpp
// In the ground collision section of update()
if (/* landed on ground */) {
    isGrounded = true;
    velocityY = 0;
    jumpsRemaining = maxJumps;  // ADD THIS LINE
}
```

### Step 6: Test
```bash
cd build
make -j8
./processing3d
```

### Step 7: Iterate
- Is it fun?
- Is the second jump too strong/weak?
- Should there be a visual indicator?

---

## Debugging Tips

### Print Debugging
```cpp
std::cout << "Jump called, remaining: " << jumpsRemaining << std::endl;
```

### Conditional Compilation
```cpp
#define DEBUG_PHYSICS 1

#if DEBUG_PHYSICS
    std::cout << "Velocity: " << velocityY << std::endl;
#endif
```

### Visual Debugging
```cpp
// Draw debug info on screen
void renderDebugInfo() {
    std::string info = "Jumps: " + std::to_string(jumpsRemaining);
    renderText(info, 10, 100);
}
```

---

## Community & Help

- **Stack Overflow**: Tag questions with `[c++]` and `[opengl]`
- **Reddit**: r/gamedev, r/opengl, r/cpp
- **Discord**: Many game dev communities have Discord servers
- **GitHub Issues**: Look at how other projects solve similar problems

---

## Final Tips

1. **Don't try to add everything at once** - Pick one feature, finish it, then move on
2. **Read other people's code** - Look at open source games for inspiration
3. **Keep backups** - Use git, commit often
4. **Take breaks** - Fresh eyes find bugs faster
5. **Have fun!** - This is a learning project, experiment freely

Good luck with your game development journey! 🎮🚀
