# 7. Menu System & UI

This document explains how the menu system works, including button rendering, text display, and user interface handling.

## Menu State Machine

The menu uses a **state machine** - a pattern where behavior changes based on current state:

```cpp
enum class MenuState {
    NONE,               // Menu closed, gameplay active
    PAUSE,              // Main pause menu
    SETTINGS,           // Settings submenu
    CONTROLS_SETTINGS,  // Controls configuration
    GRAPHICS_SETTINGS,  // Graphics options
    DIFFICULTY_SETTINGS,// Difficulty selection
    CUSTOM_SETTINGS,    // Custom difficulty sliders
    KEYBIND_WAITING,    // Waiting for key press to rebind
    HELP,               // Help/controls display
    COMPLETION          // Course completion screen with name entry
};
```

### State Transitions

```
NONE ←→ PAUSE ←→ SETTINGS ←→ CONTROLS_SETTINGS
                    ↓              ↓
              GRAPHICS_SETTINGS  KEYBIND_WAITING
                    ↓
              DIFFICULTY_SETTINGS ←→ CUSTOM_SETTINGS
              
NONE ←→ HELP (H key shortcut)
NONE → COMPLETION → NONE (triggered when reaching goal)
```

## 2D Rendering Setup

UI is rendered in 2D (orthographic projection):

```cpp
void Menu::render(int windowWidth, int windowHeight) {
    // Save 3D projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    
    // Set up 2D coordinates (0,0 at bottom-left, windowWidth,windowHeight at top-right)
    glOrtho(0, windowWidth, 0, windowHeight, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Disable 3D features
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // === DRAW UI HERE ===
    
    // Restore 3D state
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
```

## Drawing a Semi-Transparent Overlay

```cpp
// Dark overlay behind menu
glColor4f(0.0f, 0.0f, 0.0f, 0.7f);  // 70% opaque black
glBegin(GL_QUADS);
glVertex2f(0, 0);                    // Bottom-left
glVertex2f(windowWidth, 0);          // Bottom-right
glVertex2f(windowWidth, windowHeight); // Top-right
glVertex2f(0, windowHeight);         // Top-left
glEnd();
```

## Drawing Buttons

```cpp
void Menu::drawButton(float x, float y, float width, float height,
                      const std::string& text, bool selected, bool hovered) {
    // Background color based on state
    if (selected) {
        glColor4f(0.3f, 0.5f, 0.7f, 0.9f);  // Blue when selected
    } else if (hovered) {
        glColor4f(0.2f, 0.3f, 0.4f, 0.9f);  // Lighter when hovered
    } else {
        glColor4f(0.15f, 0.18f, 0.22f, 0.9f);  // Dark default
    }
    
    // Button background
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
    
    // Border
    if (selected) {
        glColor3f(0.5f, 0.7f, 1.0f);  // Bright blue border
        glLineWidth(2.0f);
    } else {
        glColor3f(0.3f, 0.35f, 0.4f);  // Gray border
        glLineWidth(1.0f);
    }
    
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
    
    // Text (centered)
    glColor3f(1.0f, 1.0f, 1.0f);  // White text
    float textWidth = getTextWidth(text, 0.5f);
    float textX = x + (width - textWidth) / 2.0f;
    float textY = y + height / 2.0f - 10.0f;  // Approximate vertical center
    drawText(textX, textY, text, 0.5f);
}
```

## Text Rendering with FreeType

### What is FreeType?

FreeType is a library that renders TrueType fonts (.ttf files) into bitmap images we can display.

### Font Initialization

