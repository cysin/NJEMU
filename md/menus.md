Menu Structure and Option Semantics
===================================

Menu Driver
-----------
- Core loop in `src/psp/ui_menu.c` handles drawing, navigation, sliders, selectors, and message boxes.
- Triggered from in-game (Home or Start+Select) or browser (O on menu item).
- Per-system option handlers located in `src/psp/menu/`:
  - `cps.c` (used for CPS1/CPS2)
  - `mvs.c`
  - `ncdz.c`

Common Menu Sections
--------------------
- Video: stretch/aspect, vsync, speedlimit, frameskip/autoframeskip, show FPS, raster enable (where applicable).
- Sound: enable, volume, samplerate (0–2), QSound/YM selection (CPS1).
- Input: autofire configuration, controller swap (P1/P2), paddle/dial settings.
- System: BIOS/region (MVS/NCDZ), cache toggle (MVS), CPU clock, language.
- Cheats: Enable/disable options per cheat entry (CPS/MVS/NCDZ).
- Save/Load State: Slot selection (if SAVE_STATE enabled).
- DIP Switches: Editable DIP values with per-option labels.
- Misc: Command list toggle, screenshot path display, reset/restart/exit.

CPS Menu (src/psp/menu/cps.c)
-----------------------------
- Raster Option: Enables CPS raster effects (CPS1). CPS2 raster handled in timer/video; option may be present for consistency.
- Autofire: Configure per-button autofire.
- Controller Swap: Cycle active controller (P1/P2).
- Video Stretch: Toggle stretch/ratio.
- Speed/Vsync: Enable speedlimit (ties flip to PSP refresh) and vsync.
- Sound: Enable, volume, samplerate; select QSound vs YM2151 (CPS1 only).
- Cheats: Toggle per cheat option.
- DIP: Edit DIP A/B/C via menu; writes to `cps1_dipswitch`/`cps2_port_value`.
- EEPROM/NVRAM: Save/clear operations for EEPROM-backed titles.

MVS Menu (src/psp/menu/mvs.c)
-----------------------------
- BIOS Select: Choose BIOS (Unibios 1.0–3.0, ASIA AES/MVS, etc.).
- Region: Set region byte (affects language/violence).
- Raster: Enable raster effects; selects timer update handler.
- Cache: Toggle cache usage (if USE_CACHE).
- Memcard Ops: Load/save/clear memcard/NVRAM.
- Autofire, Controller Swap, Video/Sound settings similar to CPS.
- Cheats and DIP editors.

NCDZ Menu (src/psp/menu/ncdz.c)
-------------------------------
- BIOS Boot: Toggle boot via BIOS vs direct game boot.
- Region: Set region byte.
- CDDA: Enable/disable CDDA playback.
- Speedlimit/Vsync/Stretch: As common.
- Autofire/Controller Swap: As common.
- Cheats and DIP (limited).
- Memcard/Backup: Options related to CDZ memcard handling.

States and Persistence
----------------------
- Menu changes update globals immediately; some actions (CPU clock) apply instantly via `set_cpu_clock`.
- Per-game options saved via `save_gamecfg`; global via `save_settings`.
- Save states: menu options to save/load selected slot; uses `state.c`.

Side Effects / Cautions
-----------------------
- AdHoc: Some options forced (sound on, vsync off, frameskip off, 333 MHz); menu may hide or lock certain items.
- Cache toggle may require restart to take effect (MVS).
- Raster enable can affect performance; turning off improves speed on PSP-1000.
- BIOS/Region changes require restart/reset to apply fully.
