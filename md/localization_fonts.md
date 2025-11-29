Localization and Fonts
======================

Text Resources
--------------
- UI strings are loaded via `src/psp/ui_text.c` / `ui_text.h`. On startup, `ui_text_init` reads the PSP system language and selects English, Japanese, Spanish, Simplified Chinese, or Traditional Chinese; if unsupported, it falls back to English.
- Encodings: Bundled Japanese/Chinese strings and command lists assume GBK. External files (command.dat, cheats, zipname.dat) should be saved as GBK to avoid garbled glyphs; the renderer is 8-bit and not UTF-8 aware.
- Language choice is persisted in global settings; changing language reloads strings and menus.

Fonts
-----
- Font assets reside in `src/psp/font`: ASCII (`ascii_14*.c`, `latin1_14.c`), GBK tables (`gbk_s14.c`, `gbk_tbl.c`), shadows/icons (`bshadow.c`, `command.c`), and logos.
- Default font for 2.3.x uses SimHei glyph data (GBK) as noted in README. UI text rendering uses custom bitmap blits in `ui_text.c`/`ui_draw.c`.
- To add/replace fonts:
  - Supply bitmap arrays with the same layout/metrics as existing files.
  - Ensure GBK coverage if you expect Japanese/Chinese UI text.
  - Update loaders in `ui_text.c` if you change filenames or formats.

Icons and Colors
----------------
- Icons stored in `src/psp/icon`; palettes and color constants defined in UI code.
- Wallpapers (32bpp mode) in `src/psp/wallpaper`; selected via settings.

File Encoding Guidance
----------------------
- When editing text files (command.dat, cheats), convert to GBK to avoid garbled characters (Notepad++ “Convert to GBK” recommended in README).
- Config files and zipname.dat should also respect the chosen UI encoding to display titles correctly.
