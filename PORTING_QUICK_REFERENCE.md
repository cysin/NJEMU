# NJEMU Linux Porting - Quick Reference

## Priority Task Order

### 🔴 Critical Path (Must Complete First)

1. **Build System** → Create Linux Makefile/CMake (2-3 days)
2. **Timing & Threading** → Replace sceRtc/sceKernel with POSIX (1 day)
3. **Graphics** → Replace PSP GU with SDL2/OpenGL (10-12 days) ⚠️ BIGGEST TASK
4. **Audio** → Replace sceAudio with SDL2 audio (5-7 days)
5. **Input** → Replace sceCtrl with SDL2 gamepad (3-4 days)
6. **File I/O** → Replace sceIo with POSIX (3-4 days)
7. **Testing** → CPS1 → CPS2 → MVS → NCDZ (7-10 days)

### 🟡 Important (Required for Full Functionality)

8. **UI System** → Port menus and text rendering (5-6 days)
9. **Path Handling** → ms0:/ → ~/.njemu/ translation (included in File I/O)

### 🟢 Optional (Can Defer)

10. **Networking** → Replace AdHoc with BSD sockets (7-10 days)
11. **Optimization** → Shaders, filters, polish (3-5 days)

---

## File Migration Map

### Core Platform Files (Replace)

| PSP File | Linux Replacement | Effort | Priority |
|----------|-------------------|--------|----------|
| `src/psp/psp.c` | `src/linux/main.c` | Medium | Critical |
| `src/psp/video.c` | `src/linux/video.c` | **High** | Critical |
| `src/psp/sound.c` | `src/linux/sound.c` | Medium | Critical |
| `src/psp/input.c` | `src/linux/input.c` | Low | Critical |
| `src/psp/ticker.c` | `src/linux/ticker.c` | Low | Critical |
| `src/psp/filer.c` | `src/linux/filer.c` | Medium | High |
| `src/psp/ui*.c` | `src/linux/ui*.c` | Medium | High |
| `src/psp/config.c` | `src/linux/config.c` | Low | High |
| `src/psp/adhoc.c` | `src/linux/network.c` | High | Optional |

### System Rendering Files (Modify)

| File | Changes Needed | Lines | Priority |
|------|----------------|-------|----------|
| `src/cps1/vidhrdw.c` | Replace GU blits | ~400 | Critical |
| `src/cps1/sprite.c` | Replace GU draw | ~300 | Critical |
| `src/cps2/vidhrdw.c` | Replace GU blits | ~500 | Critical |
| `src/cps2/sprite.c` | Replace GU draw | ~400 | Critical |
| `src/mvs/vidhrdw.c` | Replace GU blits | ~600 | High |
| `src/ncdz/vidhrdw.c` | Replace GU blits | ~600 | High |

### Common Files (Minor Modifications)

| File | Changes | Priority |
|------|---------|----------|
| `src/emumain.c` | Update screen flip, remove PSP battery | Medium |
| `src/sound/sndintrf.c` | Update audio init | High |
| `src/common/cache.c` | Replace sceIo* calls | High |
| `src/common/state.c` | Replace sceIo* calls | High |
| `src/include/osd_cpu.h` | Add Linux typedefs | Critical |
| `src/emucfg.h` | Add Linux defines | Critical |

---

## PSP SDK → Linux API Mapping

### Graphics (Highest Effort)
```
sceGuInit()           → SDL_CreateWindow() + SDL_CreateRenderer()
sceGuDrawArray()      → SDL_RenderCopy() or OpenGL glDrawArrays()
sceGuSwapBuffers()    → SDL_RenderPresent()
sceGuTexImage()       → SDL_CreateTexture() or glTexImage2D()
```

### Audio
```
sceAudioSRCOutputBlocking() → SDL_OpenAudioDevice() with callback
sceAudioSRCChReserve()      → SDL_AudioSpec configuration
```

