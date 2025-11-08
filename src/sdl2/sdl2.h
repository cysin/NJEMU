/******************************************************************************

	sdl2.h

	SDL2 main header - Linux/SDL2 port

******************************************************************************/

#ifndef SDL2_MAIN_H
#define SDL2_MAIN_H

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

#include "emucfg.h"

// Standard C includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <malloc.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

// SDL2 includes
#include <SDL2/SDL.h>

// Include our CPU interface
#include "include/osd_cpu.h"

// Include SDL2 subsystems
#include "sdl2/ui_text.h"
#include "sdl2/config.h"
#include "sdl2/filer.h"
#include "sdl2/input.h"
#include "sdl2/ticker.h"
#include "sdl2/ui.h"
#include "sdl2/ui_draw.h"
#include "sdl2/ui_menu.h"
#include "sdl2/video.h"
#include "sdl2/sound.h"
#include "sdl2/png.h"

#ifdef ADHOC
#include "sdl2/adhoc.h"
#endif

#if (EMU_SYSTEM == NCDZ)
// MP3 support for NCDZ - we'll use SDL_mixer or libmad
#include "sdl2/mp3.h"
#endif

// Loop control states
enum
{
	LOOP_EXIT = 0,
	LOOP_BROWSER,
	LOOP_RESTART,
	LOOP_RESET,
	LOOP_EXEC
};

// CPU clock settings (not used on desktop, but kept for compatibility)
enum
{
	CPUCLOCK_DEFAULT = 0,
	CPUCLOCK_LOW,
	CPUCLOCK_MEDIUM,
	CPUCLOCK_HIGH,
	CPUCLOCK_MAX
};

// PSP clock compatibility (no-ops on SDL2)
#define PSPCLOCK_222 CPUCLOCK_DEFAULT
#define PSPCLOCK_266 CPUCLOCK_DEFAULT
#define PSPCLOCK_333 CPUCLOCK_DEFAULT

// PSP file I/O compatibility
#include <fcntl.h>
typedef int SceUID;
#define sceIoOpen(path, flags, mode) open(path, flags, mode)
#define sceIoClose(fd) close(fd)
#define sceIoRead(fd, buf, size) read(fd, buf, size)
#define sceIoWrite(fd, buf, size) write(fd, buf, size)
#define sceIoLseek(fd, offset, whence) lseek(fd, offset, whence)
#define sceIoRemove(path) unlink(path)
#define sceIoRename(oldpath, newpath) rename(oldpath, newpath)
#define PSP_O_RDONLY O_RDONLY
#define PSP_O_WRONLY O_WRONLY
#define PSP_O_CREAT O_CREAT

// PSP time compatibility
typedef struct {
    unsigned short year;
    unsigned short month;
    unsigned short day;
    unsigned short hour;
    unsigned short minute;
    unsigned short second;
    unsigned int microsecond;
} pspTime;

void sceRtcGetCurrentClockLocalTime(pspTime *time);

// PSP cache functions (SDL2 stubs - no cache needed)
void *cache_alloc_state_buffer(int size);
void cache_free_state_buffer(int size);
static inline void cache_init(void) {}
static inline void cache_shutdown(void) {}
static inline int cache_start(void) { return 0; } // Return 0 = no cache available

// Cache type constants (MVS only)
#define CACHE_INFO 0
#define CACHE_CROM 1
#define CACHE_SROM 2
#define CACHE_VROM 3

// PSP system button compatibility
#define systembuttons_available 0
#define readHomeButton() 0
#define sceKernelDelayThread(us) SDL_Delay((us)/1000)

// PSP power/display compatibility
int scePowerIsBatteryCharging(void);
int scePowerGetBatteryLifePercent(void);
void sceDisplayWaitVblankStart(void);

// PSP GU (Graphics Unit) compatibility - stubs for hardware rendering
// These are no-ops for SDL2 - actual rendering happens via SDL2 API
#define GULIST_SIZE 262144
extern UINT8 gulist[GULIST_SIZE];

