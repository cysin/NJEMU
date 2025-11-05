/******************************************************************************

	ui_menu.h

	SDL2 menu system

******************************************************************************/

#ifndef SDL2_UI_MENU_H
#define SDL2_UI_MENU_H

// Menu functions
void showmenu(void);
void show_color_menu(void);

// Cheat system constants
#define MAX_CHEATS 150
#define MAX_CHEAT_OPTION 140
#define MAX_CHEAT_VALUE 10

// Cheat structures
typedef struct {
	int address;
	int value;
} cheat_value_t;

typedef struct {
	char *label;
	int num_cheat_values;
	cheat_value_t *cheat_value[MAX_CHEAT_VALUE];
} cheat_option_t;

typedef struct {
	int curr_option;
	char *cheat_name;
	short int num_cheat_options;
	cheat_option_t *cheat_option[MAX_CHEAT_OPTION];
} gamecheat_t;

// Cheat functions
int add_new_cheat(int type, char *cheat_name, gamecheat_t** new_cheat);
int add_new_cheat_option(char *label, gamecheat_t* cheat, cheat_option_t** new_cheat_option);
int add_new_cheat_value(int cpu, int address, int value, cheat_option_t *cheat_option);
int cheat_clear(void);
int axtoi(char *str_num);
int parse_cheat_option_line(char *line, gamecheat_t* cheat);
int cheats_init(void);
int menu_cheatcfg(void);
void cheats_load(void);
void cheats_load_file(char *fn);
int get_free_memory(void);
int menu_cheatcfg_memfree(void);

#endif /* SDL2_UI_MENU_H */
