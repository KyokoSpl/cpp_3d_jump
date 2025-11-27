#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <iostream>
#include <cmath>
#include "Grid.h"
#include "UserInput.h"
#include "Obstacle.h"

// Global variables
Grid* grid = nullptr;
UserInput* userInput = nullptr;
ObstacleCourse* obstacles = nullptr;
bool w = false, a = false, s = false, d = false;
bool shift = false;
int windowWidth = 1920;
int windowHeight = 1080;
GLFWwindow* window = nullptr;

// Mouse position tracking
double lastMouseX = 0;
double lastMouseY = 0;
bool firstMouse = true;

// Forward declarations
void setup();
void draw();
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);

int main() {
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
    glfwSetCursorPosCallback(window, cursorPosCallback);
    
    // Hide and capture cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    
    // Set viewport
    glViewport(0, 0, windowWidth, windowHeight);

    // Setup
    setup();

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Poll events
        glfwPollEvents();

        // Draw
        draw();

        // Swap buffers
        glfwSwapBuffers(window);
    }

    // Cleanup
    delete grid;
    delete userInput;
    delete obstacles;
    glfwTerminate();

    return 0;
}

void setup() {
    grid = new Grid(40, 20);
    userInput = new UserInput();
    obstacles = new ObstacleCourse();
    
    // Center mouse position
    lastMouseX = windowWidth / 2.0;
    lastMouseY = windowHeight / 2.0;
}

void draw() {
    // Clear screen
    glClearColor(0.1f, 0.15f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Update
    userInput->setCrouch(shift);
    userInput->update(windowWidth, windowHeight, obstacles, grid);
    userInput->move(w, s, a, d, obstacles);
    
    // Render
    grid->update();
    obstacles->render();
    userInput->render();
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_W) w = true;
        if (key == GLFW_KEY_A) a = true;
        if (key == GLFW_KEY_S) s = true;
        if (key == GLFW_KEY_D) d = true;
        if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) shift = true;
        if (key == GLFW_KEY_SPACE) userInput->jump();
        if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, true);
    } else if (action == GLFW_RELEASE) {
        if (key == GLFW_KEY_W) w = false;
        if (key == GLFW_KEY_A) a = false;
        if (key == GLFW_KEY_S) s = false;
        if (key == GLFW_KEY_D) d = false;
        if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) shift = false;
    }
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
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
