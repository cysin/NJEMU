NJEMU Timing Architecture
=========================

Global Timing Concepts
----------------------
- Target frame durations are defined per system in `src/emucfg.h`:
  - CPS1: `FPS 60.0`, `TICKS_PER_FRAME 16666.67µs`
  - CPS2: `FPS 59.633333`, `TICKS_PER_FRAME 16769.14µs`
  - MVS/NCDZ: `FPS 59.185606`, `TICKS_PER_FRAME 16896µs`
- PSP display timing for pacing/frameskip: `PSP_REFRESH_RATE 59.94`, `PSP_TICKS_PER_FRAME 16683.33µs`.
- `ticker()` (src/psp/ticker.c) provides microsecond timestamps for pacing and FPS computation.

Frameskip and Pacing (emumain.c)
--------------------------------
- Frameskip table (`skiptable[12][12]`) cycles decisions per frame; `frameskip_counter` advances modulo 12.
- `autoframeskip_reset` seeds frameskip from options, resets counters, and marks warming phase (one vblank wait to sync).
- `update_screen`:
  - Skipped frames: skip overlays, still manage popups/volume flags.
  - Speedlimit: compute `target = this_frame_base + frameskip_counter * PSP_TICKS_PER_FRAME`; busy-wait to target; optional early flip if vsync and ahead of target.
  - FPS stats: every skipcount cycle compute `game_speed_percent` and `frames_per_second` using elapsed microseconds; autoframeskip adjusts up/down based on speed.
  - Flip buffers (`video_flip_screen`) after pacing; advance `frameskip_counter`.
- `skip_this_frame()` used by timer loops to skip expensive rendering.

System Timer Architecture
-------------------------
Each system owns a `timer.c` implementing a scheduler over microsecond slices per frame.

Common patterns:
```
timer_reset():
  base_time = 0; frame_base = 0; set time_slice = 1e6 / FPS
  init CPU execute pointers and cycles_per_usec conversions
  clear timer array
  install recurring interrupts (vblank, sound, raster)

timer_update_cpu():
  frame_base = 0; timer_left = time_slice (or 0 for raster scan loop)
  set per-frame IRQ timers (vblank/raster/sound)
  while time remains:
    timer_ticks = timer_left
    time = base_time + frame_base
    for each active timer: if expired -> fire callback, disable; track nearest expiry into timer_ticks
    run CPUs for timer_ticks (respect suspend flags)
    frame_base += timer_ticks; timer_left -= timer_ticks
  base_time += time_slice; wrap at 1s and adjust active timer expirations
  (Neo Geo only) perform screenrefresh if not skipped
```

CPS1 Timer (src/cps1/timer.c)
-----------------------------
- CPUs: 68000 @10MHz, Z80 @3.579545MHz (or 8MHz for QSound). `usec_to_cycles`/`cycles_to_usec` set in `timer_reset`.
- Recurring timers:
  - Vblank: `timer_set_vblank_interrupt` schedules at `USECS_PER_SCANLINE * 256` (~end of visible area).
  - QSound IRQ: 250 Hz timer when QSound enabled.
- `timer_update_cpu` slices the full `time_slice` with dynamic `timer_ticks` based on earliest timer expiry, executes both CPUs, and increments frame counters. Rendering is triggered inside vblank callback when not skipped.
- Spin handling: `timer_adjust` can suspend active CPU (`SUSPEND_REASON_SPIN`) and re-enable via `cpu_spin_trigger` to align dual-CPU timing (when Z80 waits).

CPS2 Timer (src/cps2/timer.c)
-----------------------------
- CPUs: 68000 @11.8MHz, Z80 @8MHz (unless suspended). Cycle counts per usec are hardcoded in `timer_update_cpu`.
- Recurring timers:
  - QSound interrupt at ~251 Hz.
  - Raster interrupts from registers `scanline1/scanline2`; set each frame if not `RASTER_LINES`.
  - Vblank interrupt scheduled each frame.
- Frame loop similar to CPS1 but supports raster slices: palette rebuild may happen before CPU run if driver flags demand (`driver->flags & 2`).
- Rendering is deferred to raster/vblank callbacks (not inside timer loop) to allow partial updates.

MVS Timer (src/mvs/timer.c)
---------------------------
- CPUs: 68000 @12MHz (`cycles_per_usec=12`), Z80 @4MHz. Tracks `global_offset` seconds.
- Two update handlers selected by `timer_set_update_handler`:
  - Normal: `timer_left = TICKS_PER_FRAME`, execute timers/CPUs then call `neogeo_vblank_interrupt`, wrap time, and refresh screen if not skipped.
  - Raster: scanline loop increments `timer_left` by `USECS_PER_SCANLINE` each line, fires timers/CPUs per slice, triggers `neogeo_raster_interrupt(scanline)` each line, then vblank wrap; partial refresh via `neogeo_screenrefresh` after frame if not skipped.
- Timers are integer microseconds (`int time`), base_time wraps every second adjusting active timers.
- `timer_getscanline` exposes current scanline for drivers (either actual raster counter or derived from `frame_base`).

NCDZ Timer (src/ncdz/timer.c)
-----------------------------
- Similar to MVS with NORMAL vs RASTER handlers; CPUs: 68000 cycles_per_usec=12, Z80=6.
- Normal handler calls `neogeo_interrupt` (CDZ-specific) instead of `neogeo_vblank_interrupt`.
- Raster handler iterates scanlines, firing callbacks and raster interrupts each line.
- `timer_get_time` and `timer_getscanline` mirror MVS for BIOS/driver use.

Callbacks and Interrupt Timing
------------------------------
- CPS1: `cps1_vblank_interrupt` sets IRQ2 and triggers `cps1_screenrefresh`+`cps1_objram_latch` if not skipped.
- CPS2: `cps2_raster_interrupt` asserts IRQ4 at configured scanline and does partial draw; `cps2_vblank_interrupt` asserts IRQ2, completes rendering, latches objram.
- MVS: `neogeo_vblank_interrupt` or `neogeo_raster_interrupt` invoked from timer loops; render happens after timer loop when not skipped.
- NCDZ: `neogeo_interrupt` / `neogeo_raster_interrupt` drive CDZ video; watchdog counters in run loop enforce resets if timing stalls (`ncdz.c:233-243`).

UI/Display Pacing vs Emulation Timing
-------------------------------------
- Emulation timing (`timer_update_cpu`) advances independent of PSP display pacing; `update_screen` uses `ticker()` to align flips to PSP refresh when `option_speedlimit` is on.
- Frameskip is based on real elapsed time vs target; emulation still executes full CPU slices each frame (even if video skipped) to keep game logic/audio in sync.
- Warming phase ensures first frames align to a vblank to avoid jitter.

Sleep/Power Considerations
--------------------------
- PSP power callback sets `Sleep` flag. Run loops pause CPU/timer execution by delaying threads until `Sleep` clears, then call `autoframeskip_reset` to resync pacing.
- CPS2/MVS call `cache_sleep(1/0)` during sleep to protect disk IO.

Sound Timers
------------
- Sound chip timers are driven via timer callbacks:
  - CPS1/CPS2: QSound IRQ at 250–251 Hz from `timer_set(QSOUND_INTERRUPT, ...)`.
  - MVS/NCDZ: YM2610 timers managed via callbacks `timer_callback_2610` (set elsewhere) in `timer` array; sound latch write scheduled via `SOUNDLATCH_TIMER`.
- These timers share the same scheduler as vblank/raster timers and thus align with CPU time slicing.
