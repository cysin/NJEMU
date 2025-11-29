NJEMU Emulation Workflow
========================

Frontend and Launch
-------------------
- High-level control flow:
```
PSP boot
  └─ main() (psp.c)
       ├─ setup callbacks/clock/UI/video
       ├─ load SystemButtons PRX
       └─ file_browser()
             ├─ browse roms UI
             ├─ select game -> set game_dir/game_name
             └─ emu_main()
                    └─ machine_main (CPS1/CPS2/MVS/NCDZ)
```
- PSP entrypoint sets up callbacks/clock, initializes text rendering, controller sampling, and GU video, then loads the SystemButtons PRX before entering the browser (`src/psp/psp.c:165-230`). `Loop` state machine drives control flow (`src/psp/psp.h:68-72`).
- The ROM browser allocates directory entries, loads settings/zipname metadata, paints the UI, and waits for user selection. On launch it captures `game_dir`/`game_name`, frees browser resources, and hands off to the emulator (`src/psp/filer.c:910-1075`).
- Fatal errors or UI-triggered exits bubble back to the browser via `Loop`, `fatalerror`, and `show_fatal_error` (`src/emumain.c:148-164`, `src/emumain.c:314-325`, `src/emumain.c:331-363`).

Starting Emulation
------------------
- Per-system startup template:
```
machine_main()
  └─ loop (restart/reset support)
       ├─ Loop = LOOP_EXEC
       ├─ ui_popup_reset / fatal_error clear
       ├─ memory_init()      // ROM/BIOS load, config, alloc
       ├─ sound_init()       // chip + PSP sound thread
       ├─ input_init()       // map PSP controls
       ├─ system_init()      // video + driver init (+AdHoc)
       ├─ run_loop()         // frame loop below
       ├─ system_exit()      // video/driver tear-down, save
       ├─ input_shutdown()
       └─ sound_exit()
```
- `emu_main` resets screenshot numbering, brings up the sound thread, and dispatches to the system-specific `machine_main` mapped in `src/emucfg.h` (CPS1/CPS2/MVS/NCDZ) (`src/emumain.c:148-159`).
- Every `machine_main` follows the same template: set `Loop`, clear UI/fatal flags, call `memory_init` ➜ `sound_init` ➜ `input_init` ➜ system init ➜ run loop ➜ system exit ➜ input/sound shutdown, with outer `while` permitting restart/reset (`src/cps1/cps1.c:215-247`, `src/cps2/cps2.c:222-254`, `src/mvs/mvs.c:287-319`, `src/ncdz/ncdz.c:292-329`).
- Audio thread spins independently, pulling mixed samples via `sound->update` and pushing them to the PSP SRC channel (`src/psp/sound.c:60-171`); `sound_init` wires the correct chip backend and starts the thread (`src/sound/sndintrf.c:200-244`).

ROM / BIOS Loading
------------------
- Loader overview (per system):
```
memory_init()
  ├─ UI: clear screen + "LOAD ROM"
  ├─ load_rom_info() -> select driver + parent names
  ├─ load_gamecfg / commandlist / BIOS pick (MVS/NCDZ)
  ├─ set_cpu_clock()
  ├─ allocate regions
  ├─ pull ROM data (ZIP/cache/CD)
  └─ wire sound/memory handlers per driver
```
- CPS1: `memory_init` validates ROM info, finds the driver entry, loads per-game config/command list, sets CPU clock, and pulls CPU/Z80/GFX/SND/User regions from ZIPs via `common/loadrom.c`, then configures QSound vs YM2151 handlers based on `machine_driver_type` (`src/cps1/memintrf.c:549-719`).
- CPS2: Similar flow with cache support; ROM info lookup selects a parent set for Phoenix/cache variants, initializes caches, chooses driver, and loads ROMs before decrypt/graphics prep (`src/cps2/memintrf.c:626-780`). Cache files live under `cache/` and are read/written through `common/cache.c`.
- MVS: Allocates fixed regions, selects BIOS (with user prompt if missing), initializes cache, loads memcard/NVRAM, applies AdHoc defaults, validates ROM info, and streams BIOS/ROM data (with parent fallbacks) into memory (`src/mvs/memintrf.c:1562-1683`). Per-driver init later sets raster vs normal mode.
- NCDZ: Pre-allocates fixed memory sizes, validates `IPL.TXT` via `neogeo_check_game`, restores memcard backup, loads BIOS, and leaves CD image processing to `cdrom_init`/`cdda_init` invoked by `neogeo_main` (`src/ncdz/memintrf.c:332-394`, `src/ncdz/ncdz.c:292-323`).

Reset and Run Loops
-------------------
- Frame loop anatomy:
```
while (Loop == LOOP_EXEC)
  ├─ if Sleep: pause (and cache_sleep for CPS2/MVS)
  ├─ apply_cheat()
  ├─ timer_update_cpu()   // CPU exec + IRQ callbacks
  ├─ update_screen()      // vsync/frameskip/UI overlay
  └─ update_inputport()   // poll PSP, menu, service, snapshot
```
- Each reset switches to 16bpp render mode, clears the screen, resets autoframeskip/timers/input/sound, and wipes sprite buffers (`src/cps1/cps1.c:76-93`, `src/cps2/cps2.c:76-93`, `src/mvs/mvs.c:125-142`, `src/ncdz/ncdz.c:93-150`).
- Runtime frame loop (all systems): handle PSP sleep (optionally pausing caches), reapply cheats, call `timer_update_cpu` to execute CPUs/interrupts for a frame, call `update_screen` for vsync/frameskip/UI overlays, then `update_inputport` to poll controls/menus (`src/cps1/cps1.c:177-204`, `src/cps2/cps2.c:177-210`, `src/mvs/mvs.c:245-275`, `src/ncdz/ncdz.c:216-260`).
- Exits save per-system state: CPS1/2 write game cfg and release command lists (`src/cps1/cps1.c:99-131`, `src/cps2/cps2.c:99-131`); MVS writes memcard/NVRAM and config (`src/mvs/mvs.c:149-185`); NCDZ saves config only and leaves CD cleanup to shutdown (`src/ncdz/ncdz.c:262-317`).

