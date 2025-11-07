/******************************************************************************

	sdl2.c

	SDL2 main initialization and system functions

******************************************************************************/

#include "sdl2.h"

/******************************************************************************
	Global variables
******************************************************************************/

volatile int Loop;
volatile int Sleep;
char launchDir[MAX_PATH];
int cpu_clock_setting = CPUCLOCK_DEFAULT;
int psp_cpuclock = CPUCLOCK_DEFAULT;
int njemu_debug = 0;

/******************************************************************************
	Functions
******************************************************************************/

/*------------------------------------------------------
	Set CPU clock (no-op on desktop, kept for compatibility)
------------------------------------------------------*/

void set_cpu_clock(int value)
{
	cpu_clock_setting = value;
	// No-op on desktop - CPU clock is managed by OS
}


/*------------------------------------------------------
	Get microsecond timer
------------------------------------------------------*/

uint64_t get_ticks_us(void)
{
	return ticker();
}


/*------------------------------------------------------
	Initialize SDL2
------------------------------------------------------*/

int sdl2_init(void)
{
	// Initialize SDL2 subsystems
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_TIMER) < 0)
	{
		fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
		return 0;
	}

	// Set up launch directory
	char *base_path = SDL_GetBasePath();
	if (base_path)
	{
		strncpy(launchDir, base_path, MAX_PATH - 1);
		launchDir[MAX_PATH - 1] = '\0';
		SDL_free(base_path);
	}
	else
	{
		// Fallback to current directory
		getcwd(launchDir, MAX_PATH);
		strcat(launchDir, "/");
	}

	printf("Launch directory: %s\n", launchDir);

	Loop = LOOP_BROWSER;
	Sleep = 0;

	return 1;
}


/*------------------------------------------------------
	Shutdown SDL2
------------------------------------------------------*/

void sdl2_exit(void)
{
	SDL_Quit();
}


/*------------------------------------------------------
	Main loop - handle SDL events
------------------------------------------------------*/

void sdl2_handle_events(void)
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			Loop = LOOP_EXIT;
			break;

		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE)
			{
				// ESC key - could be used for menu
			}
			break;
		}
	}
}


/*------------------------------------------------------
	Sleep/delay function
------------------------------------------------------*/

void sdl2_delay(uint32_t ms)
{
	SDL_Delay(ms);
}


/*------------------------------------------------------
	PSP compatibility functions
------------------------------------------------------*/

// Get current time (PSP RTC compatibility)
void sceRtcGetCurrentClockLocalTime(pspTime *psp_time)
{
	time_t now;
	struct tm *local;

	now = time(NULL);
	local = localtime(&now);

	if (local && psp_time)
	{
		psp_time->year = local->tm_year + 1900;
		psp_time->month = local->tm_mon + 1;
		psp_time->day = local->tm_mday;
		psp_time->hour = local->tm_hour;
		psp_time->minute = local->tm_min;
		psp_time->second = local->tm_sec;
		psp_time->microsecond = 0;
	}
}

// Cache buffer allocation (no-op on SDL2)
void *cache_alloc_state_buffer(int size)
{
	return malloc(size);
}

// Cache buffer free (no-op on SDL2)
void cache_free_state_buffer(int size)
{
	// No-op on SDL2 - memory is freed normally
}
