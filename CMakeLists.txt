cmake_minimum_required(VERSION 3.16)
project(cps1sdl C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(USE_MUSASHI_M68K "Use Musashi 68000 core instead of Cyclone" OFF)

find_package(SDL2 REQUIRED)

add_compile_definitions(
    PLATFORM_SDL=1
    BUILD_CPS1PSP=1
    RELEASE=0
    PSP_VIDEO_32BPP=0
    INLINE=static\ inline
    inline=__inline
    __inline__=__inline
)

set(M68K_SOURCES
    src/cpu/m68000/m68000.c
    src/cpu/m68000/c68k.c
)

    if (USE_MUSASHI_M68K)
        set(MUSASHI_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/musashi_gen)
        file(MAKE_DIRECTORY ${MUSASHI_OUT_DIR})

    add_custom_command(
        OUTPUT ${MUSASHI_OUT_DIR}/m68kops.c ${MUSASHI_OUT_DIR}/m68kops.h
        COMMAND ${CMAKE_C_COMPILER} -o ${MUSASHI_OUT_DIR}/m68kmake ${CMAKE_CURRENT_SOURCE_DIR}/src/cpu/Musashi/m68kmake.c
        COMMAND ${MUSASHI_OUT_DIR}/m68kmake ${MUSASHI_OUT_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src/cpu/Musashi/m68k_in.c
        DEPENDS src/cpu/Musashi/m68kmake.c src/cpu/Musashi/m68k_in.c src/cpu/Musashi/m68kconf.h
        WORKING_DIRECTORY ${MUSASHI_OUT_DIR}
        COMMENT "Generating Musashi opcode tables"
        VERBATIM
    )

    set(M68K_SOURCES
        src/cpu/m68000/m68k_musashi.c
        src/cpu/Musashi/m68kcpu.c
        src/cpu/Musashi/softfloat/softfloat.c
        ${MUSASHI_OUT_DIR}/m68kops.c
    )
endif()

set(COMMON_SOURCES
    src/emumain.c
    src/common/cache.c
    src/common/loadrom.c
    src/common/coin.c
    src/zip/zfile.c
    src/zip/unzip.c
    src/zlib/adler32.c
    src/zlib/compress.c
    src/zlib/uncompr.c
    src/zlib/crc32.c
    src/zlib/deflate.c
    src/zlib/inflate.c
    src/zlib/inftrees.c
    src/zlib/inffast.c
    src/zlib/trees.c
    src/zlib/zutil.c
    ${M68K_SOURCES}
    src/cpu/z80/z80.c
    src/cpu/sup/z80/z80.c
    src/sound/sndintrf.c
    src/sound/2151intf.c
    src/sound/ym2151.c
    src/sound/qsound.c
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
    src/platform/stub.c
    src/platform/main_sdl.c
)

add_executable(cps1sdl ${COMMON_SOURCES})

target_include_directories(cps1sdl PRIVATE
    src
    src/include
    src/zip
    src/zlib
    src/libmad
    src/platform
    src/cpu/sup/z80
    src/cpu/Musashi
    $<IF:$<BOOL:${USE_MUSASHI_M68K}>,${MUSASHI_OUT_DIR},>
)

target_compile_options(cps1sdl PRIVATE
    -O2
    -Wall
    -Wextra
    -Wpointer-arith
    -Wwrite-strings
    -Wsign-compare
)

target_link_libraries(cps1sdl PRIVATE SDL2::SDL2 m)

target_compile_definitions(cps1sdl PRIVATE USE_SUP_Z80)
if (USE_MUSASHI_M68K)
    target_compile_definitions(cps1sdl PRIVATE USE_MUSASHI_M68K)
endif()
