Cache System
============

Purpose
-------
- Reduce RAM and load times for large ROM sets (CPS2 and MVS) by pre-processing graphics/audio data into cache files stored on the memory stick.
- Allows classic PSP (32 MB) to run large sets by streaming cached chunks instead of holding full ROMs in RAM.

Key Files
---------
- Common: `src/common/cache.c`, `src/common/cache.h`
- Accessors: `cachefile_open` in `src/common/loadrom.c`
- System integration:
  - CPS2: `src/cps2/memintrf.c` (USE_CACHE conditional)
  - MVS: `src/mvs/memintrf.c`

Cache Layout
------------
- Directory: `<launchDir>/cache/`
- Files per game (or parent) with suffix `_cache/`:
  - `cache_info`: metadata (sizes, CRCs, parent usage flags)
  - `crom`: cached C (sprite) ROM data
  - `srom`: cached S (fix) ROM data
  - `vrom`: cached V (audio) ROM data
- Parent reuse:
  - Flags `use_parent_crom/srom/vrom` in `cache.c` allow child sets to reuse parent caches when identical.
  - For CPS2 phoenix sets, `cache_parent_name` can be overridden (e.g., ssf2ta → ssf2t).

Workflow
--------
1) `cache_init()` called in `memory_init` (CPS2/MVS):
   - Determines cache_dir, parent usage, and opens `cache_info` to populate sizes.
2) When loading ROM regions:
   - If cache enabled and file exists, `cachefile_open(CACHE_*)` returns FD; data is read instead of ZIP files.
   - If cache missing, fallback to ROM ZIPs and may build cache (build path not in-tree; usually prebuilt).
3) Sleep handling:
   - During PSP sleep, CPS2/MVS run loops call `cache_sleep(1)` before sleeping and `cache_sleep(0)` after resume to avoid mid-transfer issues.
4) Parent selection:
   - CPS2 sets `cache_parent_name` based on game/parent heuristics.
   - MVS uses BIOS parent flags to decide reuse.

Constraints and Safety
----------------------
- USE_CACHE is disabled on PSP Slim for CPS2 (more RAM); enabled on classic PSP.
- Cache files are read-only during emulation; generation tools are external (not included).
- On errors opening cache files, code falls back to ROM loading and may prompt fatal error if memory insufficient.
