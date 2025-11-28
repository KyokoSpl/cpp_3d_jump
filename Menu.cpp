#include "Menu.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>

// Generate the popup beep WAV file
// Sound parameters - customize these values:
//   frequency: 200-2000 Hz recommended (440=A4, 523=C5, 659=E5, 880=A5)
//   duration: 50-200 ms for a short "blip"
static void generatePopupSound() {
    const int sampleRate = 44100;
    const int duration = 100;        // milliseconds
    const float frequency = 440.0f;  // Hz - mid frequency, pleasant
    const int numSamples = sampleRate * duration / 1000;
    
    // Create WAV file in /tmp
    const char* tmpFile = "/tmp/popup_beep.wav";
    FILE* f = fopen(tmpFile, "wb");
    if (!f) return;
    
    // WAV header
    int dataSize = numSamples * 2;  // 16-bit = 2 bytes per sample
    int fileSize = 36 + dataSize;
    
    // RIFF header
    fwrite("RIFF", 1, 4, f);
    fwrite(&fileSize, 4, 1, f);
    fwrite("WAVE", 1, 4, f);
    
    // fmt chunk
    fwrite("fmt ", 1, 4, f);
    int fmtSize = 16;
    short audioFormat = 1;  // PCM
    short numChannels = 1;  // Mono
    int byteRate = sampleRate * 2;
    short blockAlign = 2;
    short bitsPerSample = 16;
    fwrite(&fmtSize, 4, 1, f);
    fwrite(&audioFormat, 2, 1, f);
    fwrite(&numChannels, 2, 1, f);
    fwrite(&sampleRate, 4, 1, f);
    fwrite(&byteRate, 4, 1, f);
    fwrite(&blockAlign, 2, 1, f);
    fwrite(&bitsPerSample, 2, 1, f);
    
    // data chunk
    fwrite("data", 1, 4, f);
    fwrite(&dataSize, 4, 1, f);
    
    // Generate samples - sine wave with envelope for "blob" sound
    for (int i = 0; i < numSamples; i++) {
        float t = (float)i / sampleRate;
        float progress = (float)i / numSamples;
        
        // Envelope: quick attack, sustained, quick decay
        float envelope;
        if (progress < 0.1f) {
            envelope = progress / 0.1f;  // Attack
        } else if (progress < 0.6f) {
            envelope = 1.0f;  // Sustain
        } else {
            envelope = (1.0f - progress) / 0.4f;  // Decay
        }
        
        // Mix two frequencies for richer sound
        float sample = sinf(2.0f * 3.14159f * frequency * t) * 0.7f;
        sample += sinf(2.0f * 3.14159f * frequency * 1.5f * t) * 0.3f;  // Fifth harmonic
        sample *= envelope * 0.5f;  // Apply envelope and reduce volume
        
        short s = (short)(sample * 32767);
        fwrite(&s, 2, 1, f);
    }
    
    fflush(f);
    fclose(f);
}

// Play a short beep sound
static void playPopupSound() {
    generatePopupSound();  // Regenerate each time (allows frequency changes)
    
    // Run paplay in background
    int ret = system("paplay /tmp/popup_beep.wav &");
    (void)ret;  // Suppress unused warning
    
    // Small delay to ensure the shell has spawned paplay before we continue
    usleep(10000);  // 10ms
}

// Initialize static member
bool Menu::devModeEnabled = false;

// ==================== Settings Structs ====================

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
           keyReset == other.keyReset;
}

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

// ==================== Menu Class ====================

Menu::Menu() {
    state = MenuState::NONE;
    previousState = MenuState::NONE;
    selectedIndex = 0;
    settingsSelectedIndex = 0;
    customSelectedIndex = 0;
    controlsSelectedIndex = 0;
    graphicsSelectedIndex = 0;
    waitingForKeybind = -1;
    resetFeedbackTimer = 0.0f;
    applyFeedbackTimer = 0.0f;
    popupMessage = "";
    popupIsGreen = true;
    currentDifficulty = Difficulty::HUMAN;
    draggingSlider = -1;
    windowRef = nullptr;
    
    shouldRestart = false;
    shouldQuit = false;
    shouldToggleFullscreen = false;
    shouldUpdateVSync = false;
    
    screenWidth = 1920;
    screenHeight = 1080;
    
    fontLoaded = false;
    ftLibrary = nullptr;
    ftFace = nullptr;
    
    initFont("asset/BoldPixels.ttf");
    
    pauseButtons = {"Resume", "Restart", "Settings", "Quit"};
    settingsButtons = {"Controls", "Graphics", "Difficulty", "Reset Defaults", "Back"};
    difficultyOptions = {"Pussy", "Human", "Goat", "I Hate Myself", "Custom"};
    
    // Initialize sliders
    speedSlider = 0.3f;
    gravitySlider = 0.4f;
    jumpSlider = 0.5f;
    sensitivitySlider = 0.3f;
    renderDistanceSlider = 0.5f;
    fovSlider = 0.4f;
    guiScaleSlider = 0.5f;
    framerateSlider = 0.0f;
    
    applyDifficulty(Difficulty::HUMAN);
    customSettings = settings;
    settings.devMode = devModeEnabled;
    pendingSettings = settings;
}

Menu::~Menu() {
    cleanupFont();
}

void Menu::loadSettings() {
    if (settings.loadFromFile("settings.cfg")) {
        pendingSettings = settings;
        syncSlidersFromSettings();
        printf("Settings loaded from settings.cfg\n");
    } else {
        printf("No settings file found, using defaults\n");
    }
    settings.devMode = devModeEnabled;
}

void Menu::saveSettings() {
    if (settings.saveToFile("settings.cfg")) {
        printf("Settings saved to settings.cfg\n");
    } else {
        printf("Failed to save settings\n");
    }
}

void Menu::syncSlidersFromSettings() {
    // Sensitivity: 0.001 to 0.01 -> slider 0-1
    sensitivitySlider = (pendingSettings.controls.sensitivity - 0.001f) / 0.009f;
    sensitivitySlider = std::max(0.0f, std::min(1.0f, sensitivitySlider));
    
    // Render distance: 500 to 10000 -> slider 0-1
    renderDistanceSlider = (pendingSettings.graphics.renderDistance - 500.0f) / 9500.0f;
    renderDistanceSlider = std::max(0.0f, std::min(1.0f, renderDistanceSlider));
    
    // FOV: 30 to 150 -> slider 0-1
    fovSlider = (pendingSettings.graphics.fov - 30.0f) / 120.0f;
    fovSlider = std::max(0.0f, std::min(1.0f, fovSlider));
    
    // GUI Scale: 0.5 to 2.0 -> slider 0-1
    guiScaleSlider = (pendingSettings.graphics.guiScale - 0.5f) / 1.5f;
    guiScaleSlider = std::max(0.0f, std::min(1.0f, guiScaleSlider));
    
    // Framerate: 0=unlimited, 30-240 -> slider 0-1 (0=unlimited)
    if (pendingSettings.graphics.maxFramerate == 0) {
        framerateSlider = 1.0f;  // Unlimited at far right
    } else {
        framerateSlider = (pendingSettings.graphics.maxFramerate - 30.0f) / 210.0f;
        framerateSlider = std::max(0.0f, std::min(0.99f, framerateSlider));
    }
}

bool Menu::hasSettingsChanged() const {
    // Compare pending settings to current settings (ignoring devMode which is handled separately)
    GameSettings current = settings;
    GameSettings pending = pendingSettings;
    current.devMode = false;
    pending.devMode = false;
    return !(current == pending);
}

