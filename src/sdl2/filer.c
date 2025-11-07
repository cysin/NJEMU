/******************************************************************************

	filer.c

	SDL2 file browser

******************************************************************************/

#include "sdl2.h"
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

/******************************************************************************
	Global variables
******************************************************************************/

char startupDir[MAX_PATH];

/******************************************************************************
	Functions
******************************************************************************/

/*--------------------------------------------------------
	Check if file exists
--------------------------------------------------------*/

int file_exist(const char *path)
{
	struct stat st;
	return (stat(path, &st) == 0);
}


/*--------------------------------------------------------
	Find file matching pattern
--------------------------------------------------------*/

char *find_file(char *pattern, char *path)
{
	DIR *dir;
	struct dirent *entry;
	static char found_path[MAX_PATH];

	if (!pattern || !path) return NULL;

	dir = opendir(path);
	if (!dir) return NULL;

	while ((entry = readdir(dir)) != NULL)
	{
		// Simple pattern matching - TODO: implement proper wildcards
		if (strstr(entry->d_name, pattern))
		{
			snprintf(found_path, MAX_PATH, "%s/%s", path, entry->d_name);
			closedir(dir);
			return found_path;
		}
	}

	closedir(dir);
	return NULL;
}


/*--------------------------------------------------------
	Delete files matching pattern
--------------------------------------------------------*/

void delete_files(const char *dirname, const char *pattern)
{
	DIR *dir;
	struct dirent *entry;
	char filepath[MAX_PATH];

	if (!dirname || !pattern) return;

	dir = opendir(dirname);
	if (!dir) return;

	while ((entry = readdir(dir)) != NULL)
	{
		if (strstr(entry->d_name, pattern))
		{
			snprintf(filepath, MAX_PATH, "%s/%s", dirname, entry->d_name);
			unlink(filepath);
		}
	}

	closedir(dir);
}


#ifdef SAVE_STATE
/*--------------------------------------------------------
	Find save state files
--------------------------------------------------------*/

void find_state_file(UINT8 *slot)
{
	// TODO: Implement save state file discovery
	(void)slot;
}
#endif


/*--------------------------------------------------------
	Show exit screen
--------------------------------------------------------*/

void show_exit_screen(void)
{
	// TODO: Show goodbye message
	printf("Exiting...\n");
}


/*--------------------------------------------------------
	File browser main function
--------------------------------------------------------*/

void file_browser(void)
{
	// TODO: Implement full file browser UI
	// For now, this is a stub

	printf("File browser: Not yet implemented\n");
	printf("Please specify ROM path via command line\n");

	Loop = LOOP_EXIT;
}
