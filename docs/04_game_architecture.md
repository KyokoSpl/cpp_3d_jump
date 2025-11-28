# 4. Game Architecture

This document explains how the different parts of the game work together.

## Class Diagram

```
                    ┌─────────────┐
                    │   main()    │
                    │  Game Loop  │
                    └──────┬──────┘
                           │
         ┌─────────────────┼─────────────────┐
         │                 │                 │
         ▼                 ▼                 ▼
┌─────────────────┐ ┌─────────────┐ ┌─────────────────┐
│   UserInput     │ │ObstacleCourse│ │      Menu       │
│  (Player)       │ │  (World)     │ │    (UI)         │
├─────────────────┤ ├─────────────┤ ├─────────────────┤
│ position        │ │ obstacles   │ │ state           │
│ velocity        │ │ checkpoints │ │ settings        │
│ camera          │ │ deathZones  │ │ buttons         │
│ physics         │ │ goalBox     │ │ font            │
├─────────────────┤ ├─────────────┤ ├─────────────────┤
│ move()          │ │ render()    │ │ render()        │
│ update()        │ │ collision() │ │ handleKey()     │
│ render()        │ │ getFloor()  │ │ handleMouse()   │
│ jump()          │ │             │ │ save/load()     │
└─────────────────┘ └─────────────┘ └─────────────────┘
         │                 │                 │
         └─────────────────┴─────────────────┘
                           │
                    ┌──────┴──────┐
                    │    Grid     │
                    │ (Ground)    │
                    └─────────────┘
```

## main.cpp - The Game Controller

### Global Variables

```cpp
// Pointers to game systems
UserInput* userInput = nullptr;
ObstacleCourse* obstacles = nullptr;
Grid* grid = nullptr;
Menu* menu = nullptr;

// Input state (which keys are currently pressed)
bool w = false, a = false, s = false, d = false;
bool shift = false;

// Window dimensions
int windowWidth = 800;
int windowHeight = 600;

// Time tracking
float lastTime = 0;
float deltaTime = 0;
```

### Initialization Flow

```cpp
int main() {
    // 1. Initialize GLFW (window system)
    glfwInit();
    
    // 2. Create window
    window = glfwCreateWindow(800, 600, "3D Parkour", NULL, NULL);
    glfwMakeContextCurrent(window);
    
    // 3. Set up input callbacks
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    
    // 4. Create game objects
    userInput = new UserInput();
    obstacles = new ObstacleCourse();
    grid = new Grid(800, 800);
    menu = new Menu();
    
    // 5. Load settings
    menu->loadSettings();
    
    // 6. Enter game loop
    while (!glfwWindowShouldClose(window)) {
        gameLoop();
    }
    
    // 7. Cleanup
    delete userInput;
    delete obstacles;
    delete grid;
    delete menu;
    glfwTerminate();
}
```

### The Game Loop

```cpp
void gameLoop() {
    // Calculate delta time
    float currentTime = glfwGetTime();
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    
    // Clear screen
    glClearColor(0.1f, 0.15f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (!menu->isOpen()) {
        // === GAMEPLAY MODE ===
        
        // Update player state
        userInput->setCrouch(shift);
        userInput->update(windowWidth, windowHeight, obstacles, grid, deltaTime);
        userInput->move(w, s, a, d, obstacles, deltaTime);
        
        // Check for goal
        if (obstacles->isOnGoal(userInput->getPlayerX(), 
                                userInput->getPlayerY(),
                                userInput->getPlayerZ())) {
            userInput->stopTimer();
        }
        
        // Render 3D world
        obstacles->render(deltaTime);
        grid->render();
        userInput->render();  // Player character
        
        // Render HUD
        menu->renderHUD(windowWidth, windowHeight, 
                        userInput->getTimer(),
                        userInput->getDeathCount(),
                        userInput->isTimerRunning(),
                        userInput->isTimerFinished());
    } else {
        // === MENU MODE ===
        menu->render(windowWidth, windowHeight);
    }
    
    // Present frame
    glfwSwapBuffers(window);
    glfwPollEvents();
}
```

## Input Callbacks

GLFW uses **callbacks** - functions that get called when events happen:

### Keyboard Callback

```cpp
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // action: GLFW_PRESS, GLFW_RELEASE, or GLFW_REPEAT
    
    // ESC toggles pause menu
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        if (menu->isOpen()) {
            menu->close();
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            menu->open();
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        return;
    }
    
    // Forward menu input
    if (menu->isOpen()) {
        menu->handleKey(key, action);
        return;
    }
    
    // Game input
    const ControlSettings& controls = menu->getSettings().controls;
    
    if (action == GLFW_PRESS) {
        if (key == controls.keyForward) w = true;
        if (key == controls.keyJump) userInput->jump();
        // ... more keys
    } else if (action == GLFW_RELEASE) {
        if (key == controls.keyForward) w = false;
        // ... more keys
    }
}
```

