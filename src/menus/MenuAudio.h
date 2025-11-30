#ifndef MENU_AUDIO_H
#define MENU_AUDIO_H

// Audio system for menu popup sounds
// Uses miniaudio for cross-platform audio playback

namespace MenuAudio {
    // Initialize audio engine and generate sound file if needed
    void init();
    
    // Clean up audio engine
    void cleanup();
    
    // Play the popup/beep sound
    void playPopupSound();
    
    // Check if audio is initialized
    bool isInitialized();
}

#endif // MENU_AUDIO_H
