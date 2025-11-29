NJEMU Memory Architecture
=========================

Overview
--------
- Each system owns a `memintrf.c` that defines ROM/RAM regions, allocates buffers, and exposes CPU read/write handlers.
- ROM metadata is parsed from per-system driver tables (loaded via `load_rom_info`) to size regions and enumerate files. Actual data is read from ZIPs using `common/loadrom.c` (`file_open`, `rom_load`) with CRC checks; some systems stream from cache files or CD media.
- Memory layout favors aligned allocations (`MEM_ALIGN` 4 or 16) with explicit length globals (e.g., `memory_length_cpu1`) and region pointers (e.g., `memory_region_cpu1`).
- Static working RAM, palette RAM, sprite/object RAM, and shared sound RAM live in system-specific arrays (aligned for GU/DMA requirements).

ROM Loading Workflow
--------------------
1. `memory_init` (system-specific):
   - Clears region pointers/lengths, shows “LOAD ROM” UI, and loads ROM info to select the driver/parent set (`load_rom_info`).
   - Loads per-game config and (if available) command list; AdHoc forces fixed options (333 MHz, no vsync/frameskip, sound on).
   - Sets CPU clock via `set_cpu_clock(psp_cpuclock)` before allocations.
2. Region sizing/allocations:
   - CPS1/CPS2 compute lengths from ROM info; MVS/NCDZ set fixed sizes for BIOS/backup/memcard and driver ROM sizes from tables.
   - `memalign` used for most buffers; CPS2/MVS/NCDZ also place large blocks in PSP Slim extra RAM via `psp2k_mem_offset` when available. Allocation failure triggers `error_memory`/`fatalerror`.
3. File acquisition:
   - `file_open` searches `game_dir/<game>.zip`, parent ZIP, or `launchDir/roms/<parent>.zip` by CRC and name; returns `rom_fd`.
   - `rom_load` copies/byteswaps/group/skips based on ROM descriptors (`type/group/skip`) to pack interleaved graphics/program data.
   - CPS2/MVS optionally read from cache files (`common/cache.c`) to avoid RAM/IO pressure.
   - NCDZ reads BIOS from filesystem (`neocd.bin`) and CD content via `cdrom_*`.
4. Post-load transforms:
   - CPS1: decrypt drivers (e.g., `wof_decode`, `dino_decode`) based on `machine_init_type`; configure sound handlers (QSound vs YM2151).
   - CPS2: Phoenix/parent handling, palette pen-usage tables; encrypted ROMs handled elsewhere in driver.
   - MVS: C/S/V decryption per title (e.g., `kof99_decrypt_68k`, `neogeo_cmc50_m1_decrypt`); parent ROM reuse for cache.
   - NCDZ: builds zoom tables for sprites in `build_zoom_tables` (allocates `memory_region_user2`).
5. Handlers wiring:
   - Set Z80 read/write function pointers (e.g., CPS1 QSound vs standard).
   - Shared RAM pointers wired (e.g., `qsound_sharedram1/2` or Neo Geo sound latch).
6. On failure: UI shows error, waits for button, and sets `Loop = LOOP_BROWSER`.

Address Maps (Runtime)
----------------------
### CPS1 (`src/cps1/memintrf.c`)
- Program ROM: `0x000000-0x1fffff` → `memory_region_cpu1`.
- Work RAM: `0xff0000-0xffffff` (`static_ram1` backing `cps1_ram`).
- Scroll/obj RAM: `0x900000-0x92ffff` (`static_ram2` backing `cps1_gfxram`).
- IO/ports (~`0x800000` block): inputs, dips, EEPROM, coin control, output ports, sound commands.
- QSound shared RAM: `0xf18000` ranges via `qsound_sharedram1/2`.
- Z80 banking via `z80_set_bank` copies 0x4000 pages into 0x8000-0xbfff window.

### CPS2 (`src/cps2/memintrf.c`)
- Program ROM: `0x000000-0x3fffff` → `memory_region_cpu1` (encrypted content; driver decrypts during execution).
- Work RAM: multiple banks `static_ram1..6` mapped into `0xff0000` etc.
- CPS2 object RAM double-buffered (`cps2_objram[2]`) and `cps2_output` for raster registers.
- QSound shared RAM mirrors CPS1 layout at `0xf18000` range.
- Palette/tile/object data in `memory_region_gfx1` (possibly cached); pen-usage tables for rendering.

