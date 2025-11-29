BIOS and ROM Set Expectations
=============================

General
-------
- ROMs are expected to match MAME 0.152 CRCs (per README). Place ZIPs under `roms/` with correct shortnames and parent relationships.
- `zipname.dat` provides title/parent metadata and flags (hack/bootleg/bad/not work) shown in browser; must be present for CPS/MVS/NCDZ (except NCDZ skips zipname load).

CPS1/CPS2
---------
- Use standard MAME ROM sets. Clones may reference parent for missing ROMs; `parent_name` set in driver info.
- CPS2 phoenix sets: cache/parent overrides (e.g., ssf2ta→ssf2t) handled in `memintrf`.
- EEPROM-backed titles (QSound/Pang3) save NVRAM on exit.

Neo Geo MVS/AES
---------------
- BIOS ZIP (often `neogeo.zip`) required; must contain:
  - `000-lo.lo` (lorom)
  - `sfix.sfix` (fix layer)
  - `sm1.sm1` (Z80 BIOS)
  - Region-specific BIOS ROMs (e.g., `uni-bios_*.rom`, `asia-s3.rom`, `vs-bios.rom`)
- BIOS selection:
  - Menu allows choosing Unibios (1.0–3.0), AES (Asia/Japan), MVS, NEOGIT, etc.
  - AdHoc may force BIOS selection; `bios_select` invoked in `memintrf`.
- Cache: Large C/S/V data may be cached; parent ROM reuse supported.
- Region: Set via menu; stored in config; affects language/violence/coinage.

Neo Geo CDZ
-----------
- Requires filesystem BIOS `neocd.bin` in `launchDir`; validated against CRC 0xdf9de490 and patched for CD control/exit.
- IPL: `IPL.TXT` must be present in CD image; loader (`cdrom_process_ipl`) verifies and processes it.
- CD images: Provide ISO/CSO with correct track layout; CDDA handled via MP3 playback (`cd_audio.md`).
- Memcard: `backup.bin` and per-game memcard saved/loaded automatically.

CRC and Validation
------------------
- Loader checks CRCs of ROM entries against driver tables; mismatches reported as CRC errors.
- BIOS errors (missing/invalid) displayed via message boxes; emulation aborted to browser.

Paths
-----
- ROM ZIPs: `roms/<game>.zip`
- Cache (if used): `cache/<game>_cache/*`
- BIOS/CD (NCDZ): `launchDir/neocd.bin`, CD images in `roms/` or specified path.
