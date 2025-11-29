NJEMU Input Architecture
========================

PSP Input Layer (src/psp/input.c)
---------------------------------
- Initialization: `pad_init()` sets sampling cycle to default and enables analog mode; clears debouncing state.
- Polling:
  - `poll_gamepad()` reads `sceCtrlPeekBufferPositive`, masks to `PSP_CTRL_ANY`, and synthesizes D-pad presses from analog stick thresholds (Ly/Lx).
  - MVS-specific variants:
    - `poll_gamepad_fatfursp()` prevents simultaneous opposite directions by gating analog-derived directions against digital buttons.
    - `poll_gamepad_analog()` returns buttons plus raw analog X/Y packed into upper 16 bits for analog-driven games.
- Debounce/repeat: `pad_update()` tracks held time using `ticker()`; implements key repeat with initial delay and faster repeat on hold; updates global `pad` bitmask.
- Utilities: `pad_pressed`, `pad_pressed_any`, `pad_wait_clear`, `pad_wait_press` for menu flows and message boxes.

Input Flow in Emulation Loop
----------------------------
- Each system’s run loop calls `update_inputport()` once per frame after CPU/video to inject PSP inputs into emulated ports.
- `input_reset()` initializes port values, analog buffers, autofire state, and starts AdHoc input thread if needed.
- `setup_autofire()` copies autofire mappings from `input_map` into `af_map1/af_map2` and is called on reset/menu exits.

Per-System Input Handlers (update_inputport)
--------------------------------------------
Shared structure across CPS1/CPS2/MVS/NCDZ with system-specific helpers:
1. **AdHoc branch** (if `adhoc_enable`):
   - Waits for `adhoc_update` flag to clear (input thread sync).
   - Merges local/remote port values: bitwise AND of `send_data.port_value` and `recv_data.port_value`.
   - For analog titles (e.g., `forgottn`), assigns analog values from appropriate peer depending on server/client role.
   - Receives remote `loop_flag` and `paused` flags; triggers `adhoc_pause()` on pause.
   - Local PSP buttons polled; Home/start+select sets pause (server/client specific). L+R+Select toggles service; L+R+Start triggers simultaneous start.
   - Applies `adjust_input()` (per-title remap/invert) and `update_autofire()`; fills `input_flag[]` by comparing to `input_map[]`.
   - Calls system-specific port writers (`update_inputport0/1/2/3`, plus dials/paddles where applicable).
   - Packages buttons/paused/loop/frame into `send_data` and marks `adhoc_update` for the network thread.
2. **Local branch**:
   - Polls PSP pad; opens in-game menu on Home or start+select (`showmenu()`), then re-polls and refreshes autofire setup.
   - Service/start combos via L+R+Select/Start toggle service flag or dual start press.
   - Runs `adjust_input` + `update_autofire`, populates `input_flag[]`, and sets `SERV_SWITCH` if needed.
   - Calls system-specific port writers; handles per-game extras (dial update for `forgottn`).
   - Hotkeys: SNAPSHOT triggers `save_snapshot`; SWPLAYER cycles `option_controller` (p1/p2 swap) with popup; COMMANDLIST opens command list (CPS) and re-polls.
   - `input_ui_wait` delays repeated UI hotkey triggers.

Mapping to Emulated Inputs
--------------------------
- `input_map[]` maps PSP buttons to emulated inputs (`MAX_INPUTS` per system). `input_flag[]` is set each frame and consumed by `update_inputportX` to write into port value arrays (e.g., `cps1_port_value`).
- Service, coin, start, and directional/button bits are packed per hardware spec; DIP switch arrays (e.g., `cps1_dipswitch`) provide configuration readbacks.
- Analog handling:
  - CPS1 `forgottn` uses `input_analog_value` and `forgottn_update_dial`.
  - MVS/NCDZ can expose analog data via `poll_gamepad_analog` for specific drivers.

System-Specific Highlights
--------------------------
- **CPS1 (`src/cps1/inptport.c`)**:
  - Four port writers (`update_inputport0..3`) cover player inputs, system inputs, and special cases.
  - Supports autofire, controller swap, command list hotkey, screenshot, and service/start combos.
  - AdHoc thread sync with `adhoc_update` and per-frame merge; analog dial support for `forgottn`.
  - Saves DIP and controller state in save states; resets autofire on load.
- **CPS2 (`src/cps2/inptport.c`)**:
  - Similar structure; adds paddle support (`INPTYPE_pzloop2`) with extra port writer (`update_inputport3`) when needed.
  - Handles cps2-specific service/coin mapping and QSound volume fades via sound commands where applicable.
- **MVS (`src/mvs/inptport.c`)**:
  - Multiple player slots and special analog handling (fatfursp), plus coin/service mapping to Neo Geo slots.
  - AdHoc merge mirrors CPS logic; controller swap and screenshot support.
  - DIP switches handled via `mvs/dipsw.c`; service mode toggles may impact BIOS menus.
- **NCDZ (`src/ncdz/inptport.c`)**:
  - Simpler port set (three updates) with service/coin mappings; integrates CDZ-specific watchdog reset via inputs (none special).
  - No AdHoc path; uses same menu/service/screenshot handling as local branch.

Autofire Mechanism
------------------
- `af_map1/af_map2` hold pairs of PSP buttons for autofire groups; `update_autofire(buttons)` toggles mapped bits based on timer counters to simulate rapid-fire presses.
- Called after `adjust_input` so remaps/rotations apply before autofire.

Adjustments and UI Hooks
------------------------
- `adjust_input(buttons)` applies per-title quirks (e.g., rotate controls) defined in driver options.
- Menu integration: pressing Home or start+select opens the UI menu (`showmenu`), after which `setup_autofire` is rerun to sync settings.
- Snapshots (`input_flag[SNAPSHOT]`) call `save_snapshot`; command lists (CPS) open move lists.

Persistence and State
---------------------
- Save states capture controller selection, analog values, DIP switches, and service switch state; on load, autofire is reconfigured and UI delay reset.
- Game configs (`save_gamecfg`) persist input-related options (controller, autofire) on exit.

Netplay (AdHoc) Considerations
------------------------------
- AdHoc forces some options (e.g., controller role, fixed speed options) in `memory_init`.
- Input synchronization: AND merge of local/remote ports ensures both peers must press simultaneous inputs (e.g., both start) unless handled via p12_start_pressed logic; pause flag propagates between peers.
- Frame counters (`adhoc_frame`) accompany send_data to align inputs; minimal delay (`sceKernelDelayThread(100)`) avoids over-saturating the link.
