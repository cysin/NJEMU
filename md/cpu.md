CPU Core Notes
==============

Overview
--------
- NJEMU uses Cyclone 68000 and CZ80 cores for emulated CPUs.
  - Source: `src/cpu/m68000/*`, `src/cpu/z80/*`
- Timer layers (per system) drive CPU execution by providing `execute` function pointers and `icount` pointers plus cycle↔usec conversion factors.

Cyclone 68000
-------------
- `m68000_execute(cycles)` runs for requested cycles, decrementing `C68K.ICount`.
- Interrupt control via `m68000_set_irq_line(level, HOLD_LINE/ASSERT/CLEAR)` used in vblank/raster callbacks.
- Reset via `m68000_reset()` per system reset; PC can be forced for NCDZ boot logic.
- Memory access bridged through system `m68000_read_memory_x` / `m68000_write_memory_x` functions (memintrf.c per system).
- Suspend/spin:
  - Timer code uses `timer_suspend_cpu` to set `SUSPEND_REASON_*` bits; `cpu_spin_trigger` re-enables when spin wait completes.
  - CPS1 sets spin timer for Z80 waits; MVS/NCDZ timer handles spin to align dual CPU timing.

CZ80
----
- `z80_execute(cycles)` runs CPU for given cycles; `CZ80.ICount` pointer used by timers to compute elapsed time.
- Interrupts via `z80_set_irq_line` (e.g., QSound, YM2610 timers).
- Banking: CPS1 Z80 uses `z80_set_bank` to copy 0x4000 pages into 0x8000–0xbfff.
- Suspends:
  - CPS2 uses `z80_suspended` flag (phoenix sets, boot states) to skip execution.
  - Timer suspend available via `timer_suspend_cpu` in MVS/NCDZ.

Cycle Timing
------------
- Per-system timer_reset sets `usec_to_cycles` or fixed multipliers:
  - CPS1 (`src/cps1/timer.c`): 68k at 10 MHz; Z80 at 3.579545 MHz unless QSound, then 8 MHz. `usec_to_cycles`/`cycles_to_usec` used to slice frames.
  - CPS2 (`src/cps2/timer.c`): 68k executed with `(timer_ticks * 11.8MHz)`, Z80 at 8 MHz unless `z80_suspended` is set.
  - MVS (`src/mvs/timer.c`): 68k at 12 MHz, Z80 at 4 MHz with `cycles_per_usec` integers.
  - NCDZ (`src/ncdz/timer.c`): 68k at 12 MHz, Z80 at 6 MHz.
- Timers convert elapsed microseconds (`timer_ticks`) to cycle counts before calling `execute`.

Memory Interfaces
-----------------
- Each system provides `m68000_read/write_memory_8/16/32` and `z80_read/write_memory_8` handlers to map CPU address space to ROM/RAM/IO.
- CPS1/CPS2 use XOR addressing (`offset ^ 1`) for endian swap; Neo Geo handlers include palette, video registers, controller ports, sound latches, memcard.

State and Save/Load
-------------------
- CPU registers/ICount are serialized in save states through state.c hooks.
- Active CPU tracking (`active_cpu` in timers) allows `getabsolutetime` to include in-progress cycles when scheduling timers.

Compatibility Flags
-------------------
- CPS1/CPS2 drivers may set `machine_driver_type`, `machine_init_type`, etc., influencing sound CPU speed, EEPROM usage, and decode paths.
- CPS2 phoenix sets may set `z80_suspended` on boot to emulate resurrection behavior.