```cpp
bool Menu::initFont(const std::string& fontPath) {
    // Initialize FreeType library
    if (FT_Init_FreeType(&ftLibrary)) {
        return false;
    }
    
    // Load font file
    if (FT_New_Face(ftLibrary, fontPath.c_str(), 0, &ftFace)) {
        return false;
    }
    
    // Set font size (height in pixels)
    FT_Set_Pixel_Sizes(ftFace, 0, 48);  // 48 pixels tall
    
    // Generate texture for each character (A-Z, a-z, 0-9, symbols)
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // Byte alignment
    
    for (unsigned char c = 0; c < 128; c++) {
        // Render character to bitmap
        if (FT_Load_Char(ftFace, c, FT_LOAD_RENDER)) {
            continue;  // Skip if character fails to load
        }
        
        // Create OpenGL texture from bitmap
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA,
                     ftFace->glyph->bitmap.width,
                     ftFace->glyph->bitmap.rows,
                     0, GL_ALPHA, GL_UNSIGNED_BYTE,
                     ftFace->glyph->bitmap.buffer);
        
        // Texture settings (no filtering for pixel fonts)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        // Store character info
        Character ch = {
            texture,
            ftFace->glyph->bitmap.width,
            ftFace->glyph->bitmap.rows,
            ftFace->glyph->bitmap_left,
            ftFace->glyph->bitmap_top,
            ftFace->glyph->advance.x  // Advance to next character
        };
        characters[c] = ch;
    }
    
    fontLoaded = true;
    return true;
}
```

### Drawing Text

```cpp
void Menu::drawText(float x, float y, const std::string& text, float scale) {
    glEnable(GL_TEXTURE_2D);
    
    for (char c : text) {
        Character& ch = characters[c];
        
        // Calculate position
        float xpos = x + ch.bearingX * scale;
        float ypos = y - (ch.sizeY - ch.bearingY) * scale;
        
        float w = ch.sizeX * scale;
        float h = ch.sizeY * scale;
        
        // Bind character texture
        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        
        // Draw textured quad
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(xpos, ypos + h);
        glTexCoord2f(1, 0); glVertex2f(xpos + w, ypos + h);
        glTexCoord2f(1, 1); glVertex2f(xpos + w, ypos);
        glTexCoord2f(0, 1); glVertex2f(xpos, ypos);
        glEnd();
        
        // Advance cursor for next character
        x += (ch.advance >> 6) * scale;  // Advance is in 1/64 pixels
    }
    
    glDisable(GL_TEXTURE_2D);
}
```

### Getting Text Width

```cpp
float Menu::getTextWidth(const std::string& text, float scale) {
    float width = 0;
    for (char c : text) {
        Character& ch = characters[c];
        width += (ch.advance >> 6) * scale;
    }
    return width;
}
```

## Drawing UI Elements

### Checkbox

```cpp
void Menu::drawCheckbox(float x, float y, float size,
                        const std::string& label, bool checked, bool selected) {
    // Box background
    if (selected) {
        glColor4f(0.3f, 0.5f, 0.7f, 0.9f);
    } else {
        glColor4f(0.15f, 0.18f, 0.22f, 0.9f);
    }
    
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + size, y);
    glVertex2f(x + size, y + size);
    glVertex2f(x, y + size);
    glEnd();
    
    // Checkmark if checked
    if (checked) {
        glColor3f(0.3f, 1.0f, 0.3f);  // Green
        glLineWidth(3.0f);
        glBegin(GL_LINE_STRIP);
        glVertex2f(x + size * 0.2f, y + size * 0.5f);
        glVertex2f(x + size * 0.4f, y + size * 0.3f);
        glVertex2f(x + size * 0.8f, y + size * 0.8f);
        glEnd();
        glLineWidth(1.0f);
    }
    
    // Label text
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(x + size + 10, y + size/2 - 8, label, 0.4f);
}
```

### Slider

