#include "Menu.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <algorithm>

// ==================== Rendering Functions ====================

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
        panelHeight = 420.0f;
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
        
        float startY = panelY + panelHeight - 100;
        float sliderWidth = 300.0f;
        float sliderX = buttonX;
        
        // Sensitivity slider
        drawSlider(sliderX, startY, sliderWidth, 20, "Sensitivity", sensitivitySlider, controlsSelectedIndex == 0);
        char sensStr[32];
        snprintf(sensStr, sizeof(sensStr), "%.4f", 0.001f + sensitivitySlider * 0.009f);
        glColor3f(0.7f, 0.7f, 0.7f);
        drawText(sliderX + sliderWidth + 15, startY + 5, sensStr, 0.35f);
        
        // Toggle Crouch checkbox
        drawCheckbox(sliderX, startY - 45, 25, "Toggle Crouch", pendingSettings.controls.toggleCrouch, controlsSelectedIndex == 1);
        
        // Keybinds - compact layout with two columns
        float keybindY = startY - 95;
        float keybindH = 30;
        float keybindSpacing = 35;
        float halfWidth = (buttonWidth - 10) / 2;
        
        // Left column
        drawKeybind(sliderX, keybindY, halfWidth, keybindH, "Forward", pendingSettings.controls.keyForward, controlsSelectedIndex == 2, waitingForKeybind == 2);
        drawKeybind(sliderX, keybindY - keybindSpacing, halfWidth, keybindH, "Backward", pendingSettings.controls.keyBackward, controlsSelectedIndex == 3, waitingForKeybind == 3);
        drawKeybind(sliderX, keybindY - keybindSpacing * 2, halfWidth, keybindH, "Left", pendingSettings.controls.keyLeft, controlsSelectedIndex == 4, waitingForKeybind == 4);
        drawKeybind(sliderX, keybindY - keybindSpacing * 3, halfWidth, keybindH, "Right", pendingSettings.controls.keyRight, controlsSelectedIndex == 5, waitingForKeybind == 5);
        drawKeybind(sliderX, keybindY - keybindSpacing * 4, halfWidth, keybindH, "Jump", pendingSettings.controls.keyJump, controlsSelectedIndex == 6, waitingForKeybind == 6);
        
        // Right column
        drawKeybind(sliderX + halfWidth + 10, keybindY, halfWidth, keybindH, "Crouch", pendingSettings.controls.keyCrouch, controlsSelectedIndex == 7, waitingForKeybind == 7);
        drawKeybind(sliderX + halfWidth + 10, keybindY - keybindSpacing, halfWidth, keybindH, "Timer", pendingSettings.controls.keyTimer, controlsSelectedIndex == 8, waitingForKeybind == 8);
        drawKeybind(sliderX + halfWidth + 10, keybindY - keybindSpacing * 2, halfWidth, keybindH, "Reset", pendingSettings.controls.keyReset, controlsSelectedIndex == 9, waitingForKeybind == 9);
        drawKeybind(sliderX + halfWidth + 10, keybindY - keybindSpacing * 3, halfWidth, keybindH, "Help", pendingSettings.controls.keyHelp, controlsSelectedIndex == 10, waitingForKeybind == 10);
        drawKeybind(sliderX + halfWidth + 10, keybindY - keybindSpacing * 4, halfWidth, keybindH, "Leaderboard", pendingSettings.controls.keyLeaderboard, controlsSelectedIndex == 11, waitingForKeybind == 11);
        
        // Back and Apply buttons at bottom
        float btnY = panelY + 25;
        drawButton(buttonX, btnY, halfWidth, buttonHeight, "Back", controlsSelectedIndex == 13);
        drawButton(buttonX + halfWidth + 10, btnY, halfWidth, buttonHeight, "Apply", controlsSelectedIndex == 12);
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
    else if (state == MenuState::HELP) {
        panelHeight = 550.0f;
        panelY = (windowHeight - panelHeight) / 2.0f;
        
        // Redraw panel with new dimensions
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
        
        // Title
        glColor3f(1.0f, 1.0f, 1.0f);
        float titleW = getTextWidth("HELP - CONTROLS", 0.7f);
        drawText(panelX + (panelWidth - titleW) / 2.0f, panelY + panelHeight - 55, "HELP - CONTROLS", 0.7f);
        
        // Help content
        float textX = panelX + 30;
        float textY = panelY + panelHeight - 100;
        float lineHeight = 35.0f;
        
        glColor3f(0.9f, 0.7f, 0.2f);
        drawText(textX, textY, "Movement:", 0.45f);
        glColor3f(0.8f, 0.8f, 0.8f);
        drawText(textX + 20, textY - lineHeight, "WASD - Move around", 0.4f);
        drawText(textX + 20, textY - lineHeight * 2, "Space - Jump", 0.4f);
        drawText(textX + 20, textY - lineHeight * 3, "Shift - Crouch", 0.4f);
        drawText(textX + 20, textY - lineHeight * 4, "Mouse - Look around", 0.4f);
        drawText(textX + 20, textY - lineHeight * 5, "Scroll - Zoom camera in/out", 0.4f);
        
        glColor3f(0.9f, 0.7f, 0.2f);
        drawText(textX, textY - lineHeight * 6.5f, "Advanced Movement:", 0.45f);
        glColor3f(0.4f, 0.9f, 0.4f);
        drawText(textX + 20, textY - lineHeight * 7.5f, "Shift+Space - Crouch Jump (lower but faster)", 0.4f);
        drawText(textX + 20, textY - lineHeight * 8.5f, "E + Near Wall - Wall Run (while falling)", 0.4f);
        drawText(textX + 20, textY - lineHeight * 9.5f, "Space (Wall Run) - Wall Jump (jump off wall)", 0.4f);
        
        glColor3f(0.9f, 0.7f, 0.2f);
        drawText(textX, textY - lineHeight * 11.0f, "Other:", 0.45f);
        glColor3f(0.8f, 0.8f, 0.8f);
        drawText(textX + 20, textY - lineHeight * 12.0f, "T - Toggle timer", 0.4f);
        drawText(textX + 20, textY - lineHeight * 13.0f, "R - Reset stats (timer/deaths)", 0.4f);
        drawText(textX + 20, textY - lineHeight * 14.0f, "H - Show this help menu", 0.4f);
        drawText(textX + 20, textY - lineHeight * 15.0f, "ESC - Pause menu", 0.4f);
        
        // Back button
        float btnY = panelY + 25;
        drawButton(buttonX, btnY, buttonWidth, buttonHeight, "Back (ESC)", true);
    }
    else if (state == MenuState::COMPLETION) {
        panelHeight = 400.0f;
        panelY = (windowHeight - panelHeight) / 2.0f;
        
        // Redraw panel with new dimensions
        glColor4f(0.05f, 0.15f, 0.05f, 0.95f);
        glBegin(GL_QUADS);
        glVertex2f(panelX, panelY); glVertex2f(panelX + panelWidth, panelY);
        glVertex2f(panelX + panelWidth, panelY + panelHeight); glVertex2f(panelX, panelY + panelHeight);
        glEnd();
        
        glColor3f(0.2f, 0.8f, 0.2f);
        glLineWidth(3.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(panelX, panelY); glVertex2f(panelX + panelWidth, panelY);
        glVertex2f(panelX + panelWidth, panelY + panelHeight); glVertex2f(panelX, panelY + panelHeight);
        glEnd();
        
        // Congrats title
        glColor3f(0.2f, 1.0f, 0.2f);
        float titleW = getTextWidth("CONGRATULATIONS!", 0.8f);
        drawText(panelX + (panelWidth - titleW) / 2.0f, panelY + panelHeight - 60, "CONGRATULATIONS!", 0.8f);
        
        // Subtitle
        glColor3f(0.8f, 1.0f, 0.8f);
        float subW = getTextWidth("Course Completed!", 0.5f);
        drawText(panelX + (panelWidth - subW) / 2.0f, panelY + panelHeight - 100, "Course Completed!", 0.5f);
        
        // Time display
        int minutes = (int)(completionTime / 60.0f);
        int seconds = (int)completionTime % 60;
        int milliseconds = (int)((completionTime - (int)completionTime) * 100);
        
        char timeStr[64];
        snprintf(timeStr, sizeof(timeStr), "Time: %02d:%02d.%02d", minutes, seconds, milliseconds);
        glColor3f(1.0f, 1.0f, 1.0f);
        float timeW = getTextWidth(timeStr, 0.55f);
        drawText(panelX + (panelWidth - timeW) / 2.0f, panelY + panelHeight - 150, timeStr, 0.55f);
        
        // Deaths display
        char deathStr[32];
        snprintf(deathStr, sizeof(deathStr), "Deaths: %d", completionDeaths);
        glColor3f(1.0f, 0.6f, 0.6f);
        float deathW = getTextWidth(deathStr, 0.45f);
        drawText(panelX + (panelWidth - deathW) / 2.0f, panelY + panelHeight - 185, deathStr, 0.45f);
        
        // Name input label
        glColor3f(0.9f, 0.9f, 0.2f);
        float labelW = getTextWidth("Enter your name:", 0.45f);
        drawText(panelX + (panelWidth - labelW) / 2.0f, panelY + panelHeight - 235, "Enter your name:", 0.45f);
        
        // Name input box
        float boxWidth = 300.0f;
        float boxHeight = 45.0f;
        float boxX = panelX + (panelWidth - boxWidth) / 2.0f;
        float boxY = panelY + panelHeight - 290;
        
        // Box background
        glColor4f(0.15f, 0.15f, 0.15f, 0.9f);
        glBegin(GL_QUADS);
        glVertex2f(boxX, boxY); glVertex2f(boxX + boxWidth, boxY);
        glVertex2f(boxX + boxWidth, boxY + boxHeight); glVertex2f(boxX, boxY + boxHeight);
        glEnd();
        
        // Box border
        glColor3f(0.5f, 0.8f, 0.5f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(boxX, boxY); glVertex2f(boxX + boxWidth, boxY);
        glVertex2f(boxX + boxWidth, boxY + boxHeight); glVertex2f(boxX, boxY + boxHeight);
        glEnd();
        
        // Name text with cursor
        std::string displayName = playerName;
        if (((int)(completionCountdown * 2)) % 2 == 0) {
            displayName += "_";
        }
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(boxX + 15, boxY + 12, displayName, 0.45f);
        
        // Countdown display
        glColor3f(0.6f, 0.6f, 0.6f);
        char countdownStr[64];
        int countSecs = (int)completionCountdown + 1;
        snprintf(countdownStr, sizeof(countdownStr), "Restarting in %d seconds...", countSecs);
        float countW = getTextWidth(countdownStr, 0.35f);
        drawText(panelX + (panelWidth - countW) / 2.0f, panelY + 40, countdownStr, 0.35f);
        
        // Press Enter hint
        glColor3f(0.5f, 0.8f, 0.5f);
        float hintW = getTextWidth("Press ENTER to save early", 0.35f);
        drawText(panelX + (panelWidth - hintW) / 2.0f, panelY + 70, "Press ENTER to save early", 0.35f);
    }
    else if (state == MenuState::LEADERBOARD) {
        panelHeight = 580.0f;
        panelY = (windowHeight - panelHeight) / 2.0f;
        
        // Redraw panel with new dimensions
        glColor4f(0.08f, 0.1f, 0.15f, 0.95f);
        glBegin(GL_QUADS);
        glVertex2f(panelX, panelY); glVertex2f(panelX + panelWidth, panelY);
        glVertex2f(panelX + panelWidth, panelY + panelHeight); glVertex2f(panelX, panelY + panelHeight);
        glEnd();
        
        glColor3f(0.4f, 0.6f, 0.9f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(panelX, panelY); glVertex2f(panelX + panelWidth, panelY);
        glVertex2f(panelX + panelWidth, panelY + panelHeight); glVertex2f(panelX, panelY + panelHeight);
        glEnd();
        
        // Title
        glColor3f(1.0f, 0.85f, 0.2f);
        float titleW = getTextWidth("LEADERBOARD", 0.8f);
        drawText(panelX + (panelWidth - titleW) / 2.0f, panelY + panelHeight - 50, "LEADERBOARD", 0.8f);
        
        // Trophy icons
        glColor3f(1.0f, 0.85f, 0.2f);
        drawText(panelX + 30, panelY + panelHeight - 50, "[#]", 0.6f);
        float trophyW = getTextWidth("[#]", 0.6f);
        drawText(panelX + panelWidth - 30 - trophyW, panelY + panelHeight - 50, "[#]", 0.6f);
        
        // Search bar
        float searchY = panelY + panelHeight - 90;
        float searchX = panelX + 30;
        float searchWidth = panelWidth - 60;
        float searchHeight = 30;
        
        // Search box background
        glColor4f(0.12f, 0.14f, 0.18f, 1.0f);
        glBegin(GL_QUADS);
        glVertex2f(searchX, searchY); glVertex2f(searchX + searchWidth, searchY);
        glVertex2f(searchX + searchWidth, searchY + searchHeight); glVertex2f(searchX, searchY + searchHeight);
        glEnd();
        
        // Search box border
        glColor3f(0.3f, 0.4f, 0.5f);
        glLineWidth(1.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(searchX, searchY); glVertex2f(searchX + searchWidth, searchY);
        glVertex2f(searchX + searchWidth, searchY + searchHeight); glVertex2f(searchX, searchY + searchHeight);
        glEnd();
        
        // Search text or placeholder
        if (leaderboardSearch.empty()) {
            glColor3f(0.4f, 0.4f, 0.4f);
            drawText(searchX + 10, searchY + 8, "Type to search...", 0.35f);
        } else {
            glColor3f(1.0f, 1.0f, 1.0f);
            std::string displaySearch = leaderboardSearch + "_";
            drawText(searchX + 10, searchY + 8, displaySearch, 0.35f);
        }
        
        // Column headers
        float headerY = panelY + panelHeight - 130;
        float colRank = panelX + 30;
        float colName = panelX + 80;
        float colTime = panelX + 250;
        float colDeaths = panelX + 370;
        
        glColor3f(0.6f, 0.7f, 0.8f);
        drawText(colRank, headerY, "#", 0.4f);
        drawText(colName, headerY, "Name", 0.4f);
        drawText(colTime, headerY, "Time", 0.4f);
        drawText(colDeaths, headerY, "Deaths", 0.4f);
        
        // Separator line
        glColor3f(0.3f, 0.4f, 0.5f);
        glLineWidth(1.0f);
        glBegin(GL_LINES);
        glVertex2f(panelX + 20, headerY - 10);
        glVertex2f(panelX + panelWidth - 20, headerY - 10);
        glEnd();
        
        // Leaderboard entries
        const auto& entries = leaderboard.getEntries();
        float entryY = headerY - 40;
        float entryHeight = 32.0f;
        int maxVisible = 10;
        
        if (entries.empty()) {
            glColor3f(0.5f, 0.5f, 0.5f);
            float emptyW = getTextWidth("No entries yet - complete the course!", 0.4f);
            drawText(panelX + (panelWidth - emptyW) / 2.0f, entryY, "No entries yet - complete the course!", 0.4f);
        } else {
            int startIdx = leaderboardScroll;
            int endIdx = std::min(startIdx + maxVisible, (int)entries.size());
            
            for (int i = startIdx; i < endIdx; i++) {
                const LeaderboardEntry& entry = entries[i];
                float y = entryY - (i - startIdx) * entryHeight;
                
                // Highlight if this is a search match
                bool isHighlighted = (i == leaderboardHighlight);
                if (isHighlighted) {
                    glColor4f(0.2f, 0.4f, 0.6f, 0.5f);
                    glBegin(GL_QUADS);
                    glVertex2f(panelX + 25, y - 5);
                    glVertex2f(panelX + panelWidth - 25, y - 5);
                    glVertex2f(panelX + panelWidth - 25, y + entryHeight - 8);
                    glVertex2f(panelX + 25, y + entryHeight - 8);
                    glEnd();
                }
                
                // Color based on rank
                if (isHighlighted) glColor3f(0.3f, 1.0f, 0.3f);
                else if (i == 0) glColor3f(1.0f, 0.85f, 0.2f);
                else if (i == 1) glColor3f(0.75f, 0.75f, 0.8f);
                else if (i == 2) glColor3f(0.8f, 0.5f, 0.2f);
                else glColor3f(0.8f, 0.8f, 0.8f);
                
                // Rank
                char rankStr[16];
                snprintf(rankStr, sizeof(rankStr), "%d", i + 1);
                drawText(colRank, y, rankStr, 0.38f);
                
                // Medal for top 3
                if (i < 3) {
                    const char* medal = (i == 0) ? "[G]" : (i == 1) ? "[S]" : "[B]";
                    drawText(colRank + 25, y, medal, 0.28f);
                }
                
                // Name
                std::string displayName = entry.name;
                if (displayName.length() > 15) {
                    displayName = displayName.substr(0, 12) + "...";
                }
                drawText(colName, y, displayName, 0.38f);
                
                // Time
                int minutes = (int)(entry.time / 60.0f);
                float seconds = entry.time - minutes * 60.0f;
                char timeStr[32];
                snprintf(timeStr, sizeof(timeStr), "%02d:%05.2f", minutes, seconds);
                drawText(colTime, y, timeStr, 0.38f);
                
                // Deaths
                char deathStr[16];
                snprintf(deathStr, sizeof(deathStr), "%d", entry.deaths);
                drawText(colDeaths, y, deathStr, 0.38f);
            }
            
            // Scrollbar
            float scrollbarX = panelX + panelWidth - 15;
            float scrollbarTop = headerY - 20;
            float scrollbarHeight = maxVisible * entryHeight;
            float scrollbarY = scrollbarTop - scrollbarHeight;
            
            glColor4f(0.2f, 0.2f, 0.25f, 0.8f);
            glBegin(GL_QUADS);
            glVertex2f(scrollbarX, scrollbarY);
            glVertex2f(scrollbarX + 8, scrollbarY);
            glVertex2f(scrollbarX + 8, scrollbarTop);
            glVertex2f(scrollbarX, scrollbarTop);
            glEnd();
            
            if ((int)entries.size() > maxVisible) {
                float thumbRatio = (float)maxVisible / entries.size();
                float thumbHeight = std::max(20.0f, scrollbarHeight * thumbRatio);
                float scrollRatio = (float)leaderboardScroll / std::max(1, (int)entries.size() - maxVisible);
                float thumbY = scrollbarTop - thumbHeight - scrollRatio * (scrollbarHeight - thumbHeight);
                
                glColor4f(0.5f, 0.6f, 0.8f, 0.9f);
                glBegin(GL_QUADS);
                glVertex2f(scrollbarX, thumbY);
                glVertex2f(scrollbarX + 8, thumbY);
                glVertex2f(scrollbarX + 8, thumbY + thumbHeight);
                glVertex2f(scrollbarX, thumbY + thumbHeight);
                glEnd();
            }
            
            // Scroll indicators
            if (leaderboardScroll > 0) {
                glColor3f(0.5f, 0.7f, 1.0f);
                drawText(panelX + panelWidth - 70, panelY + panelHeight - 130, "^ Scroll", 0.25f);
            }
            if (endIdx < (int)entries.size()) {
                glColor3f(0.5f, 0.7f, 1.0f);
                drawText(panelX + panelWidth - 70, panelY + 75, "v Scroll", 0.25f);
            }
            
            // Entry count
            glColor3f(0.5f, 0.5f, 0.5f);
            char countStr[64];
            snprintf(countStr, sizeof(countStr), "Showing %d-%d of %d entries", 
                     startIdx + 1, endIdx, (int)entries.size());
            float countW = getTextWidth(countStr, 0.28f);
            drawText(panelX + (panelWidth - countW) / 2.0f, panelY + 50, countStr, 0.28f);
        }
        
        // Controls hint
        glColor3f(0.4f, 0.5f, 0.6f);
        drawText(panelX + 25, panelY + 30, "Mouse wheel or Arrow keys to scroll", 0.25f);
        
        // Back button hint
        glColor3f(0.6f, 0.6f, 0.6f);
        float escW = getTextWidth("ESC to close", 0.3f);
        drawText(panelX + panelWidth - escW - 25, panelY + 30, "ESC to close", 0.3f);
    }
    
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void Menu::renderResetPopup(int windowWidth, int windowHeight) {
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
    float popupX = windowWidth - popupWidth - 20.0f;
    float popupY = windowHeight - popupHeight - 20.0f;
    
    float alpha = activeTimer > 1.0f ? 1.0f : activeTimer;
    
    float bgR, bgG, bgB, borderR, borderG, borderB, iconR, iconG, iconB;
    if (popupIsGreen) {
        bgR = 0.1f; bgG = 0.3f; bgB = 0.1f;
        borderR = 0.2f; borderG = 0.9f; borderB = 0.3f;
        iconR = 0.2f; iconG = 1.0f; iconB = 0.4f;
    } else {
        bgR = 0.1f; bgG = 0.15f; bgB = 0.3f;
        borderR = 0.3f; borderG = 0.6f; borderB = 0.9f;
        iconR = 0.4f; iconG = 0.7f; iconB = 1.0f;
    }
    
    glColor4f(bgR, bgG, bgB, 0.95f * alpha);
    glBegin(GL_QUADS);
    glVertex2f(popupX, popupY);
    glVertex2f(popupX + popupWidth, popupY);
    glVertex2f(popupX + popupWidth, popupY + popupHeight);
    glVertex2f(popupX, popupY + popupHeight);
    glEnd();
    
    float pulse = 0.7f + 0.3f * sinf(activeTimer * 8.0f);
    glColor4f(borderR * pulse, borderG * pulse, borderB * pulse, alpha);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(popupX, popupY);
    glVertex2f(popupX + popupWidth, popupY);
    glVertex2f(popupX + popupWidth, popupY + popupHeight);
    glVertex2f(popupX, popupY + popupHeight);
    glEnd();
    
    glColor4f(iconR, iconG, iconB, alpha);
    glLineWidth(4.0f);
    float checkX = popupX + 30;
    float checkY = popupY + popupHeight / 2;
    glBegin(GL_LINE_STRIP);
    glVertex2f(checkX, checkY);
    glVertex2f(checkX + 10, checkY - 15);
    glVertex2f(checkX + 30, checkY + 15);
    glEnd();
    
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

void Menu::renderCheckpointPopup(int windowWidth, int windowHeight, 
                                  const std::string& message, float timer) {
    if (timer <= 0) return;
    
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
    
    float popupWidth = 380.0f;
    float popupHeight = 80.0f;
    float popupX = (windowWidth - popupWidth) / 2.0f;
    float popupY = windowHeight - popupHeight - 80.0f;
    
    float alpha = timer > 0.5f ? 1.0f : timer * 2.0f;
    
    float slideIn = timer > 1.5f ? (2.0f - timer) * 2.0f : 1.0f;
    slideIn = std::min(1.0f, std::max(0.0f, slideIn));
    popupY = popupY + (1.0f - slideIn) * 100.0f;
    
    float bgR = 0.05f, bgG = 0.25f, bgB = 0.1f;
    float borderR = 0.3f, borderG = 1.0f, borderB = 0.4f;
    
    glColor4f(bgR, bgG, bgB, 0.9f * alpha);
    glBegin(GL_QUADS);
    glVertex2f(popupX, popupY);
    glVertex2f(popupX + popupWidth, popupY);
    glVertex2f(popupX + popupWidth, popupY + popupHeight);
    glVertex2f(popupX, popupY + popupHeight);
    glEnd();
    
    float pulse = 0.7f + 0.3f * sinf(timer * 10.0f);
    glColor4f(borderR * pulse, borderG * pulse, borderB * pulse, alpha);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(popupX, popupY);
    glVertex2f(popupX + popupWidth, popupY);
    glVertex2f(popupX + popupWidth, popupY + popupHeight);
    glVertex2f(popupX, popupY + popupHeight);
    glEnd();
    
    glColor4f(0.3f, 1.0f, 0.5f, alpha);
    float flagX = popupX + 30;
    float flagY = popupY + popupHeight / 2;
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glVertex2f(flagX, flagY - 20);
    glVertex2f(flagX, flagY + 20);
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex2f(flagX, flagY + 20);
    glVertex2f(flagX + 20, flagY + 10);
    glVertex2f(flagX, flagY);
    glEnd();
    
    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    float textW = getTextWidth(message, 0.55f);
    drawText(popupX + (popupWidth - textW) / 2.0f + 15, popupY + popupHeight / 2 - 12, message, 0.55f);
    
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
    
    glColor3f(0.4f, 0.4f, 0.4f);
    drawText(padding, padding + 10, "[H] Help  [L] Leaderboard", 0.3f);
    
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