void Menu::applyPendingSettings() {
    // Check if anything actually changed
    bool settingsChanged = hasSettingsChanged();
    
    bool wasFullscreen = settings.graphics.fullscreen;
    bool wasVsync = settings.graphics.vsync;
    
    settings = pendingSettings;
    settings.devMode = devModeEnabled;
    
    if (settings.graphics.fullscreen != wasFullscreen) {
        shouldToggleFullscreen = true;
    }
    if (settings.graphics.vsync != wasVsync) {
        shouldUpdateVSync = true;
    }
    
    saveSettings();
    
    // Show feedback popup
    applyFeedbackTimer = 2.0f;
    if (settingsChanged) {
        popupMessage = "Settings Applied!";
        popupIsGreen = true;   // Green for success/changed
        playPopupSound();
    } else {
        popupMessage = "No Changes";
        popupIsGreen = false;  // Blue for no changes
    }
}

void Menu::resetToDefaults() {
    // Check if settings are already default
    GameSettings defaultSettings;
    defaultSettings.devMode = devModeEnabled;
    
    // Compare current pending settings to defaults (excluding devMode)
    GameSettings pendingCopy = pendingSettings;
    pendingCopy.devMode = false;
    GameSettings defaultCopy = defaultSettings;
    defaultCopy.devMode = false;
    bool alreadyDefault = (pendingCopy == defaultCopy);
    
    // Reset all settings to defaults
    pendingSettings = GameSettings();  // Use default constructor
    pendingSettings.devMode = devModeEnabled;
    
    // Reset difficulty sliders
    speedSlider = 0.3f;
    gravitySlider = 0.4f;
    jumpSlider = 0.5f;
    
    // Sync sliders from default settings
    syncSlidersFromSettings();
    
    // Apply the defaults
    applyDifficulty(Difficulty::HUMAN);
    applyPendingSettings();
    
    // Show feedback for 3 seconds (override applyPendingSettings popup)
    resetFeedbackTimer = 3.0f;
    applyFeedbackTimer = 0.0f;  // Don't show apply popup, show reset popup instead
    
    if (alreadyDefault) {
        popupMessage = "Already Default";
        popupIsGreen = false;  // Blue for no changes
    } else {
        popupMessage = "Defaults Restored!";
        popupIsGreen = true;   // Green for success/changed
        playPopupSound();
    }
    
    // Close the menu
    close();
}

bool Menu::initFont(const std::string& fontPath) {
    if (FT_Init_FreeType(&ftLibrary)) {
        fprintf(stderr, "ERROR: Could not init FreeType Library\n");
        return false;
    }
    
    if (FT_New_Face(ftLibrary, fontPath.c_str(), 0, &ftFace)) {
        fprintf(stderr, "ERROR: Failed to load font: %s\n", fontPath.c_str());
        FT_Done_FreeType(ftLibrary);
        ftLibrary = nullptr;
        return false;
    }
    
    FT_Set_Pixel_Sizes(ftFace, 0, 48);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    for (unsigned char c = 0; c < 128; c++) {
        if (FT_Load_Char(ftFace, c, FT_LOAD_RENDER)) continue;
        
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA,
            ftFace->glyph->bitmap.width, ftFace->glyph->bitmap.rows,
            0, GL_ALPHA, GL_UNSIGNED_BYTE, ftFace->glyph->bitmap.buffer);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        Character character = {
            texture,
            (int)ftFace->glyph->bitmap.width,
            (int)ftFace->glyph->bitmap.rows,
            ftFace->glyph->bitmap_left,
            ftFace->glyph->bitmap_top,
            (unsigned int)ftFace->glyph->advance.x
        };
        characters[c] = character;
    }
    
    fontLoaded = true;
    return true;
}

void Menu::cleanupFont() {
    for (auto& pair : characters) {
        glDeleteTextures(1, &pair.second.textureID);
    }
    characters.clear();
    
    if (ftFace) { FT_Done_Face(ftFace); ftFace = nullptr; }
    if (ftLibrary) { FT_Done_FreeType(ftLibrary); ftLibrary = nullptr; }
    fontLoaded = false;
}

void Menu::open() {
    state = MenuState::PAUSE;
    selectedIndex = 0;
    pendingSettings = settings;
    syncSlidersFromSettings();
}

void Menu::close() {
    state = MenuState::NONE;
    waitingForKeybind = -1;
}

void Menu::toggle() {
    if (state == MenuState::NONE) open();
    else close();
}

std::string Menu::getKeyName(int keyCode) {
    switch (keyCode) {
        case GLFW_KEY_SPACE: return "SPACE";
        case GLFW_KEY_LEFT_SHIFT: return "L-SHIFT";
        case GLFW_KEY_RIGHT_SHIFT: return "R-SHIFT";
        case GLFW_KEY_LEFT_CONTROL: return "L-CTRL";
        case GLFW_KEY_RIGHT_CONTROL: return "R-CTRL";
        case GLFW_KEY_LEFT_ALT: return "L-ALT";
        case GLFW_KEY_RIGHT_ALT: return "R-ALT";
        case GLFW_KEY_TAB: return "TAB";
        case GLFW_KEY_ENTER: return "ENTER";
        case GLFW_KEY_ESCAPE: return "ESC";
        case GLFW_KEY_UP: return "UP";
        case GLFW_KEY_DOWN: return "DOWN";
        case GLFW_KEY_LEFT: return "LEFT";
        case GLFW_KEY_RIGHT: return "RIGHT";
        default:
            if (keyCode >= GLFW_KEY_A && keyCode <= GLFW_KEY_Z) {
                return std::string(1, 'A' + (keyCode - GLFW_KEY_A));
            }
            if (keyCode >= GLFW_KEY_0 && keyCode <= GLFW_KEY_9) {
                return std::string(1, '0' + (keyCode - GLFW_KEY_0));
            }
            return "???";
    }
}

void Menu::applyDifficulty(Difficulty diff) {
    currentDifficulty = diff;
    switch (diff) {
        case Difficulty::PUSSY:
            settings.speed = 3.0f; settings.gravity = -0.5f; settings.jumpForce = 18.0f;
            break;
        case Difficulty::HUMAN:
            settings.speed = 5.0f; settings.gravity = -0.8f; settings.jumpForce = 15.0f;
            break;
        case Difficulty::GOAT:
            settings.speed = 7.0f; settings.gravity = -1.2f; settings.jumpForce = 13.0f;
            break;
        case Difficulty::I_HATE_MYSELF:
            settings.speed = 10.0f; settings.gravity = -2.0f; settings.jumpForce = 12.0f;
            break;
        case Difficulty::CUSTOM:
            settings.speed = 1.0f + speedSlider * 99.0f;
            settings.gravity = -0.3f - gravitySlider * 9.7f;
            settings.jumpForce = 8.0f + jumpSlider * 92.0f;
            break;
    }
}

// ==================== Drawing Functions ====================

void Menu::drawText(float x, float y, const std::string& text, float scale) {
    if (!fontLoaded) return;
    
    float guiScale = settings.graphics.guiScale;
    scale *= guiScale;
    
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    for (char c : text) {
        if (characters.find(c) == characters.end()) continue;
        Character& ch = characters[c];
        
        float xpos = x + ch.bearingX * scale;
        float ypos = y - (ch.sizeY - ch.bearingY) * scale;
        float w = ch.sizeX * scale;
        float h = ch.sizeY * scale;
        
        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(xpos, ypos + h);
        glTexCoord2f(1, 0); glVertex2f(xpos + w, ypos + h);
        glTexCoord2f(1, 1); glVertex2f(xpos + w, ypos);
        glTexCoord2f(0, 1); glVertex2f(xpos, ypos);
        glEnd();
        
        x += (ch.advance >> 6) * scale;
    }
    
    glDisable(GL_TEXTURE_2D);
}

float Menu::getTextWidth(const std::string& text, float scale) {
    if (!fontLoaded) return 0;
    
    float guiScale = settings.graphics.guiScale;
    scale *= guiScale;
    
    float width = 0;
    for (char c : text) {
        if (characters.find(c) == characters.end()) continue;
        width += (characters[c].advance >> 6) * scale;
    }
    return width;
}