```cpp
void Menu::drawSlider(float x, float y, float width, float height,
                      const std::string& label, float value, bool selected) {
    // Label above slider
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(x, y + 25, label, 0.4f);
    
    // Track background
    glColor4f(0.1f, 0.12f, 0.15f, 0.9f);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
    
    // Filled portion
    float fillWidth = width * value;  // value is 0.0 to 1.0
    if (selected) {
        glColor4f(0.3f, 0.6f, 0.9f, 0.9f);  // Blue
    } else {
        glColor4f(0.25f, 0.45f, 0.7f, 0.9f);
    }
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + fillWidth, y);
    glVertex2f(x + fillWidth, y + height);
    glVertex2f(x, y + height);
    glEnd();
    
    // Handle (circle at current position)
    float handleX = x + fillWidth;
    float handleY = y + height / 2;
    float handleRadius = height * 0.8f;
    
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(handleX, handleY);
    for (int i = 0; i <= 16; i++) {
        float angle = 2 * M_PI * i / 16;
        glVertex2f(handleX + handleRadius * cos(angle),
                   handleY + handleRadius * sin(angle));
    }
    glEnd();
}
```

### Keybind Display

```cpp
void Menu::drawKeybind(float x, float y, float width, float height,
                       const std::string& label, int keyCode,
                       bool selected, bool waiting) {
    // Background
    if (waiting) {
        glColor4f(0.4f, 0.3f, 0.1f, 0.9f);  // Yellow/orange when waiting
    } else if (selected) {
        glColor4f(0.3f, 0.5f, 0.7f, 0.9f);  // Blue when selected
    } else {
        glColor4f(0.15f, 0.18f, 0.22f, 0.9f);
    }
    
    glBegin(GL_QUADS);
    // ... draw background
    glEnd();
    
    // Label on left
    glColor3f(0.8f, 0.8f, 0.8f);
    drawText(x + 10, y + height/2 - 8, label, 0.4f);
    
    // Key name on right
    std::string keyName = waiting ? "Press key..." : getKeyName(keyCode);
    glColor3f(1.0f, 1.0f, 1.0f);
    float keyWidth = getTextWidth(keyName, 0.4f);
    drawText(x + width - keyWidth - 10, y + height/2 - 8, keyName, 0.4f);
}
```

### Key Name Conversion

```cpp
std::string Menu::getKeyName(int keyCode) {
    switch (keyCode) {
        case GLFW_KEY_SPACE: return "SPACE";
        case GLFW_KEY_LEFT_SHIFT: return "L-SHIFT";
        case GLFW_KEY_RIGHT_SHIFT: return "R-SHIFT";
        case GLFW_KEY_ENTER: return "ENTER";
        case GLFW_KEY_ESCAPE: return "ESC";
        case GLFW_KEY_UP: return "UP";
        case GLFW_KEY_DOWN: return "DOWN";
        // ... more special keys
        
        default:
            // Letter keys (A-Z)
            if (keyCode >= GLFW_KEY_A && keyCode <= GLFW_KEY_Z) {
                return std::string(1, 'A' + (keyCode - GLFW_KEY_A));
            }
            // Number keys
            if (keyCode >= GLFW_KEY_0 && keyCode <= GLFW_KEY_9) {
                return std::string(1, '0' + (keyCode - GLFW_KEY_0));
            }
            return "???";
    }
}
```

## Keyboard Navigation

```cpp
void Menu::handleKey(int key, int action) {
    if (action != GLFW_PRESS) return;
    
    if (state == MenuState::PAUSE) {
        // Navigate up/down through buttons
        if (key == GLFW_KEY_UP || key == GLFW_KEY_W) {
            selectedIndex = (selectedIndex - 1 + pauseButtons.size()) % pauseButtons.size();
        }
        else if (key == GLFW_KEY_DOWN || key == GLFW_KEY_S) {
            selectedIndex = (selectedIndex + 1) % pauseButtons.size();
        }
        // Select button
        else if (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE) {
            if (selectedIndex == 0) close();           // Resume
            else if (selectedIndex == 1) shouldRestart = true;
            else if (selectedIndex == 2) state = MenuState::SETTINGS;
            else if (selectedIndex == 3) shouldQuit = true;
        }
    }
    else if (state == MenuState::CONTROLS_SETTINGS) {
        // Slider adjustment with left/right
        if (key == GLFW_KEY_LEFT || key == GLFW_KEY_A) {
            if (controlsSelectedIndex == 0) {  // Sensitivity slider
                sensitivitySlider = std::max(0.0f, sensitivitySlider - 0.05f);
            }
        }
        // Start keybind rebinding
        else if (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE) {
            if (controlsSelectedIndex >= 2 && controlsSelectedIndex <= 10) {
                waitingForKeybind = controlsSelectedIndex;
                state = MenuState::KEYBIND_WAITING;
            }
        }
    }
    // ... handle other states
}
```

