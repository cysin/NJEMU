/******************************************************************************

	ui_draw.c

	SDL2 UI drawing functions - stub implementation

******************************************************************************/

#include "sdl2.h"
#include <stdarg.h>

/******************************************************************************
	Global variables
******************************************************************************/

UI_PALETTE ui_palette[UI_PAL_MAX] =
{
	{ 255, 255,   0 },	// UI_PAL_TITLE
	{ 255, 255, 255 },	// UI_PAL_SELECT
	{ 200, 200, 200 },	// UI_PAL_NORMAL
	{ 128, 255, 128 },	// UI_PAL_INFO
	{ 255, 128, 128 },	// UI_PAL_WARNING
	{  32,  32,  96 },	// UI_PAL_BG1
	{  16,  16,  48 },	// UI_PAL_BG2
	{ 128, 128, 255 },	// UI_PAL_FRAME
	{  64,  64, 128 },	// UI_PAL_FILESEL1
	{  48,  48,  96 },	// UI_PAL_FILESEL2
};

/******************************************************************************
	Functions
******************************************************************************/

/*--------------------------------------------------------
	Initialize UI
--------------------------------------------------------*/

void ui_init(void)
{
	printf("UI initialized\n");
}


/*--------------------------------------------------------
	Font functions - stubs
--------------------------------------------------------*/

int uifont_get_string_width(const char *s)
{
	if (!s) return 0;
	return strlen(s) * 8;  // Assume 8-pixel wide characters
}

void uifont_print(int sx, int sy, int r, int g, int b, const char *s)
{
	// TODO: Implement font rendering
	(void)sx; (void)sy; (void)r; (void)g; (void)b; (void)s;
}

void uifont_print_center(int sy, int r, int g, int b, const char *s)
{
	if (!s) return;
	int sx = (SCR_WIDTH - uifont_get_string_width(s)) / 2;
	uifont_print(sx, sy, r, g, b, s);
}

void uifont_print_shadow(int sx, int sy, int r, int g, int b, const char *s)
{
	uifont_print(sx, sy, r, g, b, s);
}

void uifont_print_shadow_center(int sy, int r, int g, int b, const char *s)
{
	uifont_print_center(sy, r, g, b, s);
}

#ifdef COMMAND_LIST
void textfont_print(int sx, int sy, int r, int g, int b, const char *s, int flag)
{
	(void)flag;
	uifont_print(sx, sy, r, g, b, s);
}
#endif

void small_font_print(int sx, int sy, const char *s, int bg)
{
	(void)bg;
	uifont_print(sx, sy, 255, 255, 255, s);
}

void small_font_printf(int x, int y, const char *text, ...)
{
	char buf[256];
	va_list args;

	va_start(args, text);
	vsnprintf(buf, sizeof(buf), text, args);
	va_end(args);

	small_font_print(x, y, buf, 1);
}

void debug_font_printf(void *frame, int x, int y, const char *text, ...)
{
	char buf[256];
	va_list args;

	(void)frame;

	va_start(args, text);
	vsnprintf(buf, sizeof(buf), text, args);
	va_end(args);

	small_font_print(x, y, buf, 1);
}


/*--------------------------------------------------------
	Icon functions - stubs
--------------------------------------------------------*/

void small_icon(int sx, int sy, int r, int g, int b, int no)
{
	(void)sx; (void)sy; (void)r; (void)g; (void)b; (void)no;
}

void small_icon_shadow(int sx, int sy, int r, int g, int b, int no)
{
	small_icon(sx, sy, r, g, b, no);
}

void small_icon_light(int sx, int sy, int r, int g, int b, int no)
{
	small_icon(sx, sy, r, g, b, no);
}

void large_icon(int sx, int sy, int r, int g, int b, int no)
{
	(void)sx; (void)sy; (void)r; (void)g; (void)b; (void)no;
}

void large_icon_shadow(int sx, int sy, int r, int g, int b, int no)
{
	large_icon(sx, sy, r, g, b, no);
}

void large_icon_light(int sx, int sy, int r, int g, int b, int no)
{
	large_icon(sx, sy, r, g, b, no);
}

