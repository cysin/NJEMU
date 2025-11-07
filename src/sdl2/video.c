/******************************************************************************

	video.c

	SDL2 video subsystem - main graphics implementation

******************************************************************************/

#include "sdl2.h"
#include <string.h>

/******************************************************************************
	Local variables
******************************************************************************/

static int pixel_format = VIDEO_32BPP;
static SDL_Surface *show_surface = NULL;
static SDL_Surface *draw_surface = NULL;
static SDL_Surface *work_surface = NULL;
static SDL_Surface *tex_surface = NULL;

/******************************************************************************
	Global variables
******************************************************************************/

int video_mode = 32;  // Default to 32-bit
void *show_frame = NULL;
void *draw_frame = NULL;
void *work_frame = NULL;
void *tex_frame = NULL;

SDL_Window *sdl_window = NULL;
SDL_Renderer *sdl_renderer = NULL;
SDL_Texture *sdl_texture = NULL;

RECT full_rect = { 0, 0, SCR_WIDTH, SCR_HEIGHT };

// UI texture buffer for thumbnails and UI rendering
void *ui_texture_buffer = NULL;

/******************************************************************************
	Functions
******************************************************************************/

/*--------------------------------------------------------
	Set video mode
--------------------------------------------------------*/

void video_set_mode(int mode)
{
	if (video_mode != mode)
	{
		if (video_mode) video_exit();

		video_mode = mode;

		video_init();
	}
}


/*--------------------------------------------------------
	Initialize video subsystem
--------------------------------------------------------*/

void video_init(void)
{
	Uint32 pixel_fmt;
	int bpp;

	// Determine pixel format
	if (video_mode == 32)
	{
		pixel_format = VIDEO_32BPP;
		pixel_fmt = SDL_PIXELFORMAT_ARGB8888;
		bpp = 4;
	}
	else
	{
		pixel_format = VIDEO_16BPP;
		pixel_fmt = SDL_PIXELFORMAT_ARGB1555;
		bpp = 2;
	}

	// Create window
	sdl_window = SDL_CreateWindow(
		"NJEMU - SDL2 Port",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		SCR_WIDTH * 2,  // 2x scale for better visibility
		SCR_HEIGHT * 2,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
	);

	if (!sdl_window)
	{
		fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
		return;
	}

	// Create renderer with vsync
	sdl_renderer = SDL_CreateRenderer(
		sdl_window,
		-1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
	);

	if (!sdl_renderer)
	{
		fprintf(stderr, "Failed to create renderer: %s\n", SDL_GetError());
		SDL_DestroyWindow(sdl_window);
		sdl_window = NULL;
		return;
	}

	// Use logical size for easy scaling
	SDL_RenderSetLogicalSize(sdl_renderer, SCR_WIDTH, SCR_HEIGHT);

	// Create streaming texture for rendering
	sdl_texture = SDL_CreateTexture(
		sdl_renderer,
		pixel_fmt,
		SDL_TEXTUREACCESS_STREAMING,
		BUF_WIDTH,
		SCR_HEIGHT
	);

	if (!sdl_texture)
	{
		fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
		SDL_DestroyRenderer(sdl_renderer);
		SDL_DestroyWindow(sdl_window);
		sdl_renderer = NULL;
		sdl_window = NULL;
		return;
	}

	// Allocate framebuffers as SDL surfaces
	show_surface = SDL_CreateRGBSurfaceWithFormat(0, BUF_WIDTH, SCR_HEIGHT, bpp * 8, pixel_fmt);
	draw_surface = SDL_CreateRGBSurfaceWithFormat(0, BUF_WIDTH, SCR_HEIGHT, bpp * 8, pixel_fmt);
	work_surface = SDL_CreateRGBSurfaceWithFormat(0, BUF_WIDTH, SCR_HEIGHT, bpp * 8, pixel_fmt);
	tex_surface = SDL_CreateRGBSurfaceWithFormat(0, BUF_WIDTH, SCR_HEIGHT, bpp * 8, pixel_fmt);

	if (!show_surface || !draw_surface || !work_surface || !tex_surface)
	{
		fprintf(stderr, "Failed to create surfaces: %s\n", SDL_GetError());
		video_exit();
		return;
	}

	// Set frame pointers
	show_frame = show_surface->pixels;
	draw_frame = draw_surface->pixels;
	work_frame = work_surface->pixels;
	tex_frame = tex_surface->pixels;

	// Allocate UI texture buffer (for thumbnails and UI rendering)
	// Always use 16-bit format for UI texture
	if (ui_texture_buffer == NULL)
	{
		ui_texture_buffer = malloc(BUF_WIDTH * SCR_HEIGHT * 2);
		if (ui_texture_buffer)
			memset(ui_texture_buffer, 0, BUF_WIDTH * SCR_HEIGHT * 2);
	}

	// Clear all buffers
	video_clear_frame(show_frame);
	video_clear_frame(draw_frame);
	video_clear_frame(work_frame);

	// Initialize UI (forward declaration - will be implemented later)
	ui_init();

	printf("Video initialized: %dx%d, %d-bit\n", SCR_WIDTH, SCR_HEIGHT, video_mode);
}


