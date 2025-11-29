Performance and Tuning
======================

CPU Clock and Frameskip
-----------------------
- CPU clock (`psp_cpuclock`): Options 222/266/300/333 MHz. Higher clocks improve performance but drain battery; AdHoc forces 333.
- Frameskip:
  - `option_autoframeskip` dynamically adjusts frameskip to maintain target speed; uses measured FPS vs `PSP_REFRESH_RATE`.
  - `option_frameskip` sets fixed skip level when autoframeskip is off.
  - `option_speedlimit` + `option_vsync` enforce pacing to PSP refresh; disabling can improve speed but may tear.

Video/UI Cost
-------------
- `PSP_VIDEO_32BPP` with wallpapers costs more VRAM/bandwidth; 16bpp is lighter. On classic PSP, disable wallpapers for speed.
- Showing FPS and overlays adds minor overhead; can disable for maximum performance.
- Raster effects (CPS/MVS) are heavier; disable raster when not needed to increase FPS.

Cache Usage
-----------
- CPS2/MVS cache reduces RAM/IO pressure on classic PSP; enable USE_CACHE where available (auto-disabled on PSP Slim for CPS2).
- During sleep, cache is paused (`cache_sleep`) to avoid IO issues.

GU List Size
------------
- `GULIST_SIZE` set per system (48 KB CPS1/CPS2, 300 KB MVS/NCDZ). If adding more primitives, ensure the list size is sufficient to avoid overruns; balance against VRAM usage.
- Raster rendering splits frames to reduce per-list size; use raster partial refresh judiciously.

Sound
-----
- Samplerate option (0–2) trades quality for performance/resample overhead (non-CPS2).
- Muting sound (`option_sound_enable = 0`) reduces sound thread work but can desync if emulation relies on sound timers; avoid in AdHoc.

Cache vs RAM (PSP Slim vs Phat)
-------------------------------
- PSP Slim has more RAM; CPS2 cache disabled by default (`USE_CACHE=0`) to leverage RAM. Classic PSP should use cache for large ROMs.
- Suspend/resume on Slim for CPS2/MVS may dump/restore high memory to `resume.bin` when free RAM is low; be aware of suspend/resume time.

Wallpapers/UI Assets
--------------------
- Using wallpapers (32bpp) and large icons can increase bandwidth; disable wallpaper in settings for performance-critical cases.

Debug/Release
-------------
- `RELEASE` build removes some kludges/debug; enabling debug may add overhead. Use release builds for performance testing.

Tips
----
- For slow games on classic PSP: enable cache (if available), disable raster, set CPU to 333, turn off wallpapers/FPS overlay, and enable autoframeskip.
- For sync-critical multiplayer: keep vsync on, frameskip off, and use 333 MHz; ensure both peers match settings.
