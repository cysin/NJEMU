/******************************************************************************

	ui_menu.c

	SDL2 menu system - stub implementation

******************************************************************************/

#include "sdl2.h"
#include "emumain.h"

/******************************************************************************
	Functions
******************************************************************************/

/*--------------------------------------------------------
	Main menu
--------------------------------------------------------*/

void showmenu(void)
{
	printf("Menu: Main menu (not yet implemented)\n");

	// For now, just return to emulation
}


/*--------------------------------------------------------
	Color settings menu
--------------------------------------------------------*/

void show_color_menu(void)
{
	printf("Menu: Color settings (not yet implemented)\n");
}


/*--------------------------------------------------------
	Cheat system stubs
--------------------------------------------------------*/

int add_new_cheat(int type, char *cheat_name, gamecheat_t** new_cheat)
{
	(void)type; (void)cheat_name; (void)new_cheat;
	return 0;
}

int add_new_cheat_option(char *label, gamecheat_t* cheat, cheat_option_t** new_cheat_option)
{
	(void)label; (void)cheat; (void)new_cheat_option;
	return 0;
}

int add_new_cheat_value(int cpu, int address, int value, cheat_option_t *cheat_option)
{
	(void)cpu; (void)address; (void)value; (void)cheat_option;
	return 0;
}

int cheat_clear(void)
{
	return 0;
}

int axtoi(char *str_num)
{
	if (!str_num) return 0;
	return (int)strtol(str_num, NULL, 16);
}

int parse_cheat_option_line(char *line, gamecheat_t* cheat)
{
	(void)line; (void)cheat;
	return 0;
}

int cheats_init(void)
{
	return 1;
}

int menu_cheatcfg(void)
{
	printf("Menu: Cheat configuration (not yet implemented)\n");
	return 0;
}

void cheats_load(void)
{
	// TODO: Load cheat database
}

void cheats_load_file(char *fn)
{
	(void)fn;
}

int get_free_memory(void)
{
	// Return a large value for desktop
	return 64 * 1024 * 1024;  // 64MB
}

int menu_cheatcfg_memfree(void)
{
	return get_free_memory();
}