/*--------------------------------------------------------
	Shutdown video subsystem
--------------------------------------------------------*/

void video_exit(void)
{
	if (tex_surface) SDL_FreeSurface(tex_surface);
	if (work_surface) SDL_FreeSurface(work_surface);
	if (draw_surface) SDL_FreeSurface(draw_surface);
	if (show_surface) SDL_FreeSurface(show_surface);

	if (sdl_texture) SDL_DestroyTexture(sdl_texture);
	if (sdl_renderer) SDL_DestroyRenderer(sdl_renderer);
	if (sdl_window) SDL_DestroyWindow(sdl_window);

	if (ui_texture_buffer)
	{
		free(ui_texture_buffer);
		ui_texture_buffer = NULL;
	}

	tex_surface = work_surface = draw_surface = show_surface = NULL;
	sdl_texture = NULL;
	sdl_renderer = NULL;
	sdl_window = NULL;

	show_frame = draw_frame = work_frame = tex_frame = NULL;
}


/*--------------------------------------------------------
	Wait for vsync
--------------------------------------------------------*/

void video_wait_vsync(void)
{
	// With SDL_RENDERER_PRESENTVSYNC, SDL_RenderPresent waits for vsync
	// For explicit wait, we can use a simple delay
	SDL_Delay(1);
}


/*--------------------------------------------------------
	Flip screen buffers and present
--------------------------------------------------------*/

void video_flip_screen(int vsync)
{
	SDL_Rect src_rect = { 0, 0, SCR_WIDTH, SCR_HEIGHT };
	SDL_Rect dst_rect = { 0, 0, SCR_WIDTH, SCR_HEIGHT };

	// Swap buffers
	SDL_Surface *temp = show_surface;
	show_surface = draw_surface;
	draw_surface = temp;

	show_frame = show_surface->pixels;
	draw_frame = draw_surface->pixels;

	// Update texture with show buffer
	SDL_UpdateTexture(sdl_texture, NULL, show_surface->pixels, show_surface->pitch);

	// Render to screen
	SDL_RenderClear(sdl_renderer);
	SDL_RenderCopy(sdl_renderer, sdl_texture, &src_rect, &dst_rect);
	SDL_RenderPresent(sdl_renderer);

	// Optional vsync wait (redundant if PRESENTVSYNC is set)
	if (vsync)
	{
		// Already handled by SDL_RenderPresent with PRESENTVSYNC flag
	}
}


/*--------------------------------------------------------
	Get address within framebuffer
--------------------------------------------------------*/

void *video_frame_addr(void *frame, int x, int y)
{
	int bpp = (video_mode == 32) ? 4 : 2;
	return (void *)((UINT8 *)frame + ((x + (y * BUF_WIDTH)) * bpp));
}


/*--------------------------------------------------------
	Clear a framebuffer
--------------------------------------------------------*/

void video_clear_frame(void *frame)
{
	if (!frame) return;

	SDL_Surface *surface = NULL;

	// Find which surface this frame belongs to
	if (frame == show_frame) surface = show_surface;
	else if (frame == draw_frame) surface = draw_surface;
	else if (frame == work_frame) surface = work_surface;
	else if (frame == tex_frame) surface = tex_surface;

	if (surface)
	{
		SDL_FillRect(surface, NULL, 0);
	}
	else
	{
		// Direct memory clear
		int size = (video_mode == 32) ? FRAMESIZE32 : FRAMESIZE;
		memset(frame, 0, size);
	}
}


/*--------------------------------------------------------
	Clear show and draw buffers
--------------------------------------------------------*/

void video_clear_screen(void)
{
	video_clear_frame(show_frame);
	video_clear_frame(draw_frame);
}


/*--------------------------------------------------------
	Clear a rectangle in framebuffer
--------------------------------------------------------*/

