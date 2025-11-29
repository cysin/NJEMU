UI, Browser, and Settings
=========================

Overview
--------
- The PSP shell handles ROM browsing, settings storage, language/fonts, wallpapers, and menu dialogs. Core files:
  - Browser: `src/psp/filer.c`
  - UI drawing/helpers: `src/psp/ui*.c`, `src/psp/wallpaper.c`, `src/psp/font`, `src/psp/icon`, `src/psp/wallpaper`
  - Config: `src/psp/config.c`, `src/psp/config.h`, settings files under `src/psp/config/`
  - Menus: `src/psp/ui_menu.c` plus per-system menu handlers in `src/psp/menu/`
  - Text/localization: `src/psp/ui_text.c`, `src/psp/ui_text.h`
  - Fonts/icons assets: `src/psp/font`, `src/psp/icon`

ROM Browser (src/psp/filer.c)
-----------------------------
- Entry from `main` -> `file_browser()`.
- Flow:
  1) Allocate `files[]` entries; load settings (`load_settings`), wallpaper/background, and title/logo splash.
  2) Load `zipname` metadata (MAME names) except NCDZ. Validate devkit model for CPS2/MVS on PSP Slim.
  3) Set `curr_dir = launchDir + "roms"`, call `getDir` to populate entries; support folders, zip files, command dat.
  4) Wait for button press, optionally check BIOS (NCDZ) and display errors.
  5) Main loop: draw directory listing with icons, flags (bad/hack/bootleg/not work), scrollbar, battery/volume. Supports title images for NCDZ.
  6) User actions: up/down/dir nav, launch (sets `game_dir`, `game_name`, cache paths), menu toggle, parent dir, delete zipname cache, sort toggles.
  7) On launch: free entries, call `emu_main()`, then return to browser if `Loop` non-zero; rebuild file list and wallpaper.
- `Loop` values drive exit/restart; `show_exit_screen` paints a wait screen when leaving.

UI Drawing (src/psp/ui_draw.c, ui.c)
-------------------------------------
- Primitives: dialogs (`draw_dialog`), icons (`small_icon*`), text (`uifont_print*`), gradients, scrollbars, battery/volume indicators.
- Backgrounds: `load_background`/`show_background` select from wallpaper enum (logo/filer/etc.); if 32bpp mode and wallpaper enabled, `load_wallpaper` renders PSP backgrounds.
- Popups: `ui_popup`/`ui_show_popup` display transient messages; `ui_popup_reset` clears queue.

Menus and System Options
------------------------
- Menu driver: `ui_menu.c` hosts generic menu loop, message boxes, sliders, selectors.
- Per-system option handlers in `src/psp/menu/`:
  - `cps.c`: CPS1/CPS2 options (raster enable, autofire, cheats, controller swap, video stretch, speedlimit/vsync, DIP config, EEPROM ops).
  - `mvs.c`: Neo Geo options (BIOS select, raster, region, memcard ops, cache toggles).
  - `ncdz.c`: CDZ options (BIOS boot toggle, region, CDDA, speedlimit).
- Options affect globals in `emumain.c` and per-system settings; saved via `save_settings`/`save_gamecfg`.
- Menu triggers: Home/start+select (if SystemButtons available) in `update_inputport`; also accessible from browser.
- DIP editors per system expose hardware DIP values with live effect on input/coinage.

Config and Persistence (src/psp/config.c)
------------------------------------------
- Settings files stored under `launchDir` (e.g., `nj.bin`, per-system cfg). Structure contains video/sound/input/cache preferences.
- Functions: `load_settings`, `save_settings`, `load_gamecfg`, `save_gamecfg`, `checkStartupDir`.
- `option_*` globals populated on load; menu writes back and saves on exit.

Fonts, Localization, Icons
--------------------------
- Fonts in `src/psp/font`; `ui_text.c` loads localized strings (GBK Japanese/Chinese support). README notes GBK charset for command lists.
- Icons in `src/psp/icon`; color palettes defined in UI code.
- Title/logo rendering in browser uses `logo()` and `small_icon_shadow` for system badges.

Wallpaper and Themes
--------------------
- `src/psp/wallpaper.c` loads images from `src/psp/wallpaper` for 32bpp mode; used in browser and dialogs.
- In 16bpp mode, UI fills background with palette colors instead of wallpaper.

Input to UI
-----------
- Browser uses `pad_wait_press`, `pad_wait_clear`, and direct `poll_gamepad` to navigate.
- Menus rely on `pad_update` + `pad_pressed` helpers to drive selection with repeat handling.
- Command.dat presence shows an icon in the browser for games with move lists (when COMMAND_LIST enabled).

Error and Help Screens
----------------------
- Fatal errors displayed via `show_fatal_error` (emumain) with dialog and battery/volume overlays.
- Message boxes (`messagebox`) used throughout browser/menu for errors (ROM missing/CRC, BIOS missing), version warnings, and waits (`PLEASE_WAIT`).
