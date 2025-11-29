cmake_minimum_required(VERSION 3.16)
project(cps1sdl C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

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
    src/cpu/m68000/m68000.c
    src/cpu/m68000/c68k.c
    src/cpu/z80/z80.c
    src/cpu/z80/cz80.c
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