## Mouse Input

```cpp
void Menu::handleMouseClick(double x, double y, int button, int action) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    
    // Flip Y coordinate (GLFW is top-down, OpenGL is bottom-up)
    y = screenHeight - y;
    
    // Calculate button positions (same as render())
    float panelX = (screenWidth - panelWidth) / 2;
    float panelY = (screenHeight - panelHeight) / 2;
    float buttonX = panelX + (panelWidth - buttonWidth) / 2;
    
    // Check each button
    float startY = panelY + panelHeight - 140;
    for (size_t i = 0; i < pauseButtons.size(); i++) {
        float btnY = startY - i * (buttonHeight + buttonSpacing);
        
        // Point-in-rectangle test
        if (x >= buttonX && x <= buttonX + buttonWidth &&
            y >= btnY && y <= btnY + buttonHeight) {
            // Button i was clicked!
            // ... handle click
            return;
        }
    }
}
```

### Slider Dragging

```cpp
void Menu::handleMouseMove(double x, double y) {
    y = screenHeight - y;
    
    if (draggingSlider >= 0) {
        // Update slider value based on mouse X position
        float sliderX = /* calculate from layout */;
        float sliderWidth = 300.0f;
        
        float newValue = (x - sliderX) / sliderWidth;
        newValue = std::max(0.0f, std::min(1.0f, newValue));  // Clamp 0-1
        
        if (draggingSlider == 0) {  // Sensitivity
            sensitivitySlider = newValue;
        }
        // ... other sliders
    }
}
```

## HUD Rendering

The HUD (Heads-Up Display) shows game info without blocking gameplay:

```cpp
void Menu::renderHUD(int windowWidth, int windowHeight, float timer, int deaths,
                     bool timerRunning, bool timerFinished) {
    // Set up 2D rendering (same as menu)
    // ...
    
    // Format timer as MM:SS.ms
    int minutes = (int)(timer / 60.0f);
    int seconds = (int)timer % 60;
    int milliseconds = (int)((timer - (int)timer) * 100);
    
    char timerStr[32];
    snprintf(timerStr, sizeof(timerStr), "%02d:%02d.%02d", minutes, seconds, milliseconds);
    
    // Color based on state
    if (timerFinished) glColor3f(0.2f, 1.0f, 0.2f);      // Green
    else if (timerRunning) glColor3f(1.0f, 1.0f, 1.0f);  // White
    else glColor3f(0.6f, 0.6f, 0.6f);                     // Gray
    
    // Draw in top-left
    float padding = 20.0f;
    drawText(padding, windowHeight - padding - 30, timerStr, 0.5f);
    
    // Deaths counter in top-right
    char deathStr[32];
    snprintf(deathStr, sizeof(deathStr), "Deaths: %d", deaths);
    glColor3f(1.0f, 0.4f, 0.4f);  // Red
    float deathWidth = getTextWidth(deathStr, 0.5f);
    drawText(windowWidth - padding - deathWidth, windowHeight - padding - 30, deathStr, 0.5f);
    
    // Help hint at bottom
    glColor3f(0.4f, 0.4f, 0.4f);
    drawText(padding, padding + 10, "[H] Help", 0.3f);
}
```

## Completion Screen & Leaderboard

When the player reaches the goal, a completion screen appears with name entry and leaderboard saving.

### Triggering Completion

In `main.cpp`, goal detection triggers the completion screen:

```cpp
// Check if player reached goal
if (obstacles->isOnGoal(playerX, playerY, playerZ) && !userInput->isTimerFinished()) {
    userInput->stopTimer();
    menu->showCompletion(userInput->getTimer(), userInput->getDeathCount());
}
```

