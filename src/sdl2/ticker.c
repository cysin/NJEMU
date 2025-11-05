/******************************************************************************

	ticker.c

	SDL2 timing functions (64-bit microsecond counter)

******************************************************************************/

#include "sdl2.h"

/******************************************************************************
	Functions
******************************************************************************/

/*--------------------------------------------------------
	Get current time in microseconds
--------------------------------------------------------*/

TICKER ticker(void)
{
	// SDL_GetTicks64() returns milliseconds, convert to microseconds
	// Note: SDL_GetPerformanceCounter() would be more accurate but requires conversion
	static uint64_t start_time = 0;
	uint64_t now;

	if (start_time == 0)
	{
		start_time = SDL_GetTicks64() * 1000;
	}

	now = SDL_GetTicks64() * 1000;

	return (TICKER)(now - start_time);
}

/*--------------------------------------------------------
	Alternative: high-precision ticker using SDL_GetPerformanceCounter
--------------------------------------------------------*/

TICKER ticker_precise(void)
{
	static uint64_t frequency = 0;
	uint64_t counter;

	if (frequency == 0)
	{
		frequency = SDL_GetPerformanceFrequency();
	}

	counter = SDL_GetPerformanceCounter();

	// Convert to microseconds
	return (TICKER)((counter * 1000000) / frequency);
}
