# NJEMU PSP to Linux/SDL2 Port - Status

## Overview
This document tracks the progress of porting NJEMU (PSP arcade emulator suite) to Linux with SDL2.

**Target Emulators:**
- CPS1PSP - Capcom System 1
- CPS2PSP - Capcom System 2 (primary target)
- MVSPSP - Neo-Geo MVS/AES
- NCDZPSP - Neo-Geo CDZ

## Architecture

The original PSP code is well-structured with clear separation:
- `src/psp/` - PSP-specific platform layer (~19MB, ~17,000 lines)
- `src/common/` - Platform-independent helpers (~90KB)
- `src/cps1/`, `src/cps2/`, `src/mvs/`, `src/ncdz/` - Emulator cores
- `src/cpu/` - CPU emulators (M68000, Z80)
- `src/sound/` - Sound synthesis

## Progress Status

### ✅ Completed

#### 1. Directory Structure
- Created `src/sdl2/` with subdirectories: `font/`, `icon/`, `menu/`, `config/`
- Mirroring PSP structure for easy comparison

#### 2. Header Files (All Core Headers Created)
- [x] `sdl2.h` - Main SDL2 header with common includes
- [x] `video.h` - Video subsystem interface
- [x] `input.h` - Input subsystem interface
- [x] `sound.h` - Audio subsystem interface
- [x] `ticker.h` - Timing functions
- [x] `config.h` - Configuration file management
- [x] `ui.h` - User interface
- [x] `ui_draw.h` - UI drawing primitives
- [x] `ui_menu.h` - Menu system
- [x] `ui_text.h` - UI text/localization
- [x] `filer.h` - File browser
- [x] `png.h` - PNG image handling

#### 3. Build System
- [x] `CMakeLists.txt` - Complete CMake build configuration
- [x] `src/makefiles/CPS2PSP.cmake` - CPS2 emulator source list
- Supports all 4 emulators via options
- Auto-detects SDL2 via find_package or pkg-config

#### 4. Core Implementations
- [x] `ticker.c` - SDL timing (microsecond precision)
- [x] `sdl2.c` - SDL2 initialization and main loop
- [x] `input.c` - Complete keyboard + gamepad support

### 🚧 In Progress

#### 5. Video Subsystem (`video.c`)
**Status:** Not started yet
**Complexity:** High - needs to replace PSP GU (Graphics Utility) library

**PSP GU Features to Replace:**
- Hardware-accelerated blitting (sceGuCopyImage → SDL_RenderCopy)
- Texture rendering (sceGuTexImage → SDL_Texture)
- Frame buffers (480x272, 16/32-bit → SDL_CreateTexture)
- Vsync (sceDisplayWaitVblankStart → SDL_RenderPresent with vsync)
- Scissor testing → SDL_RenderSetClipRect
- Alpha blending → SDL_SetTextureBlendMode

**Plan:**
1. Create SDL_Window (480x272, scalable)
2. Create SDL_Renderer with hardware acceleration
3. Use SDL_Texture for framebuffers (4 buffers: show, draw, work, tex)
4. Implement blit functions using SDL_RenderCopy
5. Software fallback for complex operations

#### 6. Audio Subsystem (`sound.c`)
**Status:** Not started yet
**Complexity:** Medium

**PSP Features to Replace:**
- Threaded audio callback (sceAudioSRCOutputBlocking → SDL_QueueAudio)
- Multiple sample rates (24/44.1/48 KHz → SDL_AudioSpec)
- Volume control → SDL_MixAudio with volume

### 📋 Pending

#### 7. UI Layer
- `ui.c` - Background, popups, message boxes, progress bars
- `ui_draw.c` - Font rendering, icons, drawing primitives
- `ui_menu.c` - Menu system, cheat support
- `ui_text.c` - Text wrapper (reuses PSP version)

**Note:** Most UI code is software-rendering and should work with minimal changes

#### 8. File Browser
- `filer.c` - Replace PSP file I/O with standard POSIX
- Update paths from PSP memory stick to Linux filesystem
- Use XDG directories (~/.config, ~/.local/share)

#### 9. Configuration
- `config.c` - Replace PSP config paths
- Linux config locations: `~/.config/njemu/` or `./config/`

#### 10. PNG Support
- `png.c` - Should mostly work, uses libpng (already cross-platform)
- May need minor path adjustments

#### 11. Font/Icon Data
- Reuse existing PSP font data (in `src/psp/font/`)
- Reuse existing PSP icon data (in `src/psp/icon/`)
- These are just data arrays, no porting needed

### ❌ Not Needed

- `SystemButtons.c` - PSP-specific HOME button handling
- `adhoc.c` - PSP Ad-Hoc multiplayer (network rewrite if needed)
- `mp3.c` - NCDZ only, uses libmad (should work as-is)
- `wallpaper.c` - 32bpp wallpaper support (optional feature)

