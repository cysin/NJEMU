Save States
===========

Overview
--------
- Save/Load infrastructure is centralized in `src/common/state.c` with per-system STATE_SAVE/STATE_LOAD macros to serialize subsystem state (video, timer, input, driver, etc.).
- Controlled by `SAVE_STATE` build flag; may not be enabled in all builds.
- Save states are per-slot (0–9) and stored under `launchDir/state/` with filenames `<game>.svN`.

State Serialization
-------------------
- `STATE_SAVE(name)` / `STATE_LOAD(name)` macros in each module (video/timer/input/memory/driver) write/read raw structs and arrays.
- Examples:
  - CPS1: video palettes/object caches (`cps1_old_palette`, `video_palette`), timers (`base_time`, `current_frame`, CPU suspend flags), input (controller, dipswitch, service switch), driver-specific data (e.g., Z80 bank).
  - CPS2: palette buffers, objram bank, timers and z80_suspended flag.
  - MVS/NCDZ: video RAM, palettes, timer expiration data, CPU suspend flags, global offsets.
- CPU cores (Cyclone 68000 / CZ80) expose state accessors for registers and PC; included in state.c serialization.

Workflow
--------
- Slot detection: `find_state_file` scans `state` dir to show available slots (browser/menu).
- Save:
  1) Collect all registered state blocks via `state_save_*` calls in modules.
  2) Write header with version/magic, followed by serialized blocks.
  3) Stored to `state/<game>.sv<slot>`.
- Load:
  1) Open file, verify header/version.
  2) Invoke `state_load_*` for each module; if mismatch or error, may set fatal error.
- Menu/UI triggers:
  - In-game menu provides save/load options; input handlers call `state_save()`/`state_load()` with selected slot.
  - AdHoc mode may send/recv state once at connect (CPS1/CPS2) via `adhoc_send_state`/`adhoc_recv_state`.

Coverage and Limitations
------------------------
- Not all external resources are serialized:
  - Cached ROM data on disk not included (assumed unchanged).
  - PSP UI state, browser, wallpapers not saved.
  - MP3/CDDA playback state for NCDZ handled separately (CDDA stop on load).
  - Power state and cache file handles not included; emulation resumes from pure emu state.
- Module order matters; state.c maintains registry to ensure consistent load order.

Error Handling
--------------
- On failure to open/read/write, `fatalerror` is invoked with descriptive messages.
- Version mismatches or missing blocks can invalidate load; user is typically returned to browser.