### Input
```
sceCtrlPeekBufferPositive() → SDL_GameControllerGetButton()
                               SDL_GameControllerGetAxis()
                               SDL_GetKeyboardState()
```

### File I/O
```
sceIoOpen()  → open()
sceIoRead()  → read()
sceIoWrite() → write()
sceIoDopen() → opendir()
sceIoDread() → readdir()
```

### Threading
```
sceKernelCreateThread() → pthread_create()
sceKernelDelayThread()  → usleep() or nanosleep()
```

### Timing
```
sceRtcGetCurrentTick() → clock_gettime(CLOCK_MONOTONIC, &ts)
```

---

## Development Phases Summary

### Phase 1: Setup (Days 1-3)
- ✅ Create `Makefile.linux` or `CMakeLists.txt`
- ✅ Install dependencies (SDL2, OpenGL, libpng, pthread)
- ✅ Create `src/linux/` directory structure
- ✅ Add Linux platform defines to `osd_cpu.h` and `emucfg.h`

### Phase 2: Core Abstractions (Days 4-7)
- ✅ Implement `ticker.c` (timing)
- ✅ Implement `thread.c` (pthread wrapper)
- ✅ Create `main.c` (entry point)
- ✅ Create stub headers for compatibility

### Phase 3: Graphics (Days 8-20) ⚠️ CRITICAL
- ✅ Implement `video.c` with SDL2/OpenGL
- ✅ Replace sprite rendering in CPS1/CPS2/MVS/NCDZ
- ✅ Test each system's video output
- ⚠️ **This is the biggest bottleneck - allocate extra time**

### Phase 4: Audio (Days 21-27)
- ✅ Implement `sound.c` with SDL2 audio callback
- ✅ Update `sndintrf.c` to use new audio system
- ✅ Test audio sync and quality

### Phase 5: Input (Days 28-31)
- ✅ Implement `input.c` with SDL2 gamepad + keyboard
- ✅ Test with multiple controllers
- ✅ Add input configuration

### Phase 6: File I/O (Days 32-35)
- ✅ Create POSIX file I/O wrappers
- ✅ Implement path translation (ms0:/ → ~/.njemu/)
- ✅ Update filer.c for ROM browsing
- ✅ Test cache, save states, configs

### Phase 7: UI (Days 36-41)
- ✅ Port menu system
- ✅ Port text rendering
- ✅ Update FPS/volume overlays

### Phase 8: Integration Testing (Days 42-50)
- ✅ Test CPS1 thoroughly
- ✅ Test CPS2 thoroughly
- ✅ Test MVS thoroughly
- ✅ Test NCDZ thoroughly
- ✅ Fix bugs and optimize

### Phase 9: Networking (Optional, Days 51-60)
- ⭕ Replace AdHoc with UDP sockets
- ⭕ Test netplay

### Phase 10: Polish (Days 61-65)
- ⭕ Add command-line options
- ⭕ Create documentation
- ⭕ Add shaders/filters
- ⭕ Package for distribution

---

## Common Pitfalls & Solutions

### Graphics Issues

**Problem**: Framebuffer stride mismatch
```c
// Wrong: Using SCR_WIDTH (480)
framebuffer[y * SCR_WIDTH + x] = pixel;

// Correct: Using BUF_WIDTH (512) for alignment
framebuffer[y * BUF_WIDTH + x] = pixel;
```

**Problem**: Color format (BGR vs RGB)
- PSP uses BGR in some modes
- SDL uses RGB by default
- Solution: Convert during texture upload or use correct SDL format

**Problem**: Slow software rendering
- Solution: Use OpenGL acceleration
- Or optimize with SIMD (SSE/AVX on x86)

### Audio Issues

**Problem**: Audio crackling/stuttering
- Cause: Buffer underruns
- Solution: Increase buffer size or reduce latency

**Problem**: Audio/video desync
- Cause: Incorrect timing loop
- Solution: Use proper frame pacing with vsync

### Input Issues

**Problem**: Input lag
- Cause: Polling too infrequently
- Solution: Poll every frame in main loop

