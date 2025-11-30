#include "Menu.h"
#include "MenuAudio.h"
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>

// GL_CLAMP_TO_EDGE may not be defined in older GL headers
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

// Initialize static member
bool Menu::devModeEnabled = false;

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
    
    // Completion screen
    playerName = "";
    completionTime = 0.0f;
    completionDeaths = 0;
    completionCountdown = 0.0f;
    completionSaved = false;
    
    // Leaderboard
    leaderboardScroll = 0;
    leaderboardSearch = "";
    leaderboardHighlight = -1;
    
    shouldRestart = false;
    shouldQuit = false;
    shouldToggleFullscreen = false;
    shouldUpdateVSync = false;
    shouldResetToStart = false;
    
    screenWidth = 1920;
    screenHeight = 1080;
    
    fontLoaded = false;
    ftLibrary = nullptr;
    ftFace = nullptr;
    
    // Initialize popup sound file at startup
    MenuAudio::init();
    
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
    MenuAudio::cleanup();
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
        MenuAudio::playPopupSound();
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
        MenuAudio::playPopupSound();
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

void Menu::showHelp() {
    state = MenuState::HELP;
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

// ==================== Completion Screen ====================

void Menu::showCompletion(float time, int deaths) {
    state = MenuState::COMPLETION;
    completionTime = time;
    completionDeaths = deaths;
    completionCountdown = 30.0f;
    completionSaved = false;
    playerName = "";
    
    MenuAudio::playPopupSound();
}

void Menu::updateCompletion(float deltaTime) {
    if (state != MenuState::COMPLETION) return;
    
    completionCountdown -= deltaTime;
    
    if (completionCountdown <= 0) {
        // Time's up - save and reset
        if (!completionSaved) {
            saveLeaderboard();
            completionSaved = true;
        }
        state = MenuState::NONE;
        shouldResetToStart = true;
    }
}

bool Menu::isCompletionDone() const {
    return state == MenuState::COMPLETION && completionCountdown <= 0;
}

void Menu::handleCharInput(unsigned int codepoint) {
    // Handle leaderboard search input
    if (state == MenuState::LEADERBOARD) {
        if (codepoint >= 32 && codepoint < 127) {
            if (leaderboardSearch.length() < 20) {
                leaderboardSearch += (char)codepoint;
                // Search for matching entry
                std::string searchLower = leaderboardSearch;
                std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
                
                leaderboardHighlight = -1;
                const auto& entries = leaderboard.getEntries();
                for (size_t i = 0; i < entries.size(); i++) {
                    std::string nameLower = entries[i].name;
                    std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                    if (nameLower.find(searchLower) != std::string::npos) {
                        leaderboardHighlight = (int)i;
                        // Scroll to show the result
                        int maxVisible = 12;
                        if (leaderboardHighlight < leaderboardScroll) {
                            leaderboardScroll = leaderboardHighlight;
                        } else if (leaderboardHighlight >= leaderboardScroll + maxVisible) {
                            leaderboardScroll = leaderboardHighlight - maxVisible + 1;
                        }
                        break;
                    }
                }
            }
        }
        return;
    }
    
    if (state != MenuState::COMPLETION) return;
    
    // Only accept printable ASCII characters
    if (codepoint >= 32 && codepoint < 127) {
        // Limit name length
        if (playerName.length() < 20) {
            playerName += (char)codepoint;
        }
    }
}

void Menu::saveLeaderboard() {
    if (completionSaved) return;
    
    leaderboard.save(playerName, completionTime, completionDeaths);
    completionSaved = true;
}

void Menu::showLeaderboard() {
    leaderboard.load();
    leaderboardScroll = 0;
    leaderboardSearch = "";
    leaderboardHighlight = -1;
    state = MenuState::LEADERBOARD;
}
