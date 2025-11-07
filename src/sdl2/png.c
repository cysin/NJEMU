/******************************************************************************

	png.c

	SDL2 PNG image loading/saving using SDL_image

******************************************************************************/

#include "sdl2.h"
#include <png.h>

/******************************************************************************
	Functions
******************************************************************************/

/*--------------------------------------------------------
	Load PNG file into framebuffer
--------------------------------------------------------*/

int load_png(void *frame, const char *fname, int width, int height)
{
	if (!frame || !fname) return 0;

	// TODO: Implement PNG loading using libpng or SDL_image
	// For now, just return failure

	printf("PNG: Would load %s\n", fname);
	return 0;
}


/*--------------------------------------------------------
	Save framebuffer as PNG (screenshot)
--------------------------------------------------------*/

int save_png(const char *fname)
{
	if (!fname) return 0;

	// TODO: Implement PNG saving
	// Would save current show_frame to PNG file

	printf("PNG: Would save screenshot to %s\n", fname);
	return 0;
}
