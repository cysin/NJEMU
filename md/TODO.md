# NJEMU Linux/SDL2 Porting - Detailed TODO List

**Last Updated**: 2025-11-29
**Status**: Not Started
**Target**: Linux with SDL2

---

## 📋 PHASE 1: INFRASTRUCTURE & BUILD SYSTEM (Days 1-3)

### 1.1 Build System Setup

- [ ] Install required dependencies on development machine
  - [ ] SDL2 development libraries (`libsdl2-dev`)
  - [ ] OpenGL development libraries (`libgl1-mesa-dev`)
  - [ ] PNG library (`libpng-dev`)
  - [ ] PulseAudio development files (`libpulse-dev`)
  - [ ] Zlib development files (`zlib1g-dev`)
  - [ ] Build tools (`build-essential`, `gcc`, `make`)

- [ ] Create Linux build configuration
  - [ ] Create `Makefile.linux` in project root
  - [ ] Define compiler settings (gcc with C99)
  - [ ] Define include paths (`-I./src -I./src/include`)
  - [ ] Define library flags (`-lSDL2 -lGL -lpthread -lpng -lz -lm`)
  - [ ] Add Linux platform defines (`-DLINUX -DUSE_SDL2`)
  - [ ] Create separate targets for each system (CPS1/CPS2/MVS/NCDZ)
  - [ ] Add debug build configuration (`-g -O0`)
  - [ ] Add release build configuration (`-O2 -DNDEBUG`)

