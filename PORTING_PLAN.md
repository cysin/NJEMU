# NJEMU Linux/SDL2 Porting Plan

## Executive Summary

**Project**: Port NJEMU (PSP arcade emulator) from PSP to Linux with SDL2
**Target Systems**: CPS1, CPS2, MVS (Neo Geo), NCDZ (Neo Geo CDZ)
**Estimated Effort**: 15,000-18,000 lines of platform code
**Timeline**: 4-8 weeks (team) / 2-4 weeks (experienced developer)

**Key Challenges**:
- Replace PSP GU (Graphics Unit) with SDL2/OpenGL
- Convert blocking audio calls to callback-based model
- Adapt PSP-specific file paths and I/O
- Port networking from proprietary AdHoc to BSD sockets
- Maintain clean separation between emulation core and platform layer

---

## Architecture Overview

```
┌─────────────────────────────────────────────────┐
│  NJEMU Core (Unchanged)                         │
│  - CPU emulation (M68000, Z80)                  │
│  - System emulation (CPS1/2, MVS, NCDZ)         │
│  - Sound chips (QSound, YM2151, YM2610)         │
└────────────┬────────────────────────────────────┘
             │
┌────────────▼────────────────────────────────────┐
│  Platform Layer (TO BE REPLACED)                │
├─────────────────────────────────────────────────┤
│  PSP → Linux/SDL2                               │
│  - Graphics: GU → SDL2/OpenGL                   │
│  - Audio: sceAudio → SDL2 Audio/PulseAudio      │
│  - Input: sceCtrl → SDL2 Gamepad                │
│  - File I/O: sceIo → POSIX                      │
│  - Timing: sceRtc → clock_gettime               │
│  - Threading: sceKernel → pthread               │
│  - Networking: sceNet → BSD sockets             │
└─────────────────────────────────────────────────┘
```

---

## Phase 1: Infrastructure & Build System Setup

**Priority**: CRITICAL
**Effort**: 2-3 days
**Dependencies**: None

### 1.1 Create Linux Build System

**Tasks**:
- [ ] Create new Makefile for Linux builds alongside PSP Makefile
  - **File**: `Makefile.linux` or modify existing `Makefile`
  - **Remove**: PSP SDK dependencies (`psp-config`, `build.mak`)
  - **Add**: SDL2, OpenGL, pthread, libpng dependencies

**Build Configuration**:
```makefile
# Compiler
CC = gcc
CXX = g++
CFLAGS = -std=c99 -O2 -Wall -Wextra -I./src -I./src/include
CXXFLAGS = -std=c++11 -O2 -Wall -Wextra -I./src -I./src/include

# Platform defines
DEFINES = -DLINUX -DUSE_SDL2

# Libraries
LIBS = -lm -lSDL2 -lGL -lpthread -lpng -lz
# Optional: -lpulse, -lsamplerate, -lmad (already in tree)

# Targets per system
BUILD_CPS1 = -DBUILD_CPS1PSP=1
BUILD_CPS2 = -DBUILD_CPS2PSP=1
BUILD_MVS = -DBUILD_MVSPSP=1
BUILD_NCDZ = -DBUILD_NCDZPSP=1

# Output
TARGET = njemu-cps2  # or cps1, mvs, ncdz
```

### 1.2 Create Platform Abstraction Directory

**Tasks**:
- [ ] Create `src/linux/` directory structure mirroring `src/psp/`
- [ ] Create stub headers for compatibility during migration
  - `src/linux/video.h`
  - `src/linux/sound.h`
  - `src/linux/input.h`
  - `src/linux/filer.h`
  - `src/linux/config.h`
  - `src/linux/ticker.h`

### 1.3 Update Include Paths and Conditional Compilation

**Files to modify**:
- [ ] `src/include/osd_cpu.h` - Add Linux type definitions
  ```c
  #ifdef LINUX
  #include <stdint.h>
  typedef uint64_t UINT64;
  typedef int64_t  INT64;
  typedef uint32_t UINT32;
  typedef int32_t  INT32;
  // ... etc
  #endif
  ```

- [ ] `src/emucfg.h` - Add Linux platform defines
  ```c
  #ifdef LINUX
  #define ALIGN_DATA      __attribute__((aligned(16)))
  #define ALIGN_PSPDATA   __attribute__((aligned(16)))
  #define MEM_ALIGN       16
  #endif
  ```

### 1.4 Dependency Installation Guide

**Document Required Libraries**:
```bash
# Ubuntu/Debian
sudo apt-get install build-essential libsdl2-dev libgl1-mesa-dev \
  libpng-dev libpulse-dev libsamplerate0-dev zlib1g-dev

# Fedora/RHEL
sudo dnf install gcc gcc-c++ SDL2-devel mesa-libGL-devel \
  libpng-devel pulseaudio-libs-devel libsamplerate-devel zlib-devel

# Arch Linux
sudo pacman -S base-devel sdl2 mesa libpng libpulse libsamplerate zlib
```

**Deliverables**: `BUILD_LINUX.md` with setup instructions

---

## Phase 2: Core Platform Abstraction Layer

**Priority**: CRITICAL
**Effort**: 3-4 days
**Dependencies**: Phase 1

### 2.1 Timing System (src/linux/ticker.c)

**Replaces**: `src/psp/ticker.c` (uses `sceRtc*`)
**Effort**: LOW (1-2 hours)

**Implementation**:
```c
// src/linux/ticker.c
#include <time.h>
#include <stdint.h>

uint64_t ticker(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000ULL;
}

uint64_t ticker_tick_resolution(void) {
    return 1000000ULL;  // 1 microsecond
}
```

**Files**:
- [ ] Create `src/linux/ticker.c`
- [ ] Create `src/linux/ticker.h`
- [ ] Update `src/psp/ticker.h` to be platform-agnostic or use ifdefs

### 2.2 Threading Abstraction (src/linux/thread.c)

