/******************************************************************************

	ui.h

	SDL2 user interface functions

******************************************************************************/

#ifndef SDL2_UI_H
#define SDL2_UI_H

#define UI_FULL_REFRESH		1
#define UI_PARTIAL_REFRESH	2

// Wallpaper IDs
enum
{
	WP_LOGO = 0,
	WP_FILER,
	WP_GAMECFG,
	WP_KEYCFG,
	WP_STATE,
	WP_COLORCFG,
	WP_DIPSW,
	WP_CMDLIST,
	NUM_WALLPAPERS
};

// Background types
enum
{
	BG_DEFAULT = 0,
	BG_USER,
	BG_LOGOONLY,
	BG_DISABLE,
	BG_MAX
};

extern int bgimage_type;
extern int bgimage_brightness;

// Background functions
void load_background(int number);
void show_background(void);

// Status indicators (battery, volume, etc.)
int draw_battery_status(int draw);
int draw_volume_status(int draw);

// UI elements
void draw_dialog(int sx, int sy, int ex, int ey);
void draw_scrollbar(int sx, int sy, int ex, int ey, int disp_lines, int total_lines, int current_line);

// Popup messages
void ui_popup_reset(void);
void ui_popup(const char *text, ...);
int ui_show_popup(int draw);

// Message screen
void msg_screen_init(int wallpaper, int icon, const char *title);
void msg_screen_clear(void);
void msg_set_text_color(UINT32 color);
void msg_printf(const char *text, ...);

// Progress indicators
void init_progress(int total, const char *text);
void update_progress(void);
void show_progress(const char *text);

// Message box types
enum
{
	MB_STARTEMULATION = 0,
#ifdef ADHOC
	MB_STARTEMULATION_ADHOC,
#endif
	MB_EXITEMULATION,
	MB_RETURNTOFILEBROWSER,
	MB_RESETEMULATION,
	MB_RESTARTEMULATION,
#if (EMU_SYSTEM != NCDZ)
	MB_GAMENOTWORK,
#endif
	MB_SETSTARTUPDIR,
#ifdef SAVE_STATE
	MB_STARTSAVESTATE,
	MB_FINISHSAVESTATE,
	MB_STARTLOADSTATE,
	MB_FINISHLOADSTATE,
	MB_DELETESTATE,
#endif
#if (EMU_SYSTEM == NCDZ)
	MB_STARTEMULATION_NOMP3,
	MB_BOOTBIOS,
	MB_BIOSNOTFOUND,
	MB_BIOSINVALID,
#endif
	MB_NUM_MAX
};

int messagebox(int number);

// Help screen types
enum
{
	HELP_FILEBROWSER = 0,
	HELP_MAINMENU,
#if (EMU_SYSTEM == MVS)
	HELP_SELECTBIOS,
#endif
	HELP_GAMECONFIG,
	HELP_KEYCONFIG,
#if (EMU_SYSTEM == CPS1 || EMU_SYSTEM == MVS)
	HELP_DIPSWITCH,
#endif
#ifdef SAVE_STATE
	HELP_STATE,
#endif
	HELP_COLORSETTINGS,
#ifdef COMMAND_LIST
	HELP_COMMANDLIST,
#endif
	HELP_CHEATCONFIG,
	HELP_NUM_MAX
};

int help(int number);

#endif /* SDL2_UI_H */
