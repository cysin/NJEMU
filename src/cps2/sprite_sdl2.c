/******************************************************************************

	sprite_sdl2.c

	CPS2 SDL2 Software Sprite Rendering - simplified implementation

******************************************************************************/

#ifdef SDL2

#include "cps2.h"

// External variables from sprite.c
extern UINT16 *scrbitmap;
extern INT16 clip_min_y;
extern INT16 clip_max_y;

/******************************************************************************
	SDL2 Software Rendering Functions
******************************************************************************/

/*------------------------------------------------------------------------
	SDL2: Draw 16x16 sprite tile directly to framebuffer
------------------------------------------------------------------------*/

void sdl2_draw_sprite_16x16(INT16 x, INT16 y, UINT32 code, UINT16 attr)
{
	UINT32 *src;
	UINT16 *dst, *pal;
	UINT32 tile, mask;
	int sy, dy;

	// Get palette
	pal = &video_palette[(attr & 0x1f) << 4];

	// Get source tile data from gfx ROM
	src = (UINT32 *)&memory_region_gfx1[code << 7];

	// Check clipping
	if (x < 64 || x >= 448 || y < clip_min_y - 16 || y >= clip_max_y)
		return;

	// Render each line
	for (sy = 0, dy = y; sy < 16 && dy < clip_max_y; sy++, dy++)
	{
		if (dy < clip_min_y)
		{
			src += 2;
			continue;
		}

		dst = &scrbitmap[(dy * BUF_WIDTH) + x];

		// Handle horizontal flip
		if (attr & 0x20)
		{
			// Flipped X
			tile = src[0];
			mask = ~tile;
			if (mask)
			{
				if (mask & 0x000f) dst[15] = pal[(tile >>  0) & 0x0f];
				if (mask & 0x00f0) dst[11] = pal[(tile >>  4) & 0x0f];
				if (mask & 0x0f00) dst[14] = pal[(tile >>  8) & 0x0f];
				if (mask & 0xf000) dst[10] = pal[(tile >> 12) & 0x0f];
				mask >>= 16;
				if (mask & 0x000f) dst[13] = pal[(tile >> 16) & 0x0f];
				if (mask & 0x00f0) dst[ 9] = pal[(tile >> 20) & 0x0f];
				if (mask & 0x0f00) dst[12] = pal[(tile >> 24) & 0x0f];
				if (mask & 0xf000) dst[ 8] = pal[(tile >> 28) & 0x0f];
			}
			tile = src[1];
			mask = ~tile;
			if (mask)
			{
				if (mask & 0x000f) dst[ 7] = pal[(tile >>  0) & 0x0f];
				if (mask & 0x00f0) dst[ 3] = pal[(tile >>  4) & 0x0f];
				if (mask & 0x0f00) dst[ 6] = pal[(tile >>  8) & 0x0f];
				if (mask & 0xf000) dst[ 2] = pal[(tile >> 12) & 0x0f];
				mask >>= 16;
				if (mask & 0x000f) dst[ 5] = pal[(tile >> 16) & 0x0f];
				if (mask & 0x00f0) dst[ 1] = pal[(tile >> 20) & 0x0f];
				if (mask & 0x0f00) dst[ 4] = pal[(tile >> 24) & 0x0f];
				if (mask & 0xf000) dst[ 0] = pal[(tile >> 28) & 0x0f];
			}
		}
		else
		{
			// Normal (no flip)
			tile = src[0];
			mask = ~tile;
			if (mask)
			{
				if (mask & 0x000f) dst[ 0] = pal[(tile >>  0) & 0x0f];
				if (mask & 0x00f0) dst[ 4] = pal[(tile >>  4) & 0x0f];
				if (mask & 0x0f00) dst[ 1] = pal[(tile >>  8) & 0x0f];
				if (mask & 0xf000) dst[ 5] = pal[(tile >> 12) & 0x0f];
				mask >>= 16;
				if (mask & 0x000f) dst[ 2] = pal[(tile >> 16) & 0x0f];
				if (mask & 0x00f0) dst[ 6] = pal[(tile >> 20) & 0x0f];
				if (mask & 0x0f00) dst[ 3] = pal[(tile >> 24) & 0x0f];
				if (mask & 0xf000) dst[ 7] = pal[(tile >> 28) & 0x0f];
			}
			tile = src[1];
			mask = ~tile;
			if (mask)
			{
				if (mask & 0x000f) dst[ 8] = pal[(tile >>  0) & 0x0f];
				if (mask & 0x00f0) dst[12] = pal[(tile >>  4) & 0x0f];
				if (mask & 0x0f00) dst[ 9] = pal[(tile >>  8) & 0x0f];
				if (mask & 0xf000) dst[13] = pal[(tile >> 12) & 0x0f];
				mask >>= 16;
				if (mask & 0x000f) dst[10] = pal[(tile >> 16) & 0x0f];
				if (mask & 0x00f0) dst[14] = pal[(tile >> 20) & 0x0f];
				if (mask & 0x0f00) dst[11] = pal[(tile >> 24) & 0x0f];
				if (mask & 0xf000) dst[15] = pal[(tile >> 28) & 0x0f];
			}
		}

		src += 2;
	}
}

