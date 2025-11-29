Linux + SDL2 Port Plan
======================

Context from docs
-----------------
- PSP-facing code lives entirely under `src/psp` (main entry, GU video, sound thread, input, browser/UI, SystemButtons, MP3, AdHoc setup). Emulation cores sit in `src/{cps1,cps2,mvs,ncdz}` with shared timing/render patterns and depend on `update_screen`/`video_flip_screen`, `sound_init`/thread, and `update_inputport` hooks.
- Timing and frameskip are controlled in `emumain.c` (`update_screen`, `skip_this_frame`, autoframeskip) while each system’s `timer.c` slices CPU time and triggers vblank/raster callbacks (`cps1/cps2/mvs/ncdz`). Frame pacing on PSP uses `ticker()` and optional vsync waits.
- Rendering builds GU command lists via `blit_*` helpers (see `blit_gu.md`, `video.md`); CPS1/2 and Neo Geo video modules rely on `blit_start/finish`, sprite/scroll/fix draw helpers, and palette LUTs (`video_clut16`). GU list sizes are tuned per system (48 KB CPS1/2, 300 KB MVS/NCDZ).
- Audio path: `sound_init` selects chip mixers (QSound/YM2151/YM2610) and spawns the PSP SRC output thread in `src/psp/sound.c`, which repeatedly calls `sound->update` (mix/resample in `src/sound/sndintrf.c`). Sleep/pause flags gate the thread.
- Input path: `src/psp/input.c` polls PSP pad (with analog→dpad synthesis and repeat), per-system `update_inputport` maps to hardware ports, handles hotkeys/menu/service, and merges AdHoc data when enabled.
- Files/configs: browser in `src/psp/filer.c` sets `launchDir`, loads `zipname.dat`, settings (`load_settings`), and per-game configs (`load_gamecfg`); save states (`state/`), cache (`cache/`), memcards/NVRAM (`memcard/`, `nvram/`), screenshots, and cheats/command lists rely on these paths.
- Optional features: AdHoc (`ADHOC` flag, `src/psp/adhoc.c` + `src/common/adhoc.c`), MP3/CDDA for NCDZ (`src/psp/mp3.c`, `src/ncdz/cdda.c`), wallpapers/32bpp UI (`PSP_VIDEO_32BPP`), SystemButtons PRX for Home button.

Porting strategy
----------------
1) Baseline build and scaffolding
- Stand up a Linux build (Make/CMake) that compiles common/emulation code without PSP SDK headers. Start with one system (e.g., CPS2) to keep the surface small.
- Isolate PSP-only headers (`psp/*`, `pspsdk`, GU/audio/ctrl) behind a new platform layer so core code can include portable headers instead.

2) Define a platform abstraction
- Introduce a `platform/` interface for: main entry/init/shutdown, video buffer management + vsync, audio output, input polling (buttons + analog), high-resolution ticks/delays, threading, file paths, popups/logging.
- Refactor `emumain.c` and per-system `*_main` to call the platform API instead of PSP-specific helpers where possible (`video_init/flip`, `pad_*`, `sound_thread_*`, `ticker`, `sceKernelDelayThread` equivalents).

3) Video pipeline (SDL2)
- Replace GU path with an SDL2 renderer/texture pipeline. Decide on target pixel format (likely 16bpp 565 to match current LUTs) and scaling/stretch options mirroring `option_stretch`, `option_rotate`, `PSP_VIDEO_32BPP`.
- Reimplement `blit_*` helpers to draw into a software frame buffer (CPU compositing of sprites/tiles using existing decoded data and pen-usage tables) or build SDL textures per layer before a final blit; keep start/end slice support for raster effects.
- Mirror `update_screen` semantics: frameskip table, autoframeskip adjustments, FPS calculation, optional vsync waits (`SDL_RenderPresent` + `SDL_Delay`/`SDL_GL_SwapWindow` timing), UI overlays or placeholders for FPS/volume popups.
- Audit GU-specific constants (GULIST_SIZE, VRAM layouts) and replace with host-side allocations.

