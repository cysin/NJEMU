# NJEMU SDL2 Port - Complete Usage Guide

## Table of Contents
1. [Quick Start](#quick-start)
2. [ROM Installation](#rom-installation)
3. [Controls](#controls)
4. [Building from Source](#building-from-source)
5. [Configuration](#configuration)
6. [Troubleshooting](#troubleshooting)
7. [Advanced Features](#advanced-features)

---

## Quick Start

### Prerequisites
- Linux operating system (Ubuntu, Debian, Fedora, Arch, etc.)
- SDL2 libraries installed
- CPS2 ROM files (legally obtained)

### Installation

1. **Install Dependencies**
```bash
# Ubuntu/Debian
sudo apt-get install libsdl2-dev libpng-dev zlib1g-dev cmake build-essential

# Fedora
sudo dnf install SDL2-devel libpng-devel zlib-devel cmake gcc-c++

# Arch Linux
sudo pacman -S sdl2 libpng zlib cmake base-devel
```

2. **Build the Emulator**
```bash
cd NJEMU
mkdir -p build
cd build
cmake .. -DBUILD_CPS2PSP=ON
make -j$(nproc)
```

3. **Run the Emulator**
```bash
./CPS2PSP
```

---

## ROM Installation

### ROM Format
NJEMU uses MAME-format ROM sets. You need CPS2 `.zip` ROM files.

### Directory Structure
```
NJEMU/
├── build/
│   ├── CPS2PSP          # Emulator executable
│   └── roms/             # Place ROM files here
│       ├── 19xx.zip
│       ├── ddtod.zip
│       ├── msh.zip
│       ├── sfa3.zip
│       └── ...
```

### Supported Games
NJEMU supports **ALL CPS2 games**, including:
- Street Fighter Alpha/Zero series
- Marvel vs. Capcom series
- X-Men vs. Street Fighter
- Vampire Savior/Darkstalkers
- Street Fighter III series
- And many more!

### Installing ROMs

1. **Create ROM directory**:
```bash
mkdir -p roms
```

2. **Copy your ROM files**:
```bash
cp /path/to/your/roms/*.zip roms/
```

3. **Launch emulator**:
```bash
./CPS2PSP
```

4. **Select your game** from the menu using number keys

### ROM Requirements
- ROMs must be in MAME .zip format
- Both split and merged ROM sets are supported
- Parent and clone ROMs are automatically handled
- CRC32 checksums are verified during loading

---

## Controls

### Keyboard Mapping

#### Menu Navigation
- **Arrow Keys**: Navigate menu
- **Enter**: Select/Start
- **Escape**: Back/Exit

#### In-Game Controls

| Key | CPS2 Function | Description |
|-----|---------------|-------------|
| **Arrow Keys** | D-Pad | Movement (Up/Down/Left/Right) |
| **Enter** | Start | Pause/Start button |
| **Right Shift** | Select | Coin/Select button |
| **A** | Square (Weak Punch) | Light attack |
| **S** | Cross (Medium Punch) | Medium attack |
| **Z** | Triangle (Strong Punch) | Heavy attack |
| **X** | Circle (Weak Kick) | Light kick |
| **Q** | L Trigger | Special button 1 |
| **W** | R Trigger | Special button 2 |
| **Escape** | Menu | Return to menu |

### Game Controller Support

NJEMU automatically detects and supports:
- Xbox 360/One controllers
- PlayStation 3/4/5 controllers
- Generic USB game controllers
- Any SDL2-compatible controller

#### Controller Mapping
- **D-Pad/Left Stick**: Movement
- **A Button**: Cross (Medium attack)
- **B Button**: Circle (Light kick)
- **X Button**: Square (Light attack)
- **Y Button**: Triangle (Heavy attack)
- **Start**: Pause/Start
- **Select/Back**: Coin
- **L Shoulder**: Special 1
- **R Shoulder**: Special 2

---

## Building from Source

### Build Options

NJEMU supports building multiple emulators:

#### CPS2 Emulator
```bash
cmake .. -DBUILD_CPS2PSP=ON
make -j$(nproc)
```

#### CPS1 Emulator
```bash
cmake .. -DBUILD_CPS1PSP=ON
make -j$(nproc)
```

#### MVS (Neo Geo) Emulator
```bash
cmake .. -DBUILD_MVSPSP=ON
make -j$(nproc)
```

#### Neo Geo CD Emulator
```bash
cmake .. -DBUILD_NCDZPSP=ON
make -j$(nproc)
```

### Build Configuration

#### Debug Build
```bash
cmake .. -DBUILD_CPS2PSP=ON -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
```

#### Release Build (Optimized)
```bash
cmake .. -DBUILD_CPS2PSP=ON -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

#### With Save State Support
```bash
cmake .. -DBUILD_CPS2PSP=ON -DSAVE_STATE=ON
make -j$(nproc)
```

---

## Configuration

### Settings File
Configuration is stored in: `~/.config/njemu/settings.cfg`

### Options Available

#### Video Settings
- **Video Mode**: 16-bit or 32-bit color
- **Stretch Mode**: Various aspect ratio options
- **VSync**: Enable/disable vertical sync
- **Show FPS**: Display frames per second

#### Sound Settings
- **Sound Enable**: On/Off
- **Sample Rate**: 11025 Hz, 22050 Hz, 44100 Hz
- **Sound Volume**: 0-10 (0 = mute, 10 = max)

#### Performance Settings
- **Speed Limit**: Enable/disable speed limiting
- **Frame Skip**: 0-11 (0 = no skip)
- **Auto Frame Skip**: Automatic frame skip for performance

### In-Emulator Settings
Settings can be adjusted via the in-game menu (press Escape during gameplay).

---

## Troubleshooting

### Common Issues

#### "No ROMs found"
**Solution**:
- Ensure ROM files are in the `roms/` directory
- ROMs must be in `.zip` format
- Check that ROM files are valid MAME dumps

#### "CRC32 not correct"
**Solution**:
- Your ROM file may be corrupted or wrong version
- Download a different ROM dump
- Verify ROM integrity

#### "Could not allocate memory"
**Solution**:
- Your system may be low on RAM
- Close other applications
- Try a different game (some require more memory)

#### No Sound
**Solution**:
```bash
# Check SDL2 audio is working
sdl2-config --libs
# Verify ALSA/PulseAudio is running
pulseaudio --check
```

#### Game Runs Too Fast/Slow
**Solution**:
- Enable "Speed Limit" in settings
- Enable "Auto Frame Skip"
- Adjust frame skip manually

#### Black Screen
**Solution**:
- ROM may not be loading correctly
- Check terminal output for error messages
- Verify ROM is supported CPS2 game

#### Input Not Working
**Solution**:
- Verify SDL2 recognizes your input device
- Check `dmesg` for controller connection
- Try keyboard controls first

---

## Advanced Features

### Frame Skip
Frame skip improves performance on slower systems by skipping rendering of some frames.

- **0**: No frame skip (best quality)
- **1-11**: Skip frames (better performance)
- **Auto**: Automatically adjust for 60 FPS

### VSync
Vertical sync eliminates screen tearing but may introduce input lag.

- **Enabled**: Smooth display, slight lag
- **Disabled**: Lower lag, possible tearing

### Save States
*(If built with -DSAVE_STATE=ON)*

Save and load game state at any time:
- **F5**: Quick save
- **F7**: Quick load
- **Shift+F5-F8**: Save to specific slot
- **Shift+F7**: Load from slot

### Screenshots
Take screenshots during gameplay:
- **F12**: Capture screenshot
- Saved to `screenshots/CPS2/` directory

### Performance Monitoring

Enable FPS display to monitor performance:
```
Show FPS: ON
```

Display shows:
- **fskp/auto**: Frame skip mode
- **NN**: Current frame skip level
- **NNN%**: Game speed percentage
- **NNfps**: Rendered frames per second

### Debug Mode

For developers and troubleshooting:
```bash
./CPS2PSP --debug
```

Shows additional information:
- ROM loading details
- Memory allocation
- Timing information
- Input states

---

## Performance Tips

### Optimal Settings for Speed
```
Video Mode: 16-bit
VSync: OFF
Auto Frame Skip: ON
Sound: 22050 Hz
```

### Optimal Settings for Quality
```
Video Mode: 32-bit
VSync: ON
Frame Skip: 0
Sound: 44100 Hz
```

### Low-End Systems
- Use 16-bit video mode
- Enable auto frame skip
- Lower sound sample rate to 22050 Hz
- Disable VSync

### High-End Systems
- Use 32-bit video mode
- Disable frame skip
- Max sound quality (44100 Hz)
- Enable VSync for smooth display

---

## Command-Line Options

```bash
./CPS2PSP [options] [rom_name]
```

### Options
- `--help`: Show help message
- `--version`: Show version information
- `--debug`: Enable debug output
- `--sound-off`: Start with sound disabled
- `--fps`: Show FPS counter
- `--fullscreen`: Start in fullscreen mode
- `rom_name`: Load specific ROM directly

### Examples

```bash
# Load specific game
./CPS2PSP sfa3

# Run with FPS display and sound off
./CPS2PSP --fps --sound-off

# Debug mode with specific ROM
./CPS2PSP --debug mshvsf
```

---

## Additional Resources

### CPS2 System Information
- **CPU**: Motorola 68000 @ 16 MHz (main), Z80 @ 8 MHz (sound)
- **Resolution**: 384×224 (some games use 384×240)
- **Colors**: 4096 colors on screen (from 32768 palette)
- **Sound**: Q-Sound (16 PCM channels)
- **Sprites**: Up to 128 16×16 sprites
- **Layers**: 3 scrolling backgrounds + sprite layer

### Legal Notice
NJEMU is an emulator for CPS2 arcade hardware. You must own the original arcade board or have legal permission to use ROM images. Downloading copyrighted ROMs is illegal in most countries.

### Getting ROMs Legally
- Purchase from rights holders
- Dump from boards you own
- Check local laws and regulations

### Community & Support
- **Original PSP Version**: [NJEMU PSP]
- **Source Code**: Check the Git repository
- **Bug Reports**: Use GitHub issues
- **Documentation**: See README_SDL2.md

---

## Credits

**Original NJEMU**: Developed for PlayStation Portable
**SDL2 Port**: Linux/cross-platform adaptation
**CPS2 Emulation**: Based on MAME CPS2 driver
**Libraries**: SDL2, zlib, libpng

---

## License

NJEMU is released under GPL license. See LICENSE file for details.

CPS2 ROM images are copyrighted by Capcom and other respective copyright holders.

---

**Enjoy your CPS2 games on Linux! 🎮**