**Problem**: Controller not detected
- Cause: SDL gamepad mapping missing
- Solution: Add custom controller mappings

### Build Issues

**Problem**: Missing SDL2 headers
```bash
# Ubuntu/Debian
sudo apt-get install libsdl2-dev

# Fedora
sudo dnf install SDL2-devel
```

**Problem**: Linking errors
- Add `-lSDL2 -lGL -lpthread -lpng -lm` to LIBS

---

## Testing Checklist

### Per-System Testing
- [ ] ROM loads successfully
- [ ] Game boots to title screen
- [ ] Gameplay runs at full speed (60fps)
- [ ] Graphics render correctly (sprites, backgrounds, palettes)
- [ ] Audio plays without crackling
- [ ] Input responds correctly (all buttons)
- [ ] Save states work (save/load)
- [ ] Screenshots save correctly
- [ ] Game-specific features work (dip switches, service menu)
- [ ] No memory leaks (run with Valgrind)
- [ ] No crashes after extended play (1+ hour)

### Recommended Test Games

**CPS1**:
- Street Fighter II (sfii)
- Final Fight (ffight)
- 1944 (1944)

**CPS2**:
- Marvel vs Capcom (mvsc)
- Street Fighter Alpha 3 (sfa3)
- Progear (progear)

**MVS**:
- Metal Slug (mslug)
- King of Fighters '98 (kof98)
- Samurai Shodown (samsho)

**NCDZ**:
- Metal Slug (if you have CD image)

---

## Performance Targets

### Minimum Hardware
- CPU: Intel i3 / AMD equivalent
- RAM: 512 MB
- GPU: Integrated graphics (Intel HD)
- Expected: Full speed for CPS1/CPS2

### Recommended Hardware
- CPU: Intel i5 / Ryzen 5
- RAM: 2 GB
- GPU: Dedicated GPU (optional)
- Expected: Full speed for all systems including MVS/NCDZ

### Raspberry Pi 4
- Should run CPS1 at full speed
- CPS2 may need frame skip
- MVS/NCDZ may be too slow (needs optimization)

---

## Quick Commands

### Build
```bash
# Using Makefile
make -f Makefile.linux BUILD_CPS2PSP=1

# Using CMake
mkdir build && cd build
cmake .. -DBUILD_CPS2=ON
make -j4
```

### Run
```bash
./njemu-cps2 --rom ~/roms/mvsc.zip --fullscreen
```

### Debug
```bash
# Memory leaks
valgrind --leak-check=full ./njemu-cps2

# Performance profiling
perf record ./njemu-cps2
perf report

# CPU profiling
gprof ./njemu-cps2 gmon.out
```

### Install Dependencies
```bash
# Ubuntu/Debian
sudo apt-get install build-essential libsdl2-dev libgl1-mesa-dev \
  libpng-dev libpulse-dev zlib1g-dev

# Fedora
sudo dnf install gcc SDL2-devel mesa-libGL-devel libpng-devel \
  pulseaudio-libs-devel zlib-devel

# Arch
sudo pacman -S base-devel sdl2 mesa libpng libpulse zlib
```

---

## Estimated Timeline

| Experience Level | Conservative | Optimistic |
|------------------|--------------|------------|
| Expert (solo) | 4 weeks | 2 weeks |
| Team (2-3) | 6 weeks | 4 weeks |
| Beginner | 12 weeks | 8 weeks |

**Critical Path Items**:
- Graphics rendering: 50% of total effort
- Audio system: 20% of total effort
- Integration/testing: 20% of total effort
- Everything else: 10% of total effort

---

## Next Steps

1. **Read** `PORTING_PLAN.md` for full details
2. **Set up** Linux build environment
3. **Start** with Phase 1 (build system)
4. **Test frequently** - don't wait until everything is done
5. **Focus on CPS1 first** - it's the simplest system
6. **Ask for help** - leverage community knowledge

Good luck with the port! 🚀
