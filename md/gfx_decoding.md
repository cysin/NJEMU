Graphics Decoding Details
=========================

Overview
--------
- Rendering relies on pre-decoded graphics data (tiles/sprites) and per-tile pen-usage tables to skip transparent tiles.
- Decoding paths differ by system:
  - CPS1/CPS2: packed tile ROMs (GFX1) unpacked with group/skip patterns; optional stars; pen-usage built per tile size.
  - Neo Geo (MVS/NCDZ): fix layer (S ROM) and sprite C ROM decoding; zoom tables for CDZ.

CPS1 Decoding (src/cps1/vidhrdw.c)
----------------------------------
- `cps1_gfx_decode()` iterates over `memory_region_gfx1`:
  - Uses grouping/skipping to separate bitplanes and build 8x8/16x16 tiles.
  - Builds `cps1_object_pen_usage` and `cps1_scroll_pen_usage` arrays to mark opaque pens per tile, used to skip empty sprites/tiles during blit.
- Stars:
  - `cps1_has_stars` set by driver; starfield rendered separately in `cps1_render_stars`.
- Palette:
  - `video_clut16` LUT precomputed to expand 15-bit palette entries with brightness scaling.

CPS2 Decoding (src/cps2/vidhrdw.c)
----------------------------------
- Similar unpacking for scroll layers; sprites handled with additional priority/z fields.
- Pen-usage tables (`gfx_pen_usage[TILE08/16/32]`) populated during load to accelerate sprite culling.
- Palette buffer and mask handling for priority/masking effects; `cps2_check_scroll2_distort` handles row/column scroll warps.

Neo Geo Decoding (src/mvs/vidhrdw.c, src/ncdz/vidhrdw.c)
--------------------------------------------------------
- Fix layer:
  - Fix tiles decoded from S ROM into fix banks; `gfx_pen_usage` per bank used to skip empty tiles.
  - `neogeo_set_fixed_layer_source` switches between banks (AES/MVS differences, Garou banks).
- Sprites:
  - C ROM data interpreted with zoom/clip tables; pen-usage (`spr_pen_usage`) guides sprite drawing.
  - Zoom tables (CDZ): `build_zoom_tables` in `ncdz/memintrf.c` precomputes tile/skip tables from zoom ROM (`gfx3`).
- Palette:
  - `video_clut16` used to expand 15-bit palette entries; two palette banks in Neo Geo (`video_palettebank[2]`).

Group/Skip Patterns
-------------------
- `rom_load` honors `group` and `skip` fields per ROM entry:
  - `group` controls interleave (e.g., group=2 means every other byte goes to a different sub-block).
  - `skip` adds spacing between writes to match hardware bus widths.
- This logic is used to unpack GFX ROMs into linear tile data before pen-usage analysis.

Pen-Usage Generation
--------------------
- After decode, each tile/sprite code is scanned to flag used pens:
  - CPS1: `cps1_object_pen_usage`, `cps1_scroll2_pen_usage`, `cps1_scroll_pen_usage[4]`.
  - CPS2: `gfx_pen_usage[TILE08/16/32]`.
  - Neo Geo: `gfx_pen_usage` per bank (`fix` and sprites).
- Rendering skips tiles whose pen-usage is zero to reduce GU load.

Special Cases
-------------
- CPS1 kludges (non-release): adjust scroll offsets for certain bootlegs (`CPS1_KLUDGE_*`).
- CPS2 phoenix sets may have palette rebuild flags (`driver->flags & 2`) requiring palette build in timer loop.
- Neo Geo raster effects: partial refresh uses `next_update_first_line` to clip sprite drawing; zoom/clip tables honor sprite size/zoom per scanline.
