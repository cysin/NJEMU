Build and Deployment
====================

Source Layout
-------------
- Make-based build system in root `Makefile`; system-specific makefiles in `src/makefiles/`.
- Four build targets controlled by defines: `BUILD_CPS1PSP`, `BUILD_CPS2PSP`, `BUILD_MVSPSP`, `BUILD_NCDZPSP` (set via Makefile or command line; only one should be 1).
- SDK dependencies: PSP SDK toolchain, pspsdk headers/libraries, GU/audio/ctrl/power modules. Kernel builds require kernel stubs and patches (`pspSdkInstallNoPlainModuleCheckPatch`, etc.).
- Typical build steps:
  1) Install pspsdk and ensure `psp-config` is on PATH.
  2) `make clean` (optional), then `make BUILD_CPS1PSP=1` (or CPS2/MVS/NCDZ target). Default Makefile has `BUILD_CPS2PSP=1`, `PSP_SLIM=1`, `COMMAND_LIST`, `ADHOC`, `SAVE_STATE`, `UI_32BPP` enabled.
  3) Outputs: `SLIM/<SYSTEM>/EBOOT.PBP` when `PSP_SLIM=1`, or `3XX/<SYSTEM>/EBOOT.PBP` when cleared. PRX mode builds `<TARGET>.prx` if `BUILD_PRX=1`.
  4) Copy EBOOT plus assets (`SystemButtons.prx`, `zipname.dat`, configs, wallpapers) to PSP/PPSSPP folder.
  5) For kernel builds, ensure appropriate flags in Makefile and kernel-mode modules allowed on device.

Key Flags and Modes
-------------------
- `PSP_SLIM` toggles cache usage (CPS2) and extra RAM handling.
- `PSP_VIDEO_32BPP` controls use of 32bpp GU mode and wallpapers.
- `ADHOC` enables AdHoc multiplayer code and requires loading adhoc modules.
- `SAVE_STATE`, `COMMAND_LIST`, `RELEASE` affect feature set and debug kludges.
- `KERNEL_MODE` builds kernel PRX and spawns user thread (`user_main`).
- `ADHOC_UPDATE_EVERY_FRAME` toggles input sync cadence.
- `USE_CACHE` controls cache usage (CPS2/MVS); set by platform and build defines.

Outputs
-------
- PBP/module info set in `src/psp/psp.c` via `PSP_MODULE_INFO` macros per build; `PBPNAME_STR` and `VERSION_STR` derived from root Makefile.
- SystemButtons.prx loaded at runtime from `launchDir`.
- Screenshots saved to `ms0:/PICTURE/<SYSTEM>` with names `<game>_##.png`.

ROM/Cache Expectations
----------------------
- ROMs placed under `launchDir/roms/`. Zipped sets from MAME 0.152 (per README) with correct CRCs.
- Parent/clone relationships resolved via `zipname.dat` (loaded in browser for CPS/MVS).
- CPS2/MVS cache files under `launchDir/cache/` if cache is used (classic PSP).
- NCDZ: Requires `IPL.TXT`, BIOS (`neocd.bin`) in `launchDir`, and CD images.
- SystemButtons.prx should live in `launchDir` for Home-button handling.

Tooling
-------
- `romcnv` directory likely contains ROM conversion utilities (not invoked by Makefile).
- No automatic dependency fetching; assumes pspsdk is installed and in PATH.
- Common build issues:
  - Missing pspsdk includes/libs → install/update toolchain.
  - Wrong build target flags → ensure only one BUILD_* is set at a time.
  - Kernel vs user mismatch → adjust `KERNEL_MODE` and module info macros.

Runtime Paths and Persistence
-----------------------------
- `launchDir` derived from current working directory; used for configs, cache, state, memcard/NVRAM.
- Memcards: `memcard/<game>.bin` (MVS), `nvram/<game>.nv`.
- States: `state/<game>.svN`.
- Configs: global system INI (cps1psp.ini/cps2psp.ini/mvspsp.ini/ncdzpsp.ini) plus per-game `config/<game>.ini`.

Running on PSP / PPSSPP
-----------------------
- HOME button integration via SystemButtons PRX; on PPSSPP/PSV where Home is unavailable, delete SystemButtons.prx and use SELECT+START for menu (per README).
- PSP Slim power handling: may dump/restore extra RAM to `resume.bin` on suspend for CPS2/MVS (see `src/psp/psp.c`).
