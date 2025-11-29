Threading and Synchronization
=============================

Thread Inventory
----------------
- **Main thread**: Runs browser/UI and the emulation loop (CPU/video/input). Entry in `src/psp/psp.c`.
- **Sound thread**: `sound_update_thread` (src/psp/sound.c) pulls mixed audio and feeds PSP SRC output.
- **AdHoc input thread** (optional): `adhoc_update_inputport` (src/common/adhoc.c) handles network send/recv of inputs.
- **Power callback thread**: `CallbackThread` (src/psp/psp.c) registers power callbacks; not long-lived.

Synchronization Primitives
--------------------------
- Flags and globals, no mutexes:
  - `Loop` controls main state (BROWSER/EXEC/RESET/RESTART/EXIT).
  - `Sleep` flag set by power callbacks to pause emulation and sound thread output.
  - AdHoc: `adhoc_update` flag indicates network thread has data to merge; `adhoc_frame` increments per send.
  - Sound: `sound_active`, `sound_enable` control thread loop and muting.
  - Cache: `cache_sleep(1/0)` used to gate cache IO during sleep for CPS2/MVS.

Thread Interactions
-------------------
- Main ↔ Sound:
  - `sound->update` set by `sound_init`; sound thread calls it continuously.
  - `sound_thread_enable/mute` toggles output; on sleep, thread spins with delays until `Sleep` clears.
  - `sound_thread_stop` waits for thread exit with `sceKernelWaitThreadEnd`.
- Main ↔ AdHoc:
  - `update_inputport` waits while `adhoc_update` is set (network thread busy), then merges `recv_data` with `send_data`.
  - After merging and preparing new `send_data`, main sets `adhoc_update = 1` for the network thread to transmit.
  - Network thread sleeps briefly (`sceKernelDelayThread(100)`) to throttle traffic.
- Main ↔ Power:
  - Power callback sets `Sleep` on suspend, clears on resume; main loops check `Sleep` each frame and delay until clear, then `autoframeskip_reset`.
  - PSP Slim memory preservation for CPS2/MVS may dump/restore extra RAM to `resume.bin` in the power callback.

Timing and Threads
------------------
- Emulation timing (`timer_update_cpu`) is single-threaded in main; sound output timing is decoupled via the sound thread and resampling bookkeeping.
- AdHoc input sync is coarse-grained per frame; no rollback/prediction. Network delays can cause short stalls in `update_inputport` waiting for `adhoc_update` to clear.

Guidelines / Hazards
--------------------
- No locking: shared flags must be updated carefully; threads rely on cooperative waits and short critical sections.
- Avoid long-running work in sound or AdHoc threads; they are intended to be lightweight.
- Sleep handling: ensure `Sleep` is checked in loops that may block (sound thread, main loop) to avoid deadlocks on suspend.
