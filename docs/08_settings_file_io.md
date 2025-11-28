# 8. Settings & File I/O

This document explains how settings are structured, saved to disk, and loaded.

## Settings Structure

### Control Settings

```cpp
struct ControlSettings {
    float sensitivity;      // Mouse sensitivity (0.001 to 0.01)
    bool toggleCrouch;      // true = toggle, false = hold
    int keyForward;         // GLFW key code
    int keyBackward;
    int keyLeft;
    int keyRight;
    int keyJump;
    int keyCrouch;
    int keyTimer;
    int keyReset;
    int keyHelp;
    
    ControlSettings();  // Constructor sets defaults
};
```

### Graphics Settings

```cpp
struct GraphicsSettings {
    bool vsync;             // Vertical sync on/off
    float renderDistance;   // How far to draw (500-10000)
    int maxFramerate;       // 0 = unlimited, else 30-240
    float guiScale;         // UI scale (0.5 to 2.0)
    bool fullscreen;        // Fullscreen mode
    float fov;              // Field of view (30-150 degrees)
    
    GraphicsSettings();
};
```

### Game Settings (Main Container)

```cpp
struct GameSettings {
    float speed;            // Movement speed
    float gravity;          // Gravity strength (negative)
    float jumpForce;        // Jump power
    bool devMode;           // God mode / no-clip
    
    ControlSettings controls;
    GraphicsSettings graphics;
    
    // File operations
    bool saveToFile(const std::string& filename);
    bool loadFromFile(const std::string& filename);
};
```

## Default Values

```cpp
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
}

GraphicsSettings::GraphicsSettings() {
    vsync = true;
    renderDistance = 3000.0f;
    maxFramerate = 0;  // Unlimited
    guiScale = 1.0f;
    fullscreen = false;
    fov = 60.0f;
}

GameSettings::GameSettings() {
    speed = 5.0f;
    gravity = -0.8f;
    jumpForce = 15.0f;
    devMode = false;
}
```

## File Format

Settings are saved as simple key=value pairs:

```
# settings.cfg
speed=5.0
gravity=-0.8
jumpForce=15.0
devMode=0

sensitivity=0.003
toggleCrouch=0
keyForward=87
keyBackward=83
keyLeft=65
keyRight=68
keyJump=32
keyCrouch=340
keyTimer=84
keyReset=82
keyHelp=72

vsync=1
renderDistance=3000.0
maxFramerate=0
guiScale=1.0
fullscreen=0
fov=60.0
```

## Saving Settings

```cpp
bool GameSettings::saveToFile(const std::string& filename) {
    // Open file for writing
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open settings file for writing: " << filename << std::endl;
        return false;
    }
    
    // Write game settings
    file << "speed=" << speed << "\n";
    file << "gravity=" << gravity << "\n";
    file << "jumpForce=" << jumpForce << "\n";
    file << "devMode=" << (devMode ? 1 : 0) << "\n";
    file << "\n";
    
    // Write control settings
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
    file << "keyHelp=" << controls.keyHelp << "\n";
    file << "\n";
    
    // Write graphics settings
    file << "vsync=" << (graphics.vsync ? 1 : 0) << "\n";
    file << "renderDistance=" << graphics.renderDistance << "\n";
    file << "maxFramerate=" << graphics.maxFramerate << "\n";
    file << "guiScale=" << graphics.guiScale << "\n";
    file << "fullscreen=" << (graphics.fullscreen ? 1 : 0) << "\n";
    file << "fov=" << graphics.fov << "\n";
    
    file.close();
    std::cout << "Settings saved to " << filename << std::endl;
    return true;
}
```

### File Stream Basics

```cpp
#include <fstream>

// Output file stream (writing)
std::ofstream file("settings.cfg");
if (file.is_open()) {
    file << "key=value\n";    // Write using << operator
    file << "number=" << 42;   // Can chain values
    file.close();              // Close when done
}

// Input file stream (reading)
std::ifstream file("settings.cfg");
if (file.is_open()) {
    std::string line;
    while (std::getline(file, line)) {  // Read line by line
        // Process line
    }
    file.close();
}
```

## Loading Settings

```cpp
bool GameSettings::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "No settings file found, using defaults" << std::endl;
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;
        
        // Find the '=' separator
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;  // No '=' found
        
        // Split into key and value
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        // Parse based on key
        if (key == "speed") speed = std::stof(value);
        else if (key == "gravity") gravity = std::stof(value);
        else if (key == "jumpForce") jumpForce = std::stof(value);
        else if (key == "devMode") devMode = (std::stoi(value) != 0);
        
        // Control settings
        else if (key == "sensitivity") controls.sensitivity = std::stof(value);
        else if (key == "toggleCrouch") controls.toggleCrouch = (std::stoi(value) != 0);
        else if (key == "keyForward") controls.keyForward = std::stoi(value);
        else if (key == "keyBackward") controls.keyBackward = std::stoi(value);
        // ... more keys
        
        // Graphics settings
        else if (key == "vsync") graphics.vsync = (std::stoi(value) != 0);
        else if (key == "renderDistance") graphics.renderDistance = std::stof(value);
        else if (key == "maxFramerate") graphics.maxFramerate = std::stoi(value);
        else if (key == "guiScale") graphics.guiScale = std::stof(value);
        else if (key == "fullscreen") graphics.fullscreen = (std::stoi(value) != 0);
        else if (key == "fov") graphics.fov = std::stof(value);
    }
    
    file.close();
    std::cout << "Settings loaded from " << filename << std::endl;
    return true;
}
```

