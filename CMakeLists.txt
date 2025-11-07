#------------------------------------------------------------------------------
#
#            CPS1/CPS2/NEOGEO/NEOGEO CDZ Emulator - SDL2/Linux Port
#
#------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.10)
project(NJEMU VERSION 2.4.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

#------------------------------------------------------------------------------
# Configuration Options
#------------------------------------------------------------------------------

option(BUILD_CPS1PSP "Build CPS1 emulator" OFF)
option(BUILD_CPS2PSP "Build CPS2 emulator" ON)
option(BUILD_MVSPSP "Build Neo-Geo MVS emulator" OFF)
option(BUILD_NCDZPSP "Build Neo-Geo CDZ emulator" OFF)

option(COMMAND_LIST "Enable command list support" ON)
option(SAVE_STATE "Enable save state support" ON)
option(ADHOC "Enable Ad-Hoc multiplayer (requires network)" OFF)

#------------------------------------------------------------------------------
# Determine target emulator
#------------------------------------------------------------------------------

if(BUILD_CPS1PSP)
    set(TARGET "CPS1PSP")
    set(EMU_SYSTEM "CPS1")
    set(BUILD_CPS2PSP OFF)
    set(BUILD_MVSPSP OFF)
    set(BUILD_NCDZPSP OFF)
elseif(BUILD_CPS2PSP)
    set(TARGET "CPS2PSP")
    set(EMU_SYSTEM "CPS2")
    set(BUILD_MVSPSP OFF)
    set(BUILD_NCDZPSP OFF)
elseif(BUILD_MVSPSP)
    set(TARGET "MVSPSP")
    set(EMU_SYSTEM "MVS")
    set(BUILD_NCDZPSP OFF)
elseif(BUILD_NCDZPSP)
    set(TARGET "NCDZPSP")
    set(EMU_SYSTEM "NCDZ")
    set(ADHOC OFF)
else()
    message(FATAL_ERROR "No emulator target selected!")
endif()

message(STATUS "Building ${TARGET} for Linux/SDL2")

#------------------------------------------------------------------------------
# Compiler Flags
#------------------------------------------------------------------------------

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall -Wno-unused-result")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fomit-frame-pointer -fstrict-aliasing")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -falign-functions=32 -falign-loops -falign-labels -falign-jumps")

# Silence some warnings for legacy code
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-pointer-sign -Wno-unused-variable")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-but-set-variable -Wno-format")

#------------------------------------------------------------------------------
# Compiler Defines
#------------------------------------------------------------------------------

add_definitions(
    -DINLINE=inline
    -Dinline=inline
    -D__inline__=inline
    -DBUILD_${TARGET}=1
    -DPBPNAME_STR="${TARGET}"
    -DVERSION_STR="${PROJECT_VERSION}"
    -DVERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    -DVERSION_MINOR=${PROJECT_VERSION_MINOR}
    -DVERSION_BUILD=${PROJECT_VERSION_PATCH}
    -DSDL2
    -DZLIB_CONST
)

if(COMMAND_LIST)
    add_definitions(-DCOMMAND_LIST=1)
endif()

if(SAVE_STATE)
    add_definitions(-DSAVE_STATE=1)
endif()

if(ADHOC)
    add_definitions(-DADHOC=1)
    set(SAVE_STATE ON)
endif()

#------------------------------------------------------------------------------
# Include Directories
#------------------------------------------------------------------------------

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/zip
    ${CMAKE_SOURCE_DIR}/src/zlib
    ${CMAKE_SOURCE_DIR}/src/libmad
)

#------------------------------------------------------------------------------
# Find SDL2
#------------------------------------------------------------------------------

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Try to find SDL2 using pkg-config if find_package fails
if(NOT SDL2_FOUND)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2)
    include_directories(${SDL2_INCLUDE_DIRS})
endif()

#------------------------------------------------------------------------------
# Source Files
#------------------------------------------------------------------------------

# Main emulation files
set(MAIN_SOURCES
    src/emumain.c
    src/zip/zfile.c
    src/zip/unzip.c
    src/sound/sndintrf.c
    src/common/cache.c
    src/common/loadrom.c
)

