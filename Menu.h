#ifndef MENU_H
#define MENU_H

#include <string>
#include <vector>
#include <functional>
#include <map>

#include <ft2build.h>
#include FT_FREETYPE_H

struct GLFWwindow;

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

enum class MenuState {
    NONE,
    PAUSE,
    SETTINGS,
    CONTROLS_SETTINGS,
    GRAPHICS_SETTINGS,
    DIFFICULTY_SETTINGS,
    CUSTOM_SETTINGS,
    KEYBIND_WAITING,      // Waiting for key press to rebind
    HELP,                 // Help/controls display
    COMPLETION            // Course completed - name entry
};

// Character info for font rendering
struct Character {
    unsigned int textureID;
    int sizeX, sizeY;
    int bearingX, bearingY;
    unsigned int advance;
};

class Menu {
public:
    static bool devModeEnabled;
    
private:
    MenuState state;
    MenuState previousState;  // For returning from keybind
    int selectedIndex;
    int settingsSelectedIndex;
    int customSelectedIndex;
    int controlsSelectedIndex;
    int graphicsSelectedIndex;
    int waitingForKeybind;    // Which keybind we're waiting to set (-1 = none)
    float resetFeedbackTimer; // Timer for showing "Settings Reset!" feedback
    float applyFeedbackTimer; // Timer for showing "Settings Applied!" feedback
    std::string popupMessage;  // Current popup message
    bool popupIsGreen;         // true = green (reset), false = blue (apply)
    
    // Completion screen
    std::string playerName;           // Name being entered
    float completionTime;             // Final time achieved
    int completionDeaths;             // Deaths during run
    float completionCountdown;        // 10 second countdown
    bool completionSaved;             // Has the score been saved
    
    Difficulty currentDifficulty;
    GameSettings settings;
    GameSettings pendingSettings;  // Settings being edited (applied on Apply)
    GameSettings customSettings;
    
    // Slider values for custom difficulty (0.0 to 1.0)
    float speedSlider;
    float gravitySlider;
    float jumpSlider;
    
    // Slider values for controls/graphics
    float sensitivitySlider;
    float renderDistanceSlider;
    float fovSlider;
    float guiScaleSlider;
    float framerateSlider;
    
    int draggingSlider;
    
    std::vector<std::string> pauseButtons;
    std::vector<std::string> settingsButtons;
    std::vector<std::string> difficultyOptions;
    
    // FreeType font rendering
    FT_Library ftLibrary;
    FT_Face ftFace;
    std::map<char, Character> characters;
    bool fontLoaded;
    
    bool initFont(const std::string& fontPath);
    void cleanupFont();
    
    void drawButton(float x, float y, float width, float height, 
                    const std::string& text, bool selected, bool hovered = false);
    void drawCheckbox(float x, float y, float size, const std::string& label, 
                      bool checked, bool selected);
    void drawSlider(float x, float y, float width, float height,
                    const std::string& label, float value, bool selected);
    void drawKeybind(float x, float y, float width, float height,
                     const std::string& label, int keyCode, bool selected, bool waiting);
    void drawText(float x, float y, const std::string& text, float scale = 1.0f);
    float getTextWidth(const std::string& text, float scale = 1.0f);
    std::string getKeyName(int keyCode);
    void applyDifficulty(Difficulty diff);
    void applyPendingSettings();
    void syncSlidersFromSettings();
    void resetToDefaults();
    bool hasSettingsChanged() const;  // Check if pending != current
    
    int screenWidth;
    int screenHeight;
    
    // Window reference for fullscreen/vsync
    GLFWwindow* windowRef;

public:
    Menu();
    ~Menu();
    
    void setWindow(GLFWwindow* window) { windowRef = window; }
    
    void open();
    void close();
    void toggle();
    void showHelp();  // Show the help menu
    bool isOpen() const { return state != MenuState::NONE; }
    MenuState getState() const { return state; }
    
    void render(int windowWidth, int windowHeight);
    void renderResetPopup(int windowWidth, int windowHeight);
    void renderCheckpointPopup(int windowWidth, int windowHeight, 
                               const std::string& message, float timer);
    void renderHUD(int windowWidth, int windowHeight, float timer, int deaths, 
                   bool timerRunning, bool timerFinished);
    void handleKey(int key, int action);
    void handleMouseClick(double x, double y, int button, int action);
    void handleMouseMove(double x, double y);
    
    const GameSettings& getSettings() const { return settings; }
    Difficulty getDifficulty() const { return currentDifficulty; }
    
    // Load/save settings
    void loadSettings();
    void saveSettings();
    
    // Completion screen
    void showCompletion(float time, int deaths);
    void updateCompletion(float deltaTime);
    bool isCompletionDone() const;
    void handleCharInput(unsigned int codepoint);
    void saveLeaderboard();
    
    bool shouldRestart;
    bool shouldQuit;
    bool shouldToggleFullscreen;
    bool shouldUpdateVSync;
    bool shouldResetToStart;  // Reset player to start after completion
    
    void resetFlags() { 
        shouldRestart = false; 
        shouldQuit = false; 
        shouldToggleFullscreen = false;
        shouldUpdateVSync = false;
        shouldResetToStart = false;
    }
};

#endif // MENU_H
