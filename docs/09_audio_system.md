# 9. Audio System (miniaudio)

This document explains how sound effects are implemented using the miniaudio library.

## Why miniaudio?

- **Single header**: Just one file (`miniaudio.h`) - no complex build setup
- **Cross-platform**: Works on Windows, macOS, Linux
- **No dependencies**: Self-contained, no external libraries needed
- **Simple API**: Easy to use for basic sound effects
- **Public Domain**: No licensing concerns (MIT-0 / Unlicense)

> **Note:** miniaudio.h is automatically downloaded by CMake on first build. This keeps the repository size small and ensures GitHub correctly detects the project as C++ rather than C.

## Including miniaudio

```cpp
// In ONE .cpp file only:
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

// In other files, just:
#include "miniaudio.h"
```

The `MINIAUDIO_IMPLEMENTATION` define tells miniaudio to include the actual code.
Only define this once to avoid "multiple definition" linker errors.

## Audio Engine

### Initialization

```cpp
// In Menu class:
ma_engine audioEngine;
bool audioInitialized;

// In Menu constructor:
Menu::Menu() {
    audioInitialized = false;
    initAudio();
}

void Menu::initAudio() {
    ma_result result = ma_engine_init(NULL, &audioEngine);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to initialize audio engine" << std::endl;
        audioInitialized = false;
    } else {
        audioInitialized = true;
        std::cout << "Audio engine initialized" << std::endl;
    }
}
```

### Cleanup

```cpp
Menu::~Menu() {
    if (audioInitialized) {
        ma_engine_uninit(&audioEngine);
    }
}
```

## Playing Sounds

### Simple One-Shot Sound

```cpp
void Menu::playSound(const char* filename) {
    if (!audioInitialized) return;
    
    // Fire-and-forget playback
    ma_engine_play_sound(&audioEngine, filename, NULL);
}

// Usage:
playSound("asset/click.wav");
```

### Generated Popup Sound

Instead of loading a file, we generate a quick beep:

```cpp
void Menu::playPopupSound() {
    if (!audioInitialized) return;
    
    // Generate a short sine wave beep
    const int sampleRate = 44100;
    const float frequency = 440.0f;  // A4 note
    const float duration = 0.1f;     // 100ms
    const int numSamples = (int)(sampleRate * duration);
    
    // Create buffer
    std::vector<float> samples(numSamples);
    
    for (int i = 0; i < numSamples; i++) {
        float t = (float)i / sampleRate;
        
        // Sine wave
        float wave = sinf(2.0f * 3.14159f * frequency * t);
        
        // Envelope (fade in/out to avoid clicks)
        float envelope = 1.0f;
        if (i < numSamples / 10) {
            envelope = (float)i / (numSamples / 10);  // Fade in
        } else if (i > numSamples * 9 / 10) {
            envelope = (float)(numSamples - i) / (numSamples / 10);  // Fade out
        }
        
        samples[i] = wave * envelope * 0.3f;  // 0.3 = volume
    }
    
    // Play the buffer
    playBuffer(samples.data(), numSamples, sampleRate);
}
```

### Playing a Buffer

```cpp
void Menu::playBuffer(const float* samples, int numSamples, int sampleRate) {
    if (!audioInitialized) return;
    
    // Create audio buffer
    ma_audio_buffer_config config = ma_audio_buffer_config_init(
        ma_format_f32,      // 32-bit float samples
        1,                  // Mono (1 channel)
        numSamples,
        samples,
        NULL
    );
    
    ma_audio_buffer buffer;
    ma_result result = ma_audio_buffer_init(&config, &buffer);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to create audio buffer" << std::endl;
        return;
    }
    
    // Create sound from buffer
    ma_sound sound;
    result = ma_sound_init_from_data_source(&audioEngine, &buffer, 0, NULL, &sound);
    if (result == MA_SUCCESS) {
        ma_sound_start(&sound);
        
        // Wait for sound to finish
        while (ma_sound_is_playing(&sound)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        ma_sound_uninit(&sound);
    }
    
    ma_audio_buffer_uninit(&buffer);
}
```

## Sound Generation Math

### Sine Wave

```cpp
float wave = sinf(2.0f * PI * frequency * time);
```

- `sinf()` - sine function (returns -1 to +1)
- `2 * PI * frequency` - angular velocity
- `time` - current sample time in seconds

### Audio Envelope

An envelope shapes the volume over time:

```
Volume
  1 |   _____
    |  /     \
    | /       \
  0 |/         \____
    +----------------> Time
     Attack  Decay
```

```cpp
float envelope = 1.0f;

// Fade in (attack) - first 10%
if (sampleIndex < totalSamples * 0.1f) {
    envelope = sampleIndex / (totalSamples * 0.1f);
}
// Fade out (decay) - last 10%
else if (sampleIndex > totalSamples * 0.9f) {
    envelope = (totalSamples - sampleIndex) / (totalSamples * 0.1f);
}

float finalSample = wave * envelope;
```

### Different Waveforms

```cpp
// Sine wave - smooth, pure tone
float sine = sinf(2.0f * PI * freq * t);

// Square wave - harsh, buzzy
float square = (fmodf(t * freq, 1.0f) < 0.5f) ? 1.0f : -1.0f;

// Sawtooth wave - bright, sharp
float saw = 2.0f * fmodf(t * freq, 1.0f) - 1.0f;

// Triangle wave - softer than square
float tri = 4.0f * fabsf(fmodf(t * freq, 1.0f) - 0.5f) - 1.0f;
```

