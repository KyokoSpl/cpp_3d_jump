#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <iostream>
#include <cmath>
#include <cstring>
#include "Grid.h"
#include "UserInput.h"
#include "Obstacle.h"
#include "Menu.h"
#include "Projectile.h"

// Global variables
Grid* grid = nullptr;
UserInput* userInput = nullptr;
ObstacleCourse* obstacles = nullptr;
Menu* menu = nullptr;
ProjectileManager* projectiles = nullptr;
bool w = false, a = false, s = false, d = false;
bool shift = false;
int windowWidth = 1920;
int windowHeight = 1080;
GLFWwindow* window = nullptr;

// Mouse position tracking
double lastMouseX = 0;
double lastMouseY = 0;
bool firstMouse = true;

// Delta time tracking
double lastFrameTime = 0.0;
float deltaTime = 0.0f;

// Forward declarations
void setup();
void draw();
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void charCallback(GLFWwindow* window, unsigned int codepoint);
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void applyGraphicsSettings(const GraphicsSettings& graphics);
void toggleFullscreen();

int main(int argc, char* argv[]) {
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--dev") == 0) {
            Menu::devModeEnabled = true;
            std::cout << "Dev mode enabled - god mode active" << std::endl;
        }
    }
    
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Configure GLFW - request legacy OpenGL context
    // Don't specify version to get default (legacy) context
    
    #ifdef __APPLE__
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // Get primary monitor for fullscreen
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    if (monitor) {
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        if (mode) {
            windowWidth = mode->width;
            windowHeight = mode->height;
        }
    }

    // Create windowed mode (not fullscreen) for better compatibility
    window = glfwCreateWindow(windowWidth, windowHeight, "3D Grid Navigation", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCharCallback(window, charCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    
    // Hide and capture cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    
    // Set viewport
    glViewport(0, 0, windowWidth, windowHeight);

    // Setup
    setup();

    // Initialize time tracking
    lastFrameTime = glfwGetTime();

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Calculate delta time
        double currentTime = glfwGetTime();
        deltaTime = static_cast<float>(currentTime - lastFrameTime);
        lastFrameTime = currentTime;
        
        // Clamp delta time to prevent huge jumps (e.g., when window is moved)
        if (deltaTime > 0.1f) deltaTime = 0.1f;

        // Poll events
        glfwPollEvents();

        // Draw
        draw();
        
        // Handle menu actions
        if (menu->shouldQuit) {
            glfwSetWindowShouldClose(window, true);
        }
        if (menu->shouldRestart) {
            userInput->resetPosition();
            userInput->resetStats();  // Reset timer and death count
            projectiles->reset();
            menu->resetFlags();
        }
        if (menu->shouldResetToStart) {
            // Reset after completion screen
            userInput->resetPosition();
            userInput->resetStats();
            projectiles->reset();
            // Reset input states to prevent residual movement
            w = s = a = d = false;
            shift = false;
            menu->shouldResetToStart = false;
        }
        if (menu->shouldToggleFullscreen) {
            toggleFullscreen();
            menu->shouldToggleFullscreen = false;
        }
        if (menu->shouldUpdateVSync) {
            glfwSwapInterval(menu->getSettings().graphics.vsync ? 1 : 0);
            menu->shouldUpdateVSync = false;
        }

        // Swap buffers
        glfwSwapBuffers(window);
    }

    // Cleanup
    delete grid;
    delete userInput;
    delete obstacles;
    delete menu;
    delete projectiles;
    glfwTerminate();

    return 0;
}

void setup() {
    grid = new Grid(40, 20);  // 40 cells * 20 size = 800 total grid size
    userInput = new UserInput();
    obstacles = new ObstacleCourse();
    menu = new Menu();
    menu->setWindow(window);
    projectiles = new ProjectileManager(800.0f);  // Match grid size
    
    // Load saved settings
    menu->loadSettings();
    
    // Apply initial settings
    const GameSettings& settings = menu->getSettings();
    userInput->setPhysics(settings.speed, settings.gravity, settings.jumpForce);
    userInput->setRenderDistance(settings.graphics.renderDistance);
    userInput->setSensitivity(settings.controls.sensitivity);
    userInput->setFOV(settings.graphics.fov);
    
    // Apply graphics settings (vsync, framerate)
    applyGraphicsSettings(settings.graphics);
    
    // Center mouse position
    lastMouseX = windowWidth / 2.0;
    lastMouseY = windowHeight / 2.0;
}

void applyGraphicsSettings(const GraphicsSettings& graphics) {
    // VSync
    glfwSwapInterval(graphics.vsync ? 1 : 0);
    
    // Note: maxFramerate limiting would need to be done manually in main loop
    // Fullscreen toggle is handled separately via shouldToggleFullscreen flag
}

void toggleFullscreen() {
    static bool isFullscreen = false;
    static int savedX = 100, savedY = 100, savedW = 1280, savedH = 720;
    
    if (!isFullscreen) {
        // Save current window position/size
        glfwGetWindowPos(window, &savedX, &savedY);
        glfwGetWindowSize(window, &savedW, &savedH);
        
        // Go fullscreen
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        windowWidth = mode->width;
        windowHeight = mode->height;
    } else {
        // Go windowed
        glfwSetWindowMonitor(window, nullptr, savedX, savedY, savedW, savedH, 0);
        windowWidth = savedW;
        windowHeight = savedH;
    }
    isFullscreen = !isFullscreen;
    
    // Update viewport
    glViewport(0, 0, windowWidth, windowHeight);
}

