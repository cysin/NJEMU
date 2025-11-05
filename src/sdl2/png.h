/******************************************************************************

	png.h

	SDL2 PNG image loading/saving

******************************************************************************/

#ifndef SDL2_PNG_H
#define SDL2_PNG_H

int load_png(void *frame, const char *fname, int width, int height);
int save_png(const char *fname);

#endif /* SDL2_PNG_H */
