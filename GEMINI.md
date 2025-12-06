# NJEMU Project Context

## Overview
NJEMU is a multi-system emulator originally designed for the PlayStation Portable (PSP). It supports:
- **CPS1** (Capcom System 1)
- **CPS2** (Capcom System 2)
- **MVS** (Neo-Geo MVS/AES)
- **NCDZ** (Neo-Geo CDZ)

The project is written in **C** and relies on MAME 0.152 ROM sets.
There is an active effort to port the emulator to PC/Linux using **SDL2**, as seen in the `CMakeLists.txt` and `porting` context.

## Directory Structure
- **`src/`**: Source code.
    - **`cps1/`, `cps2/`, `mvs/`, `ncdz/`**: Driver-specific code.
    - **`common/`**: Shared infrastructure (cache, loading).
    - **`cpu/`**: CPU cores (m68000, z80).
    - **`sound/`**: Sound chips (YM2151, etc.).
    - **`platform/`**: Platform abstraction (PSP specific or SDL specific).
- **`data/`**: Assets (images, fonts), ROM info.
- **`roms/`**: Directory for placing game ROM zip files.
- **`build/`**: Build artifacts directory.
- **`Makefile`**: Main build script for **PSP**.
- **`CMakeLists.txt`**: Build script for **SDL/PC Port**.

## Build Instructions

### 1. PC / SDL Port (Linux/Dev Env)
This is likely the primary target for current development/porting tasks.
**Requirements:** `cmake`, `gcc`, `libsdl2-dev`.

```bash
# Create build directory
mkdir -p build/sdl
cd build/sdl

# Configure (default is CPS1 port)
cmake ../.. 

# Build
make
```
*Target executable:* `cps1sdl`

### 2. PSP (Original)
**Requirements:** `pspdev` toolchain (`pspsdk`).

```bash
# Build default (CPS2 Slim)
make

# Build specific system
make BUILD_CPS1PSP=1
make BUILD_MVSPSP=1
make BUILD_NCDZPSP=1

# Clean
make clean
```

## Development Guidelines
*   **Language**: C (C99 standard).
*   **Style**:
    *   **Indentation**: Tabs.
    *   **Naming**: `snake_case` for functions/variables, `ALL_CAPS` for macros.
    *   **Comments**: C-style block comments `/* ... */`.
*   **Porting Strategy**:
    *   The codebase is transitioning from purely PSP-specific code (often mixed in `src/`) to a more platform-agnostic structure.
    *   `PLATFORM_SDL` macro is used for the SDL build.
    *   `PSP` macro is used for legacy PSP code.
*   **Testing**: No automated tests. Manual testing is required (run emulator, load ROM).

## Key Files
*   `src/emumain.c`: Main entry point logic.
*   `src/platform/main_sdl.c`: SDL specific entry point.
*   `src/common/loadrom.c`: ROM loading logic.
*   `AGENTS.md`: Detailed guidelines for AI agents (Check this for specific commit/PR rules).
