Config Files and Options
========================

Overview
--------
- NJEMU stores global and per-game configuration to persist user preferences (video, sound, input, cache, language).
- Key files reside under `launchDir`:
  - Global settings INI per system: `cps1psp.ini`, `cps2psp.ini`, `mvspsp.ini`, or `ncdzpsp.ini` (names set in `src/psp/config/*.c`).
  - Per-game config files stored in `config/<game>.ini`.
  - Zipname and command data (browser/command lists)
- Config code: `src/psp/config.c`, `src/psp/config.h`

Global Settings
---------------
- Loaded at browser start via `load_settings()`, saved on exit or via menu changes (`save_settings()`).
- Fields (examples, see `config.h`):
  - Video: `option_vsync`, `option_stretch`, `option_showfps`, `option_autoframeskip`, `option_frameskip`
  - Sound: `option_sound_enable`, `option_samplerate`, `option_sound_volume`
  - Input: autofire mappings, controller selection defaults
  - CPU clock: `psp_cpuclock`
  - Cache/wallpaper/language toggles
- UI language and fonts loaded via `ui_text.c`; README notes GBK charset for some lists.

Per-Game Config
---------------
- Loaded after driver selection (`load_gamecfg(game_name)`), saved on exit (`save_gamecfg`).
- Stores per-title options:
  - Controller selection/autofire maps (`input_map`), hotkeys (Snapshot/Commandlist/SwitchPlayer)
  - Video stretch/rotation, raster enable flags (CPS1/CPS2), speedlimit/vsync/frameskip/FPS
  - Sound enable/volume/samplerate (CPS1/MVS/NCDZ)
  - Region/BIOS and machine mode (MVS), region/boot choice (NCDZ)
  - DIP switch defaults (CPS1 DIPA/B/C, MVS soft+hard dips)
  - Cheats enablement (option index per cheat)
- Command list loading tied to parent name if present; per-game config selection respects parent clones.

Zipname and Title Metadata
--------------------------
- `zipname.dat` provides user-facing titles and parent mapping; loaded in browser (`load_zipname`).
- Missing `zipname.dat` triggers fatal error (`COULD_NOT_OPEN_ZIPNAME_DAT`) shown in browser splash.
- `zipname` entries influence flags (bootleg/hack/bad/not working) displayed in browser list.

Menu Options and Persistence
----------------------------
- Menu sliders/checkboxes update option globals; saving happens:
  - Immediately for some options (e.g., CPU clock via `set_cpu_clock`).
  - On emulation exit (`save_gamecfg`) for per-game settings.
  - On browser exit (`save_settings`) for global settings.
- Autofire setup refreshed after menu exits (`setup_autofire`).

Language/Font Assets
--------------------
- Text resources loaded from `ui_text.c` based on PSP system language (EN/JA/ES/ZH-S/ZH-T); font assets in `src/psp/font`.
- README notes GBK requirement for Japanese/Chinese command lists; convert external files to GBK for correct rendering.

Error Handling
--------------
- If config files missing/corrupt, defaults are used and files recreated on save.
- Critical metadata (zipname.dat) absence leads to fatal error and return to browser.
