#include "MenuAudio.h"
#include "../miniaudio.h"
#include <cstdio>
#include <cmath>

// miniaudio implementation - only define once in the entire project
#define MINIAUDIO_IMPLEMENTATION
#include "../miniaudio.h"

namespace MenuAudio {

// Audio engine (initialized once)
static ma_engine* audioEngine = nullptr;
static bool audioInitialized = false;

// Sound file path (in same directory as settings.cfg)
static const char* SOUND_FILE = "asset/popup_beep.wav";

// Generate the popup beep WAV file if it doesn't exist
// Sound parameters - customize these values:
//   frequency: 200-2000 Hz recommended (440=A4, 523=C5, 659=E5, 880=A5)
//   duration: 50-200 ms for a short "blip"
static void generatePopupSound() {
    // Check if file already exists
    FILE* check = fopen(SOUND_FILE, "rb");
    if (check) {
        fclose(check);
        printf("[Audio] Sound file already exists: %s\n", SOUND_FILE);
        return;
    }
    
    printf("[Audio] Generating sound file: %s\n", SOUND_FILE);
    
    const int sampleRate = 44100;
    const int duration = 100;        // milliseconds
    const float frequency = 440.0f;  // Hz - mid frequency, pleasant
    const int numSamples = sampleRate * duration / 1000;
    
    // Create WAV file in asset directory (same as other game assets)
    FILE* f = fopen(SOUND_FILE, "wb");
    if (!f) {
        printf("[Audio] ERROR: Failed to create sound file: %s\n", SOUND_FILE);
        return;
    }
    
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
    
    printf("[Audio] Sound file created successfully (%d samples, %.0f Hz, %d ms)\n", 
           numSamples, frequency, duration);
}

void init() {
    generatePopupSound();
    
    // Initialize miniaudio engine
    audioEngine = new ma_engine();
    ma_result result = ma_engine_init(NULL, audioEngine);
    if (result != MA_SUCCESS) {
        printf("[Audio] ERROR: Failed to initialize audio engine (error %d)\n", result);
        delete audioEngine;
        audioEngine = nullptr;
        return;
    }
    
    audioInitialized = true;
    printf("[Audio] Audio engine initialized successfully (miniaudio)\n");
}

void cleanup() {
    if (audioEngine) {
        ma_engine_uninit(audioEngine);
        delete audioEngine;
        audioEngine = nullptr;
        audioInitialized = false;
        printf("[Audio] Audio engine shut down\n");
    }
}

void playPopupSound() {
    if (!audioInitialized || !audioEngine) {
        printf("[Audio] WARNING: Audio not initialized, cannot play sound\n");
        return;
    }
    
    printf("[Audio] Playing sound: %s\n", SOUND_FILE);
    
    ma_result result = ma_engine_play_sound(audioEngine, SOUND_FILE, NULL);
    if (result != MA_SUCCESS) {
        printf("[Audio] ERROR: Failed to play sound (error %d)\n", result);
    }
}

bool isInitialized() {
    return audioInitialized;
}

} // namespace MenuAudio