## Build Instructions (When Ready)

```bash
mkdir build
cd build

# Build CPS2 emulator (default)
cmake .. -DBUILD_CPS2PSP=ON
make

# Build other emulators
cmake .. -DBUILD_CPS1PSP=ON
cmake .. -DBUILD_MVSPSP=ON
cmake .. -DBUILD_NCDZPSP=ON
```

## Dependencies

**Required:**
- SDL2 (libsdl2-dev)
- libpng (libpng-dev)
- zlib (zlib1g-dev)
- Standard C library (gcc/clang)

**Optional:**
- libmad (libmad0-dev) - for NCDZ MP3 playback
- SDL2_net - for network multiplayer (future)

Install on Debian/Ubuntu:
```bash
sudo apt-get install libsdl2-dev libpng-dev zlib1g-dev libmad0-dev
```

Install on Fedora:
```bash
sudo dnf install SDL2-devel libpng-devel zlib-devel libmad-devel
```

## Key Mappings

### Keyboard
- **Arrow Keys** → D-Pad
- **Enter** → Start
- **Right Shift** → Select
- **A** → Square (Button 1)
- **S** → Cross (Button 2)
- **Z** → Triangle (Button 3)
- **X** → Circle (Button 4)
- **Q** → L Trigger
- **W** → R Trigger
- **ESC** → Home/Menu

### Game Controller
- Standard SDL2 GameController API mapping
- D-Pad → D-Pad
- Face Buttons (A/B/X/Y) → PSP buttons
- Shoulders (L/R) → L/R Triggers
- Left Analog → D-Pad (with deadzone)
- Guide Button → Home/Menu

## Technical Notes

### PSP to SDL2 API Mapping

| PSP Function | SDL2 Equivalent |
|--------------|-----------------|
| `sceGuInit()` | `SDL_CreateRenderer()` |
| `sceGuSwapBuffers()` | `SDL_RenderPresent()` |
| `sceDisplayWaitVblankStart()` | `SDL_RenderPresent()` with vsync |
| `sceGuCopyImage()` | `SDL_RenderCopy()` |
| `sceCtrlPeekBufferPositive()` | `SDL_GetKeyboardState()` / `SDL_GameControllerGetButton()` |
| `sceAudioSRCOutputBlocking()` | `SDL_QueueAudio()` |
| `sceKernelDelayThread()` | `SDL_Delay()` |
| `sceRtcGetCurrentTick()` | `SDL_GetPerformanceCounter()` |
| `scePowerSetClockFrequency()` | No-op (OS managed) |

### Memory Management
- PSP uses fixed VRAM addresses (0x04000000...)
- SDL2 uses heap-allocated framebuffers
- Need to allocate 4 framebuffers: show, draw, work, tex
- Size: 512×272 × 2/4 bytes (16/32-bit)

### Screen Resolution
- PSP native: 480×272 pixels
- Linux: Window scalable, render at native PSP resolution
- Use SDL_RenderSetLogicalSize(480, 272) for easy scaling
- Optional: integer scaling for crisp pixels

### Performance
- PSP CPU: 333 MHz MIPS
- Modern x86: 2-4 GHz (10-20x faster)
- Should run at full speed with room for enhancements:
  - Higher resolutions
  - Better filters
  - Enhanced features

## Next Steps

1. **Implement `video.c`** - This is the critical path
   - Start with basic SDL_Renderer setup
   - Implement frame buffer management
   - Port blit/copy functions
   - Test with simple graphics

2. **Implement `sound.c`** - Secondary priority
   - Set up SDL_AudioSpec
   - Create audio callback thread
   - Test with simple beeps

3. **Port UI layer** - Should be straightforward
   - Most code is software rendering
   - Update font rendering for SDL surfaces
   - Test file browser

4. **Test with CPS2 emulator** - First working build
   - Load a ROM
   - Verify graphics output
   - Test input
   - Test sound

5. **Port other emulators** - Once CPS2 works
   - CPS1 (similar to CPS2)
   - MVS (more complex)
   - NCDZ (needs MP3 support)

## Estimated Completion

Based on current progress:
- **Video subsystem:** 1-2 days
- **Audio subsystem:** 1 day
- **UI layer:** 2-3 days
- **Testing & debug:** 2-3 days
- **Total:** ~1-2 weeks for first working build

## Resources

- PSP SDK Documentation: https://pspdev.github.io/
- SDL2 Documentation: https://wiki.libsdl.org/
- Original NJEMU: https://github.com/173210/njemu

---
*Last Updated: 2025-11-05*
*Branch: claude/port-psp-emulator-linux-sdl2-011CUp2ZuwZL4CBWPz83CHf7*
