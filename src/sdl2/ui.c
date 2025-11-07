/******************************************************************************

	ui.c

	SDL2 user interface - stub implementation

******************************************************************************/

#include "sdl2.h"
#include "emumain.h"
#include <stdarg.h>

/******************************************************************************
	Global variables
******************************************************************************/

int bgimage_type = BG_DEFAULT;
int bgimage_brightness = 100;

/******************************************************************************
	Local variables
******************************************************************************/

static char popup_text[256];
static int popup_active = 0;

/******************************************************************************
	Functions
******************************************************************************/

/*--------------------------------------------------------
	Background functions
--------------------------------------------------------*/

void load_background(int number)
{
	(void)number;
	// TODO: Load wallpaper image
}

void show_background(void)
{
	// TODO: Display background
}


/*--------------------------------------------------------
	Status indicators
--------------------------------------------------------*/

int draw_battery_status(int draw)
{
	(void)draw;
	// Not applicable on desktop
	return 0;
}

int draw_volume_status(int draw)
{
	(void)draw;
	// TODO: Draw volume indicator
	return 0;
}


/*--------------------------------------------------------
	UI elements
--------------------------------------------------------*/

void draw_dialog(int sx, int sy, int ex, int ey)
{
	box(sx, sy, ex, ey, UI_COLOR(UI_PAL_FRAME));
	boxfill(sx + 1, sy + 1, ex - 1, ey - 1, UI_COLOR(UI_PAL_BG1));
}

void draw_scrollbar(int sx, int sy, int ex, int ey, int disp_lines, int total_lines, int current_line)
{
	(void)disp_lines; (void)total_lines; (void)current_line;
	vline(sx, sy, ey, UI_COLOR(UI_PAL_FRAME));
}


/*--------------------------------------------------------
	Popup messages
--------------------------------------------------------*/

void ui_popup_reset(void)
{
	popup_active = 0;
	popup_text[0] = '\0';
}

void ui_popup(const char *text, ...)
{
	va_list args;

	va_start(args, text);
	vsnprintf(popup_text, sizeof(popup_text), text, args);
	va_end(args);

	popup_active = 30;  // Display for 30 frames
}

int ui_show_popup(int draw)
{
	if (!popup_active) return 0;

	if (draw)
	{
		int width = uifont_get_string_width(popup_text) + 20;
		int height = 30;
		int sx = (SCR_WIDTH - width) / 2;
		int sy = SCR_HEIGHT - 50;

		draw_dialog(sx, sy, sx + width, sy + height);
		uifont_print_center(sy + 10, UI_COLOR(UI_PAL_NORMAL), popup_text);
	}

	popup_active--;
	return 1;
}


/*--------------------------------------------------------
	Message screen
--------------------------------------------------------*/

void msg_screen_init(int wallpaper, int icon, const char *title)
{
	(void)wallpaper; (void)icon;

	video_clear_screen();

	if (title)
	{
		uifont_print_center(10, UI_COLOR(UI_PAL_TITLE), title);
	}
}

void msg_screen_clear(void)
{
	video_clear_screen();
}

void msg_set_text_color(UINT32 color)
{
	(void)color;
}

void msg_printf(const char *text, ...)
{
	char buf[256];
	va_list args;
	static int y = 40;

	va_start(args, text);
	vsnprintf(buf, sizeof(buf), text, args);
	va_end(args);

	uifont_print(10, y, UI_COLOR(UI_PAL_NORMAL), buf);
	y += 16;

	if (y > SCR_HEIGHT - 20)
	{
		y = 40;
	}
}


/*--------------------------------------------------------
	Progress indicators
--------------------------------------------------------*/

static int progress_total = 0;
static int progress_current = 0;

void init_progress(int total, const char *text)
{
	progress_total = total;
	progress_current = 0;

	msg_screen_init(WP_LOGO, ICON_SYSTEM, "Loading");
	if (text)
	{
		msg_printf("%s", text);
	}
}

void update_progress(void)
{
	progress_current++;

	if (progress_total > 0)
	{
		int percent = (progress_current * 100) / progress_total;
		int barwidth = (SCR_WIDTH - 40) * percent / 100;

		box(20, 100, SCR_WIDTH - 20, 120, UI_COLOR(UI_PAL_FRAME));
		boxfill(21, 101, 20 + barwidth, 119, UI_COLOR(UI_PAL_SELECT));
	}

	video_flip_screen(0);
}

void show_progress(const char *text)
{
	if (text)
	{
		msg_printf("%s", text);
	}
	update_progress();
}


/*--------------------------------------------------------
	Message boxes
--------------------------------------------------------*/

int messagebox(int number)
{
	const char *messages[] =
	{
		"Start emulation?",
		"Start emulation (Ad-Hoc)?",
		"Exit emulation?",
		"Return to file browser?",
		"Reset emulation?",
		"Restart emulation?",
		"Game may not work properly",
		"Set startup directory?",
		"Save state started",
		"Save state finished",
		"Load state started",
		"Load state finished",
		"Delete save state?",
		"Start emulation (no MP3)?",
		"Boot BIOS?",
		"BIOS not found",
		"BIOS invalid",
	};

	if (number < 0 || number >= MB_NUM_MAX)
		return 0;

	printf("MessageBox: %s\n", messages[number]);

	// Auto-confirm for now
	return 1;
}


/*--------------------------------------------------------
	Help screens
--------------------------------------------------------*/

int help(int number)
{
	(void)number;

	printf("Help: Display help screen %d\n", number);

	// Wait for button press
	pad_wait_press(PAD_WAIT_INFINITY);
	pad_wait_clear();

	return 1;
}
