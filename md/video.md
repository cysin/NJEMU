NJEMU Video Architecture
=======================

Top-Level Pipeline
------------------
- Systems render into software-managed buffers (tile/sprite composition), then feed the PSP GU via `blit_*` helpers and `blit_finish`.
- `update_screen` (src/emumain.c:202-307) overlays UI (FPS/volume/battery/popups), enforces speedlimit/vsync/frameskip, and flips buffers with `video_flip_screen`.
- PSP GU setup and buffer management live in `src/psp/video.c`: triple buffering (`show_frame`, `draw_frame`, `work_frame`, `tex_frame`), GU init (scissor, texture mode, dither), and swap via `sceGuSwapBuffers`.

Frame Timing and Invocation
---------------------------
- CPS1/CPS2: `cps1_vblank_interrupt`/`cps2_vblank_interrupt` (driver.c) trigger screen refresh when not skipped. CPS2 also refreshes on raster IRQ slices.
- MVS/NCDZ: `timer_update_cpu` calls `neogeo_screenrefresh` (or partial) when not skipped; raster mode calls `neogeo_partial_screenrefresh` each scanline.
- After system render, `update_screen` handles UI overlays and GU buffer swap with optional vsync wait.

PSP Video Layer (src/psp/video.c)
---------------------------------
- Initializes GU with chosen pixel format (5551 or 8888) and sets draw/disp buffers; clears all frames and enables display.
- `video_flip_screen(vsync)` waits vblank if requested, swaps draw/show buffers, and updates pointers.
- Utility helpers to clear/fill frames/rects and compute VRAM addresses for CPU-side blits.
- 32bpp mode (if `PSP_VIDEO_32BPP`) uses wallpaper/background textures; most emulation runs in 16bpp.

CPS1 Rendering (src/cps1/vidhrdw.c)
-----------------------------------
- Frame build:
  - Read layer control/scroll registers from output ports.
  - Determine layer order and enabled flags; calculate scroll offsets and star fields.
  - `blit_start` prepares sprite/tile buffers; optional transparency/priority masks set from driver data.
  - Render order: stars (if present) → layered scrolls/sprites via `cps1_render_layer`, respecting priority masks (`cps1_high_layer` for interleave).
- Object handling: `cps1_objram_latch` parses object RAM into `cps1_object` array, sets `cps1_last_object`, and rebuilds palette.
- Palette: `cps1_build_palette` uses `video_clut16` LUT (15-bit colors) and caches old palette for state save.
- Screenrefresh entry: `cps1_screenrefresh` invoked from vblank callback when not frameskipped.

CPS2 Rendering (src/cps2/vidhrdw.c)
-----------------------------------
- Supports raster partial refresh: `cps2_screenrefresh(start,end)` renders a slice between scanlines.
- Layer prep:
  - Read layer/video control, object offsets, priority control.
  - Build palette if needed (unless driver flags force per-frame rebuild).
  - `blit_start(start,end)` sets slice bounds.
  - Render sprites first (`cps2_render_object`), then interleave scroll layers by priority; `blit_finish_object` closes sprite batches between priority bands.
- Object latch: `cps2_objram_latch` reads double-buffered obj RAM, computes z-order (`z`), detects mask usage for priority masking.
- Palette/state saved via `STATE_SAVE(video)` with buffered palette handling.
- Raster IRQ registers (`scanline1/2`) set in output RAM drive partial refresh timing; vblank IRQ completes final draw and flips object buffers.

Neo Geo (MVS/NCDZ) Rendering
----------------------------
- Shared concepts between MVS (`src/mvs/vidhrdw.c`) and NCDZ (`src/ncdz/vidhrdw.c`):
  - Fix layer (8x8 tiles) drawn via `draw_fixed_layer*`, banked by `fix_bank` and pen-usage tables; palettes in `video_palettebank[2]`.
  - Sprite drawing uses hardware-style zoom/clip; two paths: `draw_sprites_hardware` or `draw_sprites_software` depending on slice height to balance performance.
  - Raster partial refresh: `neogeo_partial_screenrefresh(current_line)` starts a blit for accumulated lines, draws sprites for that span, and advances `next_update_first_line`.
  - Full refresh: `neogeo_screenrefresh` draws pending sprites to bottom, then fix layer, then `blit_finish`.
  - Video enable/disable and per-layer disable flags (`spr_disable`, `fix_disable`) gate rendering; if off, frame is cleared.
- NCDZ extras: loading screen render (`neogeo_loading_screenrefresh`) during CD access; `video_reset` toggles `video_enable`/layer disables and calls `blit_reset`; CD boots show title art when browsing.

Blit Layer (common to systems)
------------------------------
- `blit_start(start_line, end_line)` sets up draw bounds and prepares GU lists for sprites/tiles.
- Drawing helpers (per system) write into GU command buffers: `blit_draw_fix`, `blit_draw_spr`, `blit_draw_scroll*`, etc.
- `blit_finish` submits the built list; `blit_finish_object` batches sprite priorities (CPS2); `blit_finish_fix/spr/scrollh` finalize specific layers when needed.
- `blit_reset` clears state, often called on video reset.

Registers and Effects
---------------------
- CPS1/2:
  - Scroll base/offset ports (`CPS1_SCROLLx_*`), video control (`CPS1_VIDEO_CONTROL` bit15 flipscreen), layer control and priority arrays (`driver->layer_control`, `driver->priority[]`).
  - CPS2 raster: registers `scanline1/scanline2` set IRQ positions; `CPS2_OBJ_XOFFS/YOFFS` adjust sprite origins.
- Neo Geo:
  - Palette banks, fix banks (`neogeo_set_fixed_layer_source`), sprite control tables in video RAM (`sprite_zoom_control`, `sprite_x/y_control`).
  - Raster interrupts drive partial refreshes; video enable flags control full clear vs render.

Buffer Formats and Alignment
----------------------------
- Framebuffers (`show_frame`, `draw_frame`, `work_frame`, `tex_frame`) are GU-visible VRAM addresses; selected pixel format (5551 or 8888) chosen at init or via `video_set_mode`.
- Palette data kept in 16-bit PSP-native format; LUTs (`video_clut16`) precompute 15-bit color expansion for CPS/Neo Geo.
- Alignment macros (`ALIGN_PSPDATA`, `ALIGN_DATA`) ensure sprite/palette arrays are cache/GU friendly.
- GU list size tuned per system: 48 KB for CPS1/CPS2, 300 KB for MVS/NCDZ (set in `GULIST_SIZE`), to hold worst-case sprite/tile batches.

Skip/Frameskip Integration
--------------------------
- `skip_this_frame` used in timer loops to bypass rendering; callbacks check and avoid `screenrefresh` when skipped (CPS1/2, MVS/NCDZ).
- `update_screen` still manages UI overlays and flip when not skipped; warming phase waits a vblank to sync timing before starting frameskip counters.
