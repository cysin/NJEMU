# NJEMU Linux/SDL2 Porting Documentation Index

## 📚 Documentation Overview

This directory contains comprehensive documentation for porting NJEMU from PSP to Linux/SDL2.

---

## 📄 Main Documents

### 1. **TODO.md** (Also in `md/TODO.md`)
**55 KB | 323+ Tasks**

The complete, actionable task list for the entire porting project.

- ✅ Organized into 10 phases
- ✅ Each task has checkbox for tracking
- ✅ Includes implementation details
- ✅ Contains code examples
- ✅ Provides testing checklists
- ✅ Estimates effort for each task

**Start here** if you want to begin working immediately.

---

### 2. **PORTING_PLAN.md**
**40 KB | Comprehensive Strategy Document**

Detailed planning document with:

- Architecture overview (PSP → Linux mapping)
- Phase-by-phase breakdown (10 phases)
- Technical implementation strategies
- API mapping reference (PSP SDK → Linux)
- File modification checklist
- Risk assessment
- Success criteria
- Resource links

**Read this first** for understanding the overall approach.

---

### 3. **PORTING_QUICK_REFERENCE.md**
**9.5 KB | Quick Lookup Guide**

Condensed reference with:

- Priority task order
- File migration map
- API mapping cheat sheet
- Common pitfalls and solutions
- Testing checklist
- Quick commands
- Performance targets
- Estimated timeline

**Use this** for quick lookups during development.

---

## 🗂️ Original Documentation (`./md/` directory)

The following documents provide deep insight into the PSP codebase:

### Architecture & Workflow
- **arch.md** - Overall architecture, execution flow, per-system design
- **workflow2.md** - Detailed emulation workflow from boot to shutdown

### Subsystems
- **video.md** - Video pipeline, rendering, GU usage
- **sound.md** - Audio architecture, chip backends, threading
- **input.md** - Input handling, PSP controls, AdHoc

### Build & Configuration
- **build.md** - Build system, deployment, ROM expectations
- **config_files.md** - Configuration file formats
- **controls_dips.md** - Control mappings and DIP switches

### Features
- **adhoc.md** - AdHoc networking for multiplayer
- **cheats.md** & **cheat_format.md** - Cheat system
- **state.md** - Save state system
- **cache.md** - ROM cache system

### Graphics Details
- **blit_gu.md** - GU blit operations
- **gfx_decoding.md** - Graphics decoding
- **ui.md** - UI system

### Audio Details
- **cd_audio.md** & **mp3_cdda_details.md** - CD audio (NCDZ)

### System Details
- **cpu.md** - CPU emulation
- **memory.md** - Memory management
- **timing.md** - Timing and synchronization
- **threading.md** - Threading model

### Miscellaneous
- **errors.md** - Error handling
- **performance.md** - Performance optimization
- **power_systembuttons.md** - Power management
- **rom_loading.md** & **rom_tools.md** - ROM handling
- **bios_romsets.md** - BIOS requirements
- **hotkeys_options.md** - Hotkeys and options
- **localization_fonts.md** - Fonts and localization
- **menus.md** - Menu system

---

## 🎯 How to Use This Documentation

### For Planning
1. Read **PORTING_PLAN.md** to understand the big picture
2. Review **PORTING_QUICK_REFERENCE.md** for priority tasks
3. Read relevant files in `./md/` for technical details

### For Development
1. Use **TODO.md** to track daily progress
2. Refer to **PORTING_QUICK_REFERENCE.md** for API mappings
3. Consult `./md/` docs when working on specific subsystems

### For Testing
1. Use testing checklists in **TODO.md**
2. Refer to **PORTING_PLAN.md** Phase 8 for test strategy
3. Check **performance.md** for optimization tips

---

## 📊 Documentation Statistics

| Document | Size | Content Type |
|----------|------|--------------|
| TODO.md | 55 KB | Actionable tasks (323+) |
| PORTING_PLAN.md | 40 KB | Strategic plan |
| PORTING_QUICK_REFERENCE.md | 9.5 KB | Quick reference |
| md/*.md (34 files) | ~130 KB | Technical documentation |
| **Total** | **~235 KB** | Complete porting guide |

---

## 🚀 Getting Started

### Absolute Beginner Path
1. Read **PORTING_PLAN.md** (sections 1-3)
2. Skim **PORTING_QUICK_REFERENCE.md**
3. Start with **TODO.md** Phase 1

### Experienced Developer Path
1. Skim **PORTING_QUICK_REFERENCE.md**
2. Jump into **TODO.md** Phase 1
3. Refer back to docs as needed

### Deep Dive Path
1. Read all `./md/` documentation
2. Read **PORTING_PLAN.md** completely
3. Use **TODO.md** for execution

---

## 🔗 Quick Navigation

### By Task Type

**Setup & Infrastructure**
- TODO.md → Phase 1
- PORTING_PLAN.md → Phase 1
- build.md → PSP build system

**Graphics**
- TODO.md → Phase 3
- PORTING_PLAN.md → Phase 3
- video.md, blit_gu.md, gfx_decoding.md

**Audio**
- TODO.md → Phase 4
- PORTING_PLAN.md → Phase 4
- sound.md, mp3_cdda_details.md

**Input**
- TODO.md → Phase 5
- PORTING_PLAN.md → Phase 5
- input.md, controls_dips.md

**File I/O**
- TODO.md → Phase 6
- PORTING_PLAN.md → Phase 6
- cache.md, state.md, config_files.md

**UI**
- TODO.md → Phase 7
- PORTING_PLAN.md → Phase 7
- ui.md, menus.md

**Testing**
- TODO.md → Phase 8
- PORTING_PLAN.md → Phase 8
- All `./md/` docs for verification

---

## 💡 Tips for Success

1. **Don't skip Phase 1** - Proper build system saves time later
2. **Start with CPS1** - Simplest system to validate approach
3. **Test incrementally** - Don't wait until everything is done
4. **Document as you go** - Note any deviations from plan
5. **Use git effectively** - Commit after each completed task
6. **Ask for help** - Community knowledge is valuable

---

## 🔄 Document Maintenance

As development progresses:

- [ ] Update TODO.md completion percentages
- [ ] Mark completed tasks with ✅
- [ ] Document any architectural changes
- [ ] Note performance findings
- [ ] Add discovered issues to TODO.md
- [ ] Update timelines based on actual progress

---

## 📞 Support & Contribution

**Issue Tracker**: [Add your issue tracker URL]
**Discussion**: [Add your discussion forum/chat]
**Wiki**: [Add wiki URL if you create one]

---

## 📜 License

This porting documentation is provided as-is to assist with the NJEMU Linux port.
Original NJEMU code license: See `Licence.txt`

---

**Created**: 2025-11-29
**Last Updated**: 2025-11-29
**Version**: 1.0

Good luck with the port! 🚀
