ROM Tools and External Assets
=============================

romcnv Utilities
----------------
- `romcnv_cps2` / `romcnv_mvs` (see `romcnv/readme_cps2.txt`, `romcnv/readme_mvs.txt`) generate cache data for CPS2 and MVS:
  - Windows batch helpers: `cps2conv*.bat`, `mvsconv*.bat` build caches for a single ZIP or all ZIPs in a folder (with parent sets nearby).
  - Output: CPS2 creates `<game>.cache` under `cache/`; MVS creates `<game>_cache/` folder with crom/srom/vrom files. Copy to `launchDir/cache`.
  - Linux build: `make -f makefile.cps2 UNIX=1` or `make -f makefile.mvs UNIX=1`, then run `./romcnv_cps2 <rom.zip> [-zip]` or `./romcnv_mvs <rom.zip> [-all] [-slim]`.
  - `-all` walks a directory and converts all supported sets; `-slim` (MVS) skips PCM cache on Slim.
- `rominfo.cps2` / `rominfo.mvs` ship with the tools and must match the emulator version (cache format V22 per readmes).
- `cps2_zip_cache_batch.zip` holds batch scripts for compressed caches; not used by default build.

SystemButtons.prx
-----------------
- External PRX required for Home button handling; must be placed in `launchDir`.
- Obtain from project releases; not built from this source.

command.dat / zipname.dat
-------------------------
- command.dat: Standard MAME move list file; download/update externally and place alongside ROMs.
- zipname.dat: NJ-specific metadata file; may need regeneration when updating ROM sets; ensure encoding matches UI language (GBK for included data).

BIOS/Fonts/Wallpapers
---------------------
- BIOS files (MVS `neogeo.zip` content, NCDZ `neocd.bin`) and CD images must be sourced separately.
- Fonts/icons/wallpapers are bundled in `src/psp/font`, `src/psp/icon`, `src/psp/wallpaper`; custom replacements require matching formats and dimensions expected by UI code.