### MVS (Neo Geo) (`src/mvs/memintrf.c`)
- Program ROM (`P`): `memory_region_cpu1` sized per driver; mapped at `0x000000`.
- Backup RAM (`neogeo_sram16`), Work RAM (`neogeo_ram`), and memcard (`neogeo_memcard` at `0x800000`).
- Graphics:
  - `C` ROMs (sprites) -> `memory_region_gfx2` (or cached to disk).
  - `S` fix layer -> `memory_region_gfx1`.
  - `M1` Z80 ROM -> `memory_region_cpu2`.
  - `V` audio samples -> `memory_region_sound1/2`.
  - BIOS (`SP`/`LO`) -> `memory_region_user1`.
  - Optional `user2/3` for decrypted/intermediate buffers.
- Memory map handlers in `mvs/memintrf.c` route 68000 reads to palette, video registers, controllers, Z80, watchdog.

### NCDZ (`src/ncdz/memintrf.c`)
- Program ROM buffer: `memory_region_cpu1` 0x200000; BIOS in `memory_region_user1`.
- CD data buffers: `memory_region_gfx2` (CROM), `gfx3` (zoom tables source), `sound1` (ADPCM), `user2` (zoom tables) allocated on demand.
- Work RAM and memcard similar to MVS; IPL/BIOS patched for CD control in `load_bios`.
- CD-ROM read pipeline handled in `cdrom_*` modules; main memintrf only sets up buffers and validators.

Cache System
------------
- CPS2/MVS use `cache_dir` under `launchDir/cache` to store preprocessed ROM chunks:
  - `cache_info` describes sizes/CRC and parent usage.
  - `crom`, `srom`, `vrom` store graphics/audio slices to reduce runtime IO/memory.
- `cache_init` decides whether to use parent caches and sets `use_parent_*` flags; `cachefile_open` opens correct file.
- During sleep (`Sleep` flag) CPS2/MVS call `cache_sleep(1/0)` to safely pause disk access.

Shared RAM and Communication
----------------------------
- CPS1/CPS2: `qsound_sharedram1/2` provide 68000↔Z80 communication; sound commands written via `cps1_sound_command_w`.
- MVS/NCDZ: sound latch via `neogeo_sound_write` (timer callback) and Z80 ports; memcard/NVRAM accessed through mapped handlers (`neogeo_memcard16_r/w`).
- AdHoc netplay merges input ports only; memory state is not mirrored except optional send_state in connection setup.

Persistence
-----------
- MVS: Memcard `memcard/<game>.bin` and NVRAM `nvram/<game>.nv` loaded/saved on init/exit (`mvs.c:60-185`); NVRAM is byte-swapped (`swab`) before write.
- NCDZ: `backup.bin` global memcard snapshot loaded in `memory_init` and saved in `memory_shutdown`; BIOS/IPL validated each boot.
- CPS1/CPS2: EEPROM for some titles (QSound/Pang3) saved via `cps1_nvram_read_write` on exit; `EEPROM_init` called in `driver_init`.
- Game configs stored via `save_gamecfg` per title on exit.

Decryption/Transformation Hotspots
----------------------------------
- CPS1: Title-specific decode functions (e.g., `wof_decode`, `dino_decode`) modify program ROM in place. Kabuki variants use alternative Z80 handlers.
- CPS2: Phoenix sets/parent fallback handled before ROM load; decryption handled in driver code (not in memintrf).
- MVS: Numerous `*_decrypt_*` functions for P (68k) and M1 (Z80) regions; may allocate temporary buffers (`memory_region_user2/3`) for decrypted output.
- NCDZ: Zoom table builder allocates `user2` and precomputes sprite line tables from `gfx3` zoom ROM; BIOS is patched to enable CDDA/control.

Runtime Memory Safety and Alignment
-----------------------------------
- All large regions allocated with `memalign(MEM_ALIGN, length)` to ensure 4–16 byte alignment for CPU/GU performance; memset to zero after allocation.
- GU-aligned buffers for palette/VRAM (`ALIGN_PSPDATA`, `ALIGN_DATA`) reduce cache misses on PSP hardware.
- PSP Slim extra RAM: CPS2/MVS/NCDZ track `psp2k_mem_left` and place large GFX/PCM buffers there when cache is disabled (Slim builds). Power suspend may dump/restore this area to `resume.bin` (`src/psp/psp.c:30-118`); `psp2k_mem_move/free` compact and release high-memory blocks on shutdown.

Error Handling and UI
---------------------
- Missing/CRC mismatched ROMs trigger `error_file`/`error_crc`, shown on the load screen; user must acknowledge before returning to browser (`Loop = LOOP_BROWSER`).
- `fatalerror` sets a global message and leaves emulation; `show_fatal_error` displays it on exit.
- Cache/alloc failures use `error_memory(<region>)` to surface which region failed.