## Audio Format Basics

### Sample Rate

Number of samples per second:
- 44100 Hz - CD quality
- 48000 Hz - Standard for games
- 22050 Hz - Lower quality, smaller files

```cpp
const int sampleRate = 44100;  // Samples per second
float duration = 1.0f;         // 1 second
int numSamples = sampleRate * duration;  // 44100 samples
```

### Bit Depth

How precisely each sample is stored:
- 8-bit: 256 levels (-128 to 127)
- 16-bit: 65536 levels (-32768 to 32767)
- 32-bit float: Continuous (-1.0 to 1.0)

miniaudio uses `ma_format_f32` (32-bit float) for best quality.

### Channels

- Mono: 1 channel (simple)
- Stereo: 2 channels (left/right)

For simple UI sounds, mono is fine.

## When Sounds Play

### Settings Changes

```cpp
void Menu::handleKey(int key) {
    if (currentState == SETTINGS_CONTROLS) {
        if (key == GLFW_KEY_ENTER) {
            // Changed a setting
            playPopupSound();
        }
    }
}
```

### Menu Navigation

```cpp
void Menu::handleClick(float mouseX, float mouseY) {
    if (isButtonClicked(playButton)) {
        playPopupSound();
        currentState = PLAYING;
    }
}
```

### Settings Applied

```cpp
void Menu::applyPendingSettings() {
    settings = pendingSettings;
    playPopupSound();
    applyFeedbackTimer = 1.5f;
}
```

## Thread Safety

miniaudio handles audio in a separate thread internally. For simple fire-and-forget sounds, this is transparent. For more complex audio:

```cpp
// Lock if multiple threads access audio
std::mutex audioMutex;

void playThreadSafe(const char* file) {
    std::lock_guard<std::mutex> lock(audioMutex);
    ma_engine_play_sound(&audioEngine, file, NULL);
}
```

## Loading WAV Files

If you want to load sound files:

```cpp
void Menu::playWavSound(const char* filename) {
    if (!audioInitialized) return;
    
    // Simple one-shot playback
    ma_result result = ma_engine_play_sound(&audioEngine, filename, NULL);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to play sound: " << filename << std::endl;
    }
}

// Usage:
playWavSound("asset/jump.wav");
playWavSound("asset/death.wav");
playWavSound("asset/checkpoint.wav");
```

### Supported Formats

miniaudio supports:
- WAV
- MP3
- FLAC
- Vorbis (OGG)

## Volume Control

### Global Volume

```cpp
ma_engine_set_volume(&audioEngine, 0.5f);  // 50% volume
```

### Per-Sound Volume

```cpp
ma_sound sound;
ma_sound_init_from_file(&audioEngine, "file.wav", 0, NULL, NULL, &sound);
ma_sound_set_volume(&sound, 0.7f);  // 70% volume
ma_sound_start(&sound);
```

## Complete Audio Class Example

```cpp
class AudioManager {
private:
    ma_engine engine;
    bool initialized;
    
public:
    AudioManager() : initialized(false) {
        ma_result result = ma_engine_init(NULL, &engine);
        initialized = (result == MA_SUCCESS);
    }
    
    ~AudioManager() {
        if (initialized) {
            ma_engine_uninit(&engine);
        }
    }
    
    void playSound(const char* filename) {
        if (initialized) {
            ma_engine_play_sound(&engine, filename, NULL);
        }
    }
    
    void setVolume(float volume) {
        if (initialized) {
            ma_engine_set_volume(&engine, volume);
        }
    }
    
    void playBeep(float frequency, float duration) {
        if (!initialized) return;
        
        const int sampleRate = 44100;
        int numSamples = (int)(sampleRate * duration);
        std::vector<float> samples(numSamples);
        
        for (int i = 0; i < numSamples; i++) {
            float t = (float)i / sampleRate;
            float wave = sinf(2.0f * 3.14159f * frequency * t);
            
            // Simple envelope
            float env = 1.0f - (float)i / numSamples;  // Linear fade
            samples[i] = wave * env * 0.3f;
        }
        
        // Would need to play buffer here...
    }
};
```

## Common Issues

### No Sound on Linux

You may need PulseAudio or ALSA:

```bash
# Install PulseAudio
sudo apt install pulseaudio

# Or ALSA
sudo apt install alsa-utils libasound2-dev
```

### Sound Cuts Off

Ensure the engine lives long enough:

```cpp
// BAD - engine destroyed before sound finishes
void playBad() {
    ma_engine engine;
    ma_engine_init(NULL, &engine);
    ma_engine_play_sound(&engine, "file.wav", NULL);
    ma_engine_uninit(&engine);  // Cuts off sound!
}

// GOOD - engine is a class member
class AudioManager {
    ma_engine engine;  // Lives as long as the object
    // ...
};
```

### Multiple Definition Errors

Only define `MINIAUDIO_IMPLEMENTATION` in ONE file:

```cpp
// audio.cpp - only here!
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

// main.cpp - no define
#include "miniaudio.h"  // Just the header
```

## Next Steps

Continue to [Collision System](./10_collision_system.md) to learn about physics detection.