**Replaces**: PSP `sceKernel*` threading
**Effort**: MEDIUM (4-6 hours)

**Implementation**:
```c
// src/linux/thread.h
#include <pthread.h>

typedef pthread_t thread_t;
typedef pthread_mutex_t mutex_t;
typedef pthread_cond_t cond_t;

// Thread functions
thread_t thread_create(void *(*func)(void*), void *arg);
void thread_join(thread_t tid);
void thread_exit(void);
void thread_sleep(uint64_t microseconds);

// Mutex functions
mutex_t* mutex_create(void);
void mutex_lock(mutex_t *m);
void mutex_unlock(mutex_t *m);
void mutex_destroy(mutex_t *m);

// Condition variables
cond_t* cond_create(void);
void cond_wait(cond_t *c, mutex_t *m);
void cond_signal(cond_t *c);
void cond_destroy(cond_t *c);
```

**Files**:
- [ ] Create `src/linux/thread.c`
- [ ] Create `src/linux/thread.h`
- [ ] Replace all `sceKernel*` calls in sound/network threads

### 2.3 Main Entry Point (src/linux/main.c)

**Replaces**: `src/psp/psp.c`
**Effort**: MEDIUM (6-8 hours)

**Implementation**:
```c
// src/linux/main.c
#include <SDL2/SDL.h>
#include "video.h"
#include "sound.h"
#include "input.h"
#include "filer.h"

int main(int argc, char *argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    // Initialize subsystems
    video_init();
    sound_init_system();  // Initialize SDL audio subsystem
    input_init();

    // Enter file browser / ROM selection
    file_browser();

    // Cleanup
    input_shutdown();
    sound_shutdown_system();
    video_shutdown();
    SDL_Quit();

    return 0;
}
```

**Tasks**:
- [ ] Create `src/linux/main.c`
- [ ] Remove PSP module macros (`PSP_MODULE_INFO`, etc.)
- [ ] Remove PSP callback registration
- [ ] Remove CPU clock frequency control
- [ ] Implement graceful shutdown handling (Ctrl+C, window close)

---

## Phase 3: Graphics Pipeline Migration

**Priority**: CRITICAL
**Effort**: 10-12 days
**Dependencies**: Phase 2

### 3.1 Video Subsystem Architecture (src/linux/video.c)

**Replaces**: `src/psp/video.c` (~500 lines of GU code)
**Effort**: HIGH

#### 3.1.1 SDL2 Window and Renderer Setup

**Implementation Strategy**:
```c
// src/linux/video.h
#include <SDL2/SDL.h>
#include <GL/gl.h>

// Screen configuration
#define SCR_WIDTH   480
#define SCR_HEIGHT  272
#define WINDOW_SCALE 2  // Default 2x scaling (960x544)

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *screen_texture;

    // Framebuffers (software or GL textures)
    void *draw_frame;
    void *show_frame;
    void *work_frame;
    void *tex_frame;

    int current_buffer;
    int pixel_format;  // 16-bit or 32-bit
    int vsync_enabled;
} video_state_t;

extern video_state_t video_state;

// API functions
int video_init(void);
void video_shutdown(void);
void video_flip_screen(int vsync);
void video_clear_screen(void);
void video_set_mode(int width, int height, int bpp);
```

**Tasks**:
- [ ] Create SDL window with configurable resolution
- [ ] Implement software rendering path (SDL_Texture + SDL_UpdateTexture)
- [ ] Implement OpenGL rendering path (optional, for hardware acceleration)
- [ ] Handle window resizing and fullscreen toggle
- [ ] Implement VSync control

#### 3.1.2 Framebuffer Management

**PSP Approach**:
- Uses VRAM addresses directly (0x44000000 base)
- Triple buffering with `show_frame`, `draw_frame`, `work_frame`
- Direct GU command list submission

**Linux/SDL2 Approach**:
```c
// Allocate framebuffers in system RAM
#define BUF_WIDTH 512
#define FRAMESIZE16 (BUF_WIDTH * SCR_HEIGHT * 2)
#define FRAMESIZE32 (BUF_WIDTH * SCR_HEIGHT * 4)

void video_init_buffers(int bpp) {
    size_t framesize = (bpp == 32) ? FRAMESIZE32 : FRAMESIZE16;

    draw_frame = aligned_alloc(16, framesize);
    show_frame = aligned_alloc(16, framesize);
    work_frame = aligned_alloc(16, framesize);
    tex_frame  = aligned_alloc(16, framesize);

    memset(draw_frame, 0, framesize);
    memset(show_frame, 0, framesize);
    memset(work_frame, 0, framesize);
    memset(tex_frame, 0, framesize);
}

void video_flip_screen(int vsync) {
    // Swap buffers
    void *temp = show_frame;
    show_frame = draw_frame;
    draw_frame = temp;

    // Upload to SDL texture
    SDL_UpdateTexture(video_state.screen_texture, NULL,
                     show_frame, BUF_WIDTH * (bpp/8));

    // Render
    SDL_RenderClear(video_state.renderer);
    SDL_RenderCopy(video_state.renderer, video_state.screen_texture, NULL, NULL);
    SDL_RenderPresent(video_state.renderer);
}
```

**Tasks**:
- [ ] Replace VRAM allocation with heap allocation
- [ ] Implement double/triple buffering in system RAM
- [ ] Create texture upload path for SDL
- [ ] Handle pixel format conversion (BGR vs RGB, etc.)

#### 3.1.3 Sprite/Tile Rendering Replacement

**PSP GU Approach** (src/cps1/sprite.c, src/cps2/sprite.c):
- Uses `sceGuDrawArray()` with vertex lists
- Hardware texture mapping and blending
- CLUT (color lookup table) support

