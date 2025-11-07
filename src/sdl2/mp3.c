/******************************************************************************

	mp3.c

	SDL2 MP3 playback - stub implementation

******************************************************************************/

#include "sdl2.h"

/******************************************************************************
	Functions
******************************************************************************/

int mp3_init(void)
{
	// TODO: Initialize MP3 decoder (libmad or SDL_mixer)
	return 1;
}

void mp3_exit(void)
{
	// TODO: Cleanup MP3 decoder
}

int mp3_play(const char *filename)
{
	if (!filename) return 0;

	printf("MP3: Would play %s\n", filename);

	// TODO: Implement MP3 playback
	return 0;
}

void mp3_stop(void)
{
	// TODO: Stop MP3 playback
}

void mp3_pause(int pause)
{
	(void)pause;
	// TODO: Pause/resume MP3 playback
}

int mp3_get_status(void)
{
	// TODO: Return MP3 playback status
	return 0;
}
