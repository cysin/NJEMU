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

// Global variables
extern volatile int Loop;
extern volatile int Sleep;
extern char launchDir[MAX_PATH];
extern int cpu_clock_setting;
extern int njemu_debug;

// Functions
void set_cpu_clock(int value);
uint64_t get_ticks_us(void);

#endif /* SDL2_MAIN_H */