**Linux Approach - Option A: Software Blitting**:
```c
// Replace GU sprite drawing with direct pixel manipulation
void blit_sprite_software(int x, int y, int w, int h,
                         uint16_t *sprite_data, uint16_t *palette) {
    uint16_t *framebuffer = (uint16_t*)draw_frame;

    for (int py = 0; py < h; py++) {
        for (int px = 0; px < w; px++) {
            int dst_x = x + px;
            int dst_y = y + py;

            if (dst_x < 0 || dst_x >= SCR_WIDTH ||
                dst_y < 0 || dst_y >= SCR_HEIGHT) continue;

            uint8_t color_index = sprite_data[py * w + px];
            if (color_index == 0) continue;  // Transparent

            uint16_t color = palette[color_index];
            framebuffer[dst_y * BUF_WIDTH + dst_x] = color;
        }
    }
}
```

**Linux Approach - Option B: OpenGL Acceleration** (Recommended):
```c
// Use OpenGL for hardware-accelerated sprite rendering
// Convert GU vertex lists to OpenGL VBOs
void blit_sprite_opengl(sprite_t *sprite) {
    glBindTexture(GL_TEXTURE_2D, sprite->texture_id);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(sprite->x, sprite->y);
        glTexCoord2f(1, 0); glVertex2f(sprite->x + sprite->w, sprite->y);
        glTexCoord2f(1, 1); glVertex2f(sprite->x + sprite->w, sprite->y + sprite->h);
        glTexCoord2f(0, 1); glVertex2f(sprite->x, sprite->y + sprite->h);
    glEnd();
}
```

**Tasks**:
- [ ] Analyze sprite rendering in each system (CPS1/CPS2/MVS/NCDZ)
- [ ] Choose rendering approach (software, OpenGL, or hybrid)
- [ ] Replace `sceGuDrawArray()` calls with chosen method
- [ ] Implement palette/CLUT conversion
- [ ] Handle transparency and blending modes
- [ ] Optimize for performance (batch drawing, texture atlases)

#### 3.1.4 System-Specific Video Adaptations

**CPS1 (src/cps1/vidhrdw.c, src/cps1/sprite.c)**:
- [ ] Replace GU blit functions in `cps1_render_layer()`
- [ ] Adapt `cps1_screenrefresh()` to use new video API
- [ ] Maintain star field rendering

**CPS2 (src/cps2/vidhrdw.c, src/cps2/sprite.c)**:
- [ ] Replace GU sprite batching with new approach
- [ ] Adapt raster partial refresh to new pipeline
- [ ] Handle priority masking in software or shaders

**MVS/NCDZ (src/mvs/vidhrdw.c, src/ncdz/vidhrdw.c)**:
- [ ] Replace Neo Geo sprite zoom/clip hardware emulation
- [ ] Adapt fix layer rendering
- [ ] Handle raster effects

**Files to Modify**:
```
src/cps1/vidhrdw.c
src/cps1/sprite.c
src/cps2/vidhrdw.c
src/cps2/sprite.c
src/mvs/vidhrdw.c
src/ncdz/vidhrdw.c
src/psp/video.c → src/linux/video.c
```

### 3.2 Update Screen Management (src/emumain.c)

**Tasks**:
- [ ] Replace `sceDisplayWaitVblankStart()` with SDL timing
- [ ] Update `video_flip_screen()` calls to use new API
- [ ] Maintain UI overlay rendering (FPS, volume, battery)
- [ ] Remove PSP battery status (or query Linux battery via /sys)

---

## Phase 4: Audio System Migration

**Priority**: CRITICAL
**Effort**: 5-7 days
**Dependencies**: Phase 2

### 4.1 Audio Subsystem Architecture (src/linux/sound.c)

**Replaces**: `src/psp/sound.c`
**Effort**: MEDIUM-HIGH

#### 4.1.1 SDL2 Audio Implementation

**PSP Model** (Blocking):
```c
// PSP sound thread pulls samples and blocks until consumed
void sound_update_thread(void) {
    while (sound_thread_active) {
        (*sound->update)(sound_buffer[flip]);
        sceAudioSRCOutputBlocking(volume, sound_buffer[flip]);
        flip ^= 1;
    }
}
```

**SDL2 Model** (Callback):
```c
// SDL2 audio callback is invoked by audio driver when buffer needed
void audio_callback(void *userdata, Uint8 *stream, int len) {
    sound_state_t *state = (sound_state_t*)userdata;

    if (!state->enabled) {
        memset(stream, 0, len);
        return;
    }

    // Call emulator's sound update function
    (*sound->update)(state->mix_buffer);

    // Copy to SDL stream
    memcpy(stream, state->mix_buffer, len);
}

int sound_init_sdl(int sample_rate, int channels, int samples) {
    SDL_AudioSpec want, have;

    SDL_zero(want);
    want.freq = sample_rate;
    want.format = AUDIO_S16SYS;  // 16-bit signed
    want.channels = channels;
    want.samples = samples;
    want.callback = audio_callback;
    want.userdata = &sound_state;

    audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (audio_device == 0) {
        fprintf(stderr, "SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
        return -1;
    }

    SDL_PauseAudioDevice(audio_device, 0);  // Start playback
    return 0;
}
```

**Tasks**:
- [ ] Create `src/linux/sound.c` with SDL2 audio backend
- [ ] Convert from thread-based pulling to callback-based pushing
- [ ] Handle thread safety (mutex for sound state)
- [ ] Implement volume control (SDL_MixAudio)
- [ ] Support multiple sample rates (44.1kHz, 48kHz)

#### 4.1.2 Alternative: PulseAudio Direct Access

