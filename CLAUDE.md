# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

NJEMU is a collection of arcade emulators for PlayStation Portable (PSP), based on NJEmu 2.3.1. The project includes four separate emulators that share common infrastructure:

- **CPS1PSP**: Capcom System 1 (CPS1) emulator
- **CPS2PSP**: Capcom System 2 (CPS2) emulator
- **MVSPSP**: Neo-Geo MVS/AES emulator
- **NCDZPSP**: Neo-Geo CD emulator

Each emulator is built as a separate EBOOT.PBP file for PSP. The project uses MAME 0.152 romsets and supports both PSP Fat (firmware 1.50) and PSP Slim (firmware 3.71+).

## Build System

### Building an Emulator

The Makefile builds one emulator at a time. Edit the top of `Makefile` to select which emulator to build:

```make
# Uncomment ONE of these to select the emulator:
#BUILD_CPS1PSP = 1
BUILD_CPS2PSP = 1      # Currently selected
#BUILD_MVSPSP = 1
#BUILD_NCDZPSP = 1
```

Then run:
```bash
make
```

The build outputs are placed in either `SLIM/` or `3XX/` directories depending on PSP model.

### Build Configuration

Key build options (set at top of Makefile):
- `PSP_SLIM = 1`: Build for PSP Slim (3.71 firmware), omit for PSP Fat (1.50)
- `KERNEL_MODE = 1`: Enable kernel mode (PSP Fat only)
- `COMMAND_LIST = 1`: Enable command list/hotkey support
- `ADHOC = 1`: Enable Ad-Hoc multiplayer
- `SAVE_STATE = 1`: Enable save state functionality
- `UI_32BPP = 1`: Enable 32-bit color UI (vs 16-bit)
- `RELEASE = 1`: Build release version

### Prerequisites

Requires PSP SDK (PSPSDK) installed with toolchain:
- `psp-gcc`, `psp-g++`, `psp-ar`, etc.
- PSP SDK libraries
- Standard build tools: `make`, `mksfo`, `pack-pbp`

## Architecture

### High-Level Structure

```
src/
├── emumain.c/h          - Main emulation loop, frameskip, FPS control
├── emucfg.h             - System-specific configuration and defines
├── cps1/                - CPS1 emulator core
├── cps2/                - CPS2 emulator core
├── mvs/                 - Neo-Geo MVS emulator core
├── ncdz/                - Neo-Geo CD emulator core
├── cpu/                 - CPU emulators (M68000, Z80)
├── sound/               - Sound interface/emulation
├── common/              - Shared features (cache, ROM loading, save states, cheats)
├── psp/                 - PSP platform layer (video, audio, input, UI, file browser)
├── zip/zlib/            - ZIP/compression support
└── makefiles/           - Per-emulator makefile includes
```

### Emulator Core Organization

Each emulator (cps1, cps2, mvs, ncdz) follows a similar structure:
- `driver.c/h`: ROM definitions and machine driver tables
- `memintrf.c/h`: Memory interface and CPU memory maps
- `inptport.c/h`: Input port definitions and handling
- `vidhrdw.c/h`: Video hardware emulation (rendering)
- `sprite.c/h`: Sprite rendering
- `timer.c/h`: Timing and interrupt handling
- `*.c` (system-specific): System initialization and core logic

### Platform Abstraction (PSP)

The `src/psp/` directory provides the PSP-specific implementation:
- `psp.c/h`: Main PSP initialization and system control
- `video.c/h`: GU (Graphics Utility) rendering to PSP screen
- `sound.c/h`: Audio output via PSP audio library
- `input.c/h`: PSP controller input handling
- `ui*.c/h`: Menu system, file browser, settings UI
- `filer.c/h`: File browser implementation
- `config.c/h`: Configuration save/load
- `adhoc.c/h`: Ad-Hoc multiplayer support

### Compilation Model

The build system uses conditional compilation:
- Only ONE emulator is built at a time (selected via `BUILD_*` defines)
- `emucfg.h` contains `#if defined(BUILD_CPS1PSP)` blocks that configure system-specific parameters
- Each emulator includes its specific makefile from `src/makefiles/`
- The selected emulator's sources are compiled into a single binary

### Key Concepts

**Machine Driver**: Each game ROM has a "machine driver" entry in `driver.c` that specifies:
- Memory maps for CPUs
- Video/sprite configuration
- Sound hardware
- Input port mappings
- Initialization routines

**Memory Interface**: CPU emulators (M68000, Z80) access memory through abstraction layers in `memintrf.c`, which handle:
- ROM/RAM access
- Memory-mapped I/O
- Banking/address decoding

**Video Hardware**: Rendering is tile/sprite-based:
- Background tiles and sprites are decoded from ROM
- Rendered to offscreen buffers
- Composited and output via PSP GU library

**Cache System**: CPS2 on PSP Fat uses a cache system (`common/cache.c`) to pre-process graphics data to reduce runtime overhead on limited hardware.

## Common Files to Modify

When adding features or fixing bugs:
- **Emulator core logic**: `src/{cps1,cps2,mvs,ncdz}/*.c`
- **ROM definitions**: `src/{cps1,cps2,mvs,ncdz}/driver.c`
- **UI/Menu changes**: `src/psp/ui*.c`
- **Input mappings**: `src/psp/input.c` and emulator-specific `inptport.c`
- **Configuration**: `src/psp/config.c`
- **Build options**: Top of `Makefile` and `src/emucfg.h`

## Character Encoding

The codebase contains Japanese comments in Shift-JIS encoding. Command lists (for cheat/hotkey system) must use GBK charset (can convert with Notepad++).

## Version Information

Current version: 2.4.0 (defined in Makefile as VERSION_MAJOR/VERSION_MINOR/VERSION_BUILD)

ROM compatibility: MAME 0.152 romsets

## Additional Features

- Multi-language support (font files use GBK charset for Chinese)
- Cheat code support via command list system
- Save/load state functionality
- Ad-Hoc multiplayer (CPS1, CPS2, MVS only)
- DIP switch configuration
- Unibios support (MVS): Versions 1.0-4.0 and neogit bios
- PNG screenshot support
- Customizable wallpapers (32bpp mode)
