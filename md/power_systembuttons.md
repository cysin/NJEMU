Power Handling and SystemButtons
================================

Power/Suspend Behavior
----------------------
- Power callbacks registered in `src/psp/psp.c` (`PowerCallback`, `CallbackThread`):
  - On suspend (`PSP_POWER_CB_POWER_SWITCH`): sets global `Sleep = 1`.
    - PSP Slim + CPS2/MVS: if free RAM below 0x400000, dumps 4MB from high mem to `resume.bin` under `launchDir` to preserve cached data.
  - On resume (`PSP_POWER_CB_RESUME_COMPLETE`): reloads `resume.bin` into high mem if present and deletes it; clears `Sleep = 0`.
- Main emulation loops check `Sleep` each frame:
  - While `Sleep`, delay (`sceKernelDelayThread(5,000,000)`), pause cache IO (CPS2/MVS `cache_sleep(1)`), and on wake reset frameskip (`autoframeskip_reset`).
- Sound thread also checks `Sleep` and idles until cleared to avoid audio underruns.
- `Loop` drives exit/reset: `show_exit_screen` paints a wait dialog when `Loop == LOOP_EXIT` so suspend/resume doesn’t leave a blank screen.

SystemButtons PRX
-----------------
- `SystemButtons.prx` provides Home button intercept on PSP/PPSSPP/PSV; loaded at startup:
  - Path: `<launchDir>/SystemButtons.prx`
  - Loaded via `pspSdkLoadStartModule` in `src/psp/psp.c`; if loaded, sets `systembuttons_available = 1`.
  - Home button events read via `readHomeButton()` (from `SystemButtons.h`).
- If PRX is missing or on PPSSPP/PSV where Home cannot be used:
  - README suggests deleting PRX and using `SELECT+START` to open emulator menu.
  - In `update_inputport`, menu opens on Home if available, else Start+Select combination.

Menu and Exit Controls
----------------------
- Emulator menu (in-game) invoked via Home (PRX) or Start+Select combos; `showmenu()` handles UI and returns to game.
- Browser/menu exit uses `Loop` transitions; `show_exit_screen` displays a wait dialog on exit.

PSP Slim Specifics
------------------
- Extra RAM handled by dumping/restoring to `resume.bin` on suspend for CPS2/MVS when memory tight.
- CPS2 cache disabled on Slim by default (USE_CACHE=0) due to available RAM.

Recommendations
---------------
- Keep `SystemButtons.prx` alongside the EBOOT for best UX on real PSP; remove it for PPSSPP/PSV if Home is unavailable.
- Avoid heavy disk/cache operations immediately before suspending; rely on `Sleep` checks to pause.