**For lower latency** (optional):
```c
// Use PulseAudio simple API for direct control
#include <pulse/simple.h>
#include <pulse/error.h>

pa_simple *audio_stream;

int sound_init_pulse(void) {
    pa_sample_spec ss = {
        .format = PA_SAMPLE_S16LE,
        .rate = 44100,
        .channels = 2
    };

    audio_stream = pa_simple_new(NULL, "NJEMU", PA_STREAM_PLAYBACK,
                                 NULL, "Game Audio", &ss, NULL, NULL, NULL);
    return (audio_stream != NULL) ? 0 : -1;
}

void sound_output_pulse(int16_t *buffer, size_t samples) {
    pa_simple_write(audio_stream, buffer, samples * 4, NULL);
}
```

**Tasks**:
- [ ] Create alternative PulseAudio backend
- [ ] Add build-time option to choose SDL2 vs PulseAudio
- [ ] Maintain compatibility with existing `sound->update()` interface

#### 4.1.3 Sound Interface Adaptation (src/sound/sndintrf.c)

**Current Flow**:
```
sound_init()
  → Starts chip (QSound/YM2151/YM2610)
  → Sets sound->update callback
  → Starts PSP thread
```

**New Flow**:
```
sound_init()
  → Starts chip (QSound/YM2151/YM2610)  [UNCHANGED]
  → Sets sound->update callback          [UNCHANGED]
  → Starts SDL audio device              [NEW]
```

**Tasks**:
- [ ] Modify `sound_init()` to call SDL audio init instead of PSP thread
- [ ] Update `sound_exit()` to close SDL audio device
- [ ] Keep `sound_mute()`, `sound_reset()` unchanged
- [ ] Ensure resampling logic (`resample_stream`) works in callback context

**Files to Modify**:
```
src/psp/sound.c → src/linux/sound.c
src/psp/sound.h → src/linux/sound.h
src/sound/sndintrf.c (minor changes to init/shutdown)
```

### 4.2 MP3 Playback (NCDZ only)

**Replaces**: `src/psp/mp3.c`
**Effort**: LOW-MEDIUM

**Tasks**:
- [ ] Keep existing libmad decoder (already in `src/mp3/`)
- [ ] Replace PSP audio channel with SDL mixer channel
- [ ] Update `mp3_play()`, `mp3_stop()`, `mp3_pause()` to use SDL
- [ ] Handle MP3 decode thread with pthread

---

## Phase 5: Input System Migration

**Priority**: HIGH
**Effort**: 3-4 days
**Dependencies**: Phase 2

### 5.1 Input Subsystem (src/linux/input.c)

**Replaces**: `src/psp/input.c`
**Effort**: LOW-MEDIUM

#### 5.1.1 SDL2 Gamepad Implementation

**PSP Input**:
```c
SceCtrlData pad_data;
sceCtrlPeekBufferPositive(&pad_data, 1);
buttons = pad_data.Buttons;
analog_x = pad_data.Lx;
analog_y = pad_data.Ly;
```

**SDL2 Input**:
```c
SDL_GameController *controller;
SDL_Joystick *joystick;

void input_init(void) {
    // Initialize SDL gamepad subsystem
    SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);

    // Open first available controller
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            controller = SDL_GameControllerOpen(i);
            break;
        }
    }

    // Fallback to joystick if no gamepad detected
    if (!controller && SDL_NumJoysticks() > 0) {
        joystick = SDL_JoystickOpen(0);
    }
}

uint32_t poll_gamepad(void) {
    uint32_t buttons = 0;

    if (controller) {
        // Read controller buttons
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A))
            buttons |= PSP_CTRL_CROSS;
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B))
            buttons |= PSP_CTRL_CIRCLE;
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_X))
            buttons |= PSP_CTRL_SQUARE;
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_Y))
            buttons |= PSP_CTRL_TRIANGLE;

        // D-pad
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP))
            buttons |= PSP_CTRL_UP;
        // ... etc for other directions

        // Triggers/bumpers
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER))
            buttons |= PSP_CTRL_LTRIGGER;
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER))
            buttons |= PSP_CTRL_RTRIGGER;

        // Start/Select
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_START))
            buttons |= PSP_CTRL_START;
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_BACK))
            buttons |= PSP_CTRL_SELECT;

        // Analog sticks
        Sint16 lx = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
        Sint16 ly = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);

        // Convert from -32768..32767 to 0..255 (PSP range)
        analog_x = (lx + 32768) >> 8;
        analog_y = (ly + 32768) >> 8;

        // Synthesize D-pad from analog (like PSP does)
        if (analog_x < 0x40) buttons |= PSP_CTRL_LEFT;
        if (analog_x > 0xC0) buttons |= PSP_CTRL_RIGHT;
        if (analog_y < 0x40) buttons |= PSP_CTRL_UP;
        if (analog_y > 0xC0) buttons |= PSP_CTRL_DOWN;
    }

    return buttons;
}
```

#### 5.1.2 Keyboard Support (Essential for Linux)

**Tasks**:
- [ ] Add keyboard mapping for players without gamepad
- [ ] Default keyboard layout:
  ```
  Arrow Keys: D-Pad
  Z/X/A/S: Cross/Circle/Square/Triangle
  Q/W: L/R triggers
  Enter: Start
  Right Shift: Select
  ESC: Menu
  F5-F9: Save states
  F12: Screenshot
  ```

**Implementation**:
```c
uint32_t poll_keyboard(void) {
    uint32_t buttons = 0;
    const Uint8 *keystate = SDL_GetKeyboardState(NULL);

    if (keystate[SDL_SCANCODE_UP])    buttons |= PSP_CTRL_UP;
    if (keystate[SDL_SCANCODE_DOWN])  buttons |= PSP_CTRL_DOWN;
    if (keystate[SDL_SCANCODE_LEFT])  buttons |= PSP_CTRL_LEFT;
    if (keystate[SDL_SCANCODE_RIGHT]) buttons |= PSP_CTRL_RIGHT;

    if (keystate[SDL_SCANCODE_Z])     buttons |= PSP_CTRL_CROSS;
    if (keystate[SDL_SCANCODE_X])     buttons |= PSP_CTRL_CIRCLE;
    if (keystate[SDL_SCANCODE_A])     buttons |= PSP_CTRL_SQUARE;
    if (keystate[SDL_SCANCODE_S])     buttons |= PSP_CTRL_TRIANGLE;

    if (keystate[SDL_SCANCODE_Q])     buttons |= PSP_CTRL_LTRIGGER;
    if (keystate[SDL_SCANCODE_W])     buttons |= PSP_CTRL_RTRIGGER;

    if (keystate[SDL_SCANCODE_RETURN]) buttons |= PSP_CTRL_START;
    if (keystate[SDL_SCANCODE_RSHIFT]) buttons |= PSP_CTRL_SELECT;

    return buttons;
}
```