### Completion Screen State

```cpp
void Menu::showCompletion(float time, int deaths) {
    state = MenuState::COMPLETION;
    completionTime = time;
    completionDeaths = deaths;
    completionCountdown = 10.0f;  // 10 second countdown
    completionSaved = false;
    playerName = "";              // Clear previous name
    
    // Show cursor for name input
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}
```

### Character Input for Name Entry

GLFW provides a character callback for text input:

```cpp
// In main.cpp - Register callback
glfwSetCharCallback(window, charCallback);

void charCallback(GLFWwindow* window, unsigned int codepoint) {
    if (menu->getState() == MenuState::COMPLETION) {
        menu->handleCharInput(codepoint);
    }
}

// In Menu.cpp
void Menu::handleCharInput(unsigned int codepoint) {
    if (state != MenuState::COMPLETION || completionSaved) return;
    
    // Only allow printable ASCII characters, max 20 chars
    if (codepoint >= 32 && codepoint < 127 && playerName.length() < 20) {
        playerName += static_cast<char>(codepoint);
    }
}
```

### Saving to Leaderboard (JSON)

```cpp
void Menu::saveLeaderboard() {
    if (completionSaved || playerName.empty()) return;
    
    std::string name = playerName;
    
    // Read existing leaderboard
    std::ifstream inFile("leaderboard.json");
    std::string content = "";
    if (inFile.is_open()) {
        std::stringstream buffer;
        buffer << inFile.rdbuf();
        content = buffer.str();
        inFile.close();
    }
    
    // Create new entry
    std::stringstream newEntry;
    newEntry << "    {\n"
             << "      \"name\": \"" << name << "\",\n"
             << "      \"time\": " << completionTime << ",\n"
             << "      \"deaths\": " << completionDeaths << ",\n"
             << "      \"timestamp\": " << std::time(nullptr) << "\n"
             << "    }";
    
    // Write updated JSON
    std::ofstream outFile("leaderboard.json");
    outFile << "{\n  \"entries\": [\n";
    // ... append entries
    outFile << "  ]\n}\n";
    outFile.close();
    
    completionSaved = true;
}
```

### Countdown and Auto-Reset

```cpp
void Menu::updateCompletion(float deltaTime) {
    if (state != MenuState::COMPLETION) return;
    
    completionCountdown -= deltaTime;
    
    if (completionCountdown <= 0.0f) {
        // Auto-save if name entered
        if (!completionSaved && !playerName.empty()) {
            saveLeaderboard();
        }
        // Signal main loop to reset player position
        shouldResetToStart = true;
        state = MenuState::NONE;
    }
}
```

### Input State Reset on Respawn

When respawning after completion, movement keys must be reset to prevent residual movement:

```cpp
// In main.cpp
if (menu->shouldResetToStart) {
    userInput->resetPosition();
    userInput->resetStats();
    projectiles->reset();
    // Reset input states to prevent residual movement
    w = s = a = d = false;
    shift = false;
    menu->shouldResetToStart = false;
}
```

### Python Leaderboard Viewer

A companion script `leaderboard.py` displays the saved scores:

```python
import json

def main():
    with open("build/leaderboard.json", "r") as f:
        data = json.load(f)
    
    entries = sorted(data.get("entries", []), key=lambda x: x["time"])
    
    print("🏆 LEADERBOARD 🏆")
    for i, entry in enumerate(entries[:100], 1):
        medal = ["🥇", "🥈", "🥉"][i-1] if i <= 3 else ""
        minutes = int(entry["time"]) // 60
        seconds = entry["time"] % 60
        print(f"{i:3}  {entry['name']:12} {minutes:02}:{seconds:05.2f} {entry['deaths']:3} deaths {medal}")

if __name__ == "__main__":
    main()
```

## Next Steps

Continue to [Settings & File I/O](./08_settings_file_io.md) to learn about saving and loading configuration.