void video_clear_rect(void *frame, RECT *rect)
{
	if (!frame || !rect) return;

	SDL_Surface *surface = NULL;

	if (frame == show_frame) surface = show_surface;
	else if (frame == draw_frame) surface = draw_surface;
	else if (frame == work_frame) surface = work_surface;
	else if (frame == tex_frame) surface = tex_surface;

	if (surface)
	{
		SDL_Rect sdl_rect = { rect->left, rect->top,
		                      rect->right - rect->left,
		                      rect->bottom - rect->top };
		SDL_FillRect(surface, &sdl_rect, 0);
	}
}


/*--------------------------------------------------------
	Fill framebuffer with color
--------------------------------------------------------*/

void video_fill_frame(void *frame, UINT32 color)
{
	if (!frame) return;

	SDL_Surface *surface = NULL;

	if (frame == show_frame) surface = show_surface;
	else if (frame == draw_frame) surface = draw_surface;
	else if (frame == work_frame) surface = work_surface;
	else if (frame == tex_frame) surface = tex_surface;

	if (surface)
	{
		SDL_FillRect(surface, NULL, color);
	}
}


/*--------------------------------------------------------
	Fill rectangle with color
--------------------------------------------------------*/

void video_fill_rect(void *frame, UINT32 color, RECT *rect)
{
	if (!frame || !rect) return;

	SDL_Surface *surface = NULL;

	if (frame == show_frame) surface = show_surface;
	else if (frame == draw_frame) surface = draw_surface;
	else if (frame == work_frame) surface = work_surface;
	else if (frame == tex_frame) surface = tex_surface;

	if (surface)
	{
		SDL_Rect sdl_rect = { rect->left, rect->top,
		                      rect->right - rect->left,
		                      rect->bottom - rect->top };
		SDL_FillRect(surface, &sdl_rect, color);
	}
}


/*--------------------------------------------------------
	Copy rectangle with optional scaling
--------------------------------------------------------*/

void video_copy_rect(void *src, void *dst, RECT *src_rect, RECT *dst_rect)
{
	if (!src || !dst || !src_rect || !dst_rect) return;

	SDL_Surface *src_surface = NULL;
	SDL_Surface *dst_surface = NULL;

	// Find source surface
	if (src == show_frame) src_surface = show_surface;
	else if (src == draw_frame) src_surface = draw_surface;
	else if (src == work_frame) src_surface = work_surface;
	else if (src == tex_frame) src_surface = tex_surface;

	// Find destination surface
	if (dst == show_frame) dst_surface = show_surface;
	else if (dst == draw_frame) dst_surface = draw_surface;
	else if (dst == work_frame) dst_surface = work_surface;
	else if (dst == tex_frame) dst_surface = tex_surface;

	if (src_surface && dst_surface)
	{
		SDL_Rect src_r = { src_rect->left, src_rect->top,
		                   src_rect->right - src_rect->left,
		                   src_rect->bottom - src_rect->top };
		SDL_Rect dst_r = { dst_rect->left, dst_rect->top,
		                   dst_rect->right - dst_rect->left,
		                   dst_rect->bottom - dst_rect->top };

		// Use SDL_BlitScaled for scaling support
		SDL_BlitScaled(src_surface, &src_r, dst_surface, &dst_r);
	}
}


/*--------------------------------------------------------
	Copy rectangle with horizontal flip
--------------------------------------------------------*/

void video_copy_rect_flip(void *src, void *dst, RECT *src_rect, RECT *dst_rect)
{
	// SDL doesn't have built-in flip, so we'll do it manually
	// For now, just use regular copy (TODO: implement flip)
	video_copy_rect(src, dst, src_rect, dst_rect);
}


/*--------------------------------------------------------
	Copy rectangle with 270° rotation
--------------------------------------------------------*/

void video_copy_rect_rotate(void *src, void *dst, RECT *src_rect, RECT *dst_rect)
{
	// SDL doesn't have built-in rotation in surfaces
	// For now, just use regular copy (TODO: implement rotation)
	video_copy_rect(src, dst, src_rect, dst_rect);
}


/*--------------------------------------------------------
	Clear depth buffer (not used in 2D, stub)
--------------------------------------------------------*/

void video_clear_depth(void *frame)
{
	// Not needed for 2D rendering
	(void)frame;
}


/*--------------------------------------------------------
	Draw texture with custom formats
--------------------------------------------------------*/

void video_draw_texture(UINT32 src_fmt, UINT32 dst_fmt, void *src, void *dst, RECT *src_rect, RECT *dst_rect)
{
	// For now, just use regular copy
	// TODO: Handle format conversion if src_fmt != dst_fmt
	(void)src_fmt;
	(void)dst_fmt;
	video_copy_rect(src, dst, src_rect, dst_rect);
}
