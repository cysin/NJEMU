/******************************************************************************

    main_sdl.c

    Minimal CLI entry for SDL/desktop scaffold.

******************************************************************************/

#include <limits.h>
#include <stdlib.h>
#include <dirent.h>
#include <SDL.h>
#include "emumain.h"

static int pick_rom_from_dir(const char *dir)
{
	DIR *dp = opendir(dir);
	if (!dp)
		return 0;

	char entries[256][64];
	int count = 0;
	struct dirent *de;

	while ((de = readdir(dp)) != NULL && count < 256)
	{
		size_t len = strlen(de->d_name);
		if (len > 4 && (strcmp(de->d_name + len - 4, ".zip") == 0))
		{
			strncpy(entries[count], de->d_name, sizeof(entries[count]) - 1);
			entries[count][sizeof(entries[count]) - 1] = '\0';
			count++;
		}
	}
	closedir(dp);

	if (count == 0)
		return 0;

	fprintf(stderr, "Available ROMs in %s:\n", dir);
	for (int i = 0; i < count; i++)
		fprintf(stderr, " [%d] %s\n", i, entries[i]);

	fprintf(stderr, "Select index: ");
	int choice = 0;
	if (scanf("%d", &choice) != 1 || choice < 0 || choice >= count)
		return 0;

	char *name = entries[choice];
	char *dot = strrchr(name, '.');
	if (dot)
		*dot = '\0';

	strncpy(game_name, name, sizeof(game_name) - 1);
	game_name[sizeof(game_name) - 1] = '\0';
	return 1;
}

int main(int argc, char *argv[])
{
	char cwd[MAX_PATH];

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO) != 0)
	{
		fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
		return 1;
	}
	atexit(SDL_Quit);
	platform_timing_init();
	video_init();

	if (getenv("NJEMU_DEBUG"))
		njemu_debug = 1;

	if (getcwd(cwd, sizeof(cwd)) != NULL)
	{
		size_t len = strlen(cwd);
		if (len && cwd[len - 1] != '/')
			snprintf(launchDir, sizeof(launchDir), "%s/", cwd);
		else
			snprintf(launchDir, sizeof(launchDir), "%s", cwd);
	}

	if (argc < 2)
	{
		snprintf(game_dir, sizeof(game_dir), "%sroms", launchDir);
		if (!pick_rom_from_dir(game_dir))
		{
			fprintf(stderr, "Usage: %s <rom_shortname> [rom_dir]\n", argv[0]);
			return 1;
		}
	}
	else
	{
		strncpy(game_name, argv[1], sizeof(game_name) - 1);
		game_name[sizeof(game_name) - 1] = '\0';

		if (argc >= 3)
		{
			strncpy(game_dir, argv[2], sizeof(game_dir) - 1);
			game_dir[sizeof(game_dir) - 1] = '\0';
		}
		else
		{
			snprintf(game_dir, sizeof(game_dir), "%sroms", launchDir);
		}
	}

	/* ensure expected subdirectories exist */
	const char *subdirs[] = { "roms", "state", "memcard", "nvram", "cheats", "config", "cache" };
	for (size_t i = 0; i < sizeof(subdirs)/sizeof(subdirs[0]); i++)
	{
		char path[MAX_PATH];
		snprintf(path, sizeof(path), "%s%s", launchDir, subdirs[i]);
		mkdir(path, 0755);
	}

	snprintf(screenshotDir, sizeof(screenshotDir), "%sPICTURE", launchDir);
	mkdir(screenshotDir, 0755);

	Loop = LOOP_RESET;
	emu_main();
	video_exit();
	return 0;
}