#### 5.1.3 Input Configuration System

**Tasks**:
- [ ] Create input configuration file (`~/.njemu/input.cfg`)
- [ ] Support remapping buttons
- [ ] Support multiple controllers (player 1/2)
- [ ] Hot-plug detection (SDL_CONTROLLERDEVICEADDED event)

**Files to Create/Modify**:
```
src/psp/input.c → src/linux/input.c
src/psp/input.h → src/linux/input.h
Add: src/linux/input_config.c (optional)
```

### 5.2 Menu Navigation Update

**Tasks**:
- [ ] Ensure `pad_update()`, `pad_pressed()`, etc. work with new input
- [ ] Update menu system to handle keyboard and gamepad
- [ ] Add quit confirmation (Esc key or guide button)

---

## Phase 6: File I/O & Path Handling

**Priority**: HIGH
**Effort**: 3-4 days
**Dependencies**: Phase 2

### 6.1 File I/O Abstraction (src/linux/fileio.c)

**Replaces**: All `sceIo*` calls throughout codebase
**Effort**: MEDIUM

#### 6.1.1 Create POSIX Wrapper Layer

**Strategy**: Create wrapper functions that match PSP API signatures

```c
// src/linux/fileio.h
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

// File open flags (PSP-compatible)
#define PSP_O_RDONLY   O_RDONLY
#define PSP_O_WRONLY   O_WRONLY
#define PSP_O_RDWR     O_RDWR
#define PSP_O_CREAT    O_CREAT
#define PSP_O_APPEND   O_APPEND
#define PSP_O_TRUNC    O_TRUNC

// Seek constants
#define PSP_SEEK_SET   SEEK_SET
#define PSP_SEEK_CUR   SEEK_CUR
#define PSP_SEEK_END   SEEK_END

// File operations
static inline int sceIoOpen(const char *path, int flags, int mode) {
    return open(path, flags, 0644);
}

static inline int sceIoClose(int fd) {
    return close(fd);
}

static inline ssize_t sceIoRead(int fd, void *buf, size_t size) {
    return read(fd, buf, size);
}

static inline ssize_t sceIoWrite(int fd, const void *buf, size_t size) {
    return write(fd, buf, size);
}

static inline off_t sceIoLseek(int fd, off_t offset, int whence) {
    return lseek(fd, offset, whence);
}

static inline int sceIoRemove(const char *path) {
    return unlink(path);
}

static inline int sceIoRename(const char *old, const char *new) {
    return rename(old, new);
}

static inline int sceIoMkdir(const char *path, int mode) {
    return mkdir(path, 0755);
}
```

**Directory Operations**:
```c
// Directory entry structure
typedef struct {
    struct stat d_stat;
    char d_name[256];
    int valid;
} SceIoDirent;

typedef struct {
    DIR *dir;
    SceIoDirent current;
} SceIoDir;

static SceIoDir dirs[32];  // Max open directories
static int dir_count = 0;

int sceIoDopen(const char *path) {
    if (dir_count >= 32) return -1;

    DIR *d = opendir(path);
    if (!d) return -1;

    dirs[dir_count].dir = d;
    dirs[dir_count].current.valid = 0;
    return dir_count++;
}

int sceIoDread(int fd, SceIoDirent *dirent) {
    if (fd < 0 || fd >= dir_count) return -1;

    struct dirent *entry = readdir(dirs[fd].dir);
    if (!entry) return 0;

    strncpy(dirent->d_name, entry->d_name, 255);
    dirent->d_name[255] = '\0';

    // Get file stats
    char full_path[512];
    // Build full path and stat it
    // ... stat call here

    return 1;
}

int sceIoDclose(int fd) {
    if (fd < 0 || fd >= dir_count) return -1;
    closedir(dirs[fd].dir);
    return 0;
}
```

**Tasks**:
- [ ] Create `src/linux/fileio.h` with wrappers
- [ ] Option A: Use inline wrappers in header (recommended)
- [ ] Option B: Create `src/common/fileio.h` with platform ifdefs
- [ ] Test all file operations (cache, state, config)

#### 6.1.2 Path Translation Layer

