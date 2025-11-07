/******************************************************************************

	mp3.h

	SDL2 MP3 playback (for NCDZ)

******************************************************************************/

#ifndef SDL2_MP3_H
#define SDL2_MP3_H

// MP3 playback functions
int mp3_init(void);
void mp3_exit(void);
int mp3_play(const char *filename);
void mp3_stop(void);
void mp3_pause(int pause);
int mp3_get_status(void);

#endif /* SDL2_MP3_H */
