AdHoc / Netplay Architecture
============================

Overview
--------
- AdHoc multiplayer is optional (`ADHOC` build flag). It synchronizes inputs and some state between two PSPs over WLAN in infrastructure-less mode.
- Core files:
  - PSP networking driver: `src/psp/adhoc.c`
  - Common input sync thread: `src/common/adhoc.c`
  - Per-system init flows: CPS1 `cps1.c`, CPS2 `cps2.c`, MVS `mvs.c` (NCDZ has no AdHoc).
- AdHoc imposes stricter options (no vsync, fixed frameskip, sound on, 333 MHz CPU) for determinism and throughput.

Connection Flow
---------------
1) User enables AdHoc in menu (per-system option handlers).
2) On emulation start:
   - System init builds a matching string (`PBPNAME_STR` + game + BIOS for MVS).
   - `adhocInit(matching)` sets up WLAN modules.
   - `adhocSelect()` chooses server (host) or client (peer).
   - `adhoc_send_state`/`adhoc_recv_state` may exchange save state (CPS1/2).
3) If negotiation fails, `Loop` set to `LOOP_BROWSER` and emulation aborts.

Input Synchronization
---------------------
- `adhoc_update_inputport` thread (common/adhoc.c) handles network exchange of `send_data`/`recv_data`:
  - Fields: `port_value[]` (input bits), `buttons`, `loop_flag`, `paused`, `frame`.
  - Thread flips `adhoc_update` flag to indicate pending network data; per-frame `update_inputport` waits for it to clear before merging.
- Merge strategy (CPS/MVS):
  - Port values ANDed: `port_value_local & port_value_remote` to ensure both peers agree on inputs.
  - Analog values assigned per role (server vs client) for games needing dial/paddle.
  - Pause propagation: `paused` flag triggers `adhoc_pause()` on both ends.
  - `loop_flag` received from peer can force exit/reset.
- Frame pacing: `adhoc_frame` increments locally; small thread delay (`sceKernelDelayThread(100)`) avoids flooding.

Option Overrides in AdHoc
-------------------------
- Applied in `memory_init` when `adhoc_enable`:
  - CPS1: raster on, 333 MHz, vsync off, autoframeskip off, frameskip 0, showfps off, speedlimit on, sound on, samplerate 0.
  - CPS2/MVS: similar forced options; CPS2 raster option if available; MVS forces BIOS select via `bios_select`.
- Purpose: reduce jitter and keep both peers deterministic.

Networking Details
------------------
- `adhoc.c` handles:
  - Module loading (`pspSdkLoadAdhocModules`) when kernel build used.
  - Matching, connection, send/recv loops with timeouts (`ADHOC_TIMEOUT` 500 ms).
  - Sleep handling: if `Loop != LOOP_EXEC`, network thread exits early.
- Data transport is simple send/recv of small structs; no rollback or prediction.

UI/Control Integration
----------------------
- In AdHoc mode, pressing Home/start+select triggers pause request; both peers pause via `adhoc_paused`.
- Service/start combos still work; snapshots and menus are generally discouraged during sync because state is not mirrored.
- AdHoc does not replicate full game state continuously; only an optional initial send/recv state may occur on connect (CPS1/2).

Limitations and Notes
---------------------
- Determinism depends on both peers using identical ROM sets/BIOS; CRC mismatches not checked over network.
- Only two peers supported (server/client); no broadcast/multi-peer.
- Sleep/power: if PSP enters sleep, loops pause; network threads will stall until `Loop` changes or resume occurs.
