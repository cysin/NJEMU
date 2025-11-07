#------------------------------------------------------------------------------
#
#                             MVSPSP CMake Configuration
#
#------------------------------------------------------------------------------

# Include directories for MVS
include_directories(
    ${CMAKE_SOURCE_DIR}/src/cpu/m68000
    ${CMAKE_SOURCE_DIR}/src/cpu/z80
    ${CMAKE_SOURCE_DIR}/src/mvs
)

# CPU emulation cores
set(CPU_SOURCES
    src/cpu/m68000/m68000.c
    src/cpu/m68000/c68k.c
    src/cpu/z80/z80.c
    src/cpu/z80/cz80.c
)

# MVS emulation core
set(CORE_SOURCES
    ${CPU_SOURCES}
    src/mvs/mvs.c
    src/mvs/driver.c
    src/mvs/memintrf.c
    src/mvs/inptport.c
    src/mvs/dipsw.c
    src/mvs/timer.c
    src/mvs/vidhrdw.c
    src/mvs/sprite.c
    src/mvs/pd4990a.c
    src/mvs/neocrypt.c
    src/mvs/biosmenu.c
    src/sound/2610intf.c
    src/sound/ym2610.c
)
