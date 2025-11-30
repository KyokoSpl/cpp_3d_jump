#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>

// Control settings
struct ControlSettings {
    float sensitivity;
    bool toggleCrouch;      // true = toggle, false = hold
    int keyForward;
    int keyBackward;
    int keyLeft;
    int keyRight;
    int keyJump;
    int keyCrouch;
    int keyTimer;
    int keyReset;
    int keyHelp;
    int keyLeaderboard;
    
    ControlSettings();
    bool operator==(const ControlSettings& other) const;
    bool operator!=(const ControlSettings& other) const { return !(*this == other); }
};

// Graphics settings
struct GraphicsSettings {
    bool vsync;
    float renderDistance;
    int maxFramerate;       // 0 = unlimited
    float guiScale;         // 0.5 to 2.0
    bool fullscreen;
    float fov;
    
    GraphicsSettings();
    bool operator==(const GraphicsSettings& other) const;
    bool operator!=(const GraphicsSettings& other) const { return !(*this == other); }
};

// Game difficulty settings
struct GameSettings {
    float speed;
    float gravity;
    float jumpForce;
    bool devMode;
    
    // References to other settings
    ControlSettings controls;
    GraphicsSettings graphics;
    
    GameSettings();
    bool operator==(const GameSettings& other) const;
    bool operator!=(const GameSettings& other) const { return !(*this == other); }
    
    // File I/O
    bool saveToFile(const std::string& filename);
    bool loadFromFile(const std::string& filename);
};

enum class Difficulty {
    PUSSY,
    HUMAN,
    GOAT,
    I_HATE_MYSELF,
    CUSTOM
};

#endif // SETTINGS_H