4) Audio output (SDL2)
- Replace `src/psp/sound.c` with an SDL audio device callback or thread feeding `sound->update`. Preserve stereo/mono handling and samplerate options from `sndintrf.c` (resample path for YM2610/YM2151, direct path for CPS2/QSound).
- Map volume/mute and pause behavior (current code checks `Sleep`/`sound_enable`). Ensure buffer sizes align with `sound->samples` and SAFETY padding expectations.
- Rework MP3/CDDA path for NCDZ to use libmad directly with SDL audio or reuse the main audio device; keep track/loop state from `cdda.c`.

5) Input/event loop
- Implement `pad_*` equivalents using SDL events: keyboard/gamepad mapping to PSP buttons, analog axes thresholds for dpad synthesis, repeat/delay behavior. Preserve hotkeys (menu, service L+R+Select, start+select, snapshot, command list, swap player) and analog/paddle cases (`forgottn`, `pzloop2`, Neo Geo analog pollers).
- Replace Home/SystemButtons handling with a keyboard binding; keep fallback to start+select for menu.
- For AdHoc-disabled builds, stub networking; plan a later pass if multiplayer is required.

6) Timing/pacing primitives
- Reimplement `ticker()` and microsecond delays with SDL performance counters. Keep `PSP_REFRESH_RATE` equivalents for pacing and FPS math.
- Replace `sceKernelDelayThread` sleeps in loops (sleep/pause handling, AdHoc thread throttling) with SDL delays or condition waits.

7) Filesystem/config paths
- Decide on Linux paths for `launchDir` (default to CWD or XDG data directory) and ensure subdirs (`roms`, `cache`, `state`, `memcard`, `nvram`, `cheats`, `config`) are created as needed.
- Remove SystemButtons dependency; gate PSP-only assets (wallpapers, icon PRX) behind platform checks. Keep `zipname.dat`, `command.dat`, cheats, and ROM expectations unchanged.

8) UI/browser strategy
- Short-term: provide a minimal CLI launcher to select a ROM and system, bypassing `src/psp/filer.c`, while keeping menu/config/state flows reachable.
- Longer-term: port or replace the PSP UI with SDL-driven menus for options, state slots, DIP edits, and popups; reuse existing option structures and text resources where possible.

9) System-specific checks
- CPS1/CPS2: verify QSound/YM2151 paths still work with new audio device; ensure raster callbacks trigger rendering paths correctly without GU.
- MVS/NCDZ: handle memcard/NVRAM save/load on Linux; ensure raster/partial-refresh logic works with the new renderer; re-evaluate cache usage (can likely disable cache on desktop).
- NCDZ: validate CDROM/ISO loading and MP3 playback in the new platform; keep watchdog/reset behavior intact.

10) Testing and validation
- Build/run each system with representative ROMs; check timing (FPS vs target), input combos, save states, and audio sync.
- Add logging around pacing, audio buffer underruns, and rendering to surface issues early; consider a simple on-screen debug overlay in SDL mode.

TODO
----
- [x] Phase 1: Build and platform split (refs: build.md, arch.md, workflow2.md)
  - [x] Add a Linux target (Make/CMake) that builds core + one system (start CPS1) without PSP SDK headers.
    - [x] Introduce platform macro (`PLATFORM_SDL`) to exclude `src/psp/*` and PSP includes.
  - [x] Define platform interface header (video, audio, input, timing, file paths, popups/logging).
    - [x] Provide SDL stub implementation that compiles and links; keep PSP build selectable (PSP Makefile unchanged).

- [x] Phase 2: Paths, ROM/memory loading (refs: memory.md, rom_loading.md, bios_romsets.md, cache.md, config_files.md, rom_tools.md)
  - [x] Decide `launchDir` on Linux (CWD or XDG) and ensure subdirs exist: `roms`, `state`, `memcard`, `nvram`, `cheats`, `config`, `cache`.
    - [x] Update save/load helpers for states, memcard/NVRAM, cheats/command.dat, zipname.dat lookup. (Path construction uses `launchDir`; directories are created at startup; I/O calls routed through platform wrappers.)
  - [x] Adapt ROM loader to SDL platform: remove PSP file APIs, keep ZIP/cache handling, bios checks. (PSP I/O mapped to POSIX in platform layer; cache kept compilable and default-off for CPS1.)
  - [x] Ensure config load/save flows map to new paths; mirror global/per-game INIs semantics. (Load/save stubs provided; paths resolved under `launchDir`.)

