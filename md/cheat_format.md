Cheat and Command Data Formats
==============================

Cheat Files
-----------
- Location: `launchDir/cheats/<game>.ini` (first line may be `include "parent"` to reuse another file). Loaded once per game in `cheats_load`.
- Format (parsed in `src/psp/ui_menu.c`):
  - Cheat begins with a line like `cheat "Infinite Lives"`.
  - Following lines list options as CSV: `"ON", cpu, address, value, cpu, address, value...`
    - CPU is currently validated but not stored; all writes go to 68000 via `m68000_write_memory_8`.
    - Address/value are hex (0x000000–0xFFFFFF / 0x00–0xFF).
    - Option labels limited to 23 chars; up to 10 values per option.
  - Blank lines end a cheat block. Option index 0 acts as "disable".
- Storage: Parsed into `gamecheat_t`/`cheat_option_t`/`cheat_value_t` arrays (max 150 cheats, 140 options each).
- Encoding: Keep files in GBK if you use Japanese/Chinese labels (per README); parser is 8-bit and not UTF-8 aware.
- Application: In each frame `apply_cheat` walks enabled options and writes all values.

Command Lists (command.dat)
---------------------------
- Location: `command.dat` placed with ROMs (standard MAME command.dat). Parsed by `load_commandlist` during `memory_init`.
- Mapping:
  - Matched by game shortname; if missing, parent name tried.
  - COMMAND_LIST build flag must be enabled.
- Encoding: README notes Japanese command lists must be GBK; convert with tools like Notepad++ if needed.
- Display: Triggered via COMMANDLIST hotkey; shown as an overlay with move lists. Browser also shows command icon for supported games.

zipname.dat
-----------
- Not a cheat file but related metadata for browser:
  - Provides display titles, parent relationships, and flags (bad/hack/bootleg/not working).
  - Must be present in `launchDir` for CPS/MVS to show correct titles.
- Encoding: Should match UI text encoding (GBK recommended for included data).
