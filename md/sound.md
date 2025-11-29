NJEMU Sound Architecture
========================

Overview
--------
- Sound is split into two layers:
  1) Emulation backends (chip cores, mixing, resampling) in `src/sound`.
  2) PSP audio output thread in `src/psp/sound.c` that pulls mixed samples and streams to the SRC hardware.
- System selection:
  - CPS1: YM2151 or QSound depending on `machine_sound_type`.
  - CPS2: QSound only.
  - MVS/NCDZ: YM2610 (ADPCM/FM) with associated timers.
- Shared interface: `struct sound_t *sound` (buffer size, frequency, channels, stack), `sound->callback` (chip mixer), and `sound->update` (mixer+resampler wrapper set per system).

PSP Output Thread (src/psp/sound.c)
-----------------------------------
- Lifecycle:
  - `sound_thread_init()` resets thread state; `sound_thread_start()` reserves SRC channel (`sceAudioSRCChReserve(samples, freq, 2)`), creates `sound_update_thread`, and starts it; `sound_thread_stop()` stops thread and releases channel.
  - Volume set via `sound_thread_set_volume` (maps option 0–10 to `PSP_AUDIO_VOLUME_MAX`); enable/disable via `sound_thread_enable` (mute on pause).
  - `sound_thread_exit()` is called after emulation ends to stop and clean up.
- Thread loop (`sound_update_thread`):
  - If `Sleep` flag set (from power callback), delay until cleared.
  - If `sound_enable` true, call `(*sound->update)(sound_buffer[flip])`; else zero buffer.
  - Output via `sceAudioSRCOutputBlocking(sound_volume, sound_buffer[flip])`; ping-pong buffers with `flip ^= 1`.

Sound Interface and Mixing (src/sound/sndintrf.c)
-------------------------------------------------
- Buffers: 32-bit interleaved mix buffers `stream_buffer_left/right` (+SAFETY padding; CPS2 sets SAFETY=0).
- Update paths:
  - CPS1/CPS2: `sound_update_stereo` directly clips each sample from stream buffers to 16-bit output (no resample).
  - MVS/NCDZ and CPS1 mono: resampling path (if not CPS2): `samples_per_update` derived from chip frequency, FPS, and option_samplerate (0–2). `resample_stream` uses fixed-point stepping; `clip_stream` clamps to `MAXOUT/MINOUT`.
  - Mono (CPS1 with mono sound): `sound_update_mono` duplicates resampled mono to stereo outputs.
- Initialization `sound_init()`:
  - Starts chip backend (`qsound_sh_start`, `YM2151_sh_start`, or `YM2610_sh_start`).
  - Chooses `sound->update` (mono vs stereo).
  - Zeroes mix buffers; sets `stream_buffer` pointers.
  - For non-CPS2, computes `samples_per_update`, primes `samples_left_over`/`samples_this_update` to handle fractional steps between frames.
  - Starts PSP sound thread; returns 0 on failure after raising `fatalerror`.
- Teardown/reset:
  - `sound_exit()` stops chip backends and halts sound thread.
  - `sound_reset()` calls chip resets and unmutes by re-enabling `option_sound_enable`.
  - `sound_set_samplerate()` (non-CPS2) recomputes resample step and calls chip rate setters.
- Mute control: `sound_mute(int mute)` disables/enables the PSP thread output based on `option_sound_enable`.

Chip Backends and Shared RAM
----------------------------
- QSound (CPS1/CPS2):
  - Backends in `src/sound/qsound.c`; scheduled QSound IRQ at ~250–251 Hz via `timer_set(QSOUND_INTERRUPT, ...)` in system timers.
  - CPU ↔ sound comms through shared RAM windows:
    - CPS1: `qsound_sharedram1/2` mapped at 0xf18000/0xf1e000; command writes via `cps1_sound_command_w`.
    - CPS2: similar shared RAM and commands in memintrf.c.
- YM2151 (CPS1):
  - Interface in `2151intf.c`; optional MSM6295/OKI pairing for some titles (`machine_sound_type` differentiates).
  - Uses Z80 for sound program; timers serviced via main timer loop.
- YM2610 (MVS/NCDZ):
  - Interface in `2610intf.c`; drives ADPCM and FM; timers hooked via `timer_callback_2610` entries in the system timer arrays.
  - Sound latch writes scheduled by timer callbacks in `timer.c` (e.g., `SOUNDLATCH_TIMER`).

Timing Integration
------------------
- Per-frame, `timer_update_cpu` drives sound timers:
  - QSound interrupts scheduled at fixed Hz; trigger Z80 IRQ (`z80_set_irq_line`) and reschedule.
  - YM2610 timers (A/B) scheduled via `timer_adjust` and callbacks; keeps chip envelopes and ADPCM clocks accurate.
- Audio output decoupled from CPU frames: PSP thread continuously pulls mixed samples sized for `sound->samples` (chip-defined, e.g., 44100/48000) and resampler bridges frame boundaries using fractional step bookkeeping.

Per-System Sound Paths
----------------------
- CPS1:
  - `machine_sound_type` decides QSound vs YM2151 path; `memintrf.c` wires `z80_read_memory_8/z80_write_memory_8` to QSound or standard sound handlers.
  - QSound uses 8MHz Z80; standard uses ~3.58MHz.
  - Option samplerate (0–2) influences resampling step; mono titles use `sound_update_mono`.
- CPS2:
  - Always QSound; samplerate fixed (no resample); stereo path used.
  - Z80 may be suspended (phoenix/boot states) via `z80_suspended` flag in timer.
- MVS:
  - YM2610; option_samplerate influences resample step (0=high rate); sound latch and timers in `mvs/timer.c`; memintrf maps Z80 accessors.
- NCDZ:
  - Same YM2610 path as MVS; integrates CDDA separately in `psp/mp3.c` and `ncdz/cdda.c` (not part of this interface) with control hooks in `ncdz.c` run loop.

Buffers and Alignment
---------------------
- `SOUND_BUFFER_SIZE` (from `emumain.h`) defines PSP output chunk size; `sound_buffer[2][...]` aligned for SRC DMA (`ALIGN_PSPDATA`).
- Mix buffers include `SAFETY` padding to avoid overrun during resample; CPS2 sets SAFETY=0 because it runs at native sample cadence.
- Stream buffers zeroed after each mix (CPS1/CPS2) to avoid leftover samples bleeding between frames.

Error Handling and Options
--------------------------
- Failure to reserve audio channel or start thread triggers `fatalerror(TEXT(COULD_NOT_RESERVE_AUDIO_CHANNEL_FOR_SOUND))` or `...START_SOUND_THREAD`.
- Volume controlled by `option_sound_volume`; enable/disable by `option_sound_enable`.
- Option_samplerate (non-CPS2) maps to dividing effective samples per update by powers of two.

Netplay/AdHoc Considerations
----------------------------
- AdHoc affects input and timing but sound path stays local; CPS1/CPS2/MVS ensure sound options are forced enabled in AdHoc to keep sync (`option_sound_enable = 1` during memory_init).

Persistence
-----------
- CPS1/CPS2 QSound titles save EEPROM/NVRAM on exit (`cps1_nvram_read_write`) to preserve sound chip state.
- MVS/NCDZ persist memcard/NVRAM, not chip state; audio state rebuilt on reset via chip resets.
