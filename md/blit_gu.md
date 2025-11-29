Blit/GU Rendering Internals
===========================

Overview
--------
- System renderers build scene data and pass it to PSP GU through a set of `blit_*` helpers. GU command list size (`GULIST_SIZE`) is tuned per system (48 KB CPS1/CPS2, 300 KB MVS/NCDZ) to hold sprites/tiles for a frame or slice.
- Buffers: `draw_frame`, `show_frame`, `work_frame`, `tex_frame` in VRAM; `video_init` sets GU draw/disp buffers and texture mode (5551 or 8888).

blit API (common helpers)
-------------------------
- `blit_start(start_line, end_line)` initializes the GU list for a slice, sets scissor to the region, and prepares texture/clut state.
- Layer-specific functions:
  - `blit_draw_fix(x, y, code, attr)`: draw fix-layer tiles (Neo Geo).
  - `blit_draw_spr(...)` / `blit_draw_spr_line(...)`: draw sprites with zoom/clip.
  - `blit_draw_scroll*`: draw CPS scroll layers; `blit_scrollh_clear_sprite` clears masks for priority handling.
- Finishing:
  - `blit_finish()` submits the list and syncs.
  - `blit_finish_spr`, `blit_finish_fix`, `blit_finish_object` finalize batch-specific primitives (CPS2 uses priority buckets).
  - `blit_reset()` clears internal state; called on video reset.

Data Packing and Priorities
---------------------------
- CPS1: Priority masks (`cps1_transparency_scroll`) control where sprites punch through scroll layers. `cps1_high_layer` determines interleave of scroll/sprite drawing.
- CPS2: Sprites drawn first; priority values in obj attributes (`attr` high bits) determine z-buckets. `blit_finish_object(prev+1, priority)` flushes sprite batches between scroll layers according to `pri_ctrl` mapping.
- Neo Geo: Sprites may be drawn via hardware-style list (`draw_sprites_hardware`) or software loop (`draw_sprites_software`) depending on slice height; fix layer drawn after sprites.
- Raster slices: CPS2 and Neo Geo use `start/end` or `current_line` to limit drawing to affected scanlines; reduces GU load and supports per-line effects.

Palette and Texture Formats
---------------------------
- Palette data expanded to 16-bit GU format; CPS uses `video_clut16` (precomputed Bayer/dither-friendly colors). Neo Geo maintains `video_palettebank[2][0x1000]` for two banks.
- Textures typically 16-bit 5551 (`pixel_format = GU_PSM_5551`), unless 32bpp mode is selected for UI; GU texture scale is set to `1/BUF_WIDTH`.
- Dithering: GU dither matrix set but disabled by default (`sceGuDisable(GU_DITHER)`).

Buffering and VSync
-------------------
- GU command list built against `draw_frame`; `video_flip_screen` swaps draw/show buffers and optionally waits vblank. `work_frame`/`tex_frame` used by UI/PNG/wallpaper routines as scratch.
- `update_screen` optionally does an early flip for vsync pacing if ahead of target time (speedlimit + vsync enabled).

Limits and Performance
----------------------
- `GULIST_SIZE` must be large enough for worst-case sprite/tile count; MVS/NCDZ use larger lists due to many sprites.
- `blit_draw` functions should avoid overdraw; sprite pen-usage tables (`gfx_pen_usage`) used to skip blank tiles.
- Raster rendering splits frames to reduce per-list size and support scanline effects.
