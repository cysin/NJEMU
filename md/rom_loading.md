ROM Loading and Driver Info
===========================

Overview
--------
- ROM loading is driven by per-system driver metadata (`load_rom_info`) and the ROM zips present in `roms/`.
- Data sources:
  - Driver tables (`CPS1_driver`, `CPS2_driver`, Neo Geo drivers) define ROM regions, filenames, CRCs, types, grouping, and skip bytes.
  - `zipname.dat` maps shortnames to user-friendly titles and parent relationships; loaded in the browser (`load_zipname`).
  - ZIP archives: expected to match MAME 0.152 CRCs (per README) and be placed under `roms/`.

Driver Info Parsing
-------------------
- `load_rom_info(game_name)` (in each `memintrf.c`) scans the driver table for a matching shortname:
  - Populates `machine_driver_type`, `machine_init_type`, `machine_input_type`, and `parent_name`.
  - Determines region sizes and fills `rom_t` arrays for CPU, gfx, sound, and user regions:
    - Fields: `type`, `offset`, `length`, `crc`, `name`, `group`, `skip`.
  - Returns non-zero on errors:
    - 1: game not supported (no driver)
    - 2: ROM not found
    - 3: rominfo not found
  - Errors are displayed on the load screen; user must acknowledge before returning to browser.

ROM Search and CRC Matching
---------------------------
- `file_open(fname1, fname2, crc, out_name)` (src/common/loadrom.c):
  - Searches up to three ZIPs in order:
    1) `game_dir/<fname1>.zip`
    2) `game_dir/<fname2>.zip` (parent) if `fname2` provided
    3) `launchDir/roms/<fname2>.zip` (fallback parent)
  - Opens ZIP, iterates entries to match `crc`; if not found but filename exists, returns CRC error (-2).
  - On success, returns a `rom_fd` handle (via `zopen`) and copies the actual filename into `out_name`.
  - `file_close()` closes the current zip and file.

ROM Loading (rom_load)
----------------------
- `rom_load(rom_t *list, UINT8 *dest, int index, int total)` (shared helper):
  - Handles grouping (`group`) and skipping (`skip`) to interleave bytes/words from sequential ZIP entries into the destination buffer.
  - Supports packed graphics layouts: e.g., group=2, skip=2 to place even/odd bytes in separate halves.
  - Updates `index` to the next ROM entry; repeated until all entries consumed.
  - Performs no decryption; some systems decode after load (CPS1 decode functions, CPS2 phoenix, MVS decryption).

Region Allocation and Offsets
-----------------------------
- Before loading, `memory_length_*` are set from driver region sizes or fixed (Neo Geo bios).
- `memalign(MEM_ALIGN, length)` allocates region buffers; zero-initialized to avoid uninitialized reads.
- Some regions are mirrors or shared:
  - CPS1/CPS2: `memory_region_user2` reused for Z80 in non-QSound; QSound uses dedicated 0x8000 buffer.
  - Neo Geo: fix layer (S ROM) and sprites (C ROM) may be cached or shared with parent.

Parent/Clone Handling and Formats
---------------------------------
- `parent_name` set by driver tables; if present, ROM search will try parent ZIP for missing regions.
- CPS1: Program ROMs often split in 0x20000 chunks; sound Z80 may bank-switch via `z80_set_bank`; decryptors (`*_decode`) run after load when `machine_init_type` requires it.
- CPS2: Program ROMs are encrypted; loader just copies raw data. GFX1 region may come from cache (`cache_start`) or from up to 32 ROM entries unpacked by group/skip rules. Phoenix sets adjust `cache_parent_name` to reuse parent gfx cache. Z80 ROMs unencrypted.
- MVS: Uses MAME-style sets with P/M1/V/C/S regions. C/S/V may be served from cache; otherwise loaded and decrypted per title in `neocrypt.c`. BIOS files (`000-lo.lo`, `sfix.sfix`, `sm1.sm1`, region BIOS/Unibios) come from BIOS ZIP as parent. CDZ path does not use zipname; BIOS is a file.
- NCDZ: BIOS (`neocd.bin`) loaded from filesystem; CD data comes from folder/ZIP (per `getDir` logic). IPL (`IPL.TXT`) must be valid; zoom ROM (`000-lo.lo`) read separately. MP3/CDDA handled later in CD modules.

Error Messaging
---------------
- `error_file(fname)` and `error_crc(fname)` print to the loading message screen.
- `fatalerror(TEXT(COULD_NOT_ALLOCATE_MEMORY...))` used for allocation failures.
- On ROM load errors, `Loop` set to `LOOP_BROWSER` after user acknowledges.

Special Cases
-------------
- CPS2 cache: gfx data may be pulled from `cache_dir` instead of ZIP (see `cache.md`).
- NCDZ: BIOS is loaded from filesystem (`neocd.bin`) not ZIP; CD data handled separately (`cd_audio.md`).
- MVS: Decryption of P/M1/V/C ROMs post-load; may allocate temporary user regions for decrypted output.
