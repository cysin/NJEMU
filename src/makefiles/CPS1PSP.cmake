#------------------------------------------------------------------------------
#
#                             CPS1PSP CMake Configuration
#
#------------------------------------------------------------------------------

# Include directories for CPS1
include_directories(
    ${CMAKE_SOURCE_DIR}/src/cpu/m68000
    ${CMAKE_SOURCE_DIR}/src/cpu/z80
    ${CMAKE_SOURCE_DIR}/src/cps1
)

# CPU emulation cores
set(CPU_SOURCES
    src/cpu/m68000/m68000.c
    src/cpu/m68000/c68k.c
    src/cpu/z80/z80.c
    src/cpu/z80/cz80.c
    src/common/coin.c
)

# CPS1 emulation core
set(CORE_SOURCES
    ${CPU_SOURCES}
    src/cps1/cps1.c
    src/cps1/driver.c
    src/cps1/memintrf.c
    src/cps1/inptport.c
    src/cps1/dipsw.c
    src/cps1/timer.c
    src/cps1/vidhrdw.c
    src/cps1/sprite.c
    src/cps1/eeprom.c
    src/cps1/kabuki.c
    src/sound/2151intf.c
    src/sound/ym2151.c
    src/sound/qsound.c
)