**PSP Paths** → **Linux Paths**:
```c
// src/linux/paths.c
#include <stdlib.h>
#include <string.h>
#include <pwd.h>

static char base_path[512] = {0};
static char rom_path[512] = {0};

void paths_init(void) {
    // Get user home directory
    const char *home = getenv("HOME");
    if (!home) {
        struct passwd *pw = getpwuid(getuid());
        home = pw->pw_dir;
    }

    // Base data directory: ~/.njemu or ~/.local/share/njemu
    snprintf(base_path, sizeof(base_path), "%s/.njemu", home);

    // Create directories if they don't exist
    mkdir(base_path, 0755);

    char temp[512];
    snprintf(temp, sizeof(temp), "%s/screenshots", base_path);
    mkdir(temp, 0755);

    snprintf(temp, sizeof(temp), "%s/states", base_path);
    mkdir(temp, 0755);

    snprintf(temp, sizeof(temp), "%s/config", base_path);
    mkdir(temp, 0755);

    snprintf(temp, sizeof(temp), "%s/cache", base_path);
    mkdir(temp, 0755);

    snprintf(temp, sizeof(temp), "%s/nvram", base_path);
    mkdir(temp, 0755);

    snprintf(temp, sizeof(temp), "%s/memcard", base_path);
    mkdir(temp, 0755);

    // ROM path: default to ./roms or /usr/share/njemu/roms
    const char *rom_env = getenv("NJEMU_ROM_PATH");
    if (rom_env) {
        strncpy(rom_path, rom_env, sizeof(rom_path)-1);
    } else {
        snprintf(rom_path, sizeof(rom_path), "%s/roms", base_path);
        mkdir(rom_path, 0755);
    }
}

const char* translate_path(const char *psp_path, char *out_buf, size_t bufsize) {
    if (strncmp(psp_path, "ms0:/", 5) == 0) {
        // Convert ms0:/ to base path
        snprintf(out_buf, bufsize, "%s/%s", base_path, psp_path + 5);
    } else if (strncmp(psp_path, "flash0:/", 8) == 0) {
        // System files - ignore or error
        out_buf[0] = '\0';
        return NULL;
    } else {
        // Relative path - assume ROM directory
        snprintf(out_buf, bufsize, "%s/%s", rom_path, psp_path);
    }

    return out_buf;
}

// Convenience functions
const char* get_screenshot_path(void) {
    static char path[512];
    snprintf(path, sizeof(path), "%s/screenshots", base_path);
    return path;
}

const char* get_state_path(void) {
    static char path[512];
    snprintf(path, sizeof(path), "%s/states", base_path);
    return path;
}

const char* get_config_path(void) {
    static char path[512];
    snprintf(path, sizeof(path), "%s/config", base_path);
    return path;
}

const char* get_rom_path(void) {
    return rom_path;
}
```

**Tasks**:
- [ ] Create `src/linux/paths.c` and `src/linux/paths.h`
- [ ] Update all hardcoded "ms0:/" paths throughout codebase
- [ ] Support environment variables for custom paths
- [ ] Create XDG Base Directory compliant version (optional)

**Files Using Path Strings** (search for "ms0:"):
```
src/psp/filer.c
src/psp/png.c
src/psp/config.c
src/common/cache.c
src/common/state.c
src/mvs/memintrf.c (memcard, nvram)
src/ncdz/ncdz.c (CD images, IPL.TXT)
```

### 6.2 File Browser Update (src/linux/filer.c)

**Replaces**: `src/psp/filer.c`
**Effort**: MEDIUM

**Tasks**:
- [ ] Replace PSP directory enumeration with POSIX `opendir/readdir`
- [ ] Update ROM scanning logic to use new paths
- [ ] Keep existing UI code if possible, or simplify for terminal
- [ ] Option: Create GTK+ or Qt file picker dialog (advanced)

---

## Phase 7: UI System Adaptation

**Priority**: MEDIUM
**Effort**: 5-6 days
**Dependencies**: Phase 3 (Graphics)

### 7.1 UI Rendering (src/linux/ui*.c)

**Files**:
```
src/psp/ui.c → src/linux/ui.c
src/psp/ui_draw.c → src/linux/ui_draw.c
src/psp/ui_text.c → src/linux/ui_text.c
src/psp/ui_menu.c → src/linux/ui_menu.c
```

**Effort**: MEDIUM

#### 7.1.1 Text Rendering

**PSP Approach**: Custom font rendering to GU framebuffer
**Linux Approach**: Keep custom font, render to SDL framebuffer

**Tasks**:
- [ ] Keep existing font data (`src/psp/font/`)
- [ ] Update `ui_text_draw()` to write to SDL framebuffer instead of GU
- [ ] Support 16-bit and 32-bit modes
- [ ] Optional: Add TrueType font support via SDL_ttf

#### 7.1.2 Menu System

**Tasks**:
- [ ] Update menu rendering to use SDL primitives
- [ ] Replace GU rectangle/line drawing with SDL_Rect/SDL_RenderDrawLine
- [ ] Keep existing menu logic intact
- [ ] Update popup messages (FPS, volume, etc.)

#### 7.1.3 Battery/Power Display

**PSP**: Shows battery percentage and charging status
**Linux**: Optional - can query laptop battery via `/sys/class/power_supply/`

**Tasks**:
- [ ] Remove battery display or make it Linux-aware
- [ ] Keep FPS counter and volume display

### 7.2 Wallpaper System (Optional)

**File**: `src/psp/wallpaper.c`

**Tasks**:
- [ ] Port wallpaper loading to SDL (if keeping 32-bit mode)
- [ ] Or remove wallpaper feature entirely (simplify)

---

## Phase 8: System Integration & Testing

**Priority**: CRITICAL
**Effort**: 7-10 days
**Dependencies**: Phases 1-7

### 8.1 Incremental System Testing

#### 8.1.1 CPS1 First (Simplest System)

**Rationale**: CPS1 has no raster effects, simpler sprite system

**Tasks**:
- [ ] Compile CPS1 build: `make BUILD_CPS1PSP=1`
- [ ] Test ROM loading (Street Fighter II, etc.)
- [ ] Verify CPU emulation runs (M68000 + Z80)
- [ ] Debug video output (sprites, backgrounds, palettes)
- [ ] Debug audio output (YM2151 or QSound)
- [ ] Test input (player controls)
- [ ] Test save states
- [ ] Test screenshots
- [ ] Profile performance (should run full speed on modern PC)

**Common Issues**:
- Incorrect framebuffer stride (BUF_WIDTH vs SCR_WIDTH)
- Palette/color format mismatches (BGR vs RGB)
- Audio buffer underruns (adjust buffer size)
- Input lag (use proper timing)

#### 8.1.2 CPS2 Second

**Additional Complexity**: Raster interrupts, larger ROMs, cache system

**Tasks**:
- [ ] Test with cache-heavy games (Marvel vs Capcom, etc.)
- [ ] Verify raster effects work
- [ ] Test partial screen refresh
- [ ] Validate cache file loading
- [ ] Profile performance (may need optimization)

