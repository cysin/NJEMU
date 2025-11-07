/******************************************************************************

	ui_text.c

	SDL2 UI text - simple English text array

******************************************************************************/

#include "sdl2.h"

// UI text array - simplified version for SDL2
const char *ui_text[UI_TEXT_MAX];

// Simple English text (stub - minimal entries for linking)
static const char *text_ENGLISH[] =
{
	"",           // Empty string placeholders
	"\n",
	"Please wait...",
	"Could not open file",
	"Error",
	"OK",
	"Cancel",
	"Loading...",
	"Decoding graphics...",
	"Start emulation",
	"Exit emulation",
};

void ui_text_init(void)
{
	int i;
	int max_entries = sizeof(text_ENGLISH) / sizeof(text_ENGLISH[0]);

	// Initialize all entries to empty string
	for (i = 0; i < UI_TEXT_MAX; i++)
	{
		if (i < max_entries && text_ENGLISH[i])
			ui_text[i] = text_ENGLISH[i];
		else
			ui_text[i] = "";
	}
}