### Mouse Movement Callback

```cpp
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    if (menu->isOpen()) return;  // Don't rotate camera when in menu
    
    // Calculate movement since last frame
    static double lastX = xpos;
    static double lastY = ypos;
    
    double dx = xpos - lastX;
    double dy = ypos - lastY;
    
    lastX = xpos;
    lastY = ypos;
    
    // Rotate camera
    userInput->rotate(dx, dy);
}
```

### Scroll Callback

```cpp
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    // Zoom camera in/out
    userInput->adjustCameraDistance(yoffset);
}
```

## State Machine Pattern

The menu uses a **state machine** - different behavior based on current state:

```cpp
enum class MenuState {
    NONE,               // Menu closed
    PAUSE,              // Main pause menu
    SETTINGS,           // Settings submenu
    CONTROLS_SETTINGS,  // Controls settings
    GRAPHICS_SETTINGS,  // Graphics settings
    DIFFICULTY_SETTINGS,// Difficulty settings
    CUSTOM_SETTINGS,    // Custom difficulty
    KEYBIND_WAITING,    // Waiting for key press
    HELP                // Help screen
};

void Menu::render(...) {
    if (state == MenuState::NONE) return;  // Don't render if closed
    
    if (state == MenuState::PAUSE) {
        // Draw pause menu buttons
    }
    else if (state == MenuState::SETTINGS) {
        // Draw settings buttons
    }
    else if (state == MenuState::CONTROLS_SETTINGS) {
        // Draw control keybind UI
    }
    // ... etc
}

void Menu::handleKey(int key, int action) {
    if (state == MenuState::PAUSE) {
        // Handle pause menu navigation
        if (key == GLFW_KEY_ENTER) {
            if (selectedIndex == 2) {
                state = MenuState::SETTINGS;  // Transition to settings
            }
        }
    }
    else if (state == MenuState::KEYBIND_WAITING) {
        // Record the key press for rebinding
        pendingSettings.controls.keyForward = key;
        state = MenuState::CONTROLS_SETTINGS;  // Return to controls
    }
    // ... handle other states
}
```

## Component Communication

How do systems talk to each other?

### 1. Direct Method Calls

```cpp
// Main loop calls player update with course reference
userInput->update(windowWidth, windowHeight, obstacles, grid, deltaTime);

// Player checks collision with course
if (course->checkCollision(newX, playerY, playerZ, collisionRadius)) {
    // Can't move there
}
```

### 2. Getter Methods

```cpp
// Menu reads player stats for HUD
menu->renderHUD(windowWidth, windowHeight,
                userInput->getTimer(),        // Get timer value
                userInput->getDeathCount(),   // Get death count
                userInput->isTimerRunning(),
                userInput->isTimerFinished());
```

### 3. Settings Propagation

```cpp
// Apply settings from menu to player
const GameSettings& settings = menu->getSettings();
userInput->setPhysics(settings.speed, settings.gravity, settings.jumpForce);
userInput->setSensitivity(settings.controls.sensitivity);
userInput->setFOV(settings.graphics.fov);
```

### 4. Boolean Flags

```cpp
// Menu sets flags that main loop checks
if (menu->shouldQuit) {
    glfwSetWindowShouldClose(window, true);
}
if (menu->shouldRestart) {
    userInput->resetPosition();
    userInput->resetStats();
    menu->resetFlags();
}
```

## Object Lifetime

### Creation (Heap Allocation)

```cpp
// Created at startup
userInput = new UserInput();
obstacles = new ObstacleCourse();
```

Using `new` allocates on the **heap** - memory that persists until explicitly deleted.

### Destruction

```cpp
// Deleted at shutdown
delete userInput;
delete obstacles;
```

Always match `new` with `delete` to avoid memory leaks!

### Why Heap Instead of Stack?

Stack allocation:
```cpp
UserInput player;  // Created on stack, destroyed when function ends
```

Heap allocation:
```cpp
UserInput* player = new UserInput();  // Lives until delete
```

We use heap because:
1. Objects need to live across many frames
2. We create them in `main()` but use them in callbacks
3. More control over lifetime

## Error Handling

The code uses simple checks:

```cpp
// Check GLFW initialization
if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
}

// Check window creation
window = glfwCreateWindow(800, 600, "Game", NULL, NULL);
if (!window) {
    glfwTerminate();
    return -1;
}

// Null checks before using pointers
if (course) {
    course->checkCollision(...);
}
```

## Frame Rate Independence

All movement is multiplied by delta time:

```cpp
// In UserInput::update()
float timeScale = deltaTime * 60.0f;  // Normalized to 60 FPS

// Apply gravity
yVel += gravity * timeScale;

// Move player
playerY += yVel * timeScale;
```

This ensures consistent game speed regardless of frame rate.

## Next Steps

Continue to [Player Movement & Physics](./05_player_physics.md) to dive deep into how the player moves.
