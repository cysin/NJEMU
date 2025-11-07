/******************************************************************************

	config.c

	SDL2 configuration file management

******************************************************************************/

#include "sdl2.h"
#include "emumain.h"
#include <sys/stat.h>

/******************************************************************************
	Local variables
******************************************************************************/

static char config_dir[MAX_PATH];
static char config_file[MAX_PATH];

/******************************************************************************
	Functions
******************************************************************************/

/*--------------------------------------------------------
	Get configuration directory
--------------------------------------------------------*/

static void get_config_dir(void)
{
	char *home = getenv("HOME");

	if (home)
	{
		snprintf(config_dir, MAX_PATH, "%s/.config/njemu", home);
	}
	else
	{
		snprintf(config_dir, MAX_PATH, "%sconfig", launchDir);
	}

	// Create directory if it doesn't exist
	mkdir(config_dir, 0755);
}


/*--------------------------------------------------------
	Load global settings
--------------------------------------------------------*/

void load_settings(void)
{
	get_config_dir();

	snprintf(config_file, MAX_PATH, "%s/settings.cfg", config_dir);

	// TODO: Implement actual config file parsing
	// For now, use defaults

	printf("Config: Would load from %s\n", config_file);
}


/*--------------------------------------------------------
	Save global settings
--------------------------------------------------------*/

void save_settings(void)
{
	get_config_dir();

	snprintf(config_file, MAX_PATH, "%s/settings.cfg", config_dir);

	// TODO: Implement actual config file writing

	printf("Config: Would save to %s\n", config_file);
}


/*--------------------------------------------------------
	Load game-specific configuration
--------------------------------------------------------*/

void load_gamecfg(const char *name)
{
	if (!name) return;

	get_config_dir();

	snprintf(config_file, MAX_PATH, "%s/%s.cfg", config_dir, name);

	// TODO: Implement game config loading

	printf("Config: Would load game config from %s\n", config_file);
}


/*--------------------------------------------------------
	Save game-specific configuration
--------------------------------------------------------*/

void save_gamecfg(const char *name)
{
	if (!name) return;

	get_config_dir();

	snprintf(config_file, MAX_PATH, "%s/%s.cfg", config_dir, name);

	// TODO: Implement game config saving

	printf("Config: Would save game config to %s\n", config_file);
}
