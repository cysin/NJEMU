NJEMU Architecture
==================

Top-Level Layout
----------------
- `src/psp`: PSP platform layer (main entry, UI, input, audio thread, GU video, file browser, AdHoc, settings, PNG/MP3 helpers).
- `src/{cps1,cps2,mvs,ncdz}`: System-specific emulation glue (memory map/loader, video, sprites, timers/interrupts, input adapters, drivers/decoders).
- `src/cpu/{m68000,z80}`: Cyclone 68000 and CZ80 cores.
- `src/sound`: Chip backends and mix/resample interface (QSound, YM2151, YM2610).
- `src/common`: Shared ROM/cache loading, state, and utilities.
- `src/include`: Shared headers; `emucfg.h` selects system build (CPS1/CPS2/MVS/NCDZ).

Execution Flow (PSP shell)
--------------------------
```
main (src/psp/psp.c)
  ├─ SetupCallbacks (power/sleep)
  ├─ ui_text_init / pad_init / video_init
  ├─ load SystemButtons PRX
  └─ file_browser() (src/psp/filer.c)
        ├─ scan roms, load settings/zipname
        ├─ user selects game -> set game_dir/game_name
        └─ emu_main() (src/emumain.c)
              ├─ sound_thread_init()
              └─ machine_main (mapped in emucfg.h)
```
- `Loop` enum (`src/psp/psp.h:68-72`) controls transitions between browser, exec, reset, restart, exit.
- Sleep/resume is coordinated via `Sleep` flag in power callbacks (`src/psp/psp.c:123-158`).

Common Runtime Primitives
-------------------------
- **Timing & Frameskip**: `update_screen` in `src/emumain.c` handles vsync pacing, frameskip table, FPS calc, UI overlays, and flipping.
- **Options/State**: Global options (vsync, frameskip, sound, stretch) and counters (`emumain.c:24-74`); fatal errors captured via `fatalerror` then shown in `show_fatal_error`.
- **Sound Thread**: PSP SRC channel in `src/psp/sound.c` runs independently, fed by `sound->update` set in `src/sound/sndintrf.c`.
- **Input**: `src/psp/input.c` polls PSP pad; each system wraps `update_inputport` to map buttons, autofire, service/menu combos, AdHoc merge.
- **Assets/IO**: PNG, wallpaper, UI fonts, and config storage live in `src/psp/*`; ROM loading and ZIP/caches via `src/common/loadrom.c` and `src/common/cache.c`.

Per-System Architecture
-----------------------
All systems share a template in their `*_main` files: memory_init → sound_init → input_init → system init → run loop → system exit → shutdown.

### CPS1 (`src/cps1`)
- Memory: `memintrf.c` loads CPU1 (68000), CPU2 (Z80), gfx, snd, user regions from ZIP, sets up shared RAM, and selects sound handlers (QSound vs YM2151) based on `machine_driver_type` (`memintrf.c:549-719`).
- Video: `vidhrdw.c` decodes CPS1 tiles/sprites, manages palettes and object latching; `sprite.c` handles sprite blit lists.
- Timing: `timer.c` schedules vblank at scanline 256, runs 68000+Z80 slices, and triggers IRQ2 with render/latch in `driver.c:369-378`.
- Input: `inptport.c` maps PSP controls to CPS inputs, handles service/start combos, autofire, menu, and AdHoc merging (`inptport.c:940-1084`).
- Driver glue: `driver.c` contains game-specific init/decodes and EEPROM hooks; `cps1.c` orchestrates init/reset/run/exit.

### CPS2 (`src/cps2`)
- Memory/cache: `memintrf.c` supports cache files on non-Slim PSP, handles Phoenix/parent set selection, loads ROM regions, and prepares gfx pen usage tables.
- Video: `vidhrdw.c`/`sprite.c` handle CPS2 tilemaps, sprites, and palette rebuilds; `driver.c` contains raster/vblank callbacks.
- Timing: `timer.c` programs raster interrupts from registers, runs 68000 and Z80 (unless suspended), rebuilds palette when flagged, and defers rendering to IRQ callbacks (`timer.c:131-188`).
- Input: `inptport.c` supports paddle types (`INPTYPE_pzloop2`) and multi-input ports; service/menu logic mirrors CPS1.
- Core: `cps2.c` mirrors CPS1 flow with cache sleep handling during PSP sleep.

