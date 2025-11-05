/******************************************************************************

	input.h

	SDL2 input subsystem

******************************************************************************/

#ifndef SDL2_INPUT_H
#define SDL2_INPUT_H

#include <SDL2/SDL.h>

// Button definitions - matching PSP controller layout
#define PSP_CTRL_SELECT		0x00000001
#define PSP_CTRL_START		0x00000008
#define PSP_CTRL_UP			0x00000010
#define PSP_CTRL_RIGHT		0x00000020
#define PSP_CTRL_DOWN		0x00000040
#define PSP_CTRL_LEFT		0x00000080
#define PSP_CTRL_LTRIGGER	0x00000100
#define PSP_CTRL_RTRIGGER	0x00000200
#define PSP_CTRL_TRIANGLE	0x00001000
#define PSP_CTRL_CIRCLE		0x00002000
#define PSP_CTRL_CROSS		0x00004000
#define PSP_CTRL_SQUARE		0x00008000
#define PSP_CTRL_HOME		0x00010000

#define PSP_CTRL_ANY		\
	(PSP_CTRL_SELECT |		\
	 PSP_CTRL_START |		\
	 PSP_CTRL_UP |			\
	 PSP_CTRL_RIGHT |		\
	 PSP_CTRL_DOWN |		\
	 PSP_CTRL_LEFT |		\
	 PSP_CTRL_LTRIGGER |	\
	 PSP_CTRL_RTRIGGER |	\
	 PSP_CTRL_TRIANGLE |	\
	 PSP_CTRL_CIRCLE |		\
	 PSP_CTRL_CROSS |		\
	 PSP_CTRL_SQUARE)

#define PAD_WAIT_INFINITY	-1

// Input functions
void pad_init(void);
UINT32 poll_gamepad(void);
#if (EMU_SYSTEM == MVS)
UINT32 poll_gamepad_fatfursp(void);
UINT32 poll_gamepad_analog(void);
#endif
void pad_update(void);
int pad_pressed(UINT32 code);
int pad_pressed_any(UINT32 disable_code);
void pad_wait_clear(void);
void pad_wait_press(int msec);

// SDL2 specific
extern SDL_GameController *sdl_controller;
extern SDL_Joystick *sdl_joystick;

#endif /* SDL2_INPUT_H */