- [ ] Test initial build (won't link yet, just compile)
  - [ ] Try compiling existing source files
  - [ ] Identify compilation errors
  - [ ] Document PSP-specific functions that need replacement

### 1.2 Platform Abstraction Directory

- [ ] Create directory structure
  - [ ] Create `src/linux/` directory
  - [ ] Create `src/linux/include/` subdirectory (if needed)

- [ ] Create stub header files
  - [ ] Create `src/linux/video.h` (empty skeleton)
  - [ ] Create `src/linux/sound.h` (empty skeleton)
  - [ ] Create `src/linux/input.h` (empty skeleton)
  - [ ] Create `src/linux/ticker.h` (empty skeleton)
  - [ ] Create `src/linux/thread.h` (empty skeleton)
  - [ ] Create `src/linux/fileio.h` (empty skeleton)
  - [ ] Create `src/linux/filer.h` (empty skeleton)
  - [ ] Create `src/linux/config.h` (empty skeleton)
  - [ ] Create `src/linux/paths.h` (empty skeleton)
  - [ ] Create `src/linux/ui.h` (empty skeleton)

### 1.3 Update Core Headers

- [ ] Modify `src/include/osd_cpu.h`
  - [ ] Add `#ifdef LINUX` section
  - [ ] Define `UINT64` as `uint64_t`
  - [ ] Define `INT64` as `int64_t`
  - [ ] Define `UINT32` as `uint32_t`
  - [ ] Define `INT32` as `int32_t`
  - [ ] Define `UINT16` as `uint16_t`
  - [ ] Define `INT16` as `int16_t`
  - [ ] Define `UINT8` as `uint8_t`
  - [ ] Define `INT8` as `int8_t`
  - [ ] Include `<stdint.h>` for Linux

- [ ] Modify `src/emucfg.h`
  - [ ] Add `#ifdef LINUX` section
  - [ ] Define `ALIGN_DATA` as `__attribute__((aligned(16)))`
  - [ ] Define `ALIGN_PSPDATA` as `__attribute__((aligned(16)))`
  - [ ] Define `MEM_ALIGN` as `16`

### 1.4 Documentation

- [ ] Create `BUILD_LINUX.md`
  - [ ] Document dependency installation for Ubuntu/Debian
  - [ ] Document dependency installation for Fedora/RHEL
  - [ ] Document dependency installation for Arch Linux
  - [ ] Add build instructions
  - [ ] Add troubleshooting section

---

## 📋 PHASE 2: CORE PLATFORM ABSTRACTION (Days 4-7)

### 2.1 Timing System

- [ ] Create `src/linux/ticker.c`
  - [ ] Include `<time.h>` and `<stdint.h>`
  - [ ] Implement `ticker()` function using `clock_gettime(CLOCK_MONOTONIC)`
  - [ ] Convert timespec to microseconds (return `tv_sec * 1000000 + tv_nsec / 1000`)
  - [ ] Implement `ticker_tick_resolution()` returning `1000000ULL`
  - [ ] Add error handling for `clock_gettime()` failure

- [ ] Create `src/linux/ticker.h`
  - [ ] Declare `uint64_t ticker(void)`
  - [ ] Declare `uint64_t ticker_tick_resolution(void)`
  - [ ] Add include guards

- [ ] Test timing functions
  - [ ] Create simple test program to verify microsecond accuracy
  - [ ] Compare against `sceRtc` behavior on PSP (if reference available)

### 2.2 Threading Abstraction

- [ ] Create `src/linux/thread.h`
  - [ ] Include `<pthread.h>`
  - [ ] Define `thread_t` as `pthread_t`
  - [ ] Define `mutex_t` as `pthread_mutex_t`
  - [ ] Define `cond_t` as `pthread_cond_t`
  - [ ] Declare thread management functions
    - [ ] `thread_t thread_create(void *(*func)(void*), void *arg)`
    - [ ] `void thread_join(thread_t tid)`
    - [ ] `void thread_exit(void)`
    - [ ] `void thread_sleep(uint64_t microseconds)`
  - [ ] Declare mutex functions
    - [ ] `mutex_t* mutex_create(void)`
    - [ ] `void mutex_lock(mutex_t *m)`
    - [ ] `void mutex_unlock(mutex_t *m)`
    - [ ] `void mutex_destroy(mutex_t *m)`
  - [ ] Declare condition variable functions
    - [ ] `cond_t* cond_create(void)`
    - [ ] `void cond_wait(cond_t *c, mutex_t *m)`
    - [ ] `void cond_signal(cond_t *c)`
    - [ ] `void cond_destroy(cond_t *c)`

- [ ] Create `src/linux/thread.c`
  - [ ] Implement `thread_create()` using `pthread_create()`
  - [ ] Implement `thread_join()` using `pthread_join()`
  - [ ] Implement `thread_exit()` using `pthread_exit()`
  - [ ] Implement `thread_sleep()` using `usleep()` or `nanosleep()`
  - [ ] Implement `mutex_create()` allocating and initializing mutex
  - [ ] Implement `mutex_lock()` using `pthread_mutex_lock()`
  - [ ] Implement `mutex_unlock()` using `pthread_mutex_unlock()`
  - [ ] Implement `mutex_destroy()` using `pthread_mutex_destroy()`
  - [ ] Implement `cond_create()` allocating and initializing cond var
  - [ ] Implement `cond_wait()` using `pthread_cond_wait()`
  - [ ] Implement `cond_signal()` using `pthread_cond_signal()`
  - [ ] Implement `cond_destroy()` using `pthread_cond_destroy()`
  - [ ] Add error handling for all pthread functions

### 2.3 File I/O Abstraction

- [ ] Create `src/linux/fileio.h`
  - [ ] Include standard headers (`<fcntl.h>`, `<unistd.h>`, `<dirent.h>`)
  - [ ] Define PSP file flags as POSIX equivalents
    - [ ] `PSP_O_RDONLY` → `O_RDONLY`
    - [ ] `PSP_O_WRONLY` → `O_WRONLY`
    - [ ] `PSP_O_RDWR` → `O_RDWR`
    - [ ] `PSP_O_CREAT` → `O_CREAT`
    - [ ] `PSP_O_APPEND` → `O_APPEND`
    - [ ] `PSP_O_TRUNC` → `O_TRUNC`
  - [ ] Define PSP seek constants
    - [ ] `PSP_SEEK_SET` → `SEEK_SET`
    - [ ] `PSP_SEEK_CUR` → `SEEK_CUR`
    - [ ] `PSP_SEEK_END` → `SEEK_END`
  - [ ] Define `SceIoDirent` structure
  - [ ] Declare file operation wrappers (inline or regular functions)
    - [ ] `sceIoOpen(path, flags, mode)`
    - [ ] `sceIoClose(fd)`
    - [ ] `sceIoRead(fd, buf, size)`
    - [ ] `sceIoWrite(fd, buf, size)`
    - [ ] `sceIoLseek(fd, offset, whence)`
    - [ ] `sceIoRemove(path)`
    - [ ] `sceIoRename(old, new)`
    - [ ] `sceIoMkdir(path, mode)`
  - [ ] Declare directory operation wrappers
    - [ ] `sceIoDopen(path)`
    - [ ] `sceIoDread(fd, dirent)`
    - [ ] `sceIoDclose(fd)`

- [ ] Implement file I/O wrappers (if not using inline)
  - [ ] Choose implementation strategy (inline vs separate .c file)
  - [ ] If separate: Create `src/linux/fileio.c` with implementations
  - [ ] Test each function with simple file operations

### 2.4 Path Translation

- [ ] Create `src/linux/paths.h`
  - [ ] Declare `void paths_init(void)`
  - [ ] Declare `const char* translate_path(const char *psp_path, char *buf, size_t size)`
  - [ ] Declare `const char* get_base_path(void)`
  - [ ] Declare `const char* get_rom_path(void)`
  - [ ] Declare `const char* get_screenshot_path(void)`
  - [ ] Declare `const char* get_state_path(void)`
  - [ ] Declare `const char* get_config_path(void)`
  - [ ] Declare `const char* get_cache_path(void)`
  - [ ] Declare `const char* get_nvram_path(void)`
  - [ ] Declare `const char* get_memcard_path(void)`

- [ ] Create `src/linux/paths.c`
  - [ ] Include headers (`<stdlib.h>`, `<string.h>`, `<pwd.h>`, `<sys/stat.h>`)
  - [ ] Define static path buffers (base_path, rom_path, etc.)
  - [ ] Implement `paths_init()`
    - [ ] Get HOME directory from environment or `getpwuid()`
    - [ ] Set base_path to `~/.njemu/`
    - [ ] Create base directory if it doesn't exist
    - [ ] Create subdirectories: screenshots, states, config, cache, nvram, memcard
    - [ ] Get ROM path from `NJEMU_ROM_PATH` env var or default to `~/.njemu/roms`
    - [ ] Create ROM directory if using default path
  - [ ] Implement `translate_path()`
    - [ ] Handle `ms0:/` prefix → convert to base_path
    - [ ] Handle `flash0:/` prefix → return NULL or error
    - [ ] Handle relative paths → assume ROM directory
    - [ ] Handle absolute paths → pass through unchanged
  - [ ] Implement getter functions for each path type
  - [ ] Add logging/debug output for path translation (optional)

### 2.5 Main Entry Point

- [ ] Create `src/linux/main.c`
  - [ ] Include SDL2 headers (`<SDL2/SDL.h>`)
  - [ ] Include platform headers (video, sound, input, paths)
  - [ ] Implement `main(int argc, char *argv[])`
    - [ ] Parse command-line arguments (basic, expand later)
    - [ ] Initialize paths system (`paths_init()`)
    - [ ] Initialize SDL subsystems
      - [ ] `SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER)`
      - [ ] Check for errors and print to stderr
    - [ ] Initialize video subsystem (`video_init()`)
    - [ ] Initialize audio subsystem (`sound_init_system()`)
    - [ ] Initialize input subsystem (`input_init()`)
    - [ ] Call file browser / ROM selection (`file_browser()`)
    - [ ] Cleanup on exit
      - [ ] `input_shutdown()`
      - [ ] `sound_shutdown_system()`
      - [ ] `video_shutdown()`
      - [ ] `SDL_Quit()`
    - [ ] Return exit code
  - [ ] Add signal handlers for graceful shutdown (SIGINT, SIGTERM)
  - [ ] Add basic error logging

- [ ] Create `src/linux/main.h` (if needed)
  - [ ] Declare any global variables or helper functions

---

## 📋 PHASE 3: GRAPHICS PIPELINE (Days 8-20) ⚠️ BIGGEST TASK

### 3.1 Video Subsystem - Basic Setup

- [ ] Create `src/linux/video.h`
  - [ ] Include SDL2 headers
  - [ ] Define screen constants
    - [ ] `SCR_WIDTH` (480)
    - [ ] `SCR_HEIGHT` (272)
    - [ ] `BUF_WIDTH` (512 for alignment)
    - [ ] `WINDOW_SCALE` (default scaling factor)
  - [ ] Define pixel format constants
  - [ ] Define video state structure
    - [ ] SDL_Window pointer
    - [ ] SDL_Renderer pointer
    - [ ] SDL_Texture pointer(s)
    - [ ] Framebuffer pointers (draw_frame, show_frame, work_frame, tex_frame)
    - [ ] Current buffer index
    - [ ] Pixel format (16-bit or 32-bit)
    - [ ] VSync enabled flag
  - [ ] Declare video API functions
    - [ ] `int video_init(void)`
    - [ ] `void video_shutdown(void)`
    - [ ] `void video_flip_screen(int vsync)`
    - [ ] `void video_clear_screen(void)`
    - [ ] `void video_clear_frame(void *frame)`
    - [ ] `void video_set_mode(int width, int height, int bpp)`
    - [ ] `void* video_get_draw_frame(void)`
    - [ ] `void* video_get_show_frame(void)`
    - [ ] `void* video_get_work_frame(void)`
    - [ ] `void* video_get_tex_frame(void)`

- [ ] Create `src/linux/video.c`
  - [ ] Define global video state variable
  - [ ] Implement `video_init()`
    - [ ] Create SDL window
      - [ ] Calculate window size (SCR_WIDTH * scale, SCR_HEIGHT * scale)
      - [ ] Set window title ("NJEMU - Linux")
      - [ ] Set window flags (SDL_WINDOW_SHOWN, optionally RESIZABLE)
    - [ ] Create SDL renderer
      - [ ] Use hardware acceleration if available
      - [ ] Set VSync based on configuration
    - [ ] Determine pixel format (start with 16-bit RGB565)
    - [ ] Allocate framebuffers
      - [ ] Calculate frame size (BUF_WIDTH * SCR_HEIGHT * bytes_per_pixel)
      - [ ] Use `aligned_alloc(16, framesize)` or `posix_memalign()`
      - [ ] Allocate draw_frame, show_frame, work_frame, tex_frame
      - [ ] Zero all buffers with memset
    - [ ] Create SDL texture for screen
      - [ ] Use appropriate pixel format (SDL_PIXELFORMAT_RGB565 for 16-bit)
      - [ ] Set texture access to SDL_TEXTUREACCESS_STREAMING
      - [ ] Size: SCR_WIDTH x SCR_HEIGHT
    - [ ] Initialize video state
    - [ ] Return 0 on success, -1 on error
  - [ ] Implement `video_shutdown()`
    - [ ] Free framebuffers
    - [ ] Destroy SDL texture
    - [ ] Destroy SDL renderer
    - [ ] Destroy SDL window
  - [ ] Implement `video_flip_screen(int vsync)`
    - [ ] Swap draw_frame and show_frame pointers
    - [ ] Update SDL texture from show_frame
      - [ ] Use `SDL_UpdateTexture()` or `SDL_LockTexture()`
      - [ ] Handle stride correctly (BUF_WIDTH vs SCR_WIDTH)
    - [ ] Clear renderer
    - [ ] Copy texture to renderer
    - [ ] Present renderer (SDL_RenderPresent)
    - [ ] Handle VSync parameter (already set in renderer init)
  - [ ] Implement `video_clear_screen()`
    - [ ] Clear current draw buffer
  - [ ] Implement `video_clear_frame(void *frame)`
    - [ ] Memset frame to 0
  - [ ] Implement getter functions
  - [ ] Add error handling and logging throughout

### 3.2 Video Subsystem - Pixel Format Support

- [ ] Test 16-bit rendering (RGB565)
  - [ ] Create test pattern in framebuffer
  - [ ] Verify colors display correctly
  - [ ] Check for BGR vs RGB issues

- [ ] Add 32-bit rendering support (RGBA8888)
  - [ ] Add conditional allocation based on BPP
  - [ ] Update SDL texture format
  - [ ] Update frame size calculations
  - [ ] Test both modes

- [ ] Handle pixel format conversions if needed
  - [ ] PSP may use BGR ordering
  - [ ] SDL typically uses RGB
  - [ ] Add conversion function if colors are wrong

### 3.3 Sprite/Tile Rendering - Strategy Decision

- [ ] Analyze current rendering approach
  - [ ] Read through `src/cps1/sprite.c`
  - [ ] Read through `src/cps2/sprite.c`
  - [ ] Read through `src/mvs/vidhrdw.c`
  - [ ] Document how GU commands are used
  - [ ] Identify sprite/tile data structures

- [ ] Choose rendering strategy
  - [ ] Option A: Pure software rendering (direct pixel manipulation)
    - [ ] Pros: Simple, portable, no GPU dependency
    - [ ] Cons: May be slow, CPU-intensive
  - [ ] Option B: OpenGL acceleration
    - [ ] Pros: Fast, hardware-accelerated
    - [ ] Cons: More complex, requires OpenGL knowledge
  - [ ] Option C: Hybrid (software compositing + GPU upload)
    - [ ] Pros: Balance of simplicity and performance
    - [ ] Cons: May have bandwidth issues
  - [ ] **Decision**: Document chosen approach with rationale

### 3.4 Sprite/Tile Rendering - Software Path (if chosen)

- [ ] Create software blit functions
  - [ ] Implement `blit_sprite_16(x, y, w, h, sprite_data, palette, framebuffer)`
    - [ ] Loop through sprite pixels
    - [ ] Apply transparency (skip color index 0)
    - [ ] Apply palette lookup
    - [ ] Write to framebuffer with bounds checking
    - [ ] Handle clipping at screen edges
  - [ ] Implement `blit_tile_16(x, y, tile_data, palette, framebuffer)`
    - [ ] Similar to sprite but for tile rendering
  - [ ] Add 32-bit versions if needed
  - [ ] Optimize with compiler intrinsics or SIMD later

- [ ] Implement blending modes
  - [ ] Additive blending
  - [ ] Alpha blending
  - [ ] Priority/Z-order handling

### 3.5 Sprite/Tile Rendering - OpenGL Path (if chosen)

- [ ] Set up OpenGL context
  - [ ] Initialize OpenGL with SDL
  - [ ] Create OpenGL context instead of SDL renderer
  - [ ] Set up orthographic projection (2D)
  - [ ] Configure viewport

- [ ] Create texture management
  - [ ] Implement texture upload for sprites/tiles
  - [ ] Create texture atlas for efficiency
  - [ ] Handle palette textures (CLUT)

- [ ] Implement OpenGL blit functions
  - [ ] `blit_sprite_gl(x, y, w, h, texture_id, uv_coords)`
    - [ ] Use `glBegin(GL_QUADS)` or VBOs
    - [ ] Set texture coordinates
    - [ ] Set vertex positions
    - [ ] Draw quad
  - [ ] Batch rendering for performance
    - [ ] Accumulate draw calls
    - [ ] Submit in batches

- [ ] Implement shaders (optional, modern OpenGL)
  - [ ] Vertex shader for 2D positioning
  - [ ] Fragment shader for palette lookup
  - [ ] Compile and link shaders

### 3.6 CPS1 Graphics Integration

- [ ] Modify `src/cps1/sprite.c`
  - [ ] Find all `sceGuDrawArray()` calls
  - [ ] Replace with software blit or OpenGL calls
  - [ ] Update sprite data structure if needed
  - [ ] Test with simple game (Street Fighter II)

- [ ] Modify `src/cps1/vidhrdw.c`
  - [ ] Update `cps1_screenrefresh()` function
  - [ ] Replace GU-specific calls
  - [ ] Keep rendering logic intact
  - [ ] Update `blit_start()` and `blit_finish()` calls
  - [ ] Test layer rendering (backgrounds, sprites, stars)

- [ ] Update `src/cps1/vidhrdw.h` if needed
  - [ ] Remove PSP-specific defines
  - [ ] Add Linux-specific defines

- [ ] Test CPS1 rendering
  - [ ] Load a CPS1 ROM
  - [ ] Verify title screen displays
  - [ ] Check sprite rendering
  - [ ] Check background scrolling
  - [ ] Check palette accuracy
  - [ ] Verify no graphical glitches

### 3.7 CPS2 Graphics Integration

- [ ] Modify `src/cps2/sprite.c`
  - [ ] Replace `sceGuDrawArray()` calls
  - [ ] Handle sprite batching
  - [ ] Update for priority rendering
  - [ ] Test with Marvel vs Capcom

- [ ] Modify `src/cps2/vidhrdw.c`
  - [ ] Update `cps2_screenrefresh()` function
  - [ ] Handle raster partial refresh
    - [ ] Update `cps2_screenrefresh(start_line, end_line)`
    - [ ] Ensure partial rendering works correctly
  - [ ] Replace GU blit calls
  - [ ] Test raster effects

- [ ] Update `src/cps2/vidhrdw.h`
  - [ ] Remove PSP-specific defines

- [ ] Test CPS2 rendering
  - [ ] Load CPS2 ROM
  - [ ] Verify raster interrupts work
  - [ ] Check priority masking
  - [ ] Test complex scenes with many sprites

### 3.8 MVS Graphics Integration

- [ ] Modify `src/mvs/vidhrdw.c`
  - [ ] Update `neogeo_screenrefresh()` function
  - [ ] Update `neogeo_partial_screenrefresh()` for raster
  - [ ] Replace GU calls
  - [ ] Handle Neo Geo fix layer
    - [ ] Update `draw_fixed_layer()` implementations
  - [ ] Handle sprite rendering
    - [ ] Update `draw_sprites_hardware()` or `draw_sprites_software()`
    - [ ] Handle sprite zoom/clip hardware emulation
  - [ ] Test with Metal Slug or KOF

- [ ] Test MVS rendering
  - [ ] Load MVS ROM
  - [ ] Verify sprite scaling works
  - [ ] Check fix layer rendering
  - [ ] Test raster effects if used
  - [ ] Verify large sprite handling

### 3.9 NCDZ Graphics Integration

- [ ] Modify `src/ncdz/vidhrdw.c`
  - [ ] Similar updates as MVS (shares code)
  - [ ] Update loading screen rendering
    - [ ] `neogeo_loading_screenrefresh()`
  - [ ] Handle CD-specific rendering states
  - [ ] Test with Neo Geo CD image

- [ ] Test NCDZ rendering
  - [ ] Load CD image
  - [ ] Verify loading screen displays
  - [ ] Test in-game graphics
  - [ ] Check video enable/disable logic

### 3.10 Update Core Graphics Hooks

- [ ] Modify `src/emumain.c`
  - [ ] Update `update_screen()` function
    - [ ] Replace `sceDisplayWaitVblankStart()` with timing logic
    - [ ] Keep UI overlay rendering
    - [ ] Update `video_flip_screen()` call
    - [ ] Keep frameskip logic
    - [ ] Keep FPS calculation
  - [ ] Remove or adapt PSP battery display
    - [ ] Option: Query Linux battery via `/sys/class/power_supply/`
    - [ ] Option: Remove feature entirely
  - [ ] Update screenshot function
    - [ ] Ensure `save_snapshot()` works with new framebuffers

---

## 📋 PHASE 4: AUDIO SYSTEM (Days 21-27)

### 4.1 Audio Subsystem - SDL2 Backend

- [ ] Create `src/linux/sound.h`
  - [ ] Include SDL2 audio headers
  - [ ] Define audio state structure
    - [ ] SDL_AudioDeviceID
    - [ ] Audio enabled flag
    - [ ] Volume level
    - [ ] Sample rate
    - [ ] Buffer size
    - [ ] Mutex for thread safety
  - [ ] Declare audio API functions
    - [ ] `int sound_init_system(void)`
    - [ ] `void sound_shutdown_system(void)`
    - [ ] `int sound_thread_init(void)`
    - [ ] `int sound_thread_start(int samples, int freq)`
    - [ ] `void sound_thread_stop(void)`
    - [ ] `void sound_thread_exit(void)`
    - [ ] `void sound_thread_enable(int enable)`
    - [ ] `void sound_thread_set_volume(int volume)`
  - [ ] Declare external interface from emulator core
    - [ ] `extern struct sound_t *sound` (defined in sndintrf.c)

- [ ] Create `src/linux/sound.c`
  - [ ] Include necessary headers
  - [ ] Define static audio state variable
  - [ ] Implement SDL2 audio callback
    - [ ] `void audio_callback(void *userdata, Uint8 *stream, int len)`
    - [ ] Lock mutex
    - [ ] Check if audio is enabled
    - [ ] If disabled: memset stream to 0
    - [ ] If enabled: call `sound->update()` to fill mix buffer
    - [ ] Copy from mix buffer to stream
    - [ ] Unlock mutex
    - [ ] Handle buffer size mismatches
  - [ ] Implement `sound_init_system()`
    - [ ] Initialize SDL audio subsystem (already done in main)
    - [ ] Create mutex
    - [ ] Initialize audio state
    - [ ] Return 0 on success
  - [ ] Implement `sound_shutdown_system()`
    - [ ] Close audio device if open
    - [ ] Destroy mutex
  - [ ] Implement `sound_thread_init()`
    - [ ] Reset audio state flags
    - [ ] Return 0
  - [ ] Implement `sound_thread_start(int samples, int freq)`
    - [ ] Set up SDL_AudioSpec
      - [ ] Set frequency (44100, 48000, etc.)
      - [ ] Set format (AUDIO_S16SYS for 16-bit signed)
      - [ ] Set channels (2 for stereo)
      - [ ] Set samples (buffer size)
      - [ ] Set callback to `audio_callback`
      - [ ] Set userdata to audio state
    - [ ] Open SDL audio device
      - [ ] `SDL_OpenAudioDevice(NULL, 0, &want, &have, 0)`
      - [ ] Check for errors
      - [ ] Store device ID
    - [ ] Start playback
      - [ ] `SDL_PauseAudioDevice(device_id, 0)`
    - [ ] Return 0 on success, -1 on error
  - [ ] Implement `sound_thread_stop()`
    - [ ] Pause audio device
    - [ ] Close audio device
  - [ ] Implement `sound_thread_exit()`
    - [ ] Call `sound_thread_stop()`
  - [ ] Implement `sound_thread_enable(int enable)`
    - [ ] Lock mutex
    - [ ] Set enabled flag
    - [ ] Unlock mutex
  - [ ] Implement `sound_thread_set_volume(int volume)`
    - [ ] Lock mutex
    - [ ] Set volume level (0-10 scale or 0-128 SDL scale)
    - [ ] Unlock mutex
    - [ ] Note: May need to apply volume in callback with SDL_MixAudioFormat

### 4.2 Audio Subsystem - Alternative PulseAudio Backend (Optional)

- [ ] **Skip initially** - Use SDL2 backend first
- [ ] Consider later if SDL2 has latency issues
- [ ] If needed:
  - [ ] Create `src/linux/sound_pulse.c`
  - [ ] Implement using PulseAudio simple API
  - [ ] Add build flag to choose backend

### 4.3 Sound Interface Integration

- [ ] Modify `src/sound/sndintrf.c`
  - [ ] Find `sound_init()` function
  - [ ] Locate PSP thread creation code
    - [ ] Look for `sound_thread_start()` call
  - [ ] Ensure it calls Linux version instead
  - [ ] Verify `sound->update` callback is set correctly
  - [ ] Keep chip initialization code unchanged
  - [ ] Find `sound_exit()` function
  - [ ] Update to call `sound_thread_exit()`
  - [ ] Keep `sound_mute()` function working
  - [ ] Keep `sound_reset()` function working

- [ ] Test sound interface
  - [ ] Verify chip backends initialize (QSound, YM2151, YM2610)
  - [ ] Verify callback mechanism works
  - [ ] Test mute/unmute
  - [ ] Test volume changes

### 4.4 Audio Thread Safety

- [ ] Ensure thread-safe access to sound state
  - [ ] Verify mutex protects enabled flag
  - [ ] Verify mutex protects volume
  - [ ] Check for race conditions
  - [ ] Use proper memory barriers if needed

- [ ] Handle SDL audio thread behavior
  - [ ] Understand that callback runs in separate thread
  - [ ] Ensure `sound->update()` is thread-safe
  - [ ] Verify no deadlocks occur

### 4.5 Audio Testing

- [ ] Test CPS1 audio
  - [ ] Load CPS1 game
  - [ ] Verify YM2151 sound plays correctly
  - [ ] Test QSound games
  - [ ] Check for crackling or distortion
  - [ ] Test at different sample rates

- [ ] Test CPS2 audio
  - [ ] Load CPS2 game
  - [ ] Verify QSound plays correctly
  - [ ] Check audio sync with video
  - [ ] Test for buffer underruns

- [ ] Test MVS audio
  - [ ] Load MVS game
  - [ ] Verify YM2610 plays correctly
  - [ ] Test ADPCM samples
  - [ ] Test FM synthesis
  - [ ] Check audio quality

- [ ] Test NCDZ audio
  - [ ] Load NCDZ game
  - [ ] Verify YM2610 plays correctly
  - [ ] Test CDDA audio (separate from chip audio)
  - [ ] Verify MP3 playback (if implemented)

### 4.6 MP3 Playback (NCDZ only)

- [ ] Analyze `src/psp/mp3.c`
  - [ ] Understand current implementation
  - [ ] Identify PSP-specific audio channel usage
  - [ ] Check how it integrates with libmad decoder

- [ ] Create `src/linux/mp3.c` (or modify existing)
  - [ ] Keep libmad decoder usage (already in tree)
  - [ ] Replace PSP audio output with SDL mixer
  - [ ] Update `mp3_play()` function
  - [ ] Update `mp3_stop()` function
  - [ ] Update `mp3_pause()` function
  - [ ] Update `mp3_resume()` function
  - [ ] Handle MP3 decode thread with pthread

- [ ] Test MP3 playback
  - [ ] Load NCDZ game with CD audio
  - [ ] Verify background music plays
  - [ ] Test pause/resume
  - [ ] Check synchronization with game

---

## 📋 PHASE 5: INPUT SYSTEM (Days 28-31)

### 5.1 Input Subsystem - SDL2 Gamepad

- [ ] Create `src/linux/input.h`
  - [ ] Include SDL2 headers
  - [ ] Define PSP button constants (maintain compatibility)
    - [ ] `PSP_CTRL_UP`, `PSP_CTRL_DOWN`, etc.
    - [ ] `PSP_CTRL_CROSS`, `PSP_CTRL_CIRCLE`, etc.
    - [ ] `PSP_CTRL_LTRIGGER`, `PSP_CTRL_RTRIGGER`
    - [ ] `PSP_CTRL_START`, `PSP_CTRL_SELECT`
  - [ ] Define input state structure
    - [ ] SDL_GameController pointer
    - [ ] SDL_Joystick pointer (fallback)
    - [ ] Current button state
    - [ ] Previous button state (for edge detection)
    - [ ] Analog X/Y values
  - [ ] Declare input API functions
    - [ ] `int input_init(void)`
    - [ ] `void input_shutdown(void)`
    - [ ] `uint32_t poll_gamepad(void)`
    - [ ] `uint32_t poll_keyboard(void)`
    - [ ] `void pad_init(void)`
    - [ ] `void pad_update(void)`
    - [ ] `int pad_pressed(uint32_t buttons)`
    - [ ] `int pad_pressed_any(void)`
    - [ ] `void pad_wait_clear(void)`
    - [ ] `void pad_wait_press(uint32_t buttons)`
  - [ ] Declare analog functions (for MVS/specific games)
    - [ ] `uint32_t poll_gamepad_analog(void)`
    - [ ] `uint32_t poll_gamepad_fatfursp(void)` (if needed)

- [ ] Create `src/linux/input.c`
  - [ ] Define static input state
  - [ ] Define static debounce/repeat state
  - [ ] Implement `input_init()`
    - [ ] Initialize SDL gamecontroller subsystem
    - [ ] Scan for available controllers
      - [ ] Loop through `SDL_NumJoysticks()`
      - [ ] Check `SDL_IsGameController(i)`
      - [ ] Open first available controller
    - [ ] If no gamecontroller, open as joystick (fallback)
    - [ ] Initialize input state
    - [ ] Return 0 on success
  - [ ] Implement `input_shutdown()`
    - [ ] Close gamecontroller/joystick
    - [ ] Free resources
  - [ ] Implement `poll_gamepad()`
    - [ ] Initialize button state to 0
    - [ ] If gamecontroller exists:
      - [ ] Read D-pad buttons
        - [ ] Map SDL_CONTROLLER_BUTTON_DPAD_UP to PSP_CTRL_UP
        - [ ] Map SDL_CONTROLLER_BUTTON_DPAD_DOWN to PSP_CTRL_DOWN
        - [ ] Map SDL_CONTROLLER_BUTTON_DPAD_LEFT to PSP_CTRL_LEFT
        - [ ] Map SDL_CONTROLLER_BUTTON_DPAD_RIGHT to PSP_CTRL_RIGHT
      - [ ] Read face buttons
        - [ ] Map SDL_CONTROLLER_BUTTON_A to PSP_CTRL_CROSS
        - [ ] Map SDL_CONTROLLER_BUTTON_B to PSP_CTRL_CIRCLE
        - [ ] Map SDL_CONTROLLER_BUTTON_X to PSP_CTRL_SQUARE
        - [ ] Map SDL_CONTROLLER_BUTTON_Y to PSP_CTRL_TRIANGLE
      - [ ] Read shoulder buttons
        - [ ] Map SDL_CONTROLLER_BUTTON_LEFTSHOULDER to PSP_CTRL_LTRIGGER
        - [ ] Map SDL_CONTROLLER_BUTTON_RIGHTSHOULDER to PSP_CTRL_RTRIGGER
      - [ ] Read start/select
        - [ ] Map SDL_CONTROLLER_BUTTON_START to PSP_CTRL_START
        - [ ] Map SDL_CONTROLLER_BUTTON_BACK to PSP_CTRL_SELECT
      - [ ] Read analog stick (left stick)
        - [ ] Get SDL_CONTROLLER_AXIS_LEFTX and LEFTY
        - [ ] Convert from -32768..32767 to 0..255 (PSP range)
        - [ ] Store in analog_x, analog_y
        - [ ] Synthesize D-pad from analog stick
          - [ ] If analog_x < threshold (e.g., 0x40): set PSP_CTRL_LEFT
          - [ ] If analog_x > threshold (e.g., 0xC0): set PSP_CTRL_RIGHT
          - [ ] If analog_y < threshold: set PSP_CTRL_UP
          - [ ] If analog_y > threshold: set PSP_CTRL_DOWN
    - [ ] Return combined button state
  - [ ] Implement `poll_keyboard()`
    - [ ] Get keyboard state with `SDL_GetKeyboardState()`
    - [ ] Initialize button state to 0
    - [ ] Map keys to PSP buttons
      - [ ] Arrow keys to D-pad
      - [ ] Z/X/A/S to Cross/Circle/Square/Triangle
      - [ ] Q/W to L/R triggers
      - [ ] Enter to Start
      - [ ] Right Shift to Select
      - [ ] Escape for menu (optional: map to PSP_CTRL_HOME)
    - [ ] Return button state
  - [ ] Implement `pad_init()`
    - [ ] Initialize debounce state
    - [ ] Reset button hold timers
  - [ ] Implement `pad_update()`
    - [ ] Get current buttons (poll_gamepad() | poll_keyboard())
    - [ ] Update debounce/repeat logic
      - [ ] Check which buttons are newly pressed
      - [ ] Implement key repeat after hold time
      - [ ] Update global `pad` variable
    - [ ] Store previous button state
  - [ ] Implement `pad_pressed(uint32_t buttons)`
    - [ ] Return true if any of specified buttons pressed
  - [ ] Implement `pad_pressed_any()`
    - [ ] Return true if any button pressed
  - [ ] Implement `pad_wait_clear()`
    - [ ] Loop until no buttons pressed
  - [ ] Implement `pad_wait_press(uint32_t buttons)`
    - [ ] Loop until specified buttons pressed

### 5.2 Input - Analog Support

- [ ] Implement `poll_gamepad_analog()`
  - [ ] Similar to poll_gamepad
  - [ ] Pack analog X/Y into upper 16 bits of return value
  - [ ] Used by MVS games with analog support

- [ ] Implement `poll_gamepad_fatfursp()` (if needed)
  - [ ] Special handling for Fatal Fury Special
  - [ ] Prevents simultaneous opposite directions
  - [ ] Gate analog-derived directions against digital

### 5.3 Input Configuration System

- [ ] Design input configuration file format
  - [ ] Decide on format (INI, JSON, or custom)
  - [ ] Define mappings for each button
  - [ ] Support multiple controllers

- [ ] Implement configuration loading
  - [ ] Create `src/linux/input_config.c`
  - [ ] Load config from `~/.njemu/input.cfg`
  - [ ] Parse mappings
  - [ ] Apply to input system

- [ ] Implement configuration saving
  - [ ] Save mappings to config file
  - [ ] Allow runtime remapping (optional)

### 5.4 Hot-plug Support

- [ ] Handle SDL controller events
  - [ ] Listen for SDL_CONTROLLERDEVICEADDED
  - [ ] Listen for SDL_CONTROLLERDEVICEREMOVED
  - [ ] Update controller state accordingly
  - [ ] Display notification to user (optional)

### 5.5 Input Testing

- [ ] Test gamepad input
  - [ ] Connect Xbox/PlayStation controller
  - [ ] Verify all buttons map correctly
  - [ ] Test D-pad
  - [ ] Test analog stick (and analog->dpad synthesis)
  - [ ] Test in-game controls

- [ ] Test keyboard input
  - [ ] Verify all key mappings work
  - [ ] Test in menus
  - [ ] Test in gameplay
  - [ ] Verify simultaneous key presses

- [ ] Test input in each system
  - [ ] CPS1 - test controls in Street Fighter II
  - [ ] CPS2 - test controls in Marvel vs Capcom
  - [ ] MVS - test controls in Metal Slug
  - [ ] NCDZ - test CD-specific controls

---

## 📋 PHASE 6: FILE I/O & PATH HANDLING (Days 32-35)

### 6.1 Update File Loading Code

- [ ] Modify `src/common/cache.c`
  - [ ] Find all `sceIo*` function calls
  - [ ] Replace with POSIX equivalents (using fileio.h wrappers)
  - [ ] Update all file paths
    - [ ] Replace "ms0:/" with `get_cache_path()`
  - [ ] Test cache file operations
    - [ ] Create cache file
    - [ ] Read cache file
    - [ ] Verify data integrity

- [ ] Modify `src/common/state.c`
  - [ ] Find all `sceIo*` function calls
  - [ ] Replace with POSIX equivalents
  - [ ] Update save state paths
    - [ ] Use `get_state_path()`
  - [ ] Test save state operations
    - [ ] Save state
    - [ ] Load state
    - [ ] Verify state data

- [ ] Modify `src/common/loadrom.c` (if it exists)
  - [ ] Update ROM loading file I/O
  - [ ] Replace `sceIo*` calls
  - [ ] Update paths

### 6.2 Update System-Specific File I/O

- [ ] Modify `src/cps1/memintrf.c`
  - [ ] Find file I/O operations
  - [ ] Update paths for config files
  - [ ] Replace `sceIo*` calls

- [ ] Modify `src/cps2/memintrf.c`
  - [ ] Update cache-related file I/O
  - [ ] Update config file handling
  - [ ] Replace `sceIo*` calls
  - [ ] Test with cache-heavy games

- [ ] Modify `src/mvs/memintrf.c`
  - [ ] Update memcard file handling
    - [ ] Use `get_memcard_path()`
  - [ ] Update NVRAM file handling
    - [ ] Use `get_nvram_path()`
  - [ ] Update BIOS loading
  - [ ] Replace `sceIo*` calls
  - [ ] Test memcard save/load

- [ ] Modify `src/ncdz/memintrf.c`
  - [ ] Update CD image loading
  - [ ] Update backup file handling
  - [ ] Update BIOS loading
  - [ ] Replace `sceIo*` calls

- [ ] Modify `src/ncdz/ncdz.c`
  - [ ] Update IPL.TXT validation
  - [ ] Update CD-specific paths
  - [ ] Replace `sceIo*` calls

### 6.3 Update Configuration System

- [ ] Modify `src/psp/config.c` → `src/linux/config.c`
  - [ ] Update all file paths
    - [ ] Use `get_config_path()` for game configs
    - [ ] Use `get_base_path()` for global config
  - [ ] Replace `sceIo*` calls
  - [ ] Keep config file format compatible (or update)
  - [ ] Test config save/load

### 6.4 Update File Browser

- [ ] Modify `src/psp/filer.c` → `src/linux/filer.c`
  - [ ] Replace `sceIoDopen/sceIoDread/sceIoDclose`
  - [ ] Use `opendir/readdir/closedir`
  - [ ] Update directory scanning logic
  - [ ] Update ROM path handling
    - [ ] Use `get_rom_path()`
  - [ ] Update zipname.dat loading
  - [ ] Keep UI code (or simplify)
  - [ ] Test ROM browsing
    - [ ] Verify ROM list populates
    - [ ] Verify navigation works
    - [ ] Verify game selection launches correctly

### 6.5 Update Screenshot System

- [ ] Modify `src/psp/png.c` → `src/linux/png.c`
  - [ ] Update screenshot save path
    - [ ] Use `get_screenshot_path()`
  - [ ] Replace `sceIo*` calls with POSIX
  - [ ] Keep PNG encoding (or use libpng directly)
  - [ ] Test screenshot functionality
    - [ ] Take screenshot in-game
    - [ ] Verify file is created
    - [ ] Verify PNG is valid
    - [ ] Verify correct location

### 6.6 Directory Structure Creation

- [ ] Ensure paths_init() creates all needed directories
  - [ ] Verify `~/.njemu/` exists
  - [ ] Verify `~/.njemu/roms/` exists
  - [ ] Verify `~/.njemu/screenshots/` exists
  - [ ] Verify `~/.njemu/states/` exists
  - [ ] Verify `~/.njemu/config/` exists
  - [ ] Verify `~/.njemu/cache/` exists
  - [ ] Verify `~/.njemu/nvram/` exists
  - [ ] Verify `~/.njemu/memcard/` exists

- [ ] Add error handling for directory creation failures

### 6.7 File I/O Testing

- [ ] Test file operations
  - [ ] Create test file
  - [ ] Write data
  - [ ] Read data back
  - [ ] Verify data matches
  - [ ] Delete file

- [ ] Test directory operations
  - [ ] Create directory
  - [ ] List directory contents
  - [ ] Remove directory

- [ ] Test path translation
  - [ ] Verify "ms0:/config/test.ini" → "~/.njemu/config/test.ini"
  - [ ] Verify relative paths work
  - [ ] Verify absolute paths work

---

## 📋 PHASE 7: UI SYSTEM (Days 36-41)

### 7.1 UI Rendering Basics

- [ ] Modify `src/psp/ui.c` → `src/linux/ui.c`
  - [ ] Update framebuffer access
    - [ ] Use `video_get_draw_frame()` instead of direct GU pointers
  - [ ] Replace GU drawing functions
    - [ ] Replace rectangle fills with direct pixel writes or SDL_Rect
    - [ ] Replace line draws with pixel-by-pixel or SDL functions
  - [ ] Keep UI layout logic
  - [ ] Test basic UI rendering

- [ ] Modify `src/psp/ui_draw.c` → `src/linux/ui_draw.c`
  - [ ] Replace GU primitive drawing
  - [ ] Implement `draw_rectangle_fill(x, y, w, h, color)`
    - [ ] Direct framebuffer manipulation
    - [ ] Or use SDL_RenderFillRect if using renderer
  - [ ] Implement `draw_rectangle_outline(x, y, w, h, color)`
  - [ ] Implement `draw_line(x1, y1, x2, y2, color)`
  - [ ] Test drawing primitives

### 7.2 Text Rendering

- [ ] Modify `src/psp/ui_text.c` → `src/linux/ui_text.c`
  - [ ] Keep existing font data (`src/psp/font/`)
  - [ ] Update `ui_text_draw()` function
    - [ ] Write to SDL framebuffer instead of GU
    - [ ] Handle 16-bit and 32-bit modes
    - [ ] Maintain character positioning
  - [ ] Test text rendering
    - [ ] Display test string
    - [ ] Verify alignment
    - [ ] Verify colors

- [ ] Optional: Add TrueType font support
  - [ ] Use SDL_ttf library
  - [ ] Load system fonts or bundled fonts
  - [ ] Render text to textures
  - [ ] More flexible than bitmap fonts

### 7.3 Menu System

- [ ] Modify `src/psp/ui_menu.c` → `src/linux/ui_menu.c`
  - [ ] Update menu rendering to use new UI functions
  - [ ] Keep menu navigation logic
  - [ ] Update background rendering
  - [ ] Test main menu
    - [ ] Verify menu displays
    - [ ] Verify navigation works
    - [ ] Verify selections work

- [ ] Test in-game menu
  - [ ] Open menu during gameplay (Home or Start+Select)
  - [ ] Navigate menu
  - [ ] Change settings
  - [ ] Resume game
  - [ ] Verify settings persist

### 7.4 Popup Messages

- [ ] Update FPS display
  - [ ] Modify code in `src/emumain.c`
  - [ ] Ensure FPS counter displays correctly
  - [ ] Test at various frame rates

- [ ] Update volume display
  - [ ] Ensure volume popup works when changing volume
  - [ ] Test volume adjustment

- [ ] Update other popups
  - [ ] Screenshot saved message
  - [ ] Save state messages
  - [ ] Error messages

### 7.5 Wallpaper System (Optional)

- [ ] Decide whether to keep wallpaper feature
  - [ ] Option A: Keep it
    - [ ] Modify `src/psp/wallpaper.c` → `src/linux/wallpaper.c`
    - [ ] Update to load wallpapers into SDL textures
    - [ ] Render as background
  - [ ] Option B: Remove it
    - [ ] Simplify by using solid color backgrounds
    - [ ] Remove wallpaper-related code

### 7.6 UI Testing

- [ ] Test file browser UI
  - [ ] Navigate ROM list
  - [ ] Verify scrolling works
  - [ ] Verify selection highlighting
  - [ ] Verify game info display (if present)

- [ ] Test in-game UI
  - [ ] FPS counter
  - [ ] Volume display
  - [ ] Screenshot confirmation
  - [ ] Save state menu

- [ ] Test all menu screens
  - [ ] Main menu
  - [ ] Options menu
  - [ ] Key config menu (if present)
  - [ ] About screen

### 7.7 Battery/Power Display

- [ ] Remove PSP battery display
  - [ ] Remove `scePowerGetBatteryLifePercent()` calls
  - [ ] Remove battery icon rendering

- [ ] Optional: Add Linux battery support
  - [ ] Query `/sys/class/power_supply/BAT0/capacity`
  - [ ] Query `/sys/class/power_supply/BAT0/status`
  - [ ] Display for laptop users
  - [ ] Make it conditional (don't error on desktops)

---

## 📋 PHASE 8: INTEGRATION & TESTING (Days 42-50)

### 8.1 CPS1 System Testing

- [ ] Set up test environment
  - [ ] Create ROM directory
  - [ ] Copy CPS1 ROMs (Street Fighter II, Final Fight, etc.)
  - [ ] Ensure ZIP files are valid

- [ ] Build CPS1 executable
  - [ ] `make BUILD_CPS1PSP=1`
  - [ ] Fix any compilation errors
  - [ ] Fix any linking errors

- [ ] Test ROM loading
  - [ ] Run emulator
  - [ ] Browse to ROM
  - [ ] Select ROM
  - [ ] Verify ROM loads without errors
  - [ ] Check console output for errors

- [ ] Test CPU emulation
  - [ ] Verify game boots to title screen
  - [ ] Verify game is playable
  - [ ] Check for timing issues
  - [ ] Monitor CPU usage

- [ ] Test graphics
  - [ ] Verify sprites render correctly
  - [ ] Verify backgrounds render correctly
  - [ ] Check palette accuracy
  - [ ] Check for graphical glitches
  - [ ] Test layer priority
  - [ ] Test scrolling
  - [ ] Test star field (if applicable)

- [ ] Test audio
  - [ ] Verify music plays
  - [ ] Verify sound effects play
  - [ ] Check for audio crackling
  - [ ] Check for audio/video sync
  - [ ] Test at different sample rates

- [ ] Test input
  - [ ] Test all buttons (punch, kick, etc.)
  - [ ] Test D-pad movement
  - [ ] Test analog stick (if using)
  - [ ] Test special moves
  - [ ] Verify no input lag

- [ ] Test save states
  - [ ] Save state during gameplay
  - [ ] Load state
  - [ ] Verify game state restored correctly
  - [ ] Test multiple save slots
  - [ ] Test save/load across sessions

- [ ] Test screenshots
  - [ ] Take screenshot
  - [ ] Verify file created in correct location
  - [ ] Verify PNG is valid and displays correctly

- [ ] Test menus
  - [ ] Open in-game menu
  - [ ] Navigate all menu options
  - [ ] Change settings
  - [ ] Verify settings apply

- [ ] Test configuration
  - [ ] Change game options
  - [ ] Exit and restart
  - [ ] Verify options persist

- [ ] Stress test
  - [ ] Play for extended period (1+ hour)
  - [ ] Check for memory leaks (use `htop` or `top`)
  - [ ] Check for performance degradation
  - [ ] Verify no crashes

- [ ] Test multiple games
  - [ ] Street Fighter II (sfii)
  - [ ] Final Fight (ffight)
  - [ ] 1944 (1944)
  - [ ] Cadillacs and Dinosaurs (dino)
  - [ ] At least 5 different games

- [ ] Run Valgrind
  - [ ] `valgrind --leak-check=full ./njemu-cps1`
  - [ ] Play game briefly
  - [ ] Exit cleanly
  - [ ] Review memory leak report
  - [ ] Fix any leaks found

### 8.2 CPS2 System Testing

- [ ] Build CPS2 executable
  - [ ] `make BUILD_CPS2PSP=1`
  - [ ] Fix compilation/linking errors

- [ ] Test ROM loading
  - [ ] Test regular ROMs
  - [ ] Test cache system (if applicable)
  - [ ] Test large ROMs (Marvel vs Capcom, etc.)

- [ ] Test CPU emulation
  - [ ] Verify games boot
  - [ ] Check for timing accuracy
  - [ ] Test complex games

- [ ] Test graphics
  - [ ] Test raster interrupts
  - [ ] Test partial screen refresh
  - [ ] Test priority masking
  - [ ] Test sprite batching
  - [ ] Check for glitches

- [ ] Test audio
  - [ ] Verify QSound works correctly
  - [ ] Test audio quality
  - [ ] Check sync

- [ ] Test input
  - [ ] Test all button combinations
  - [ ] Test special inputs (paddle for Puzz Loop 2)

- [ ] Test save states
  - [ ] Save/load multiple times
  - [ ] Verify state integrity

- [ ] Test cache system
  - [ ] Verify cache files created
  - [ ] Verify cache files loaded
  - [ ] Test cache file location

- [ ] Stress test
  - [ ] Extended gameplay
  - [ ] Memory leak check

- [ ] Test multiple games
  - [ ] Marvel vs Capcom (mvsc)
  - [ ] Street Fighter Alpha 3 (sfa3)
  - [ ] Progear (progear)
  - [ ] Puzz Loop 2 (pzloop2)
  - [ ] At least 5 different games

### 8.3 MVS System Testing

- [ ] Build MVS executable
  - [ ] `make BUILD_MVS=1` (or BUILD_MVSPSP=1)
  - [ ] Fix compilation/linking errors

- [ ] Test BIOS loading
  - [ ] Verify BIOS loads (Unibios, etc.)
  - [ ] Test BIOS selection if multiple available
  - [ ] Test without BIOS (should prompt or error)

- [ ] Test ROM loading
  - [ ] Test parent/clone relationships
  - [ ] Test large ROMs
  - [ ] Test cache system

- [ ] Test CPU emulation
  - [ ] Verify games boot
  - [ ] Test 68000 and Z80
  - [ ] Check timing

- [ ] Test graphics
  - [ ] Test fix layer rendering
  - [ ] Test sprite rendering
  - [ ] Test sprite scaling/zoom
  - [ ] Test large sprites
  - [ ] Check for glitches

- [ ] Test audio
  - [ ] Verify YM2610 works
  - [ ] Test ADPCM samples
  - [ ] Test FM synthesis
  - [ ] Check audio quality

- [ ] Test input
  - [ ] Test standard controls
  - [ ] Test analog controls (if supported)
  - [ ] Test multi-button inputs

- [ ] Test memcard
  - [ ] Save game progress
  - [ ] Exit and restart
  - [ ] Verify progress loaded

- [ ] Test NVRAM
  - [ ] Change game settings
  - [ ] Exit and restart
  - [ ] Verify settings persist

- [ ] Stress test
  - [ ] Extended gameplay
  - [ ] Memory leak check

- [ ] Test multiple games
  - [ ] Metal Slug (mslug)
  - [ ] King of Fighters '98 (kof98)
  - [ ] Samurai Shodown (samsho)
  - [ ] Fatal Fury Special (fatfursp)
  - [ ] At least 5 different games

### 8.4 NCDZ System Testing

- [ ] Build NCDZ executable
  - [ ] `make BUILD_NCDZ=1` (or BUILD_NCDZPSP=1)
  - [ ] Fix compilation/linking errors

- [ ] Test BIOS loading
  - [ ] Verify BIOS loads (neocd.bin)
  - [ ] Test IPL.TXT validation

- [ ] Test CD image loading
  - [ ] Test ISO/BIN+CUE format
  - [ ] Test CD mounting
  - [ ] Verify game loads from CD

- [ ] Test CPU emulation
  - [ ] Verify games boot
  - [ ] Test CD-specific boot sequence
  - [ ] Check loading times

- [ ] Test graphics
  - [ ] Test loading screen
  - [ ] Test in-game graphics
  - [ ] Same as MVS once loaded

- [ ] Test audio
  - [ ] Test YM2610
  - [ ] Test CDDA audio
  - [ ] Test MP3 playback (if using MP3 tracks)
  - [ ] Verify audio mixing (chip + CDDA)

- [ ] Test input
  - [ ] Standard controls
  - [ ] CD-specific inputs

- [ ] Test saves
  - [ ] Save game
  - [ ] Load game
  - [ ] Verify backup file handling

- [ ] Stress test
  - [ ] Extended gameplay
  - [ ] Memory leak check

- [ ] Test multiple games
  - [ ] Test at least 2-3 CD games if available

### 8.5 Performance Profiling

- [ ] Profile CPU usage
  - [ ] Use `perf record ./njemu-cps2`
  - [ ] Identify hotspots
  - [ ] Focus on graphics and sound code

- [ ] Profile with gprof (if needed)
  - [ ] Compile with `-pg` flag
  - [ ] Run emulator
  - [ ] Analyze `gmon.out` with `gprof`

- [ ] Identify bottlenecks
  - [ ] Check if graphics rendering is slow
  - [ ] Check if audio callback is taking too long
  - [ ] Check for inefficient loops

- [ ] Optimize hot paths
  - [ ] Optimize sprite blitting
  - [ ] Use SIMD instructions (SSE, AVX) if beneficial
  - [ ] Reduce function call overhead
  - [ ] Optimize memory access patterns

### 8.6 Cross-Platform Testing

- [ ] Test on Ubuntu 22.04
  - [ ] Install dependencies
  - [ ] Build all systems
  - [ ] Run test suite
  - [ ] Document any issues

- [ ] Test on Fedora (latest)
  - [ ] Install dependencies
  - [ ] Build all systems
  - [ ] Run test suite
  - [ ] Document any issues

- [ ] Test on Arch Linux
  - [ ] Install dependencies
  - [ ] Build all systems
  - [ ] Run test suite
  - [ ] Document any issues

- [ ] Test on Debian 11+
  - [ ] Install dependencies
  - [ ] Build all systems
  - [ ] Run test suite
  - [ ] Document any issues

- [ ] Optional: Test on Raspberry Pi 4
  - [ ] Build with ARM optimizations
  - [ ] Test performance
  - [ ] Document limitations

### 8.7 Bug Fixing

- [ ] Create issue tracker
  - [ ] Use GitHub Issues or similar
  - [ ] Document all found bugs

- [ ] Prioritize bugs
  - [ ] Critical: Crashes, data loss
  - [ ] High: Major features broken
  - [ ] Medium: Minor glitches
  - [ ] Low: Cosmetic issues

- [ ] Fix critical bugs first
  - [ ] Address crashes
  - [ ] Fix save state corruption
  - [ ] Fix audio/video major issues

- [ ] Fix high priority bugs
  - [ ] Graphical glitches
  - [ ] Input problems
  - [ ] Performance issues

- [ ] Fix medium/low priority bugs as time permits

---

## 📋 PHASE 9: NETWORKING (Days 51-60) [OPTIONAL]

### 9.1 Decision Point

- [ ] Decide whether to implement networking
  - [ ] Consider: Is netplay a priority?
  - [ ] Consider: Single-player fully working first?
  - [ ] **Recommendation**: Skip initially, add later

### 9.2 If Implementing Networking

- [ ] Analyze existing AdHoc code
  - [ ] Study `src/psp/adhoc.c`
  - [ ] Study `src/common/adhoc.c`
  - [ ] Understand protocol and state machine
  - [ ] Document packet formats

- [ ] Choose networking library
  - [ ] Option A: Raw BSD sockets (most control)
  - [ ] Option B: libenet (easier, reliable UDP)
  - [ ] Option C: Custom P2P library
  - [ ] **Decision**: Document choice

- [ ] Create `src/linux/network.c`
  - [ ] Implement connection management
  - [ ] Implement server/client roles
  - [ ] Implement packet send/receive
  - [ ] Handle timeouts and disconnections

- [ ] Update system-specific code
  - [ ] Modify input handlers for netplay
  - [ ] Implement input synchronization
  - [ ] Handle save state exchange on connect

- [ ] Implement matchmaking (optional)
  - [ ] Create lobby server
  - [ ] Implement game discovery
  - [ ] Handle NAT traversal if needed

- [ ] Test networking
  - [ ] Test local LAN play
  - [ ] Test over Internet
  - [ ] Test latency compensation
  - [ ] Test with different games

### 9.3 If Skipping Networking

- [ ] Remove AdHoc code (optional)
  - [ ] Remove `#ifdef ADHOC` blocks
  - [ ] Simplify input code
  - [ ] Remove from build system

- [ ] Document future networking plans
  - [ ] Add TODO for future implementation
  - [ ] Document protocol for reference

---

## 📋 PHASE 10: OPTIMIZATION & POLISH (Days 61-65)

### 10.1 Command-Line Interface

- [ ] Add command-line argument parsing
  - [ ] Use `getopt()` or similar
  - [ ] Support options:
    - [ ] `--rom <path>` - Direct ROM loading
    - [ ] `--fullscreen` - Start in fullscreen
    - [ ] `--window` - Start in window mode
    - [ ] `--scale <factor>` - Window scaling (2x, 3x)
    - [ ] `--filter <type>` - Video filter (none, linear)
    - [ ] `--vsync` - Enable VSync
    - [ ] `--no-vsync` - Disable VSync
    - [ ] `--volume <0-10>` - Set volume
    - [ ] `--config <path>` - Custom config file
    - [ ] `--help` - Show help message
    - [ ] `--version` - Show version info

- [ ] Implement help message
  - [ ] List all options
  - [ ] Show usage examples

- [ ] Test command-line options
  - [ ] Test each option individually
  - [ ] Test combinations

### 10.2 Configuration Files

- [ ] Design global config format
  - [ ] Decide on format (INI recommended)
  - [ ] Define all configurable options
    - [ ] Video: resolution, scaling, filter, vsync
    - [ ] Audio: volume, sample rate
    - [ ] Input: button mappings
    - [ ] Paths: ROM directory, save directory
    - [ ] General: default system, language

- [ ] Implement config loading
  - [ ] Parse INI file
  - [ ] Apply settings on startup
  - [ ] Handle missing config gracefully

- [ ] Implement config saving
  - [ ] Save settings on exit
  - [ ] Save when changed in UI

- [ ] Test config system
  - [ ] Verify settings persist
  - [ ] Test invalid config files
  - [ ] Test missing config file

### 10.3 Build System Improvements

- [ ] Create CMake build system (optional but recommended)
  - [ ] Create `CMakeLists.txt`
  - [ ] Define all targets (CPS1, CPS2, MVS, NCDZ)
  - [ ] Add dependency finding
  - [ ] Add install targets
  - [ ] Support out-of-tree builds

- [ ] Add install target
  - [ ] Install executables to `/usr/local/bin` or `~/.local/bin`
  - [ ] Install data files (if any)
  - [ ] Install man pages (if created)
  - [ ] Install desktop files

- [ ] Create desktop entry file
  - [ ] Create `.desktop` files for each system
  - [ ] Add to install target
  - [ ] Test desktop integration

- [ ] Create package scripts
  - [ ] Debian package (`.deb`)
  - [ ] RPM package (`.rpm`)
  - [ ] Arch AUR package (optional)
  - [ ] AppImage (optional, good for portability)

### 10.4 Video Filters and Shaders

- [ ] Implement bilinear filtering
  - [ ] If using OpenGL: `glTexParameteri(GL_LINEAR)`
  - [ ] If using SDL: SDL_SetTextureScaleMode

- [ ] Add CRT shader (optional)
  - [ ] Implement scanline effect
  - [ ] Add curvature (optional)
  - [ ] Make configurable

- [ ] Add other filters (optional)
  - [ ] HQx filter
  - [ ] xBR filter
  - [ ] Make selectable via config

### 10.5 Additional Features

- [ ] Add rewind feature (optional)
  - [ ] Save state history
  - [ ] Allow rewinding gameplay
  - [ ] Configurable hotkey

- [ ] Add fast-forward (optional)
  - [ ] Run emulation faster than real-time
  - [ ] Configurable hotkey (e.g., Tab)

- [ ] Add slow-motion (optional)
  - [ ] Run at reduced speed
  - [ ] Useful for learning/practice

- [ ] Add cheat database loader (optional)
  - [ ] Load cheat files
  - [ ] Apply cheats in-game
  - [ ] UI for cheat selection

### 10.6 Documentation

- [ ] Update README.md
  - [ ] Overview of project
  - [ ] Supported systems
  - [ ] Features
  - [ ] Screenshots (optional)
  - [ ] Link to documentation

- [ ] Create comprehensive INSTALL.md
  - [ ] Dependency installation for all major distros
  - [ ] Build instructions
  - [ ] Troubleshooting section
  - [ ] FAQ

- [ ] Create USER_GUIDE.md
  - [ ] How to add ROMs
  - [ ] Controls
  - [ ] Menu navigation
  - [ ] Configuration options
  - [ ] Tips and tricks

- [ ] Create DEVELOPER.md
  - [ ] Code structure
  - [ ] Build system
  - [ ] Adding new features
  - [ ] Debugging tips

- [ ] Create man pages (optional)
  - [ ] `man njemu-cps1`, etc.
  - [ ] Document all command-line options

### 10.7 Final Testing

- [ ] Full regression test
  - [ ] Test all systems
  - [ ] Test all major features
  - [ ] Test on all target platforms

- [ ] Performance verification
  - [ ] Verify full-speed emulation
  - [ ] Check CPU usage is reasonable
  - [ ] Verify no memory leaks

- [ ] User acceptance testing
  - [ ] Have others test the emulator
  - [ ] Gather feedback
  - [ ] Fix reported issues

### 10.8 Release Preparation

- [ ] Choose version number
  - [ ] Use semantic versioning (e.g., v1.0.0)

- [ ] Tag release in git
  - [ ] Create annotated tag
  - [ ] Push to remote

- [ ] Create release binaries
  - [ ] Build for x86_64
  - [ ] Build for ARM (if supporting)
  - [ ] Create packages

- [ ] Write release notes
  - [ ] List features
  - [ ] List known issues
  - [ ] Credit contributors

- [ ] Publish release
  - [ ] GitHub Releases
  - [ ] Other platforms as appropriate

---

## 📋 TRACKING AND ORGANIZATION

### General Progress Tracking

- [ ] Set up development branch
  - [ ] Create `linux-port` branch or similar
  - [ ] Commit regularly with clear messages

- [ ] Use git effectively
  - [ ] Commit after each completed task
  - [ ] Use descriptive commit messages
  - [ ] Create feature branches for major changes

- [ ] Document decisions
  - [ ] Keep notes on architectural decisions
  - [ ] Document workarounds and hacks
  - [ ] Note areas needing future improvement

### Daily/Weekly Goals

- [ ] Set daily goals
  - [ ] Pick 3-5 tasks from this list each day
  - [ ] Focus on completing them

- [ ] Weekly review
  - [ ] Review progress
  - [ ] Adjust timeline if needed
  - [ ] Update this TODO list

### Testing Checklist Template (Per System)

For each system (CPS1/CPS2/MVS/NCDZ), verify:

- [ ] Compiles without errors
- [ ] ROM loads successfully
- [ ] Game boots to title screen
- [ ] Graphics render correctly
- [ ] Audio plays without issues
- [ ] Input is responsive
- [ ] Save states work
- [ ] Screenshots work
- [ ] Menus function properly
- [ ] No crashes during gameplay
- [ ] No memory leaks (Valgrind clean)
- [ ] Performance is acceptable
- [ ] Tested with at least 5 different games

---

## 📊 COMPLETION SUMMARY

### Phase Completion Tracking

- [ ] Phase 1: Infrastructure & Build System (0/14 tasks)
- [ ] Phase 2: Core Platform Abstraction (0/28 tasks)
- [ ] Phase 3: Graphics Pipeline (0/62 tasks)
- [ ] Phase 4: Audio System (0/24 tasks)
- [ ] Phase 5: Input System (0/18 tasks)
- [ ] Phase 6: File I/O & Path Handling (0/23 tasks)
- [ ] Phase 7: UI System (0/20 tasks)
- [ ] Phase 8: Integration & Testing (0/87 tasks)
- [ ] Phase 9: Networking [OPTIONAL] (0/15 tasks)
- [ ] Phase 10: Optimization & Polish (0/32 tasks)

**Total Core Tasks**: ~323 tasks
**Optional Tasks**: ~15 tasks

---

## 🎯 PRIORITY MARKERS

**Critical** 🔴 - Must be done for basic functionality
**High** 🟡 - Important for full functionality
**Medium** 🟢 - Nice to have
**Optional** ⭕ - Can be deferred

---

**Last Updated**: 2025-11-29
**Estimated Completion**: 4-8 weeks (team) / 2-4 weeks (expert solo)
**Current Phase**: Not Started

Good luck with the port! Check off items as you complete them and update the completion summary regularly.