### MVS (Neo Geo AES/MVS) (`src/mvs`)
- Memory/cache: `memintrf.c` selects BIOS, uses cache for large C/S/V ROMs, loads memcard/NVRAM, and configures driver type (normal vs raster) (`memintrf.c:1562-1683`).
- Video: `vidhrdw.c` handles Neo Geo fix layer, sprites, palette; raster line effects supported.
- Timing: `timer.c` chooses update handler (normal vs raster) via `timer_set_update_handler`; each frame runs 68000+Z80 with per-line callbacks in raster mode and triggers `neogeo_screenrefresh` when not skipped.
- Input: `inptport.c` maps PSP pad/analog, supports multi-slot controllers and special cases (fatfursp, paddles), and AdHoc merge.
- Core: `mvs.c` manages memcard/NVRAM persistence and BIOS/region settings; run loop pauses cache on PSP sleep.

### NCDZ (Neo Geo CDZ) (`src/ncdz`)
- Memory: Fixed allocations sized for CD content; validates IPL, loads BIOS, restores memcard backup (`memintrf.c:332-394`).
- CD/CDDA: `ncdz.c` integrates `cdrom_init`/`cdda_init` to stream audio/data; watchdog handling and load-finished signaling re-enable layers mid-run.
- Video/Sound/Input: Neo Geo-style `vidhrdw.c`, `inptport.c`, `timer.c` similar to MVS, with CD-specific boot sequence in `ncdz.c:93-150`.
- Timing: Normal vs raster handlers in `timer.c` with CDZ-specific interrupt (`neogeo_interrupt`) and watchdog reset logic in run loop.

Timing and Rendering Pipeline
-----------------------------
```
Frame start
  ├─ timer_update_cpu (system-specific)
  │    ├─ set IRQ timers (vblank / raster / sound)
  │    ├─ run CPUs for time slices, firing callbacks
  │    └─ advance base_time / frame counters
  ├─ IRQ callbacks:
  │    ├─ CPS1: vblank IRQ2 -> screenrefresh + obj latch
  │    ├─ CPS2: raster IRQ4 -> partial draw; vblank IRQ2 -> final draw + obj latch
  │    ├─ MVS/NCDZ: raster or vblank -> interrupts + screenrefresh
  └─ update_screen (emumain.c)
       ├─ UI overlays (FPS/volume/battery/popup)
       ├─ speedlimit/vsync pacing + frameskip adjust
       └─ video_flip_screen -> GU swap (src/psp/video.c)
```
- GU setup and buffer management live in `src/psp/video.c`; system video layers write into software buffers and then blit via GU lists (`blit_finish`).

Audio Pipeline
--------------
```
sound_init (src/sound/sndintrf.c)
  ├─ start chip backend:
  │    - CPS1: YM2151 or QSound
  │    - CPS2: QSound
  │    - MVS/NCDZ: YM2610
  ├─ set mixer callback + resample params
  └─ start PSP sound thread (src/psp/sound.c)

Sound thread loop
  while active:
    if muted -> zero buffer
    else sound->update(stream_buffer)
    sceAudioSRCOutputBlocking(volume, buffer)
```
- Muting and volume are controlled via `sound_thread_enable`/`sound_thread_set_volume` from options; `sound_reset` calls per-chip resets.

Input, UI, and Menus
--------------------
- PSP poll in `src/psp/input.c` feeds system-specific `update_inputport`:
  - Opens in-game menu on Home or start+select (via `showmenu`).
  - Service/start combos (L+R+Select/Start) set service flags or simultaneous starts.
  - Autofire mapping and analog adjustments.
  - AdHoc mode merges remote port values and pause state (`adhoc_update_inputport` in `src/common/adhoc.c`).
- UI drawing (backgrounds, popups, dialogs, wallpapers) in `src/psp/ui*.c`, `src/psp/wallpaper.c`; fatal errors shown via `emumain.c:331-363`.
- Screenshots: `save_snapshot` in `emumain.c` mutes sound/cache, finds next filename, and calls `save_png`.

Data Persistence and Cache
--------------------------
- Game config and command lists saved per title (`save_gamecfg`, `free_commandlist`), invoked on exit in CPS1/CPS2/MVS/NCDZ cores.
- CPS2/MVS cache large ROMs under `cache/` using `cache_init`/`cachefile_open` (`src/common/cache.c`); MVS also caches C/S/V data and supports parent ROM reuse.
- MVS/NCDZ persist memcard/NVRAM backups to `memcard/*.bin`, `nvram/*.nv`, and `backup.bin` on shutdown; CDZ persists via `memory_shutdown`.

AdHoc (Multiplayer)
-------------------
- Optional build path (`ADHOC`): each system init negotiates roles (server/client) and exchanges state/input (`adhocInit`, `adhocSelect` in `src/psp/adhoc.c` and `src/common/adhoc.c`).
- Input thread merges `send_data`/`recv_data`, propagates pause flags, and can send save states on connect (see CPS1/CPS2 `inptport.c` AdHoc sections).
- AdHoc imposes fixed options (e.g., no vsync, frameskip off, 333 MHz) in memory_init for deterministic sync.
