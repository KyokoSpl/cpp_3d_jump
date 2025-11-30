#include "Settings.h"
#include <GLFW/glfw3.h>
#include <fstream>
#include <string>

// ==================== ControlSettings ====================

ControlSettings::ControlSettings() {
    sensitivity = 0.003f;
    toggleCrouch = false;
    keyForward = GLFW_KEY_W;
    keyBackward = GLFW_KEY_S;
    keyLeft = GLFW_KEY_A;
    keyRight = GLFW_KEY_D;
    keyJump = GLFW_KEY_SPACE;
    keyCrouch = GLFW_KEY_LEFT_SHIFT;
    keyTimer = GLFW_KEY_T;
    keyReset = GLFW_KEY_R;
    keyHelp = GLFW_KEY_H;
    keyLeaderboard = GLFW_KEY_L;
}

bool ControlSettings::operator==(const ControlSettings& other) const {
    return sensitivity == other.sensitivity &&
           toggleCrouch == other.toggleCrouch &&
           keyForward == other.keyForward &&
           keyBackward == other.keyBackward &&
           keyLeft == other.keyLeft &&
           keyRight == other.keyRight &&
           keyJump == other.keyJump &&
           keyCrouch == other.keyCrouch &&
           keyTimer == other.keyTimer &&
           keyReset == other.keyReset &&
           keyHelp == other.keyHelp &&
           keyLeaderboard == other.keyLeaderboard;
}

// ==================== GraphicsSettings ====================

GraphicsSettings::GraphicsSettings() {
    vsync = true;
    renderDistance = 3000.0f;
    maxFramerate = 0;  // unlimited
    guiScale = 1.0f;
    fullscreen = false;
    fov = 60.0f;
}

bool GraphicsSettings::operator==(const GraphicsSettings& other) const {
    return vsync == other.vsync &&
           renderDistance == other.renderDistance &&
           maxFramerate == other.maxFramerate &&
           guiScale == other.guiScale &&
           fullscreen == other.fullscreen &&
           fov == other.fov;
}

// ==================== GameSettings ====================

GameSettings::GameSettings() {
    speed = 5.0f;
    gravity = -0.8f;
    jumpForce = 15.0f;
    devMode = false;
}

bool GameSettings::operator==(const GameSettings& other) const {
    return speed == other.speed &&
           gravity == other.gravity &&
           jumpForce == other.jumpForce &&
           devMode == other.devMode &&
           controls == other.controls &&
           graphics == other.graphics;
}

bool GameSettings::saveToFile(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) return false;
    
    file << "# Game Settings\n";
    file << "speed=" << speed << "\n";
    file << "gravity=" << gravity << "\n";
    file << "jumpForce=" << jumpForce << "\n";
    file << "\n# Controls\n";
    file << "sensitivity=" << controls.sensitivity << "\n";
    file << "toggleCrouch=" << (controls.toggleCrouch ? 1 : 0) << "\n";
    file << "keyForward=" << controls.keyForward << "\n";
    file << "keyBackward=" << controls.keyBackward << "\n";
    file << "keyLeft=" << controls.keyLeft << "\n";
    file << "keyRight=" << controls.keyRight << "\n";
    file << "keyJump=" << controls.keyJump << "\n";
    file << "keyCrouch=" << controls.keyCrouch << "\n";
    file << "keyTimer=" << controls.keyTimer << "\n";
    file << "keyReset=" << controls.keyReset << "\n";
    file << "\n# Graphics\n";
    file << "vsync=" << (graphics.vsync ? 1 : 0) << "\n";
    file << "renderDistance=" << graphics.renderDistance << "\n";
    file << "maxFramerate=" << graphics.maxFramerate << "\n";
    file << "guiScale=" << graphics.guiScale << "\n";
    file << "fullscreen=" << (graphics.fullscreen ? 1 : 0) << "\n";
    file << "fov=" << graphics.fov << "\n";
    
    file.close();
    return true;
}

bool GameSettings::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        
        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);
        
        // Parse values
        if (key == "speed") speed = std::stof(value);
        else if (key == "gravity") gravity = std::stof(value);
        else if (key == "jumpForce") jumpForce = std::stof(value);
        else if (key == "sensitivity") controls.sensitivity = std::stof(value);
        else if (key == "toggleCrouch") controls.toggleCrouch = (std::stoi(value) != 0);
        else if (key == "keyForward") controls.keyForward = std::stoi(value);
        else if (key == "keyBackward") controls.keyBackward = std::stoi(value);
        else if (key == "keyLeft") controls.keyLeft = std::stoi(value);
        else if (key == "keyRight") controls.keyRight = std::stoi(value);
        else if (key == "keyJump") controls.keyJump = std::stoi(value);
        else if (key == "keyCrouch") controls.keyCrouch = std::stoi(value);
        else if (key == "keyTimer") controls.keyTimer = std::stoi(value);
        else if (key == "keyReset") controls.keyReset = std::stoi(value);
        else if (key == "vsync") graphics.vsync = (std::stoi(value) != 0);
        else if (key == "renderDistance") graphics.renderDistance = std::stof(value);
        else if (key == "maxFramerate") graphics.maxFramerate = std::stoi(value);
        else if (key == "guiScale") graphics.guiScale = std::stof(value);
        else if (key == "fullscreen") graphics.fullscreen = (std::stoi(value) != 0);
        else if (key == "fov") graphics.fov = std::stof(value);
    }
    
    file.close();
    return true;
}