void draw() {
    // Clear screen
    glClearColor(0.1f, 0.15f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Only update game if menu is closed
    if (!menu->isOpen()) {
        // Update
        userInput->setCrouch(shift);
        userInput->update(windowWidth, windowHeight, obstacles, grid, deltaTime);
        userInput->move(w, s, a, d, obstacles, deltaTime);
        
        // Apply current physics settings
        const GameSettings& settings = menu->getSettings();
        userInput->setPhysics(settings.speed, settings.gravity, settings.jumpForce);
        userInput->setDevMode(settings.devMode);
        userInput->setRenderDistance(settings.graphics.renderDistance);
        userInput->setSensitivity(settings.controls.sensitivity);
        userInput->setFOV(settings.graphics.fov);
        
        // Update projectiles
        projectiles->update(deltaTime);
        
        // Check for projectile collision with player (skip if dev mode)
        if (!menu->getSettings().devMode && 
            projectiles->checkPlayerCollision(
                userInput->getPlayerX(),
                userInput->getPlayerY(),
                userInput->getPlayerZ(),
                userInput->getCollisionRadius(),
                userInput->getPlayerHeight(),
                userInput->getIsCrouching())) {
            // Player hit! Respawn (counts as death)
            userInput->respawn(obstacles);
            projectiles->reset();
        }
        
        // Check if player reached the goal (trigger even if timer not running)
        if (!userInput->isTimerFinished() && obstacles->isOnGoal(
                userInput->getPlayerX(),
                userInput->getPlayerY(),
                userInput->getPlayerZ())) {
            userInput->stopTimer();  // Stop timer at goal
            // Show completion screen
            menu->showCompletion(userInput->getTimer(), userInput->getDeathCount());
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    
    // Update completion countdown
    menu->updateCompletion(deltaTime);
    
    // Check if completion is done and reset cursor
    if (menu->shouldResetToStart) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    
    // Render game world
    grid->update();
    obstacles->render(deltaTime);
    projectiles->render();
    userInput->render();
    
    // Render checkpoint popup if active
    if (userInput->getCheckpointPopupTimer() > 0) {
        menu->renderCheckpointPopup(windowWidth, windowHeight, 
                                     userInput->getCheckpointMessage(),
                                     userInput->getCheckpointPopupTimer());
    }
    
    // Render HUD (timer and death count) when menu is closed
    if (!menu->isOpen()) {
        menu->renderHUD(windowWidth, windowHeight, userInput->getTimer(), 
                        userInput->getDeathCount(), userInput->isTimerRunning(),
                        userInput->isTimerFinished());
    }
    
    // Render menu on top if open
    menu->render(windowWidth, windowHeight);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // During completion screen, only handle specific keys
    if (menu->getState() == MenuState::COMPLETION) {
        menu->handleKey(key, action);
        return;
    }
    
    // Toggle menu with Escape
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        if (menu->isOpen()) {
            if (menu->getState() == MenuState::PAUSE) {
                menu->close();
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            } else {
                // Pass to menu to go back
                menu->handleKey(key, action);
            }
        } else {
            menu->open();
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        return;
    }
    
    // If menu is open, pass input to menu
    if (menu->isOpen()) {
        menu->handleKey(key, action);
        
        // Check if menu was closed (resume)
        if (!menu->isOpen()) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        return;
    }
    
    // Get current keybinds from settings
    const ControlSettings& controls = menu->getSettings().controls;
    
    // Game input handling using configurable keybinds
    if (action == GLFW_PRESS) {
        if (key == controls.keyForward) w = true;
        if (key == controls.keyLeft) a = true;
        if (key == controls.keyBackward) s = true;
        if (key == controls.keyRight) d = true;
        if (key == controls.keyCrouch) {
            shift = true;
            userInput->setCrouch(true);  // Immediately crouch
        }
        if (key == controls.keyJump) {
            if (shift && userInput->getIsCrouching()) {
                userInput->crouchJump();  // Crouch jump when crouching + space
            } else {
                userInput->jump();
            }
        }
        if (key == controls.keyTimer) userInput->toggleTimer();
        if (key == controls.keyReset) userInput->resetStats();
        if (key == GLFW_KEY_E) userInput->setWallRunKey(true);  // Wall run key
        if (key == controls.keyHelp) {  // Help menu
            menu->open();
            menu->showHelp();
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    } else if (action == GLFW_RELEASE) {
        if (key == controls.keyForward) w = false;
        if (key == controls.keyLeft) a = false;
        if (key == controls.keyBackward) s = false;
        if (key == controls.keyRight) d = false;
        if (key == controls.keyCrouch) {
            shift = false;
            userInput->setCrouch(false);  // Immediately uncrouch
        }
        if (key == GLFW_KEY_E) userInput->setWallRunKey(false);  // Release wall run
    }
}

void charCallback(GLFWwindow* window, unsigned int codepoint) {
    (void)window;  // Unused
    
    // Forward character input to menu for name entry
    menu->handleCharInput(codepoint);
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    // Handle menu mouse movement
    if (menu->isOpen()) {
        menu->handleMouseMove(xpos, ypos);
        return;
    }
    
    if (firstMouse) {
        lastMouseX = xpos;
        lastMouseY = ypos;
        firstMouse = false;
    }

    float dx = static_cast<float>(xpos - lastMouseX);
    float dy = static_cast<float>(ypos - lastMouseY);

    lastMouseX = xpos;
    lastMouseY = ypos;

    userInput->rotate(dx, dy);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)window;   // Unused
    (void)xoffset;  // Unused (horizontal scroll)
    
    if (menu->isOpen()) return;  // Don't zoom when menu is open
    
    userInput->adjustCameraDistance(static_cast<float>(yoffset));
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    (void)mods;  // Unused
    
    if (menu->isOpen()) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        menu->handleMouseClick(xpos, ypos, button, action);
        
        // Check if menu was closed
        if (!menu->isOpen()) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}
