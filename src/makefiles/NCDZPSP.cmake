#------------------------------------------------------------------------------
#
#                             NCDZPSP CMake Configuration
#
#------------------------------------------------------------------------------

# Include directories for NCDZ
include_directories(
    ${CMAKE_SOURCE_DIR}/src/cpu/m68000
    ${CMAKE_SOURCE_DIR}/src/cpu/z80
    ${CMAKE_SOURCE_DIR}/src/ncdz
)

# CPU emulation cores
set(CPU_SOURCES
    src/cpu/m68000/m68000.c
    src/cpu/m68000/c68k.c
    src/cpu/z80/z80.c
    src/cpu/z80/cz80.c
)

# NCDZ emulation core
set(CORE_SOURCES
    ${CPU_SOURCES}
    src/ncdz/ncdz.c
    src/ncdz/cdrom.c
    src/ncdz/cdda.c
    src/ncdz/driver.c
    src/ncdz/memintrf.c
    src/ncdz/inptport.c
    src/ncdz/timer.c
    src/ncdz/vidhrdw.c
    src/ncdz/sprite.c
    src/sound/2610intf.c
    src/sound/ym2610.c
)
