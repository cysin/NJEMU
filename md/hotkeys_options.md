Hotkeys, Options, and DIPs
==========================

Hotkeys (runtime behavior)
--------------------------
- Browser: Circle = launch/enter, Cross = cancel/parent, Triangle or Home (SystemButtons) = exit prompt, R trigger = BIOS menu (MVS) or boot BIOS prompt (NCDZ), Start = set startup dir, Select = help.
- In-game (common across CPS1/CPS2/MVS/NCDZ):
  - Menu: Home when SystemButtons.prx is loaded, otherwise Start+Select.
  - Service: L+R+Select; simultaneous start: L+R+Start (CPS1/CPS2).
  - Screenshot: `input_map[SNAPSHOT]` (unbound by default; bind in System Key Settings); saves PNG to `ms0:/PICTURE/<SYSTEM>`.
  - Command list: `input_map[COMMANDLIST]` (if COMMAND_LIST build) opens `command.dat` overlay.
  - Swap controller: `input_map[SWPLAYER]` toggles P1/P2 (CPS/MVS; shows popup).
  - AdHoc: Menu combo sets `adhoc_paused`; peers sync pause state.
- Analog/dial cases: Forgotten Worlds uses dial update; Neo Geo Irrmaze/Popbounc read analog axes; MVS selects poller (`poll_gamepad_fatfursp` for fatfursp) to avoid opposite-directions.

Options (commonly exposed in menus)
-----------------------------------
- Video: stretch/aspect, vsync, speedlimit, frameskip/autoframeskip, show FPS, raster enable (CPS1/CPS2/Neo Geo), screen mode (16/32 bpp where applicable).
- Sound: enable/disable, volume, samplerate (0–2 for resample), QSound/YM chip selection (CPS1).
- Input: autofire mapping, controller swap, analog/dial settings (Forgotten Worlds), paddle options (CPS2 pzloop2).
- System-specific:
  - CPS: EEPROM ops, raster, kludges (hidden under RELEASE toggles), cheats enablement.
  - MVS: BIOS select (Unibios/AES/MVS), region, raster toggle, cache toggle, memcard ops.
  - NCDZ: BIOS boot vs game boot, CDDA enable, region, watchdog notices.
- CPU clock: selectable (222/266/300/333) via settings; forced to 333 in AdHoc.

DIP Switches
------------
- Each system exposes DIP editors in menu:
  - CPS1/CPS2: DIP A/B/C arrays (`cps1_dipswitch`, `cps2_port_value` reads). Options cover difficulty, coinage, demo sounds, etc.
  - MVS: DIP settings for system mode, coinage, blood/violence; BIOS and region may supersede some DIP behaviors.
  - NCDZ: Minimal DIP usage; region set via menu.
- Changes are written to DIP arrays used by memory handlers (e.g., `cps1_dsw_a_r`) and saved in per-game config.

Persistence
-----------
- Hotkey-configurable items (autofire, controller) saved per game via `save_gamecfg`.
- Menu settings saved globally (`save_settings`) and per game; DIP settings persisted in game config and save states.

Notes
-----
- Default button bindings come from per-system config files:
  - CPS six-button: B1=Cross, B2=L, B3=Square, B4=Circle, B5=R, B6=Triangle; Start=Start, Coin=Select.
  - CPS four-button: B1=Square, B2=Cross, B3=Triangle, B4=Circle.
  - Neo Geo: A=Cross, B=Circle, C=Square, D=Triangle.
- All hotkey entries default to 0 (unbound) except menu combo; bind via System Key Settings or config INIs.
- Some hotkeys are guarded by build flags (COMMAND_LIST, SAVE_STATE). If disabled at build, menu items/hotkeys may be absent.
