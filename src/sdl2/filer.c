/******************************************************************************

	filer.c

	SDL2 file browser - simplified implementation

******************************************************************************/

#include "sdl2.h"
#include "emumain.h"
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>

/******************************************************************************
	Local structures
******************************************************************************/

#define MAX_ROM_LIST 256

typedef struct {
	char name[MAX_PATH];
	char display_name[64];
} rom_entry_t;

/******************************************************************************
	Global variables
******************************************************************************/

char startupDir[MAX_PATH];
static char roms_dir[MAX_PATH];
static rom_entry_t rom_list[MAX_ROM_LIST];
static int rom_count = 0;

// Extern from emumain.c
extern char game_name[16];
extern void emu_main(void);

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
		// Simple pattern matching
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
	printf("\n");
	printf("=====================================\n");
	printf("    NJEMU - Exiting\n");
	printf("    Thank you for playing!\n");
	printf("=====================================\n");
	printf("\n");
}


/*--------------------------------------------------------
	Scan ROM directory
--------------------------------------------------------*/

static int scan_roms_directory(void)
{
	DIR *dir;
	struct dirent *entry;
	char *ext;

	rom_count = 0;

	printf("Scanning ROMs directory: %s\n", roms_dir);

	dir = opendir(roms_dir);
	if (!dir)
	{
		printf("ERROR: Cannot open ROMs directory: %s\n", roms_dir);
		return 0;
	}

	while ((entry = readdir(dir)) != NULL && rom_count < MAX_ROM_LIST)
	{
		// Skip directories and hidden files
		if (entry->d_name[0] == '.') continue;

		// Check for .zip extension
		ext = strrchr(entry->d_name, '.');
		if (!ext || strcasecmp(ext, ".zip") != 0) continue;

		// Add to list
		snprintf(rom_list[rom_count].name, MAX_PATH, "%s/%s", roms_dir, entry->d_name);
		strncpy(rom_list[rom_count].display_name, entry->d_name, sizeof(rom_list[rom_count].display_name) - 1);
		rom_count++;
	}

	closedir(dir);

	printf("Found %d ROM files\n", rom_count);
	return rom_count;
}


/*--------------------------------------------------------
	Show ROM list and get selection
--------------------------------------------------------*/

static int show_rom_menu(void)
{
	int i, selection = -1;
	char input[16];

	if (rom_count == 0)
	{
		printf("\nNo ROMs found in %s\n", roms_dir);
		printf("Please place .zip ROM files in the roms directory\n");
		return -1;
	}

	while (1)
	{
		printf("\n");
		printf("========================================\n");
		printf("  NJEMU - ROM Selection Menu\n");
		printf("========================================\n");
		printf("\n");

		for (i = 0; i < rom_count && i < 20; i++)
		{
			printf("  %2d. %s\n", i + 1, rom_list[i].display_name);
		}

		if (rom_count > 20)
		{
			printf("  ... and %d more\n", rom_count - 20);
		}

		printf("\n");
		printf("  Q. Quit\n");
		printf("\n");
		printf("========================================\n");
		printf("Select ROM number (1-%d) or Q to quit: ", rom_count);

		if (fgets(input, sizeof(input), stdin) == NULL)
			break;

		// Remove newline
		input[strcspn(input, "\n")] = 0;

		// Check for quit
		if (toupper(input[0]) == 'Q')
		{
			return -1;
		}

		// Try to parse number
		selection = atoi(input);
		if (selection >= 1 && selection <= rom_count)
		{
			return selection - 1;
		}

		printf("Invalid selection. Please try again.\n");
	}

	return -1;
}


/*--------------------------------------------------------
	Extract game name from ROM filename
--------------------------------------------------------*/

static void extract_game_name(const char *filename, char *output)
{
	char temp[MAX_PATH];
	char *basename, *dot;
	int i;

	// Get basename
	basename = strrchr(filename, '/');
	if (basename)
		basename++;
	else
		basename = (char *)filename;

	strncpy(temp, basename, sizeof(temp) - 1);
	temp[sizeof(temp) - 1] = '\0';

	// Remove extension
	dot = strrchr(temp, '.');
	if (dot) *dot = '\0';

	// Convert to lowercase
	for (i = 0; temp[i]; i++)
	{
		temp[i] = tolower(temp[i]);
	}

	// Copy to output (max 15 chars + null terminator)
	strncpy(output, temp, 15);
	output[15] = '\0';

	printf("Game name: %s\n", output);
}


/*--------------------------------------------------------
	File browser main function
--------------------------------------------------------*/

void file_browser(void)
{
	int selection;

	printf("\n");
	printf("=========================================\n");
	printf("  NJEMU - CPS2 Emulator for Linux/SDL2\n");
	printf("=========================================\n");
	printf("\n");

	// Set up ROMs directory
    snprintf(roms_dir, MAX_PATH, "%s/roms", launchDir);
    // Ensure bounded copy into startupDir
    strncpy(startupDir, roms_dir, sizeof(startupDir) - 1);
    startupDir[sizeof(startupDir) - 1] = '\0';

	// Create roms directory if it doesn't exist
	mkdir(roms_dir, 0755);

	// Load settings (currently just creates config dir)
	load_settings();

	// Scan for ROMs
	if (!scan_roms_directory())
	{
		printf("\nNo ROMs found. Place .zip ROM files in: %s\n", roms_dir);
		printf("Press Enter to exit...\n");
		getchar();
		Loop = LOOP_EXIT;
		return;
	}

	// Main menu loop
	while (1)
	{
		selection = show_rom_menu();

		if (selection < 0)
		{
			// User quit
			Loop = LOOP_EXIT;
			break;
		}

		// Set game name for emulator
		extract_game_name(rom_list[selection].name, game_name);

        // Set game directory for ROM loading (bounded copy)
        strncpy(game_dir, roms_dir, MAX_PATH - 1);
        game_dir[MAX_PATH - 1] = '\0';

		printf("\n");
		printf("========================================\n");
		printf("Starting emulation: %s\n", rom_list[selection].display_name);
		printf("========================================\n");
		printf("\n");

		// Run the emulator
		Loop = LOOP_EXEC;
		emu_main();

		// Check loop status after emulation
		if (Loop == LOOP_EXIT)
		{
			break;
		}
		else if (Loop == LOOP_BROWSER)
		{
			// Return to menu
			printf("\nReturned to browser\n");
			continue;
		}
	}

	show_exit_screen();
}