- [x] Phase 3: Timing and pacing (refs: timing.md, workflow2.md)
  - [x] Replace `ticker()`/`sceKernelDelayThread` with SDL performance counters and `SDL_Delay`.
    - [x] Preserve `PSP_REFRESH_RATE` equivalent for FPS/frameskip math.
  - [x] Port `update_screen` pacing (frameskip table, autoframeskip, FPS calc, vsync wait) to platform layer; add debug logging/overlay for verification.
  - [x] Validate CPU cycle/timer mapping against Cyclone/CZ80 expectations (refs: cpu.md).

- [x] Phase 4: Input/event loop (refs: input.md, controls_dips.md, hotkeys_options.md, threading.md)
  - [x] Implement `pad_*` equivalents with SDL events: keyboard/gamepad mapping, analog→dpad synthesis, repeat timing.
    - [x] Map menu/service combos (Home→key or Start+Select), hotkeys (snapshot, command list, swap player), analog/paddle cases (`forgottn`, `pzloop2`, Neo Geo analog pollers).
  - [x] Wire per-system `update_inputport` to platform input; stub AdHoc merge initially.
  - [x] Keep threading model in mind for input/sound; avoid races on shared flags.

- [x] Phase 5: Video pipeline on SDL2 (refs: video.md, blit_gu.md, gfx_decoding.md)
  - [x] Choose pixel format (565 vs 8888) and allocate software framebuffer(s) per system with palette LUT use (`video_clut16`) and slice bounds (`start/end`).
    - [x] Reimplement `blit_*` helpers to render sprites/tiles/fix/scroll into the buffer; honor raster partial refresh (`cps2 blit_start`, `neogeo_partial_screenrefresh`).
  - [x] Present via SDL texture/renderer with stretch/aspect options (`option_stretch/rotate`, 32bpp UI fallback).

- [x] Phase 6: Audio output on SDL2 (refs: sound.md)
  - [x] Replace PSP SRC thread with SDL audio callback/thread pulling `sound->update`; keep samplerate/resample handling from `sndintrf.c`.
    - [x] Map volume/mute/pause semantics (Sleep replacement) and size buffers to avoid underruns.
  - [ ] Plan MP3/CDDA path for NCDZ (libmad reuse) once core audio is stable.

- [ ] Phase 7: UI/launcher (refs: ui.md, menus.md, workflow2.md, localization_fonts.md)
  - [x] Implement a minimal CLI/SDL launcher to select system + ROM, bypassing `src/psp/filer.c`; parse `game_dir`/`game_name`.
  - [x] Port or stub in-game menu: expose core options (video stretch/vsync/speedlimit, sound enable/volume, save/load state slots, DIP editors).
    - [ ] Reuse option structures; render minimal SDL overlays first.
  - [ ] Decide on font/text handling (use existing bitmaps or SDL_ttf); respect localization guidance.

- [ ] Phase 8: System-specific validation (refs: arch.md, cd_audio.md)
  - [ ] CPS1/CPS2: verify QSound/YM2151 paths on new audio; ensure raster callbacks render correctly.
  - [ ] MVS/NCDZ: test raster/partial refresh, memcard/NVRAM persistence, cache-disabled path stability.
  - [ ] NCDZ: validate CD image loading and CDDA/MP3 playback; confirm watchdog/reset behavior.

- [ ] Phase 9: Optional features/deferrals (refs: adhoc.md, mp3_cdda_details.md, power_systembuttons.md)
  - [ ] Keep AdHoc disabled initially; design SDL/net wrapper later if needed.
  - [ ] Drop PSP power/sleep handling; optionally add SDL pause/resume hooks.
  - [ ] Wallpapers/command list/cheat dialogs: stub UI hooks and plan SDL overlays later.
  - [ ] Cheats/command lists: keep parser functionality intact (refs: cheats.md, cheat_format.md); add simple toggles in UI later.

- [ ] Phase 10: Testing plan (refs: errors.md, state.md, performance.md)
  - [ ] Define per-system smoke tests (ROMs/BIOS list) with raster on/off, save/load state, input combos, audio sync.
    - [ ] Add debug logging/overlay for timing/audio buffer usage; clean up before release.
  - [ ] Verify save states, config persistence, cheats/command lists across loads; note any regressions.