if(ADHOC)
    list(APPEND MAIN_SOURCES src/common/adhoc.c)
endif()

if(COMMAND_LIST)
    list(APPEND MAIN_SOURCES src/common/cmdlist.c)
endif()

if(SAVE_STATE)
    list(APPEND MAIN_SOURCES src/common/state.c)
endif()

# SDL2 platform layer
set(SDL2_SOURCES
    src/sdl2/sdl2.c
    src/sdl2/config.c
    src/sdl2/filer.c
    src/sdl2/input.c
    src/sdl2/ticker.c
    src/sdl2/ui.c
    src/sdl2/ui_draw.c
    src/sdl2/ui_menu.c
    src/sdl2/ui_text.c
    src/sdl2/video.c
    src/sdl2/sound.c
    src/sdl2/png.c
)

if(ADHOC)
    list(APPEND SDL2_SOURCES src/sdl2/adhoc.c)
endif()

if(BUILD_NCDZPSP)
    list(APPEND SDL2_SOURCES src/sdl2/mp3.c)
endif()

# Font files (reuse from PSP)
set(FONT_SOURCES
    src/psp/font/graphic.c
    src/psp/font/ascii_14p.c
    src/psp/font/font_s.c
    src/psp/font/bshadow.c
    src/psp/font/command.c
    src/psp/font/ascii_14.c
    src/psp/font/latin1_14.c
    src/psp/font/gbk_s14.c
    src/psp/font/gbk_tbl.c
)

# Icon files (reuse from PSP) - select based on emulator
if(EMU_SYSTEM STREQUAL "CPS1" OR EMU_SYSTEM STREQUAL "CPS2")
    set(ICON_SOURCES
        src/psp/icon/cps_s.c
        src/psp/icon/cps_l.c
    )
elseif(EMU_SYSTEM STREQUAL "MVS")
    set(ICON_SOURCES
        src/psp/icon/mvs_s.c
        src/psp/icon/mvs_l.c
    )
elseif(EMU_SYSTEM STREQUAL "NCDZ")
    set(ICON_SOURCES
        src/psp/icon/ncdz_s.c
        src/psp/icon/ncdz_l.c
    )
else()
    # Default to CPS
    set(ICON_SOURCES
        src/psp/icon/cps_s.c
        src/psp/icon/cps_l.c
    )
endif()

# Zlib
set(ZLIB_SOURCES
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
)

#------------------------------------------------------------------------------
# Include emulator-specific makefiles
#------------------------------------------------------------------------------

if(BUILD_CPS1PSP)
    include(src/makefiles/CPS1PSP.cmake)
elseif(BUILD_CPS2PSP)
    include(src/makefiles/CPS2PSP.cmake)
elseif(BUILD_MVSPSP)
    include(src/makefiles/MVSPSP.cmake)
elseif(BUILD_NCDZPSP)
    include(src/makefiles/NCDZPSP.cmake)
endif()

#------------------------------------------------------------------------------
# Executable
#------------------------------------------------------------------------------

add_executable(${TARGET}
    ${MAIN_SOURCES}
    ${SDL2_SOURCES}
    ${FONT_SOURCES}
    ${ICON_SOURCES}
    ${ZLIB_SOURCES}
    ${CORE_SOURCES}
)

#------------------------------------------------------------------------------
# Link Libraries
#------------------------------------------------------------------------------

target_link_libraries(${TARGET}
    ${SDL2_LIBRARIES}
    m
    pthread
    png
    z
)

if(ADHOC)
    # Network libraries for Ad-Hoc (if we implement it)
    target_link_libraries(${TARGET})
endif()

if(BUILD_NCDZPSP)
    # MAD MP3 library for NCDZ
    target_link_libraries(${TARGET} ${CMAKE_SOURCE_DIR}/src/libmad/libmad.a)
endif()

#------------------------------------------------------------------------------
# Installation
#------------------------------------------------------------------------------

install(TARGETS ${TARGET} DESTINATION bin)

message(STATUS "Configuration complete. Run 'make' to build.")
