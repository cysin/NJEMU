#------------------------------------------------------------------------------
#
#                             CPS2PSP CMake Configuration
#
#------------------------------------------------------------------------------

# Include directories for CPS2
include_directories(
    ${CMAKE_SOURCE_DIR}/src/cpu/m68000
    ${CMAKE_SOURCE_DIR}/src/cpu/z80
    ${CMAKE_SOURCE_DIR}/src/cps2
)

# CPU emulation cores
set(CPU_SOURCES
    src/cpu/m68000/m68000.c
    src/cpu/m68000/c68k.c
    src/cpu/z80/z80.c
    src/cpu/z80/cz80.c
    src/common/coin.c
)

# CPS2 emulation core
set(CORE_SOURCES
    ${CPU_SOURCES}
    src/cps2/cps2.c
    src/cps2/cps2crpt.c
    src/cps2/driver.c
    src/cps2/memintrf.c
    src/cps2/inptport.c
    src/cps2/timer.c
    src/cps2/vidhrdw.c
    src/cps2/sprite.c
    src/cps2/eeprom.c
    src/sound/qsound.c
)