### String Parsing Functions

```cpp
#include <string>

// Find position of character
size_t pos = line.find('=');  // Returns position or std::string::npos

// Extract substring
std::string before = line.substr(0, pos);      // From start to pos
std::string after = line.substr(pos + 1);      // From pos+1 to end

// Convert string to number
float f = std::stof("3.14");   // String to float
int i = std::stoi("42");       // String to int
double d = std::stod("2.718"); // String to double
```

## Applying Settings

### Menu Apply Function

```cpp
void Menu::applyPendingSettings() {
    // Copy pending settings to active settings
    settings = pendingSettings;
    
    // Apply graphics settings immediately
    if (windowRef) {
        // VSync
        glfwSwapInterval(settings.graphics.vsync ? 1 : 0);
        
        // Fullscreen toggle
        if (settings.graphics.fullscreen != /* current state */) {
            shouldToggleFullscreen = true;
        }
    }
    
    // Show feedback popup
    applyFeedbackTimer = 1.5f;
    popupMessage = "Settings Applied!";
    
    // Save to file
    settings.saveToFile("settings.cfg");
}
```

### Syncing Sliders from Settings

When opening settings menu, sliders must match current values:

```cpp
void Menu::syncSlidersFromSettings() {
    // Convert actual values to 0-1 slider range
    
    // Sensitivity: 0.001 to 0.01 → 0 to 1
    sensitivitySlider = (pendingSettings.controls.sensitivity - 0.001f) / 0.009f;
    
    // Render distance: 500 to 10000 → 0 to 1
    renderDistanceSlider = (pendingSettings.graphics.renderDistance - 500.0f) / 9500.0f;
    
    // FOV: 30 to 150 → 0 to 1
    fovSlider = (pendingSettings.graphics.fov - 30.0f) / 120.0f;
    
    // Framerate: 30 to 240 (or unlimited) → 0 to 1
    if (pendingSettings.graphics.maxFramerate == 0) {
        framerateSlider = 1.0f;  // Unlimited at max
    } else {
        framerateSlider = (pendingSettings.graphics.maxFramerate - 30) / 210.0f;
    }
    
    // GUI scale: 0.5 to 2.0 → 0 to 1
    guiScaleSlider = (pendingSettings.graphics.guiScale - 0.5f) / 1.5f;
}
```

### Converting Slider to Value

```cpp
// When slider changes:
sensitivitySlider = newValue;  // 0.0 to 1.0
pendingSettings.controls.sensitivity = 0.001f + sensitivitySlider * 0.009f;

// Framerate with unlimited option:
if (framerateSlider >= 0.99f) {
    pendingSettings.graphics.maxFramerate = 0;  // Unlimited
} else {
    pendingSettings.graphics.maxFramerate = 30 + (int)(framerateSlider * 210.0f);
}
```

## Comparison Operators

To detect if settings have changed:

```cpp
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
           keyHelp == other.keyHelp;
}

// Usage:
if (pendingSettings.controls != settings.controls) {
    // Controls have been changed
}
```

## Reset to Defaults

```cpp
void Menu::resetToDefaults() {
    // Create fresh default settings
    pendingSettings = GameSettings();  // Default constructor
    
    // Sync sliders to match
    syncSlidersFromSettings();
    
    // Show feedback
    resetFeedbackTimer = 1.5f;
    popupMessage = "Settings Reset!";
    
    // Play notification sound
    playPopupSound();
}
```

## Propagating Settings to Game

In the main loop, settings are applied to game systems:

```cpp
// Every frame:
const GameSettings& settings = menu->getSettings();

// Apply physics settings
userInput->setPhysics(settings.speed, settings.gravity, settings.jumpForce);

// Apply control settings
userInput->setSensitivity(settings.controls.sensitivity);

// Apply graphics settings
userInput->setRenderDistance(settings.graphics.renderDistance);
userInput->setFOV(settings.graphics.fov);
userInput->setDevMode(settings.devMode);
```

## Settings File Location

The settings file is created in the same directory as the executable:

```cpp
// In Menu::loadSettings()
settings.loadFromFile("settings.cfg");

// In Menu::applyPendingSettings()
settings.saveToFile("settings.cfg");
```

This means `build/settings.cfg` when running from the build directory.

## Error Handling

```cpp
bool GameSettings::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        // File doesn't exist - not an error, just use defaults
        std::cout << "No settings file found, using defaults" << std::endl;
        return false;
    }
    
    try {
        // Parse settings...
        float value = std::stof(valueStr);  // May throw
    } catch (const std::exception& e) {
        std::cerr << "Error parsing setting: " << e.what() << std::endl;
        // Keep default value for this setting
    }
    
    return true;
}
```

## Next Steps

Continue to [Audio System](./09_audio_system.md) to learn about sound effects.