int ui_light_update(void)
{
	return 0;
}


/*--------------------------------------------------------
	Volume display - stub
--------------------------------------------------------*/

void draw_volume(int volume)
{
	(void)volume;
}


/*--------------------------------------------------------
	Drawing primitives
--------------------------------------------------------*/

static void put_pixel(int x, int y, UINT32 color)
{
	if (!draw_frame) return;
	if (x < 0 || x >= SCR_WIDTH || y < 0 || y >= SCR_HEIGHT) return;

	if (video_mode == 32)
	{
		UINT32 *pixels = (UINT32 *)draw_frame;
		pixels[x + y * BUF_WIDTH] = color;
	}
	else
	{
		UINT16 *pixels = (UINT16 *)draw_frame;
		pixels[x + y * BUF_WIDTH] = (UINT16)color;
	}
}

void hline(int sx, int ex, int y, int r, int g, int b)
{
	UINT32 color = MAKECOL32(r, g, b);
	for (int x = sx; x <= ex; x++)
	{
		put_pixel(x, y, color);
	}
}

void hline_alpha(int sx, int ex, int y, int r, int g, int b, int alpha)
{
	(void)alpha;
	hline(sx, ex, y, r, g, b);
}

void hline_gradation(int sx, int ex, int y, int r1, int g1, int b1, int r2, int g2, int b2, int alpha)
{
	(void)r2; (void)g2; (void)b2; (void)alpha;
	hline(sx, ex, y, r1, g1, b1);
}

void vline(int x, int sy, int ey, int r, int g, int b)
{
	UINT32 color = MAKECOL32(r, g, b);
	for (int y = sy; y <= ey; y++)
	{
		put_pixel(x, y, color);
	}
}

void vline_alpha(int x, int sy, int ey, int r, int g, int b, int alpha)
{
	(void)alpha;
	vline(x, sy, ey, r, g, b);
}

void vline_gradation(int x, int sy, int ey, int r1, int g1, int b1, int r2, int g2, int b2, int alpha)
{
	(void)r2; (void)g2; (void)b2; (void)alpha;
	vline(x, sy, ey, r1, g1, b1);
}

void box(int sx, int sy, int ex, int ey, int r, int g, int b)
{
	hline(sx, ex, sy, r, g, b);
	hline(sx, ex, ey, r, g, b);
	vline(sx, sy, ey, r, g, b);
	vline(ex, sy, ey, r, g, b);
}

void boxfill(int sx, int sy, int ex, int ey, int r, int g, int b)
{
	RECT rect = { sx, sy, ex, ey };
	UINT32 color = MAKECOL32(r, g, b);
	video_fill_rect(draw_frame, color, &rect);
}

void boxfill_alpha(int sx, int sy, int ex, int ey, int r, int g, int b, int alpha)
{
	(void)alpha;
	boxfill(sx, sy, ex, ey, r, g, b);
}

void boxfill_gradation(int sx, int sy, int ex, int ey, int r1, int g1, int b1, int r2, int g2, int b2, int alpha, int dir)
{
	(void)r2; (void)g2; (void)b2; (void)alpha; (void)dir;
	boxfill(sx, sy, ex, ey, r1, g1, b1);
}

void draw_box_shadow(int sx, int sy, int ex, int ey)
{
	(void)sx; (void)sy; (void)ex; (void)ey;
}

void draw_bar_shadow(void)
{
}


/*--------------------------------------------------------
	UI color management
--------------------------------------------------------*/

void get_ui_color(UI_PALETTE *pal, int *r, int *g, int *b)
{
	if (!pal) return;
	if (r) *r = pal->r;
	if (g) *g = pal->g;
	if (b) *b = pal->b;
}

void set_ui_color(UI_PALETTE *pal, int r, int g, int b)
{
	if (!pal) return;
	pal->r = r;
	pal->g = g;
	pal->b = b;
}


/*--------------------------------------------------------
	Logo - stub
--------------------------------------------------------*/

void logo(int sx, int sy, int r, int g, int b)
{
	(void)sx; (void)sy; (void)r; (void)g; (void)b;
	// TODO: Draw logo
}
