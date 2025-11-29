Controls and DIP Switch Reference
=================================

PSP Button Mapping (Defaults)
-----------------------------
Mappings are defined per system in `input_map[]` (see `src/*/inptport.c` and config files). Defaults:
- PSP D-pad/analog → P1 directions.
- CPS six-button config (most fighters): B1=Cross, B2=L, B3=Square, B4=Circle, B5=R, B6=Triangle; Start=Start; Coin=Select.
- CPS four-button configs: B1=Square, B2=Cross, B3=Triangle, B4=Circle (quiz/rot variations adjust rotation/labels).
- Neo Geo: A=Cross, B=Circle, C=Square, D=Triangle; Start=Start; Coin=Select.
- Hotkeys (unbound by default unless stated):
  - Menu: Home (SystemButtons) or Start+Select.
  - Service: L+R+Select; simultaneous start (CPS1/2): L+R+Start.
  - BIOS menu (browser): R trigger on MVS; boot BIOS prompt on NCDZ.
  - COMMANDLIST/SNAPSHOT/SWPLAYER: bind in System Key Settings.

Per-System Button Maps (high level)
-----------------------------------
- CPS1/CPS2:
  - Uses the defaults above unless overridden by per-game config; multiple config variants exist (2/3/4/6-button drivers, quiz layout).
  - SERVICE bit set via L+R+Select combo; DIP service/test also accessible in menu.
- MVS:
  - Neo Geo 4-button layout: Square/Cross/Circle/Triangle → A/B/C/D.
  - Coin: Select; Start: Start.
  - Additional slots: controller swap for multi-slot BIOS via menu.
  - BIOS menu shortcut: R trigger in browser.
- NCDZ:
  - Same as MVS for pad mapping.
  - CD controls handled by game; no extra hotkeys beyond common set.

DIP Switch References
---------------------
Values are read from DIP arrays per system; editors in menu map human-readable options to bits.
- CPS1 (`cps1_dipswitch`):
  - DIP A/B/C cover coinage, difficulty, lives, demo sounds, continue, cabinet type. Bit positions vary by game; see `src/cps1/dipsw.c`.
- CPS2 (`cps2_dipswitch`):
  - Similar fields; `cps2_port_value` includes DIP reads and special inputs. See `src/cps2/inptport.c`.
- MVS:
  - DIP settings control cabinet mode (MVS/AES), coinage, blood/violence, demo sounds. Region/Bios may override some DIP behavior. See `src/mvs/dipsw.c`.
- NCDZ:
  - Minimal DIP use; region set via menu. DIP handling is limited in CDZ mode.

Analog/Paddle/Dial
------------------
- CPS1 Forgotten Worlds: uses `input_analog_value` and `forgottn_update_dial`; analog mapped via `adjust_input`.
- CPS2 Pzloop2: paddle mode (`machine_input_type == INPTYPE_pzloop2`) uses extra input port writer.
- MVS fatfursp: uses `poll_gamepad_fatfursp` to avoid simultaneous opposite directions.

Autofire
--------
- Configured per game; `af_map1/af_map2` derived from `input_map` hotkeys.
- `update_autofire` toggles mapped buttons based on frame counters to simulate rapid fire.

Service/Coin
------------
- Coin inputs mapped to Select (per player where applicable).
- Service switch toggled via L+R+Select combo; also available via DIP/service mode.