void Menu::drawButton(float x, float y, float width, float height, 
                      const std::string& text, bool selected, bool hovered) {
    if (selected) {
        glColor4f(0.3f, 0.5f, 0.7f, 0.9f);
    } else if (hovered) {
        glColor4f(0.25f, 0.35f, 0.45f, 0.9f);
    } else {
        glColor4f(0.15f, 0.2f, 0.25f, 0.9f);
    }
    
    glBegin(GL_QUADS);
    glVertex2f(x, y); glVertex2f(x + width, y);
    glVertex2f(x + width, y + height); glVertex2f(x, y + height);
    glEnd();
    
    glColor3f(0.5f, 0.6f, 0.7f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y); glVertex2f(x + width, y);
    glVertex2f(x + width, y + height); glVertex2f(x, y + height);
    glEnd();
    
    glColor3f(1.0f, 1.0f, 1.0f);
    float textW = getTextWidth(text, 0.5f);
    drawText(x + (width - textW) / 2, y + height / 2 - 10, text, 0.5f);
}

void Menu::drawCheckbox(float x, float y, float size, const std::string& label, 
                        bool checked, bool selected) {
    // Box
    if (selected) {
        glColor4f(0.3f, 0.5f, 0.7f, 0.9f);
    } else {
        glColor4f(0.15f, 0.2f, 0.25f, 0.9f);
    }
    glBegin(GL_QUADS);
    glVertex2f(x, y); glVertex2f(x + size, y);
    glVertex2f(x + size, y + size); glVertex2f(x, y + size);
    glEnd();
    
    glColor3f(0.5f, 0.6f, 0.7f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y); glVertex2f(x + size, y);
    glVertex2f(x + size, y + size); glVertex2f(x, y + size);
    glEnd();
    
    // Checkmark
    if (checked) {
        glColor3f(0.2f, 0.9f, 0.2f);
        glLineWidth(3.0f);
        glBegin(GL_LINES);
        glVertex2f(x + size * 0.2f, y + size * 0.5f);
        glVertex2f(x + size * 0.4f, y + size * 0.2f);
        glVertex2f(x + size * 0.4f, y + size * 0.2f);
        glVertex2f(x + size * 0.8f, y + size * 0.8f);
        glEnd();
    }
    
    // Label
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(x + size + 15, y + size / 2 - 8, label, 0.4f);
}

void Menu::drawSlider(float x, float y, float width, float height,
                      const std::string& label, float value, bool selected) {
    // Label
    glColor3f(0.8f, 0.8f, 0.8f);
    drawText(x, y + height + 5, label, 0.35f);
    
    // Track
    glColor4f(0.1f, 0.12f, 0.15f, 0.9f);
    glBegin(GL_QUADS);
    glVertex2f(x, y); glVertex2f(x + width, y);
    glVertex2f(x + width, y + height); glVertex2f(x, y + height);
    glEnd();
    
    // Fill
    if (selected) {
        glColor4f(0.3f, 0.6f, 0.8f, 0.9f);
    } else {
        glColor4f(0.2f, 0.4f, 0.6f, 0.9f);
    }
    glBegin(GL_QUADS);
    glVertex2f(x, y); glVertex2f(x + width * value, y);
    glVertex2f(x + width * value, y + height); glVertex2f(x, y + height);
    glEnd();
    
    // Border
    glColor3f(0.5f, 0.6f, 0.7f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y); glVertex2f(x + width, y);
    glVertex2f(x + width, y + height); glVertex2f(x, y + height);
    glEnd();
    
    // Handle
    float handleX = x + width * value;
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(handleX - 5, y - 3);
    glVertex2f(handleX + 5, y - 3);
    glVertex2f(handleX + 5, y + height + 3);
    glVertex2f(handleX - 5, y + height + 3);
    glEnd();
}