#### 8.1.3 MVS Third

**Additional Complexity**: Neo Geo sprite hardware, memcard/NVRAM

**Tasks**:
- [ ] Test BIOS loading (Unibios, etc.)
- [ ] Verify large sprite handling
- [ ] Test memcard saves
- [ ] Validate parent/clone ROM loading
- [ ] Test cache system

#### 8.1.4 NCDZ Last (Most Complex)

**Additional Complexity**: CD-ROM, CDDA audio, IPL validation

**Tasks**:
- [ ] Implement CD-ROM image loading
- [ ] Test CDDA audio (MP3 streaming)
- [ ] Validate IPL.TXT parsing
- [ ] Test loading screens
- [ ] Full integration test

### 8.2 Save State Testing

**Tasks**:
- [ ] Verify save state format is platform-independent
- [ ] Test save/load for all systems
- [ ] Ensure save states from PSP version are compatible (if desired)

### 8.3 Performance Optimization

**Tasks**:
- [ ] Profile CPU usage (use `perf`, `gprof`, or Valgrind)
- [ ] Identify bottlenecks (likely in sprite rendering)
- [ ] Optimize hot paths
- [ ] Consider OpenGL acceleration if software rendering is too slow
- [ ] Add frame skip logic if needed (already in emumain.c)

### 8.4 Memory Leak Testing

**Tasks**:
- [ ] Run Valgrind memcheck
- [ ] Fix any leaks in file loading, graphics, audio
- [ ] Ensure clean shutdown (no leaked resources)

---

## Phase 9: Networking (Optional)

**Priority**: MEDIUM (Optional feature)
**Effort**: 7-10 days
**Dependencies**: Phase 8

### 9.1 AdHoc Replacement (src/linux/network.c)

**Replaces**: `src/psp/adhoc.c`, `src/common/adhoc.c`
**Effort**: HIGH

#### 9.1.1 Network Architecture Decision

**Options**:
1. **UDP-based P2P** (closest to PSP AdHoc)
   - Use BSD sockets for direct peer-to-peer
   - Implement lobby/matchmaking server

2. **GGPO/Rollback Netcode** (modern approach)
   - Frame-perfect synchronization
   - Requires deeper integration

3. **libenet** (existing library)
   - Reliable UDP with connection management
   - Easier than raw sockets

**Recommendation**: Start with UDP P2P, add GGPO later if needed

#### 9.1.2 Implementation Tasks

**If implementing networking**:
- [ ] Replace `sceNet*` calls with BSD socket API
- [ ] Implement server/client connection logic
- [ ] Port input synchronization (`send_data`/`recv_data`)
- [ ] Test over LAN
- [ ] Add latency compensation
- [ ] Create lobby system (optional)

**If skipping networking** (recommended for initial port):
- [ ] Remove `#ifdef ADHOC` code blocks
- [ ] Remove networking from build system
- [ ] Focus on single-player experience

---

## Phase 10: Optimization & Polish

**Priority**: LOW
**Effort**: 3-5 days
**Dependencies**: Phase 8

### 10.1 Build System Polish

**Tasks**:
- [ ] Create CMake build system (optional, better than Makefile)
- [ ] Add install target (`make install`)
- [ ] Create desktop entry file (`.desktop` for Linux GUI)
- [ ] Package icon/logo
- [ ] Support out-of-tree builds

**Example CMakeLists.txt**:
```cmake
cmake_minimum_required(VERSION 3.10)
project(NJEMU C)

set(CMAKE_C_STANDARD 99)

# Find dependencies
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(PNG REQUIRED)
find_package(Threads REQUIRED)

# Source files
file(GLOB_RECURSE CORE_SOURCES
    src/cpu/*.c
    src/sound/*.c
    src/common/*.c
    src/zlib/*.c
)

# Platform sources
file(GLOB_RECURSE LINUX_SOURCES
    src/linux/*.c
)

# System-specific sources (use option to select)
option(BUILD_CPS1 "Build CPS1 emulator" ON)
option(BUILD_CPS2 "Build CPS2 emulator" OFF)
option(BUILD_MVS "Build MVS emulator" OFF)
option(BUILD_NCDZ "Build NCDZ emulator" OFF)

if(BUILD_CPS1)
    file(GLOB_RECURSE SYSTEM_SOURCES src/cps1/*.c)
    set(TARGET_NAME njemu-cps1)
    add_definitions(-DBUILD_CPS1PSP=1)
endif()

# Executable
add_executable(${TARGET_NAME} ${CORE_SOURCES} ${LINUX_SOURCES} ${SYSTEM_SOURCES})

# Include directories
target_include_directories(${TARGET_NAME} PRIVATE
    src
    src/include
    ${SDL2_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(${TARGET_NAME}
    ${SDL2_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${PNG_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    m
)

# Install
install(TARGETS ${TARGET_NAME} DESTINATION bin)
```

### 10.2 Documentation

**Tasks**:
- [ ] Update README.md for Linux
- [ ] Create INSTALL.md with dependency instructions
- [ ] Document configuration files
- [ ] Create user manual (controls, features)
- [ ] Add troubleshooting guide

### 10.3 Configuration System

**Tasks**:
- [ ] Create `~/.njemu/njemu.cfg` for global settings
- [ ] Support command-line arguments (`--fullscreen`, `--rom`, etc.)
- [ ] Add config GUI (optional, GTK+ or Qt)

**Example Command-Line**:
```bash
njemu-cps2 --rom ~/roms/mvsf.zip --fullscreen --filter linear
```

### 10.4 Quality of Life Features

**Tasks**:
- [ ] Add shader support (CRT effect, scanlines)
- [ ] Add rewind feature
- [ ] Add turbo/slow-motion hotkeys
- [ ] Add cheats database loader
- [ ] Add customizable video filters

