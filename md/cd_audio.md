CD / Audio Resources (NCDZ)
===========================

Overview
--------
- NCDZ emulation includes CD-ROM data streaming and CDDA/MP3 audio playback in addition to YM2610 sound.
- Core modules:
  - CD-ROM: `src/ncdz/cdrom.c` (data reads, sector buffering)
  - CDDA: `src/ncdz/cdda.c` (audio track control)
  - MP3 playback: `src/psp/mp3.c` (PSP-side MP3 decoder used for CDDA)
  - Integration points in `src/ncdz/ncdz.c`

CD-ROM Data Path
----------------
- BIOS/boot sequence expects IPL data (`IPL.TXT`) and game data from disc image (ISO/CSO).
- `cdrom_init()` sets up reading; `cdrom_process_ipl()` processes IPL and loads into memory (`memory_region_cpu1` etc.).
- During gameplay, `cdrom_*` functions fetch sectors on demand; loading screens trigger `neogeo_loading_screenrefresh` to draw progress while data streams.
- Watchdog in run loop (`watchdog_counter`) forces reset if data load hangs.

CDDA / MP3 Playback
-------------------
- `cdda_init()` initializes audio playback; uses PSP MP3 decoder in `psp/mp3.c`.
- Controls:
  - `cdda_play(track)`, `cdda_stop()`, `cdda_pause()` manage audio; `neogeo_cdda_check()` is called each frame in run loop to update playback state.
  - On snapshot or exit, CDDA is stopped (`cdda_stop` in `ncdz_run` exit path).
- Integration with sleep:
  - When saving snapshots (`save_snapshot` in `emumain.c`), MP3 is paused (`mp3_pause`) to avoid buffer underruns; resumed afterward.

Video Interaction
-----------------
- During CD loads, `neogeo_loading_screenrefresh` renders progress; after completion, `neogeo_loadfinished` flag triggers enabling sprites/fix layers (`video_enable_w`, `fix_disable_w`, `spr_disable_w`).

Persistence and Errors
----------------------
- BIOS/ROM validation happens in `memory_init`; CD-ROM errors set `fatal_error` and show message (`ERROR_WHILE_PROCESSING_IPL_TXT`).
- No CDDA state saved in save states; playback restarts according to game logic after load/reset.

Limitations
-----------
- Only NCDZ uses CD/MP3 path; CPS1/CPS2/MVS rely on cartridge ROM data only.
- Requires valid CD images and `IPL.TXT`; BIOS patches applied in `load_bios` to enable CD control and exit hooks.
