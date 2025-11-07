/******************************************************************************

	sound.c

	SDL2 audio subsystem

******************************************************************************/

#include "sdl2.h"
#include "emumain.h"
#include <string.h>

/******************************************************************************
	Local variables
******************************************************************************/

static volatile int sound_active;
static SDL_Thread *sound_thread = NULL;
static int sound_volume;
static int sound_enable;
static INT16 sound_buffer[2][SOUND_BUFFER_SIZE];

static struct sound_t sound_info;

/******************************************************************************
	Global variables
******************************************************************************/

struct sound_t *sound = &sound_info;
SDL_AudioDeviceID sdl_audio_device = 0;

/******************************************************************************
	Local functions
******************************************************************************/

/*--------------------------------------------------------
	Sound update thread
--------------------------------------------------------*/

static int sound_update_thread(void *arg)
{
	int flip = 0;

	(void)arg;

	while (sound_active)
	{
		if (Sleep)
		{
			SDL_Delay(5);
			continue;
		}

		// Fill buffer
		if (sound_enable && sound->update)
		{
			(*sound->update)(sound_buffer[flip]);
		}
		else
		{
			memset(sound_buffer[flip], 0, SOUND_BUFFER_SIZE * sizeof(INT16));
		}

		// Queue audio to SDL
		if (sdl_audio_device)
		{
			// Apply volume (simple multiplication)
			if (sound_volume < SDL_MIX_MAXVOLUME)
			{
				INT16 temp_buffer[SOUND_BUFFER_SIZE];
				memcpy(temp_buffer, sound_buffer[flip], SOUND_BUFFER_SIZE * sizeof(INT16));

				for (int i = 0; i < SOUND_BUFFER_SIZE; i++)
				{
					temp_buffer[i] = (temp_buffer[i] * sound_volume) / SDL_MIX_MAXVOLUME;
				}

				SDL_QueueAudio(sdl_audio_device, temp_buffer, SOUND_BUFFER_SIZE * sizeof(INT16));
			}
			else
			{
				SDL_QueueAudio(sdl_audio_device, sound_buffer[flip], SOUND_BUFFER_SIZE * sizeof(INT16));
			}

			// Wait for buffer to drain a bit
			while (SDL_GetQueuedAudioSize(sdl_audio_device) > SOUND_BUFFER_SIZE * sizeof(INT16) * 2)
			{
				SDL_Delay(1);
				if (!sound_active) break;
			}
		}

		flip ^= 1;
	}

	return 0;
}


/******************************************************************************
	Global functions
******************************************************************************/

/*--------------------------------------------------------
	Initialize sound subsystem
--------------------------------------------------------*/

void sound_thread_init(void)
{
	sound_active = 0;
	sound_thread = NULL;
	sound_volume = SDL_MIX_MAXVOLUME;
	sound_enable = 0;
}


/*--------------------------------------------------------
	Shutdown sound subsystem
--------------------------------------------------------*/

void sound_thread_exit(void)
{
	sound_thread_stop();
}


/*--------------------------------------------------------
	Enable/disable sound
--------------------------------------------------------*/

void sound_thread_enable(int enable)
{
	if (sound_active)
	{
		sound_enable = enable;

		if (sound_enable)
		{
			sound_thread_set_volume();
		}
		else
		{
			sound_volume = 0;
			if (sdl_audio_device)
			{
				SDL_ClearQueuedAudio(sdl_audio_device);
			}
		}
	}
}


/*--------------------------------------------------------
	Set sound volume
--------------------------------------------------------*/

void sound_thread_set_volume(void)
{
	extern int option_sound_volume;
	sound_volume = SDL_MIX_MAXVOLUME * (option_sound_volume * 10) / 100;
}


/*--------------------------------------------------------
	Start sound thread
--------------------------------------------------------*/

int sound_thread_start(void)
{
	SDL_AudioSpec want, have;

	sound_active = 0;
	sound_thread = NULL;
	sound_volume = SDL_MIX_MAXVOLUME;
	sound_enable = 0;

	memset(sound_buffer[0], 0, sizeof(sound_buffer[0]));
	memset(sound_buffer[1], 0, sizeof(sound_buffer[1]));

	// Close existing audio device
	if (sdl_audio_device)
	{
		SDL_CloseAudioDevice(sdl_audio_device);
		sdl_audio_device = 0;
	}

	// Set up audio spec
	SDL_zero(want);
	want.freq = sound->frequency;
	want.format = AUDIO_S16LSB;  // 16-bit signed little-endian
	want.channels = sound->channels;
	want.samples = sound->samples;
	want.callback = NULL;  // Use queue-based audio

	// Open audio device
	sdl_audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
	if (!sdl_audio_device)
	{
		fprintf(stderr, "Failed to open audio device: %s\n", SDL_GetError());
		return 0;
	}

	printf("Audio opened: %d Hz, %d channels, %d samples\n",
	       have.freq, have.channels, have.samples);

	// Create sound thread
	sound_active = 1;
	sound_thread = SDL_CreateThread(sound_update_thread, "Sound thread", NULL);
	if (!sound_thread)
	{
		fprintf(stderr, "Failed to create sound thread: %s\n", SDL_GetError());
		SDL_CloseAudioDevice(sdl_audio_device);
		sdl_audio_device = 0;
		sound_active = 0;
		return 0;
	}

	// Start audio playback
	SDL_PauseAudioDevice(sdl_audio_device, 0);

	sound_thread_set_volume();

	return 1;
}


/*--------------------------------------------------------
	Stop sound thread
--------------------------------------------------------*/

void sound_thread_stop(void)
{
	if (sound_thread)
	{
		sound_volume = 0;
		sound_enable = 0;
		sound_active = 0;

		// Wait for thread to finish
		SDL_WaitThread(sound_thread, NULL);
		sound_thread = NULL;
	}

	if (sdl_audio_device)
	{
		SDL_PauseAudioDevice(sdl_audio_device, 1);
		SDL_CloseAudioDevice(sdl_audio_device);
		sdl_audio_device = 0;
	}
}