void Menu::drawKeybind(float x, float y, float width, float height,
                       const std::string& label, int keyCode, bool selected, bool waiting) {
    // Label
    glColor3f(0.8f, 0.8f, 0.8f);
    drawText(x, y + height / 2 - 8, label, 0.4f);
    
    // Key box
    float boxX = x + width - 100;
    float boxW = 100;
    
    if (waiting) {
        glColor4f(0.6f, 0.3f, 0.3f, 0.9f);
    } else if (selected) {
        glColor4f(0.3f, 0.5f, 0.7f, 0.9f);
    } else {
        glColor4f(0.15f, 0.2f, 0.25f, 0.9f);
    }
    
    glBegin(GL_QUADS);
    glVertex2f(boxX, y); glVertex2f(boxX + boxW, y);
    glVertex2f(boxX + boxW, y + height); glVertex2f(boxX, y + height);
    glEnd();
    
    glColor3f(0.5f, 0.6f, 0.7f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(boxX, y); glVertex2f(boxX + boxW, y);
    glVertex2f(boxX + boxW, y + height); glVertex2f(boxX, y + height);
    glEnd();
    
    // Key name
    glColor3f(1.0f, 1.0f, 1.0f);
    std::string keyName = waiting ? "..." : getKeyName(keyCode);
    float textW = getTextWidth(keyName, 0.4f);
    drawText(boxX + (boxW - textW) / 2, y + height / 2 - 8, keyName, 0.4f);
}

// ==================== Render ====================

void Menu::render(int windowWidth, int windowHeight) {
    screenWidth = windowWidth;
    screenHeight = windowHeight;
    
    // Decrease feedback timers (approx 60fps)
    if (resetFeedbackTimer > 0) {
        resetFeedbackTimer -= 0.016f;
    }
    if (applyFeedbackTimer > 0) {
        applyFeedbackTimer -= 0.016f;
    }
    
    // Always render the popup notification even if menu is closed
    float activeTimer = resetFeedbackTimer > 0 ? resetFeedbackTimer : applyFeedbackTimer;
    if (activeTimer > 0) {
        renderResetPopup(windowWidth, windowHeight);
    }
    
    if (state == MenuState::NONE) return;
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, 0, windowHeight, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Overlay
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f(windowWidth, 0);
    glVertex2f(windowWidth, windowHeight); glVertex2f(0, windowHeight);
    glEnd();
    
    float panelWidth = 450.0f;
    float panelHeight = 500.0f;
    
    // Increase panel height for menus with more content
    if (state == MenuState::SETTINGS) {
        panelHeight = 420.0f;  // 5 buttons need more space
    } else if (state == MenuState::CONTROLS_SETTINGS) {
        panelHeight = 580.0f;
    } else if (state == MenuState::GRAPHICS_SETTINGS) {
        panelHeight = 480.0f;
    }
    
    float panelX = (windowWidth - panelWidth) / 2.0f;
    float panelY = (windowHeight - panelHeight) / 2.0f;
    
    // Panel
    glColor4f(0.1f, 0.12f, 0.15f, 0.95f);
    glBegin(GL_QUADS);
    glVertex2f(panelX, panelY); glVertex2f(panelX + panelWidth, panelY);
    glVertex2f(panelX + panelWidth, panelY + panelHeight); glVertex2f(panelX, panelY + panelHeight);
    glEnd();
    
    glColor3f(0.3f, 0.4f, 0.5f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(panelX, panelY); glVertex2f(panelX + panelWidth, panelY);
    glVertex2f(panelX + panelWidth, panelY + panelHeight); glVertex2f(panelX, panelY + panelHeight);
    glEnd();
    
    float buttonWidth = 350.0f;
    float buttonHeight = 45.0f;
    float buttonX = panelX + (panelWidth - buttonWidth) / 2.0f;
    float buttonSpacing = 12.0f;
    float itemHeight = 40.0f;
    float itemSpacing = 50.0f;
    
    if (state == MenuState::PAUSE) {
        glColor3f(1.0f, 1.0f, 1.0f);
        float titleW = getTextWidth("PAUSED", 0.8f);
        drawText(panelX + (panelWidth - titleW) / 2.0f, panelY + panelHeight - 60, "PAUSED", 0.8f);
        
        float startY = panelY + panelHeight - 140;
        for (size_t i = 0; i < pauseButtons.size(); i++) {
            float btnY = startY - i * (buttonHeight + buttonSpacing);
            drawButton(buttonX, btnY, buttonWidth, buttonHeight, pauseButtons[i], (int)i == selectedIndex);
        }
    }
    else if (state == MenuState::SETTINGS) {
        glColor3f(1.0f, 1.0f, 1.0f);
        float titleW = getTextWidth("SETTINGS", 0.8f);
        drawText(panelX + (panelWidth - titleW) / 2.0f, panelY + panelHeight - 60, "SETTINGS", 0.8f);
        
        float startY = panelY + panelHeight - 130;
        for (size_t i = 0; i < settingsButtons.size(); i++) {
            float btnY = startY - i * (buttonHeight + buttonSpacing);
            drawButton(buttonX, btnY, buttonWidth, buttonHeight, settingsButtons[i], (int)i == settingsSelectedIndex);
        }
    }
    else if (state == MenuState::CONTROLS_SETTINGS) {
        glColor3f(1.0f, 1.0f, 1.0f);
        float titleW = getTextWidth("CONTROLS", 0.8f);
        drawText(panelX + (panelWidth - titleW) / 2.0f, panelY + panelHeight - 60, "CONTROLS", 0.8f);
        
        float startY = panelY + panelHeight - 110;
        float sliderWidth = 300.0f;
        float sliderX = buttonX;
        
        // Sensitivity slider
        drawSlider(sliderX, startY, sliderWidth, 20, "Sensitivity", sensitivitySlider, controlsSelectedIndex == 0);
        char sensStr[32];
        snprintf(sensStr, sizeof(sensStr), "%.4f", 0.001f + sensitivitySlider * 0.009f);
        glColor3f(0.7f, 0.7f, 0.7f);
        drawText(sliderX + sliderWidth + 15, startY + 5, sensStr, 0.35f);
        
        // Toggle Crouch checkbox
        drawCheckbox(sliderX, startY - 55, 25, "Toggle Crouch", pendingSettings.controls.toggleCrouch, controlsSelectedIndex == 1);
        
        // Keybinds
        float keybindY = startY - 110;
        drawKeybind(sliderX, keybindY, buttonWidth, 35, "Forward", pendingSettings.controls.keyForward, controlsSelectedIndex == 2, waitingForKeybind == 2);
        drawKeybind(sliderX, keybindY - 45, buttonWidth, 35, "Backward", pendingSettings.controls.keyBackward, controlsSelectedIndex == 3, waitingForKeybind == 3);
        drawKeybind(sliderX, keybindY - 90, buttonWidth, 35, "Left", pendingSettings.controls.keyLeft, controlsSelectedIndex == 4, waitingForKeybind == 4);
        drawKeybind(sliderX, keybindY - 135, buttonWidth, 35, "Right", pendingSettings.controls.keyRight, controlsSelectedIndex == 5, waitingForKeybind == 5);
        drawKeybind(sliderX, keybindY - 180, buttonWidth, 35, "Jump", pendingSettings.controls.keyJump, controlsSelectedIndex == 6, waitingForKeybind == 6);
        drawKeybind(sliderX, keybindY - 225, buttonWidth, 35, "Crouch", pendingSettings.controls.keyCrouch, controlsSelectedIndex == 7, waitingForKeybind == 7);
        
        // Back and Apply buttons at bottom
        float btnY = panelY + 25;
        float halfWidth = (buttonWidth - 10) / 2;
        drawButton(buttonX, btnY, halfWidth, buttonHeight, "Back", controlsSelectedIndex == 9);
        drawButton(buttonX + halfWidth + 10, btnY, halfWidth, buttonHeight, "Apply", controlsSelectedIndex == 8);
    }
    else if (state == MenuState::GRAPHICS_SETTINGS) {
        glColor3f(1.0f, 1.0f, 1.0f);
        float titleW = getTextWidth("GRAPHICS", 0.8f);
        drawText(panelX + (panelWidth - titleW) / 2.0f, panelY + panelHeight - 60, "GRAPHICS", 0.8f);
        
        float startY = panelY + panelHeight - 110;
        float sliderWidth = 280.0f;
        float sliderX = buttonX;
        char valueStr[32];
        
        // VSync checkbox
        drawCheckbox(sliderX, startY, 25, "VSync", pendingSettings.graphics.vsync, graphicsSelectedIndex == 0);
        
        // Fullscreen checkbox
        drawCheckbox(sliderX + 180, startY, 25, "Fullscreen", pendingSettings.graphics.fullscreen, graphicsSelectedIndex == 1);
        
        // Render Distance slider
        drawSlider(sliderX, startY - 60, sliderWidth, 20, "Render Distance", renderDistanceSlider, graphicsSelectedIndex == 2);
        snprintf(valueStr, sizeof(valueStr), "%.0f", 500.0f + renderDistanceSlider * 9500.0f);
        glColor3f(0.7f, 0.7f, 0.7f);
        drawText(sliderX + sliderWidth + 15, startY - 55, valueStr, 0.35f);
        
        // FOV slider
        drawSlider(sliderX, startY - 120, sliderWidth, 20, "Field of View", fovSlider, graphicsSelectedIndex == 3);
        snprintf(valueStr, sizeof(valueStr), "%.0f", 30.0f + fovSlider * 120.0f);
        glColor3f(0.7f, 0.7f, 0.7f);
        drawText(sliderX + sliderWidth + 15, startY - 115, valueStr, 0.35f);
        
        // Max Framerate slider
        drawSlider(sliderX, startY - 180, sliderWidth, 20, "Max Framerate", framerateSlider, graphicsSelectedIndex == 4);
        if (framerateSlider >= 0.99f) {
            snprintf(valueStr, sizeof(valueStr), "Unlimited");
        } else {
            snprintf(valueStr, sizeof(valueStr), "%d", 30 + (int)(framerateSlider * 210.0f));
        }
        glColor3f(0.7f, 0.7f, 0.7f);
        drawText(sliderX + sliderWidth + 15, startY - 175, valueStr, 0.35f);
        
        // GUI Scale slider
        drawSlider(sliderX, startY - 240, sliderWidth, 20, "GUI Scale", guiScaleSlider, graphicsSelectedIndex == 5);
        snprintf(valueStr, sizeof(valueStr), "%.1fx", 0.5f + guiScaleSlider * 1.5f);
        glColor3f(0.7f, 0.7f, 0.7f);
        drawText(sliderX + sliderWidth + 15, startY - 235, valueStr, 0.35f);
        
        // Back and Apply buttons at bottom
        float btnY = panelY + 25;
        float halfWidth = (buttonWidth - 10) / 2;
        drawButton(buttonX, btnY, halfWidth, buttonHeight, "Back", graphicsSelectedIndex == 7);
        drawButton(buttonX + halfWidth + 10, btnY, halfWidth, buttonHeight, "Apply", graphicsSelectedIndex == 6);
    }
    else if (state == MenuState::DIFFICULTY_SETTINGS) {
        glColor3f(1.0f, 1.0f, 1.0f);
        float titleW = getTextWidth("DIFFICULTY", 0.8f);
        drawText(panelX + (panelWidth - titleW) / 2.0f, panelY + panelHeight - 60, "DIFFICULTY", 0.8f);
        
        float startY = panelY + panelHeight - 120;
        for (size_t i = 0; i < difficultyOptions.size(); i++) {
            float btnY = startY - i * (buttonHeight + buttonSpacing * 0.5f);
            std::string label = difficultyOptions[i];
            if ((int)i == (int)currentDifficulty) label = "> " + label + " <";
            drawButton(buttonX, btnY, buttonWidth, buttonHeight, label, (int)i == settingsSelectedIndex);
        }
        
        float backY = startY - difficultyOptions.size() * (buttonHeight + buttonSpacing * 0.5f) - buttonSpacing;
        drawButton(buttonX, backY, buttonWidth, buttonHeight, "Back", (int)difficultyOptions.size() == settingsSelectedIndex);
    }
    else if (state == MenuState::CUSTOM_SETTINGS) {
        glColor3f(1.0f, 1.0f, 1.0f);
        float titleW = getTextWidth("CUSTOM", 0.8f);
        drawText(panelX + (panelWidth - titleW) / 2.0f, panelY + panelHeight - 60, "CUSTOM", 0.8f);
        
        float sliderWidth = 300.0f;
        float sliderX = buttonX;
        float startY = panelY + panelHeight - 130;
        char valueStr[32];
        
        drawSlider(sliderX, startY, sliderWidth, 20, "Speed", speedSlider, customSelectedIndex == 0);
        snprintf(valueStr, sizeof(valueStr), "%.1f", 1.0f + speedSlider * 99.0f);
        glColor3f(0.7f, 0.7f, 0.7f);
        drawText(sliderX + sliderWidth + 15, startY + 5, valueStr, 0.35f);
        
        drawSlider(sliderX, startY - 70, sliderWidth, 20, "Gravity", gravitySlider, customSelectedIndex == 1);
        snprintf(valueStr, sizeof(valueStr), "%.1f", -0.3f - gravitySlider * 9.7f);
        glColor3f(0.7f, 0.7f, 0.7f);
        drawText(sliderX + sliderWidth + 15, startY - 65, valueStr, 0.35f);
        
        drawSlider(sliderX, startY - 140, sliderWidth, 20, "Jump Force", jumpSlider, customSelectedIndex == 2);
        snprintf(valueStr, sizeof(valueStr), "%.1f", 8.0f + jumpSlider * 92.0f);
        glColor3f(0.7f, 0.7f, 0.7f);
        drawText(sliderX + sliderWidth + 15, startY - 135, valueStr, 0.35f);
        
        float btnY = startY - 220;
        drawButton(buttonX, btnY, buttonWidth, buttonHeight, "Apply", customSelectedIndex == 3);
        drawButton(buttonX, btnY - buttonHeight - buttonSpacing, buttonWidth, buttonHeight, "Back", customSelectedIndex == 4);
    }
    else if (state == MenuState::KEYBIND_WAITING) {
        glColor3f(1.0f, 1.0f, 1.0f);
        float titleW = getTextWidth("PRESS A KEY", 0.8f);
        drawText(panelX + (panelWidth - titleW) / 2.0f, panelY + panelHeight / 2, "PRESS A KEY", 0.8f);
        
        glColor3f(0.6f, 0.6f, 0.6f);
        float subW = getTextWidth("(ESC to cancel)", 0.4f);
        drawText(panelX + (panelWidth - subW) / 2.0f, panelY + panelHeight / 2 - 50, "(ESC to cancel)", 0.4f);
    }
    
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void Menu::renderResetPopup(int windowWidth, int windowHeight) {
    // Use whichever timer is active
    float activeTimer = resetFeedbackTimer > 0 ? resetFeedbackTimer : applyFeedbackTimer;
    if (activeTimer <= 0) return;
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, 0, windowHeight, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    float popupWidth = 350.0f;
    float popupHeight = 100.0f;
    float popupX = windowWidth - popupWidth - 20.0f;  // Top right with padding
    float popupY = windowHeight - popupHeight - 20.0f;  // Top right with padding
    
    // Fade out effect in last second
    float alpha = activeTimer > 1.0f ? 1.0f : activeTimer;
    
    // Colors based on type (green for reset, blue for apply)
    float bgR, bgG, bgB, borderR, borderG, borderB, iconR, iconG, iconB;
    if (popupIsGreen) {
        // Green theme for reset
        bgR = 0.1f; bgG = 0.3f; bgB = 0.1f;
        borderR = 0.2f; borderG = 0.9f; borderB = 0.3f;
        iconR = 0.2f; iconG = 1.0f; iconB = 0.4f;
    } else {
        // Blue theme for apply
        bgR = 0.1f; bgG = 0.15f; bgB = 0.3f;
        borderR = 0.3f; borderG = 0.6f; borderB = 0.9f;
        iconR = 0.4f; iconG = 0.7f; iconB = 1.0f;
    }
    
    // Popup background
    glColor4f(bgR, bgG, bgB, 0.95f * alpha);
    glBegin(GL_QUADS);
    glVertex2f(popupX, popupY);
    glVertex2f(popupX + popupWidth, popupY);
    glVertex2f(popupX + popupWidth, popupY + popupHeight);
    glVertex2f(popupX, popupY + popupHeight);
    glEnd();
    
    // Popup border (pulsing)
    float pulse = 0.7f + 0.3f * sinf(activeTimer * 8.0f);
    glColor4f(borderR * pulse, borderG * pulse, borderB * pulse, alpha);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(popupX, popupY);
    glVertex2f(popupX + popupWidth, popupY);
    glVertex2f(popupX + popupWidth, popupY + popupHeight);
    glVertex2f(popupX, popupY + popupHeight);
    glEnd();
    
    // Checkmark icon
    glColor4f(iconR, iconG, iconB, alpha);
    glLineWidth(4.0f);
    float checkX = popupX + 30;
    float checkY = popupY + popupHeight / 2;
    glBegin(GL_LINE_STRIP);
    glVertex2f(checkX, checkY);
    glVertex2f(checkX + 10, checkY - 15);
    glVertex2f(checkX + 30, checkY + 15);
    glEnd();
    
    // Text (use dynamic message)
    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    float textW = getTextWidth(popupMessage, 0.55f);
    drawText(popupX + (popupWidth - textW) / 2.0f + 15, popupY + popupHeight / 2 - 12, popupMessage, 0.55f);
    
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void Menu::renderHUD(int windowWidth, int windowHeight, float timer, int deaths,
                     bool timerRunning, bool timerFinished) {
    if (!fontLoaded) return;
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, 0, windowHeight, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    float scale = 0.5f;
    float padding = 20.0f;
    
    int minutes = (int)(timer / 60.0f);
    int seconds = (int)timer % 60;
    int milliseconds = (int)((timer - (int)timer) * 100);
    
    char timerStr[32];
    snprintf(timerStr, sizeof(timerStr), "%02d:%02d.%02d", minutes, seconds, milliseconds);
    
    if (timerFinished) glColor3f(0.2f, 1.0f, 0.2f);
    else if (timerRunning) glColor3f(1.0f, 1.0f, 1.0f);
    else glColor3f(0.6f, 0.6f, 0.6f);
    
    drawText(padding, windowHeight - padding - 30, timerStr, scale);
    
    if (!timerRunning && !timerFinished) {
        glColor3f(0.5f, 0.5f, 0.5f);
        drawText(padding, windowHeight - padding - 60, "[T] Start  [R] Reset", 0.35f);
    } else if (timerRunning) {
        glColor3f(0.5f, 0.5f, 0.5f);
        drawText(padding, windowHeight - padding - 60, "[T] Pause  [R] Reset", 0.35f);
    } else if (timerFinished) {
        glColor3f(0.2f, 1.0f, 0.2f);
        drawText(padding, windowHeight - padding - 60, "GOAL REACHED!", 0.35f);
    }
    
    char deathStr[32];
    snprintf(deathStr, sizeof(deathStr), "Deaths: %d", deaths);
    glColor3f(1.0f, 0.4f, 0.4f);
    float deathWidth = getTextWidth(deathStr, scale);
    drawText(windowWidth - padding - deathWidth, windowHeight - padding - 30, deathStr, scale);
    
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// ==================== Input Handling ====================

void Menu::handleKey(int key, int action) {
    if (action != GLFW_PRESS) return;
    
    // Handle keybind waiting
    if (state == MenuState::KEYBIND_WAITING || waitingForKeybind >= 0) {
        if (key == GLFW_KEY_ESCAPE) {
            waitingForKeybind = -1;
            state = MenuState::CONTROLS_SETTINGS;
            return;
        }
        
        // Set the keybind
        switch (waitingForKeybind) {
            case 2: pendingSettings.controls.keyForward = key; break;
            case 3: pendingSettings.controls.keyBackward = key; break;
            case 4: pendingSettings.controls.keyLeft = key; break;
            case 5: pendingSettings.controls.keyRight = key; break;
            case 6: pendingSettings.controls.keyJump = key; break;
            case 7: pendingSettings.controls.keyCrouch = key; break;
        }
        waitingForKeybind = -1;
        state = MenuState::CONTROLS_SETTINGS;
        return;
    }
    
    if (state == MenuState::PAUSE) {
        if (key == GLFW_KEY_UP || key == GLFW_KEY_W) {
            selectedIndex = (selectedIndex - 1 + pauseButtons.size()) % pauseButtons.size();
        }
        else if (key == GLFW_KEY_DOWN || key == GLFW_KEY_S) {
            selectedIndex = (selectedIndex + 1) % pauseButtons.size();
        }
        else if (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE) {
            if (selectedIndex == 0) close();
            else if (selectedIndex == 1) { shouldRestart = true; close(); }
            else if (selectedIndex == 2) { state = MenuState::SETTINGS; settingsSelectedIndex = 0; }
            else if (selectedIndex == 3) shouldQuit = true;
        }
    }
    else if (state == MenuState::SETTINGS) {
        if (key == GLFW_KEY_UP || key == GLFW_KEY_W) {
            settingsSelectedIndex = (settingsSelectedIndex - 1 + settingsButtons.size()) % settingsButtons.size();
        }
        else if (key == GLFW_KEY_DOWN || key == GLFW_KEY_S) {
            settingsSelectedIndex = (settingsSelectedIndex + 1) % settingsButtons.size();
        }
        else if (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE) {
            if (settingsSelectedIndex == 0) { state = MenuState::CONTROLS_SETTINGS; controlsSelectedIndex = 0; }
            else if (settingsSelectedIndex == 1) { state = MenuState::GRAPHICS_SETTINGS; graphicsSelectedIndex = 0; }
            else if (settingsSelectedIndex == 2) { state = MenuState::DIFFICULTY_SETTINGS; settingsSelectedIndex = (int)currentDifficulty; }
            else if (settingsSelectedIndex == 3) { resetToDefaults(); }
            else if (settingsSelectedIndex == 4) { state = MenuState::PAUSE; }
        }
        else if (key == GLFW_KEY_ESCAPE) { state = MenuState::PAUSE; }
    }
    else if (state == MenuState::CONTROLS_SETTINGS) {
        if (key == GLFW_KEY_UP || key == GLFW_KEY_W) {
            controlsSelectedIndex = (controlsSelectedIndex - 1 + 10) % 10;
        }
        else if (key == GLFW_KEY_DOWN || key == GLFW_KEY_S) {
            controlsSelectedIndex = (controlsSelectedIndex + 1) % 10;
        }
        else if (key == GLFW_KEY_LEFT || key == GLFW_KEY_A) {
            if (controlsSelectedIndex == 0) {
                sensitivitySlider = std::max(0.0f, sensitivitySlider - 0.05f);
                pendingSettings.controls.sensitivity = 0.001f + sensitivitySlider * 0.009f;
            }
        }
        else if (key == GLFW_KEY_RIGHT || key == GLFW_KEY_D) {
            if (controlsSelectedIndex == 0) {
                sensitivitySlider = std::min(1.0f, sensitivitySlider + 0.05f);
                pendingSettings.controls.sensitivity = 0.001f + sensitivitySlider * 0.009f;
            }
        }
        else if (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE) {
            if (controlsSelectedIndex == 1) {
                pendingSettings.controls.toggleCrouch = !pendingSettings.controls.toggleCrouch;
            }
            else if (controlsSelectedIndex >= 2 && controlsSelectedIndex <= 7) {
                waitingForKeybind = controlsSelectedIndex;
                state = MenuState::KEYBIND_WAITING;
            }
            else if (controlsSelectedIndex == 8) {
                applyPendingSettings();
                close();  // Close entire menu
            }
            else if (controlsSelectedIndex == 9) {
                state = MenuState::SETTINGS;
                settingsSelectedIndex = 0;
            }
        }
        else if (key == GLFW_KEY_ESCAPE) { state = MenuState::SETTINGS; settingsSelectedIndex = 0; }
    }
    else if (state == MenuState::GRAPHICS_SETTINGS) {
        if (key == GLFW_KEY_UP || key == GLFW_KEY_W) {
            graphicsSelectedIndex = (graphicsSelectedIndex - 1 + 8) % 8;
        }
        else if (key == GLFW_KEY_DOWN || key == GLFW_KEY_S) {
            graphicsSelectedIndex = (graphicsSelectedIndex + 1) % 8;
        }
        else if (key == GLFW_KEY_LEFT || key == GLFW_KEY_A) {
            if (graphicsSelectedIndex == 2) {
                renderDistanceSlider = std::max(0.0f, renderDistanceSlider - 0.05f);
                pendingSettings.graphics.renderDistance = 500.0f + renderDistanceSlider * 9500.0f;
            }
            else if (graphicsSelectedIndex == 3) {
                fovSlider = std::max(0.0f, fovSlider - 0.05f);
                pendingSettings.graphics.fov = 30.0f + fovSlider * 120.0f;
            }
            else if (graphicsSelectedIndex == 4) {
                framerateSlider = std::max(0.0f, framerateSlider - 0.05f);
                if (framerateSlider >= 0.99f) pendingSettings.graphics.maxFramerate = 0;
                else pendingSettings.graphics.maxFramerate = 30 + (int)(framerateSlider * 210.0f);
            }
            else if (graphicsSelectedIndex == 5) {
                guiScaleSlider = std::max(0.0f, guiScaleSlider - 0.1f);
                pendingSettings.graphics.guiScale = 0.5f + guiScaleSlider * 1.5f;
            }
        }
        else if (key == GLFW_KEY_RIGHT || key == GLFW_KEY_D) {
            if (graphicsSelectedIndex == 2) {
                renderDistanceSlider = std::min(1.0f, renderDistanceSlider + 0.05f);
                pendingSettings.graphics.renderDistance = 500.0f + renderDistanceSlider * 9500.0f;
            }
            else if (graphicsSelectedIndex == 3) {
                fovSlider = std::min(1.0f, fovSlider + 0.05f);
                pendingSettings.graphics.fov = 30.0f + fovSlider * 120.0f;
            }
            else if (graphicsSelectedIndex == 4) {
                framerateSlider = std::min(1.0f, framerateSlider + 0.05f);
                if (framerateSlider >= 0.99f) pendingSettings.graphics.maxFramerate = 0;
                else pendingSettings.graphics.maxFramerate = 30 + (int)(framerateSlider * 210.0f);
            }
            else if (graphicsSelectedIndex == 5) {
                guiScaleSlider = std::min(1.0f, guiScaleSlider + 0.1f);
                pendingSettings.graphics.guiScale = 0.5f + guiScaleSlider * 1.5f;
            }
        }
        else if (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE) {
            if (graphicsSelectedIndex == 0) pendingSettings.graphics.vsync = !pendingSettings.graphics.vsync;
            else if (graphicsSelectedIndex == 1) pendingSettings.graphics.fullscreen = !pendingSettings.graphics.fullscreen;
            else if (graphicsSelectedIndex == 6) { applyPendingSettings(); close(); }  // Close entire menu
            else if (graphicsSelectedIndex == 7) { state = MenuState::SETTINGS; settingsSelectedIndex = 1; }
        }
        else if (key == GLFW_KEY_ESCAPE) { state = MenuState::SETTINGS; settingsSelectedIndex = 1; }
    }
    else if (state == MenuState::DIFFICULTY_SETTINGS) {
        int maxIdx = difficultyOptions.size();
        if (key == GLFW_KEY_UP || key == GLFW_KEY_W) {
            settingsSelectedIndex = (settingsSelectedIndex - 1 + maxIdx + 1) % (maxIdx + 1);
        }
        else if (key == GLFW_KEY_DOWN || key == GLFW_KEY_S) {
            settingsSelectedIndex = (settingsSelectedIndex + 1) % (maxIdx + 1);
        }
        else if (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE) {
            if (settingsSelectedIndex == maxIdx) { state = MenuState::SETTINGS; settingsSelectedIndex = 2; }
            else if (settingsSelectedIndex == (int)Difficulty::CUSTOM) { state = MenuState::CUSTOM_SETTINGS; customSelectedIndex = 0; }
            else { applyDifficulty((Difficulty)settingsSelectedIndex); state = MenuState::SETTINGS; settingsSelectedIndex = 2; }
        }
        else if (key == GLFW_KEY_ESCAPE) { state = MenuState::SETTINGS; settingsSelectedIndex = 2; }
    }
    else if (state == MenuState::CUSTOM_SETTINGS) {
        if (key == GLFW_KEY_UP || key == GLFW_KEY_W) {
            customSelectedIndex = (customSelectedIndex - 1 + 5) % 5;
        }
        else if (key == GLFW_KEY_DOWN || key == GLFW_KEY_S) {
            customSelectedIndex = (customSelectedIndex + 1) % 5;
        }
        else if (key == GLFW_KEY_LEFT || key == GLFW_KEY_A) {
            if (customSelectedIndex == 0) speedSlider = std::max(0.0f, speedSlider - 0.05f);
            else if (customSelectedIndex == 1) gravitySlider = std::max(0.0f, gravitySlider - 0.05f);
            else if (customSelectedIndex == 2) jumpSlider = std::max(0.0f, jumpSlider - 0.05f);
        }
        else if (key == GLFW_KEY_RIGHT || key == GLFW_KEY_D) {
            if (customSelectedIndex == 0) speedSlider = std::min(1.0f, speedSlider + 0.05f);
            else if (customSelectedIndex == 1) gravitySlider = std::min(1.0f, gravitySlider + 0.05f);
            else if (customSelectedIndex == 2) jumpSlider = std::min(1.0f, jumpSlider + 0.05f);
        }
        else if (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE) {
            if (customSelectedIndex == 3) { applyDifficulty(Difficulty::CUSTOM); state = MenuState::DIFFICULTY_SETTINGS; }
            else if (customSelectedIndex == 4) { state = MenuState::DIFFICULTY_SETTINGS; }
        }
        else if (key == GLFW_KEY_ESCAPE) { state = MenuState::DIFFICULTY_SETTINGS; }
    }
}

void Menu::handleMouseClick(double x, double y, int button, int action) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    y = screenHeight - y;  // Flip Y coordinate
    
    if (action == GLFW_RELEASE) {
        draggingSlider = -1;
        return;
    }
    
    // Calculate panel dimensions (same as in render)
    float panelWidth = 450.0f;
    float panelHeight = 500.0f;
    
    // Match dynamic panel height from render()
    if (state == MenuState::SETTINGS) {
        panelHeight = 420.0f;
    } else if (state == MenuState::CONTROLS_SETTINGS) {
        panelHeight = 580.0f;
    } else if (state == MenuState::GRAPHICS_SETTINGS) {
        panelHeight = 480.0f;
    }
    
    float panelX = (screenWidth - panelWidth) / 2.0f;
    float panelY = (screenHeight - panelHeight) / 2.0f;
    float buttonWidth = 350.0f;
    float buttonHeight = 45.0f;
    float buttonX = panelX + (panelWidth - buttonWidth) / 2.0f;
    float buttonSpacing = 12.0f;
    float sliderWidth = 300.0f;
    float sliderX = buttonX;
    
    // Helper lambda to check if point is in rect
    auto inRect = [](double px, double py, float rx, float ry, float rw, float rh) {
        return px >= rx && px <= rx + rw && py >= ry && py <= ry + rh;
    };
    
    if (state == MenuState::PAUSE) {
        float startY = panelY + panelHeight - 140;
        for (size_t i = 0; i < pauseButtons.size(); i++) {
            float btnY = startY - i * (buttonHeight + buttonSpacing);
            if (inRect(x, y, buttonX, btnY, buttonWidth, buttonHeight)) {
                if (i == 0) close();
                else if (i == 1) { shouldRestart = true; close(); }
                else if (i == 2) { state = MenuState::SETTINGS; settingsSelectedIndex = 0; }
                else if (i == 3) shouldQuit = true;
                return;
            }
        }
    }
    else if (state == MenuState::SETTINGS) {
        float startY = panelY + panelHeight - 130;
        for (size_t i = 0; i < settingsButtons.size(); i++) {
            float btnY = startY - i * (buttonHeight + buttonSpacing);
            if (inRect(x, y, buttonX, btnY, buttonWidth, buttonHeight)) {
                if (i == 0) { state = MenuState::CONTROLS_SETTINGS; controlsSelectedIndex = 0; }
                else if (i == 1) { state = MenuState::GRAPHICS_SETTINGS; graphicsSelectedIndex = 0; }
                else if (i == 2) { state = MenuState::DIFFICULTY_SETTINGS; settingsSelectedIndex = (int)currentDifficulty; }
                else if (i == 3) { resetToDefaults(); }
                else if (i == 4) { state = MenuState::PAUSE; }
                return;
            }
        }
    }
    else if (state == MenuState::CONTROLS_SETTINGS) {
        float startY = panelY + panelHeight - 110;
        
        // Sensitivity slider
        if (inRect(x, y, sliderX, startY, sliderWidth, 20)) {
            draggingSlider = 0;
            sensitivitySlider = std::max(0.0f, std::min(1.0f, (float)(x - sliderX) / sliderWidth));
            pendingSettings.controls.sensitivity = 0.001f + sensitivitySlider * 0.009f;
            return;
        }
        
        // Toggle crouch checkbox
        if (inRect(x, y, sliderX, startY - 55, 25, 25)) {
            pendingSettings.controls.toggleCrouch = !pendingSettings.controls.toggleCrouch;
            return;
        }
        
        // Keybinds
        float keybindY = startY - 110;
        float keybindBoxX = buttonX + buttonWidth - 100;
        for (int i = 0; i < 6; i++) {
            if (inRect(x, y, keybindBoxX, keybindY - i * 45, 100, 35)) {
                waitingForKeybind = 2 + i;
                state = MenuState::KEYBIND_WAITING;
                return;
            }
        }
        
        // Back and Apply buttons
        float btnY = panelY + 25;
        float halfWidth = (buttonWidth - 10) / 2;
        if (inRect(x, y, buttonX, btnY, halfWidth, buttonHeight)) {
            state = MenuState::SETTINGS;
            settingsSelectedIndex = 0;
            return;
        }
        if (inRect(x, y, buttonX + halfWidth + 10, btnY, halfWidth, buttonHeight)) {
            applyPendingSettings();
            close();
            return;
        }
    }
    else if (state == MenuState::GRAPHICS_SETTINGS) {
        float startY = panelY + panelHeight - 110;
        float gfxSliderWidth = 280.0f;
        
        // VSync checkbox
        if (inRect(x, y, sliderX, startY, 25, 25)) {
            pendingSettings.graphics.vsync = !pendingSettings.graphics.vsync;
            return;
        }
        
        // Fullscreen checkbox
        if (inRect(x, y, sliderX + 180, startY, 25, 25)) {
            pendingSettings.graphics.fullscreen = !pendingSettings.graphics.fullscreen;
            return;
        }
        
        // Render Distance slider
        if (inRect(x, y, sliderX, startY - 60, gfxSliderWidth, 20)) {
            draggingSlider = 2;
            renderDistanceSlider = std::max(0.0f, std::min(1.0f, (float)(x - sliderX) / gfxSliderWidth));
            pendingSettings.graphics.renderDistance = 500.0f + renderDistanceSlider * 9500.0f;
            return;
        }
        
        // FOV slider
        if (inRect(x, y, sliderX, startY - 120, gfxSliderWidth, 20)) {
            draggingSlider = 3;
            fovSlider = std::max(0.0f, std::min(1.0f, (float)(x - sliderX) / gfxSliderWidth));
            pendingSettings.graphics.fov = 30.0f + fovSlider * 120.0f;
            return;
        }
        
        // Framerate slider
        if (inRect(x, y, sliderX, startY - 180, gfxSliderWidth, 20)) {
            draggingSlider = 4;
            framerateSlider = std::max(0.0f, std::min(1.0f, (float)(x - sliderX) / gfxSliderWidth));
            if (framerateSlider >= 0.99f) pendingSettings.graphics.maxFramerate = 0;
            else pendingSettings.graphics.maxFramerate = 30 + (int)(framerateSlider * 210.0f);
            return;
        }
        
        // GUI Scale slider
        if (inRect(x, y, sliderX, startY - 240, gfxSliderWidth, 20)) {
            draggingSlider = 5;
            guiScaleSlider = std::max(0.0f, std::min(1.0f, (float)(x - sliderX) / gfxSliderWidth));
            pendingSettings.graphics.guiScale = 0.5f + guiScaleSlider * 1.5f;
            return;
        }
        
        // Back and Apply buttons
        float btnY = panelY + 25;
        float halfWidth = (buttonWidth - 10) / 2;
        if (inRect(x, y, buttonX, btnY, halfWidth, buttonHeight)) {
            state = MenuState::SETTINGS;
            settingsSelectedIndex = 1;
            return;
        }
        if (inRect(x, y, buttonX + halfWidth + 10, btnY, halfWidth, buttonHeight)) {
            applyPendingSettings();
            close();
            return;
        }
    }
    else if (state == MenuState::DIFFICULTY_SETTINGS) {
        float startY = panelY + panelHeight - 120;
        for (size_t i = 0; i < difficultyOptions.size(); i++) {
            float btnY = startY - i * (buttonHeight + buttonSpacing * 0.5f);
            if (inRect(x, y, buttonX, btnY, buttonWidth, buttonHeight)) {
                if (i == (size_t)Difficulty::CUSTOM) {
                    state = MenuState::CUSTOM_SETTINGS;
                    customSelectedIndex = 0;
                } else {
                    applyDifficulty((Difficulty)i);
                    state = MenuState::SETTINGS;
                    settingsSelectedIndex = 2;
                }
                return;
            }
        }
        float backY = startY - difficultyOptions.size() * (buttonHeight + buttonSpacing * 0.5f) - buttonSpacing;
        if (inRect(x, y, buttonX, backY, buttonWidth, buttonHeight)) {
            state = MenuState::SETTINGS;
            settingsSelectedIndex = 2;
            return;
        }
    }
    else if (state == MenuState::CUSTOM_SETTINGS) {
        float startY = panelY + panelHeight - 130;
        
        // Speed slider
        if (inRect(x, y, sliderX, startY, sliderWidth, 20)) {
            draggingSlider = 10;
            speedSlider = std::max(0.0f, std::min(1.0f, (float)(x - sliderX) / sliderWidth));
            return;
        }
        
        // Gravity slider
        if (inRect(x, y, sliderX, startY - 70, sliderWidth, 20)) {
            draggingSlider = 11;
            gravitySlider = std::max(0.0f, std::min(1.0f, (float)(x - sliderX) / sliderWidth));
            return;
        }
        
        // Jump slider
        if (inRect(x, y, sliderX, startY - 140, sliderWidth, 20)) {
            draggingSlider = 12;
            jumpSlider = std::max(0.0f, std::min(1.0f, (float)(x - sliderX) / sliderWidth));
            return;
        }
        
        // Apply and Back
        float btnY = startY - 220;
        if (inRect(x, y, buttonX, btnY, buttonWidth, buttonHeight)) {
            applyDifficulty(Difficulty::CUSTOM);
            state = MenuState::DIFFICULTY_SETTINGS;
            return;
        }
        if (inRect(x, y, buttonX, btnY - buttonHeight - buttonSpacing, buttonWidth, buttonHeight)) {
            state = MenuState::DIFFICULTY_SETTINGS;
            return;
        }
    }
}

void Menu::handleMouseMove(double x, double y) {
    y = screenHeight - y;  // Flip Y coordinate
    
    if (draggingSlider < 0) return;
    
    float panelWidth = 450.0f;
    float panelX = (screenWidth - panelWidth) / 2.0f;
    float buttonWidth = 350.0f;
    float buttonX = panelX + (panelWidth - buttonWidth) / 2.0f;
    float sliderWidth = 300.0f;
    float gfxSliderWidth = 280.0f;
    
    float value;
    
    // Controls settings sliders
    if (draggingSlider == 0) {
        value = std::max(0.0f, std::min(1.0f, (float)(x - buttonX) / sliderWidth));
        sensitivitySlider = value;
        pendingSettings.controls.sensitivity = 0.001f + sensitivitySlider * 0.009f;
    }
    // Graphics settings sliders
    else if (draggingSlider == 2) {
        value = std::max(0.0f, std::min(1.0f, (float)(x - buttonX) / gfxSliderWidth));
        renderDistanceSlider = value;
        pendingSettings.graphics.renderDistance = 500.0f + renderDistanceSlider * 9500.0f;
    }
    else if (draggingSlider == 3) {
        value = std::max(0.0f, std::min(1.0f, (float)(x - buttonX) / gfxSliderWidth));
        fovSlider = value;
        pendingSettings.graphics.fov = 30.0f + fovSlider * 120.0f;
    }
    else if (draggingSlider == 4) {
        value = std::max(0.0f, std::min(1.0f, (float)(x - buttonX) / gfxSliderWidth));
        framerateSlider = value;
        if (framerateSlider >= 0.99f) pendingSettings.graphics.maxFramerate = 0;
        else pendingSettings.graphics.maxFramerate = 30 + (int)(framerateSlider * 210.0f);
    }
    else if (draggingSlider == 5) {
        value = std::max(0.0f, std::min(1.0f, (float)(x - buttonX) / gfxSliderWidth));
        guiScaleSlider = value;
        pendingSettings.graphics.guiScale = 0.5f + guiScaleSlider * 1.5f;
    }
    // Custom difficulty sliders
    else if (draggingSlider == 10) {
        value = std::max(0.0f, std::min(1.0f, (float)(x - buttonX) / sliderWidth));
        speedSlider = value;
    }
    else if (draggingSlider == 11) {
        value = std::max(0.0f, std::min(1.0f, (float)(x - buttonX) / sliderWidth));
        gravitySlider = value;
    }
    else if (draggingSlider == 12) {
        value = std::max(0.0f, std::min(1.0f, (float)(x - buttonX) / sliderWidth));
        jumpSlider = value;
    }
}