---

## Testing Strategy

### Unit Testing
- [ ] Test file I/O on various Linux distros
- [ ] Test audio on different audio systems (PulseAudio, ALSA, PipeWire)
- [ ] Test input with various controllers

### Integration Testing
- [ ] Test each system (CPS1/CPS2/MVS/NCDZ) with 5+ games each
- [ ] Verify save states work across sessions
- [ ] Test performance on low-end hardware (Raspberry Pi, etc.)

### Compatibility Testing
- [ ] Ubuntu 20.04+, 22.04+
- [ ] Fedora 38+
- [ ] Arch Linux
- [ ] Debian 11+
- [ ] Raspberry Pi OS

---

## Risk Assessment

### High Risk Areas

1. **Graphics Performance**
   - **Risk**: Software rendering may be too slow
   - **Mitigation**: Implement OpenGL acceleration early
   - **Fallback**: Optimize software rendering, add frame skip

2. **Audio Latency**
   - **Risk**: Callback model may introduce audio lag
   - **Mitigation**: Use small buffer sizes, test multiple backends
   - **Fallback**: Add audio sync options

3. **Input Lag**
   - **Risk**: SDL event polling may add latency
   - **Mitigation**: Poll at correct frequency, minimize processing
   - **Fallback**: Add input lag compensation

### Medium Risk Areas

1. **File Path Compatibility**
   - **Risk**: Hardcoded PSP paths everywhere
   - **Mitigation**: Create robust path translation layer

2. **Memory Management**
   - **Risk**: PSP VRAM addressing assumes specific layout
   - **Mitigation**: Careful buffer management, thorough testing

3. **Build System Complexity**
   - **Risk**: Multiple build configurations hard to maintain
   - **Mitigation**: Use CMake, automate testing

---

## Success Criteria

### Minimum Viable Port (MVP)
- [ ] CPS1 and CPS2 systems work with 90%+ game compatibility
- [ ] Full-speed emulation on mid-range PC (i5/Ryzen 5 or better)
- [ ] Stable audio and video with no crashes
- [ ] Keyboard and gamepad input working
- [ ] Save states functional
- [ ] Screenshots working

### Full Port
- [ ] All 4 systems (CPS1/CPS2/MVS/NCDZ) working
- [ ] 95%+ game compatibility
- [ ] Optimized performance (full speed even on low-end hardware)
- [ ] Complete UI system
- [ ] Networking functional (if implemented)
- [ ] Comprehensive documentation

### Stretch Goals
- [ ] GTK+/Qt GUI frontend
- [ ] RetroArch core integration
- [ ] Shader support
- [ ] Debugger interface
- [ ] Raspberry Pi optimization

---

## Development Workflow Recommendation

### Week 1-2: Foundation
- Days 1-3: Phase 1 (Build system)
- Days 4-7: Phase 2 (Platform abstraction)
- Days 8-10: Begin Phase 3 (Graphics basics)

### Week 3-4: Core Systems
- Days 11-17: Complete Phase 3 (Graphics)
- Days 18-21: Phase 4 (Audio)
- Days 22-24: Phase 5 (Input)

### Week 5-6: Integration
- Days 25-28: Phase 6 (File I/O)
- Days 29-32: Phase 7 (UI)
- Days 33-38: Phase 8 (Testing & debugging)

### Week 7-8: Polish (Optional)
- Days 39-42: Phase 9 (Networking, if desired)
- Days 43-48: Phase 10 (Optimization & polish)

---

## Resources and References

### SDL2 Documentation
- https://wiki.libsdl.org/SDL2/FrontPage
- https://lazyfoo.net/tutorials/SDL/ (tutorials)

### Graphics
- OpenGL: https://learnopengl.com/
- SDL_gpu: https://github.com/grimfang4/sdl-gpu

### Audio
- PulseAudio: https://www.freedesktop.org/wiki/Software/PulseAudio/
- libsamplerate: http://www.mega-nerd.com/SRC/

### Tools
- Valgrind: https://valgrind.org/ (memory debugging)
- gprof: https://sourceware.org/binutils/docs/gprof/ (profiling)
- perf: https://perf.wiki.kernel.org/ (profiling)

---

## Appendix: File Modification Checklist

### Files to Create
```
src/linux/main.c
src/linux/video.c/h
src/linux/sound.c/h
src/linux/input.c/h
src/linux/ticker.c/h
src/linux/thread.c/h
src/linux/fileio.h
src/linux/paths.c/h
src/linux/filer.c/h
src/linux/ui.c/h
src/linux/ui_draw.c/h
src/linux/ui_text.c/h
src/linux/ui_menu.c/h
src/linux/config.c/h
Makefile.linux or CMakeLists.txt
BUILD_LINUX.md
```

### Files to Modify
```
src/include/osd_cpu.h (add Linux types)
src/emucfg.h (add Linux defines)
src/emumain.c (update screen management)
src/sound/sndintrf.c (update audio init)
src/common/cache.c (update file I/O)
src/common/state.c (update file I/O)
src/cps1/vidhrdw.c (update video calls)
src/cps1/sprite.c (replace GU rendering)
src/cps2/vidhrdw.c (update video calls)
src/cps2/sprite.c (replace GU rendering)
src/mvs/vidhrdw.c (update video calls)
src/ncdz/vidhrdw.c (update video calls)
All system-specific memintrf.c files (path handling)
```

### Files to Remove (PSP-only)
```
src/psp/SystemButtons.S
src/psp/sceCtrl_driver_371.S
src/psp/psp.c (replaced by src/linux/main.c)
src/makefiles/build.mak (PSP SDK specific)
```

---

## Contact and Support

**Original NJEMU Author**: [Check README for credits]
**Porting Project**: [Your details]
**Issue Tracker**: [GitHub/GitLab URL]

---

*This porting plan is a living document and should be updated as development progresses.*