// GU constants
#define GU_DIRECT 0
#define GU_SPRITES 4
#define GU_POINTS 1
#define GU_PSM_5551 0
#define GU_PSM_T8 3
#define GU_SYNC_FINISH 0
#define GU_TEXTURE_16BIT (1<<0)
#define GU_COLOR_5551 (1<<2)
#define GU_COLOR_8888 (1<<3)
#define GU_VERTEX_16BIT (1<<7)
#define GU_TRANSFORM_2D (1<<23)
#define GU_ALPHA_TEST (1<<0)
#define GU_DEPTH_TEST (1<<1)
#define GU_SCISSOR_TEST (1<<2)
#define GU_STENCIL_TEST (1<<3)
#define GU_BLEND (1<<4)
#define GU_CULL_FACE (1<<5)
#define GU_DITHER (1<<6)
#define GU_FOG (1<<7)
#define GU_CLIP_PLANES (1<<8)
#define GU_TEXTURE_2D (1<<9)
#define GU_LIGHTING (1<<10)
#define GU_LIGHT0 (1<<11)
#define GU_LIGHT1 (1<<12)
#define GU_LIGHT2 (1<<13)
#define GU_LIGHT3 (1<<14)
#define GU_COLOR_BUFFER_BIT 1
#define GU_DEPTH_BUFFER_BIT 2
#define GU_FAST_CLEAR_BIT 16
#define GU_NEAREST 0
#define GU_TRUE 1
#define GU_FALSE 0
#define TEXTURE_FLAGS (GU_TEXTURE_16BIT | GU_COLOR_5551 | GU_VERTEX_16BIT | GU_TRANSFORM_2D)
#define PRIMITIVE_FLAGS (GU_COLOR_8888 | GU_VERTEX_16BIT | GU_TRANSFORM_2D)

// GU function stubs (no-ops for now - proper implementation would use SDL2 rendering)
static inline void sceGuStart(int mode, void *list) { (void)mode; (void)list; }
static inline void sceGuDrawBufferList(int format, void *buffer, int width) { (void)format; (void)buffer; (void)width; }
static inline void* sceGuGetMemory(int size) { static char temp[1024*1024]; return temp; }
static inline void sceGuDrawArray(int prim, int flags, int count, void *indices, void *vertices) {
    (void)prim; (void)flags; (void)count; (void)indices; (void)vertices;
}
static inline void sceGuFinish(void) {}
static inline void sceGuSync(int mode, int what) { (void)mode; (void)what; }
static inline void sceGuDepthBuffer(void *buffer, int width) { (void)buffer; (void)width; }
static inline void sceGuScissor(int x, int y, int w, int h) { (void)x; (void)y; (void)w; (void)h; }
static inline void sceGuEnable(int mode) { (void)mode; }
static inline void sceGuDisable(int mode) { (void)mode; }
static inline void sceGuClear(int flags) { (void)flags; }
static inline void sceGuTexMode(int mode, int mip, int swizzle, int indexed) { (void)mode; (void)mip; (void)swizzle; (void)indexed; }
static inline void sceGuTexFilter(int min, int mag) { (void)min; (void)mag; }
static inline void sceGuTexImage(int mip, int width, int height, int stride, void *pixels) {
    (void)mip; (void)width; (void)height; (void)stride; (void)pixels;
}
static inline void sceGuClutLoad(int num, void *clut) { (void)num; (void)clut; }
static inline void sceGuDepthMask(int mask) { (void)mask; }

#ifdef SDL2
// SDL2: store textures linearly
#define SWIZZLED8_8x8(tex, idx) (&tex[(idx) << 6])
#define SWIZZLED8_16x16(tex, idx) (&tex[(idx) << 8])
#define SWIZZLED8_32x32(tex, idx) (&tex[(idx) << 10])
#else
// PSP swizzled layout
#define SWIZZLED8_8x8(tex, idx) (&tex[((idx & ~1) << 6) | ((idx & 1) << 3)])
#define SWIZZLED8_16x16(tex, idx) (&tex[((idx & ~31) << 8) | ((idx & 31) << 7)])
#define SWIZZLED8_32x32(tex, idx) (&tex[((idx & ~15) << 10) | ((idx & 15) << 8)])
#endif

// Non-swizzled texture macros
#define NONE_SWIZZLED_8x8(tex, idx) (&tex[(idx) << 6])
#define NONE_SWIZZLED_16x16(tex, idx) (&tex[(idx) << 8])
#define NONE_SWIZZLED_32x32(tex, idx) (&tex[(idx) << 10])

// Global variables
extern volatile int Loop;
extern volatile int Sleep;
extern char launchDir[MAX_PATH];
extern int cpu_clock_setting;
extern int psp_cpuclock;
extern int njemu_debug;

// Functions
void set_cpu_clock(int value);
uint64_t get_ticks_us(void);

#endif /* SDL2_MAIN_H */
