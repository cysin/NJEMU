Errors, Messages, and Logging
=============================

Message/Status Output
---------------------
- `msg_printf(...)`: Writes formatted text to the message screen during loading/reset/exit (e.g., "LOADING <file>", "CHECKING_ROM_INFO").
- `msg_screen_init(background, icon, title)`: Prepares a background (logo/wallpaper) and header for status messages.
- Many loader steps call `msg_printf(TEXT(...))` to show progress (CRC checks, decrypting, cache checks).

Fatal Errors
------------
- `fatalerror(const char *fmt, ...)` sets a global fatal flag and message, and forces `Loop = LOOP_BROWSER`.
- `show_fatal_error()` displays a modal dialog with the fatal message:
  - Uses logo background, title icon, and waits for any button press.
  - Includes battery/volume overlays; exits to browser afterwards.
- Typical fatal sources:
  - Memory allocation failures (`COULD_NOT_ALLOCATE_MEMORY...`)
  - Missing critical files (`COULD_NOT_OPEN_ZIPNAME_DAT`, missing BIOS)
  - Audio init failures (`COULD_NOT_RESERVE_AUDIO_CHANNEL_FOR_SOUND`, `COULD_NOT_START_SOUND_THREAD`)
  - Cache failures (unable to open cache files when required)
  - CD/IPL errors for NCDZ (`ERROR_WHILE_PROCESSING_IPL_TXT`)

Load/ROM Errors
---------------
- `error_file(fname)`: ROM file not found; printed on load screen.
- `error_crc(fname)`: ROM file found but CRC mismatch; printed on load screen.
- After reporting, loader waits for button press (`pad_wait_press`) and returns to browser (`Loop = LOOP_BROWSER`).
- BIOS errors (NCDZ) displayed via `messagebox` with specific codes (BIOS not found/invalid).

Browser/UI Errors
-----------------
- In browser, missing `zipname.dat` triggers `fatalerror` then `show_fatal_error`, followed by `show_exit_screen`.
- Version/device errors (e.g., PSP Slim requirement for CPS2/MVS) displayed via `messagebox` and exit to browser.

Runtime Popups
--------------
- `ui_popup`/`ui_show_popup` provide transient notifications (e.g., controller swap, reset warnings, watchdog resets).
- `save_snapshot` shows popup on success; watchdog in NCDZ run loop pops `RESET_CAUSED_BY_WATCHDOG_COUNTER`.
- Controller swap popup (`CONTROLLER_PLAYERx`), memcard/NVRAM warnings, cache status messages may appear depending on system.

Logging and Debug
-----------------
- No verbose file logging; all messaging is onscreen.
- `njemu_debug` global exists but not widely used; default 0.
- Developers can add `msg_printf` or `fatalerror` for additional diagnostics; no stdout/stderr on PSP runtime.

Return Paths
------------
- Fatal or load errors return to browser with `Loop = LOOP_BROWSER`.
- `Loop = LOOP_EXIT` triggers exit screen; `LOOP_RESET/RESTART` handled internally by system loops.

Message Reference (common TEXT IDs)
-----------------------------------
- LOAD/ROM: `LOAD_ROM`, `CHECKING_ROM_INFO`, `ROMSET_x`, `ROMSET_x_PARENT_x`, `THIS_GAME_NOT_SUPPORTED`, `ROM_NOT_FOUND`, `ROMINFO_NOT_FOUND`, `COULD_NOT_OPEN_ZIPNAME_DAT`.
- CRC/FILE: `LOADING <file>`, `COULD_NOT_ALLOCATE_MEMORY_...`, `COULD_NOT_ALLOCATE_MEMORY_FOR_DECRYPT_ROM`, `ERROR_WHILE_PROCESSING_IPL_TXT`.
- Sound: `COULD_NOT_RESERVE_AUDIO_CHANNEL_FOR_SOUND`, `COULD_NOT_START_SOUND_THREAD`.
- Cache: cache open failures surfaced via `fatalerror`.
- BIOS: `BIOSNOTFOUND`, `BIOSINVALID` (NCDZ), `CHECKING_BIOS`.
- Misc: `PLEASE_WAIT`, `EXIT_EMULATION2`, watchdog/reset popups.