/*------------------------------------------------------------------------
	SDL2: Draw 8x8 scroll1 tile directly to framebuffer
------------------------------------------------------------------------*/

void sdl2_draw_scroll1_8x8(INT16 x, INT16 y, UINT32 code, UINT16 attr)
{
	UINT32 *src;
	UINT16 *dst, *pal;
	UINT32 tile, mask;
	int sy, dy;

	// Get palette (scroll1 uses palettes 32-47)
	pal = &video_palette[((attr & 0x1f) + 32) << 4];

	// Get source tile data
	src = (UINT32 *)&memory_region_gfx1[code << 6];

	// Check clipping
	if (x < 64 || x >= 448 || y < clip_min_y - 8 || y >= clip_max_y)
		return;

	// Render each line
	for (sy = 0, dy = y; sy < 8 && dy < clip_max_y; sy++, dy++)
	{
		if (dy < clip_min_y)
		{
			src += 2;
			continue;
		}

		dst = &scrbitmap[(dy * BUF_WIDTH) + x];

		// 8x8 tile is stored differently - only use second UINT32
		tile = src[1];
		mask = ~tile;
		if (mask)
		{
			if (mask & 0x000f) dst[0] = pal[(tile >>  0) & 0x0f];
			if (mask & 0x00f0) dst[4] = pal[(tile >>  4) & 0x0f];
			if (mask & 0x0f00) dst[1] = pal[(tile >>  8) & 0x0f];
			if (mask & 0xf000) dst[5] = pal[(tile >> 12) & 0x0f];
			mask >>= 16;
			if (mask & 0x000f) dst[2] = pal[(tile >> 16) & 0x0f];
			if (mask & 0x00f0) dst[6] = pal[(tile >> 20) & 0x0f];
			if (mask & 0x0f00) dst[3] = pal[(tile >> 24) & 0x0f];
			if (mask & 0xf000) dst[7] = pal[(tile >> 28) & 0x0f];
		}

		src += 2;
	}
}

/*------------------------------------------------------------------------
	SDL2: Draw 32x32 scroll3 tile directly to framebuffer
------------------------------------------------------------------------*/

void sdl2_draw_scroll3_32x32(INT16 x, INT16 y, UINT32 code, UINT16 attr)
{
	UINT32 *src;
	UINT16 *dst, *pal;
	UINT32 tile, mask;
	int sy, dy, sx;

	// Get palette (scroll3 uses palettes 96-111)
	pal = &video_palette[((attr & 0x1f) + 96) << 4];

	// Get source tile data
	src = (UINT32 *)&memory_region_gfx1[code << 9];

	// Check clipping
	if (x < 64 - 32 || x >= 448 || y < clip_min_y - 32 || y >= clip_max_y)
		return;

	// Render each line
	for (sy = 0, dy = y; sy < 32 && dy < clip_max_y; sy++, dy++)
	{
		if (dy < clip_min_y)
		{
			src += 4;
			continue;
		}

		dst = &scrbitmap[(dy * BUF_WIDTH) + x];

		// 32x32 tile has 4 UINT32s per line
		for (sx = 0; sx < 4; sx++)
		{
			tile = src[sx];
			mask = ~tile;
			if (mask)
			{
				int offset = sx * 8;
				if (mask & 0x000f) dst[offset+0] = pal[(tile >>  0) & 0x0f];
				if (mask & 0x00f0) dst[offset+4] = pal[(tile >>  4) & 0x0f];
				if (mask & 0x0f00) dst[offset+1] = pal[(tile >>  8) & 0x0f];
				if (mask & 0xf000) dst[offset+5] = pal[(tile >> 12) & 0x0f];
				mask >>= 16;
				if (mask & 0x000f) dst[offset+2] = pal[(tile >> 16) & 0x0f];
				if (mask & 0x00f0) dst[offset+6] = pal[(tile >> 20) & 0x0f];
				if (mask & 0x0f00) dst[offset+3] = pal[(tile >> 24) & 0x0f];
				if (mask & 0xf000) dst[offset+7] = pal[(tile >> 28) & 0x0f];
			}
		}

		src += 4;
	}
}

#endif /* SDL2 */
