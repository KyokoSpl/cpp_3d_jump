#include "Menu.h"
#include <GLFW/glfw3.h>
#include <algorithm>

// ==================== Input Handling Functions ====================

void Menu::handleKey(int key, int action) {
    if (action != GLFW_PRESS) return;
    
    // Handle completion screen input
    if (state == MenuState::COMPLETION) {
        if (key == GLFW_KEY_BACKSPACE) {
            if (!playerName.empty()) {
                playerName.pop_back();
            }
        }
        else if (key == GLFW_KEY_ENTER) {
            if (!completionSaved) {
                saveLeaderboard();
                completionSaved = true;
            }
            state = MenuState::NONE;
            shouldResetToStart = true;
        }
        return;
    }
    
    // Handle keybind waiting
    if (state == MenuState::KEYBIND_WAITING || waitingForKeybind >= 0) {
        if (key == GLFW_KEY_ESCAPE) {
            waitingForKeybind = -1;
            state = MenuState::CONTROLS_SETTINGS;
            return;
        }
        
        switch (waitingForKeybind) {
            case 2: pendingSettings.controls.keyForward = key; break;
            case 3: pendingSettings.controls.keyBackward = key; break;
            case 4: pendingSettings.controls.keyLeft = key; break;
            case 5: pendingSettings.controls.keyRight = key; break;
            case 6: pendingSettings.controls.keyJump = key; break;
            case 7: pendingSettings.controls.keyCrouch = key; break;
            case 8: pendingSettings.controls.keyTimer = key; break;
            case 9: pendingSettings.controls.keyReset = key; break;
            case 10: pendingSettings.controls.keyHelp = key; break;
            case 11: pendingSettings.controls.keyLeaderboard = key; break;
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
            controlsSelectedIndex = (controlsSelectedIndex - 1 + 14) % 14;
        }
        else if (key == GLFW_KEY_DOWN || key == GLFW_KEY_S) {
            controlsSelectedIndex = (controlsSelectedIndex + 1) % 14;
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
            else if (controlsSelectedIndex >= 2 && controlsSelectedIndex <= 11) {
                waitingForKeybind = controlsSelectedIndex;
                state = MenuState::KEYBIND_WAITING;
            }
            else if (controlsSelectedIndex == 12) {
                applyPendingSettings();
                close();
            }
            else if (controlsSelectedIndex == 13) {
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
            else if (graphicsSelectedIndex == 6) { applyPendingSettings(); close(); }
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
    else if (state == MenuState::HELP) {
        if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE || key == GLFW_KEY_H) {
            close();
        }
    }
    else if (state == MenuState::LEADERBOARD) {
        if (key == GLFW_KEY_ESCAPE) {
            close();
        }
        else if (key == GLFW_KEY_BACKSPACE) {
            if (!leaderboardSearch.empty()) {
                leaderboardSearch.pop_back();
                leaderboardHighlight = -1;
                if (!leaderboardSearch.empty()) {
                    const auto& entries = leaderboard.getEntries();
                    for (size_t i = 0; i < entries.size(); i++) {
                        std::string nameLower = entries[i].name;
                        std::string searchLower = leaderboardSearch;
                        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                        std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
                        if (nameLower.find(searchLower) != std::string::npos) {
                            leaderboardHighlight = i;
                            if (leaderboardHighlight < leaderboardScroll || leaderboardHighlight >= leaderboardScroll + 10) {
                                leaderboardScroll = std::max(0, leaderboardHighlight - 5);
                            }
                            break;
                        }
                    }
                }
            }
        }
        else if (key == GLFW_KEY_UP) {
            if (leaderboardScroll > 0) leaderboardScroll--;
        }
        else if (key == GLFW_KEY_DOWN) {
            int maxScroll = std::max(0, (int)leaderboard.size() - 10);
            if (leaderboardScroll < maxScroll) leaderboardScroll++;
        }
        else if (key == GLFW_KEY_PAGE_UP) {
            leaderboardScroll = std::max(0, leaderboardScroll - 10);
        }
        else if (key == GLFW_KEY_PAGE_DOWN) {
            int maxScroll = std::max(0, (int)leaderboard.size() - 10);
            leaderboardScroll = std::min(maxScroll, leaderboardScroll + 10);
        }
        else if (key == GLFW_KEY_HOME) {
            leaderboardScroll = 0;
        }
        else if (key == GLFW_KEY_END) {
            leaderboardScroll = std::max(0, (int)leaderboard.size() - 10);
        }
    }
}

void Menu::handleKeyHeld(int key) {
    if (state == MenuState::LEADERBOARD) {
        if (key == GLFW_KEY_UP || key == GLFW_KEY_W) {
            if (leaderboardScroll > 0) leaderboardScroll--;
        }
        else if (key == GLFW_KEY_DOWN || key == GLFW_KEY_S) {
            int maxScroll = std::max(0, (int)leaderboard.size() - 10);
            if (leaderboardScroll < maxScroll) leaderboardScroll++;
        }
    }
}

void Menu::handleScroll(double yoffset) {
    if (state == MenuState::LEADERBOARD) {
        int scrollAmount = (int)(-yoffset * 3);
        leaderboardScroll += scrollAmount;
        
        int maxScroll = std::max(0, (int)leaderboard.size() - 10);
        if (leaderboardScroll < 0) leaderboardScroll = 0;
        if (leaderboardScroll > maxScroll) leaderboardScroll = maxScroll;
    }
}

void Menu::handleMouseClick(double x, double y, int button, int action) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    y = screenHeight - y;
    
    if (action == GLFW_RELEASE) {
        draggingSlider = -1;
        return;
    }
    
    float panelWidth = 450.0f;
    float panelHeight = 500.0f;
    
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
        
        if (inRect(x, y, sliderX, startY, sliderWidth, 20)) {
            draggingSlider = 0;
            sensitivitySlider = std::max(0.0f, std::min(1.0f, (float)(x - sliderX) / sliderWidth));
            pendingSettings.controls.sensitivity = 0.001f + sensitivitySlider * 0.009f;
            return;
        }
        
        if (inRect(x, y, sliderX, startY - 55, 25, 25)) {
            pendingSettings.controls.toggleCrouch = !pendingSettings.controls.toggleCrouch;
            return;
        }
        
        float keybindY = startY - 110;
        float keybindBoxX = buttonX + buttonWidth - 100;
        for (int i = 0; i < 6; i++) {
            if (inRect(x, y, keybindBoxX, keybindY - i * 45, 100, 35)) {
                waitingForKeybind = 2 + i;
                state = MenuState::KEYBIND_WAITING;
                return;
            }
        }
        
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
        
        if (inRect(x, y, sliderX, startY, 25, 25)) {
            pendingSettings.graphics.vsync = !pendingSettings.graphics.vsync;
            return;
        }
        
        if (inRect(x, y, sliderX + 180, startY, 25, 25)) {
            pendingSettings.graphics.fullscreen = !pendingSettings.graphics.fullscreen;
            return;
        }
        
        if (inRect(x, y, sliderX, startY - 60, gfxSliderWidth, 20)) {
            draggingSlider = 2;
            renderDistanceSlider = std::max(0.0f, std::min(1.0f, (float)(x - sliderX) / gfxSliderWidth));
            pendingSettings.graphics.renderDistance = 500.0f + renderDistanceSlider * 9500.0f;
            return;
        }
        
        if (inRect(x, y, sliderX, startY - 120, gfxSliderWidth, 20)) {
            draggingSlider = 3;
            fovSlider = std::max(0.0f, std::min(1.0f, (float)(x - sliderX) / gfxSliderWidth));
            pendingSettings.graphics.fov = 30.0f + fovSlider * 120.0f;
            return;
        }
        
        if (inRect(x, y, sliderX, startY - 180, gfxSliderWidth, 20)) {
            draggingSlider = 4;
            framerateSlider = std::max(0.0f, std::min(1.0f, (float)(x - sliderX) / gfxSliderWidth));
            if (framerateSlider >= 0.99f) pendingSettings.graphics.maxFramerate = 0;
            else pendingSettings.graphics.maxFramerate = 30 + (int)(framerateSlider * 210.0f);
            return;
        }
        
        if (inRect(x, y, sliderX, startY - 240, gfxSliderWidth, 20)) {
            draggingSlider = 5;
            guiScaleSlider = std::max(0.0f, std::min(1.0f, (float)(x - sliderX) / gfxSliderWidth));
            pendingSettings.graphics.guiScale = 0.5f + guiScaleSlider * 1.5f;
            return;
        }
        
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
        
        if (inRect(x, y, sliderX, startY, sliderWidth, 20)) {
            draggingSlider = 10;
            speedSlider = std::max(0.0f, std::min(1.0f, (float)(x - sliderX) / sliderWidth));
            return;
        }
        
        if (inRect(x, y, sliderX, startY - 70, sliderWidth, 20)) {
            draggingSlider = 11;
            gravitySlider = std::max(0.0f, std::min(1.0f, (float)(x - sliderX) / sliderWidth));
            return;
        }
        
        if (inRect(x, y, sliderX, startY - 140, sliderWidth, 20)) {
            draggingSlider = 12;
            jumpSlider = std::max(0.0f, std::min(1.0f, (float)(x - sliderX) / sliderWidth));
            return;
        }
        
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
    y = screenHeight - y;
    
    if (draggingSlider < 0) return;
    
    float panelWidth = 450.0f;
    float panelX = (screenWidth - panelWidth) / 2.0f;
    float buttonWidth = 350.0f;
    float buttonX = panelX + (panelWidth - buttonWidth) / 2.0f;
    float sliderWidth = 300.0f;
    float gfxSliderWidth = 280.0f;
    
    float value;
    
    if (draggingSlider == 0) {
        value = std::max(0.0f, std::min(1.0f, (float)(x - buttonX) / sliderWidth));
        sensitivitySlider = value;
        pendingSettings.controls.sensitivity = 0.001f + sensitivitySlider * 0.009f;
    }
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
