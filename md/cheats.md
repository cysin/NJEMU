Cheats and Command Lists
========================

Cheat System
------------
- Core structures: `gamecheat_t`, `cheat_option_t`, `cheat_value_t` (see `src/psp/ui_menu.h`).
- Storage: Per-game files at `launchDir/cheats/<game>.ini` (first line may be `include "parent"`). Labels capped at 23 chars, 150 cheats max, 140 options per cheat, 10 values per option.
- Parsing: `cheats_load`/`parse_cheat_option_line` read CSV option lines `"Label", cpu, address, value, ...`; CPU token is validated but discarded (writes are always to 68K). Option 0 acts as “disable”.
- Application:
  - Each system’s run loop calls `apply_cheat()` every frame (CPS1/CPS2/MVS/NCDZ).
  - For each enabled cheat (`curr_option != 0`), iterate values and write 8-bit data to emulated 68000 memory (`m68000_write_memory_8`) every frame.
- UI:
  - Cheats are managed from in-game menu (per-system menu files). Users can enable/disable options per cheat entry.
  - Changes take effect immediately (next frame apply).
- Persistence:
  - Cheat enablement saved in per-game config (`save_gamecfg`), so selections survive restarts.
- Limitations:
  - No conditional triggers beyond selected option; writes happen every frame, so some games may need cautious use.
  - AdHoc: cheats still applied locally; may desync peers if not identical.

Command Lists
-------------
- Purpose: display move lists/commands per game.
- Data: `command.dat` (MAME-style) parsed and stored via `load_commandlist` (per-system `memory_init`).
- Usage:
  - Hotkey `COMMANDLIST` (see input handlers) opens command list UI (`commandlist(1)`).
  - Browser shows command.dat entries as icons in file list when present.
- Scope:
  - Loaded for CPS1/CPS2/MVS/NCDZ if `COMMAND_LIST` build flag enabled and command.dat available for the game or parent.
  - Freed on emulation exit (`free_commandlist`).

Auto-Fire
---------
- Configured via menu; stored in `af_map1/af_map2`.
- `update_autofire(buttons)` toggles mapped button bits according to a frame-based counter to produce rapid-fire presses.
- Re-initialized on `setup_autofire` (input reset/menu exit) and restored on state load.
