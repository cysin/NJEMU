# Repository Guidelines

## Project Structure & Module Organization
- Core sources live in `src/`, split by system: `src/cps1`, `src/cps2`, `src/mvs`, and `src/ncdz` house emulator drivers; `src/common`, `src/cpu`, `src/sound`, `src/zip`, `src/zlib`, and `src/libmad` provide shared infrastructure; `src/psp` wraps PSP UI/input/video. Headers are under `src/include` and `src/SDK/include`.
- Assets and lookup tables used by the UI and ROM menus are under `data/` (`rominfo`, `zipname`, `SystemButtons`).
- Reference docs and gamelists live in `docs/` (platform-specific readmes and ROM lists). No automated test directory exists yet.

## Build, Test, and Development Commands
- Requires the PSP toolchain (`pspdev`/`pspsdk`) available on PATH with `PSPDEV` and `PSPSDK` set.
- Default build (CPS2 Slim target): `make` → produces `SLIM/CPS2/EBOOT.PBP`.
- Build a different system: `make BUILD_CPS1PSP=1`, or `make BUILD_MVSPSP=1`, or `make BUILD_NCDZPSP=1` (only one BUILD_ flag should be `1`). Add `PSP_SLIM=` to build for 3.xx firmware layout.
- Clean artifacts: `make clean`.
- Manual test: copy the generated `EBOOT.PBP` to a PSP/PPSSPP memory stick folder (`/PSP/GAME/CPS2/` etc.), place the correct ROM set in the expected directory, launch, and exercise core flows (menu navigation, load, save state, audio/video).

## Coding Style & Naming Conventions
- C code with tab indentation; follow existing alignment and keep preprocessor blocks tight. Use `snake_case` for functions/variables, ALL_CAPS for macros/constants, and `static` for translation-unit locals.
- Comments use the existing C block style and should explain intent, not mechanics. Keep strings ASCII unless the file already contains specific encodings (some fonts/labels use GBK).
- Prefer adding options via the config headers (`emucfg.h`) and guarding features with existing `#ifdef` flags (`ADHOC`, `SAVE_STATE`, `COMMAND_LIST`, etc.).

## Testing Guidelines
- No automated tests are present; rely on build success plus manual run on real hardware or PPSSPP. When adding platform features, verify both Slim and 3.xx layouts if applicable.
- For bug fixes, add a small reproducible scenario to the PR description (ROM name, menu path, buttons pressed) so reviewers can replicate.

## Commit & Pull Request Guidelines
- Git history uses short, descriptive summaries (e.g., `compiling on fedora and add unibios 4.0 support for mvs`). Keep the subject under ~70 chars, describe the main effect, and group unrelated changes into separate commits.
- PRs should include: scope summary, target system (CPS1/CPS2/MVS/NCDZ), flags toggled, manual test notes (hardware/PPSSPP, ROMs tried), and any asset changes under `data/`. Attach screenshots for UI-visible tweaks when possible.
- Link related issues or discussions and call out compatibility risks (firmware expectations, BIOS requirements, ROM set version 0.152).