CPU Scheduling and Interrupts
-----------------------------
- Timing overview:
```
Frame start
  ├─ timer_update_cpu()
  │    ├─ set IRQ timers (vblank / raster / sound)
  │    ├─ while time_left:
  │    │     ├─ pick next timer expiry
  │    │     ├─ run CPUs for delta
  │    │     └─ fire callbacks as they expire
  │    └─ bump base_time, wrap every 1s
  └─ Callbacks:
       ├─ CPS1: vblank IRQ2 -> screenrefresh + obj latch
       ├─ CPS2: raster IRQ4 -> partial draw; vblank IRQ2 -> final draw/obj latch
       ├─ MVS/NCDZ: raster or vblank -> interrupts + screenrefresh
```
- CPS1 timer sets a vblank interrupt at scanline 256 each frame; the timer loop slices the frame, dispatches pending timers, runs M68000+Z80, and advances base time (`src/cps1/timer.c:290-339`). The vblank callback asserts IRQ2, renders if not skipped, and latches object RAM (`src/cps1/driver.c:369-378`).
- CPS2 timer additionally programs raster interrupts from register values, rebuilds palettes when flagged, runs 68000/Z80 for each slice, and defers rendering to raster/vblank callbacks (`src/cps2/timer.c:131-188`). Raster IRQ triggers partial draws, vblank finalizes refresh and sprite latching (`src/cps2/driver.c:150-183`).
- MVS/NCDZ timers support normal vs raster drivers via `timer_set_update_handler`; both step M68000 and Z80, invoke callbacks, and refresh the screen when not skipped. Raster mode walks scanlines, firing per-line interrupts before refresh (`src/mvs/timer.c:275-379`, `src/ncdz/timer.c:294-368`). Normal mode performs a single vblank interrupt before drawing (`src/mvs/timer.c:275-329`, `src/ncdz/timer.c:294-347`).
- CPU cores are Cyclone 68000 and CZ80 (`src/cpu/m68000` and `src/cpu/z80`), invoked through `m68000_execute`/`z80_execute` pointers wired during `timer_reset` (`src/cps1/timer.c:150-160`, `src/mvs/timer.c:141-152`, `src/ncdz/timer.c:160-170`).

Rendering Path
--------------
- Render pipeline sketch:
```
IRQ callback (vblank/raster)
  ├─ build palette (as needed)
  ├─ screenrefresh (tiles/sprites per system)
  ├─ blit_finish() to GU list
  └─ GU swap happens in update_screen() via video_flip_screen()
```
- System video modules decode graphics and handle palette/sprite/tile drawing: CPS1/CPS2 in `vidhrdw.c` and `sprite.c` under each system; Neo Geo in `mvs/vidhrdw.c` and `ncdz/vidhrdw.c`. Rendering is triggered from interrupt callbacks and finishes with `blit_finish` to push GU sprites (`src/cps1/driver.c:369-378`, `src/cps2/driver.c:165-178`).
- `update_screen` performs UI overlays (FPS, volume, battery, popups), manages vsync pacing, performs frame flipping, and dynamically adjusts frameskip when enabled (`src/emumain.c:202-307`). Actual draw buffers and GU setup live in `src/psp/video.c`.

Audio Path
----------
- Audio chain:
```
sound_init()
  ├─ start chip backend (QSound / YM2151 / YM2610)
  ├─ set mixer callback + resample params
  └─ start PSP sound thread

sound thread loop:
  while active:
    if muted -> zero buffer else sound->update()
    sceAudioSRCOutputBlocking(...)
```
- `sound_init` selects chip backends (QSound, YM2151, YM2610), assigns the mixer callback, configures resampling, and starts the PSP sound thread (`src/sound/sndintrf.c:200-244`). Per-chip reset/stop is handled in the same module (`src/sound/sndintrf.c:250-285`).
- The PSP sound thread continuously requests samples and feeds them to `sceAudioSRCOutputBlocking`, respecting mute/volume flags (`src/psp/sound.c:60-171`).

Input, UI, and Misc
-------------------
- Input/menu interaction:
```
update_inputport()
  ├─ poll PSP pad (with analog -> dpad helpers)
  ├─ open menu on Home or start+select
  ├─ handle service/start combos (L+R+Select/Start)
  ├─ apply adjust_input + autofire
  ├─ write input flags to emulated ports
  ├─ adhoc: merge remote ports + pause state
  └─ if SNAPSHOT -> save_snapshot()
```
- `update_inputport` polls PSP controls, opens the in-game menu on Home/start+select, handles service/start combos, applies input mappings/autofire, triggers snapshots, and writes values into emulated input ports; AdHoc mode merges remote inputs and pause state (`src/cps1/inptport.c:940-1084`, similar structures in CPS2/MVS/NCDZ input files).
- Cheats are applied each frame by writing patched values into emulated memory (`src/cps1/cps1.c:134-171` and counterparts in other systems).
- Screenshots call `save_png` after muting audio/cache and auto-incrementing filenames (`src/emumain.c:400-453`), and fatal errors halt emulation with a dialog until a button press (`src/emumain.c:314-325`, `src/emumain.c:331-363`).
