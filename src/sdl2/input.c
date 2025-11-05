/******************************************************************************

	input.c

	SDL2 input subsystem - keyboard and gamepad

******************************************************************************/

#include "sdl2.h"

/******************************************************************************
	Local variables
******************************************************************************/

static UINT32 pad;
static UINT8 pressed_check;
static UINT8 pressed_count;
static UINT8 pressed_delay;
static TICKER curr_time;
static TICKER prev_time;

/******************************************************************************
	Global variables
******************************************************************************/

SDL_GameController *sdl_controller = NULL;
SDL_Joystick *sdl_joystick = NULL;

/******************************************************************************
	Functions
******************************************************************************/

/*--------------------------------------------------------
	Initialize input subsystem
--------------------------------------------------------*/

void pad_init(void)
{
	// Initialize gamepad/joystick subsystem
	SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");

	// Try to open first available game controller
	for (int i = 0; i < SDL_NumJoysticks(); i++)
	{
		if (SDL_IsGameController(i))
		{
			sdl_controller = SDL_GameControllerOpen(i);
			if (sdl_controller)
			{
				printf("Opened game controller: %s\n", SDL_GameControllerName(sdl_controller));
				break;
			}
		}
	}

	// If no controller, try to open joystick
	if (!sdl_controller && SDL_NumJoysticks() > 0)
	{
		sdl_joystick = SDL_JoystickOpen(0);
		if (sdl_joystick)
		{
			printf("Opened joystick: %s\n", SDL_JoystickName(sdl_joystick));
		}
	}

	pad = 0;
	pressed_check = 0;
	pressed_count = 0;
	pressed_delay = 0;
}


/*--------------------------------------------------------
	Poll gamepad/keyboard state
--------------------------------------------------------*/

UINT32 poll_gamepad(void)
{
	UINT32 buttons = 0;
	const Uint8 *keystate = SDL_GetKeyboardState(NULL);

	// Keyboard mapping
	if (keystate[SDL_SCANCODE_RETURN]) buttons |= PSP_CTRL_START;
	if (keystate[SDL_SCANCODE_RSHIFT]) buttons |= PSP_CTRL_SELECT;
	if (keystate[SDL_SCANCODE_UP])     buttons |= PSP_CTRL_UP;
	if (keystate[SDL_SCANCODE_DOWN])   buttons |= PSP_CTRL_DOWN;
	if (keystate[SDL_SCANCODE_LEFT])   buttons |= PSP_CTRL_LEFT;
	if (keystate[SDL_SCANCODE_RIGHT])  buttons |= PSP_CTRL_RIGHT;
	if (keystate[SDL_SCANCODE_A])      buttons |= PSP_CTRL_SQUARE;   // A = Square
	if (keystate[SDL_SCANCODE_S])      buttons |= PSP_CTRL_CROSS;    // S = Cross
	if (keystate[SDL_SCANCODE_Z])      buttons |= PSP_CTRL_TRIANGLE; // Z = Triangle
	if (keystate[SDL_SCANCODE_X])      buttons |= PSP_CTRL_CIRCLE;   // X = Circle
	if (keystate[SDL_SCANCODE_Q])      buttons |= PSP_CTRL_LTRIGGER; // Q = L trigger
	if (keystate[SDL_SCANCODE_W])      buttons |= PSP_CTRL_RTRIGGER; // W = R trigger
	if (keystate[SDL_SCANCODE_ESCAPE]) buttons |= PSP_CTRL_HOME;     // ESC = Home

	// Game controller mapping
	if (sdl_controller)
	{
		if (SDL_GameControllerGetButton(sdl_controller, SDL_CONTROLLER_BUTTON_START))
			buttons |= PSP_CTRL_START;
		if (SDL_GameControllerGetButton(sdl_controller, SDL_CONTROLLER_BUTTON_BACK))
			buttons |= PSP_CTRL_SELECT;
		if (SDL_GameControllerGetButton(sdl_controller, SDL_CONTROLLER_BUTTON_DPAD_UP))
			buttons |= PSP_CTRL_UP;
		if (SDL_GameControllerGetButton(sdl_controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN))
			buttons |= PSP_CTRL_DOWN;
		if (SDL_GameControllerGetButton(sdl_controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT))
			buttons |= PSP_CTRL_LEFT;
		if (SDL_GameControllerGetButton(sdl_controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT))
			buttons |= PSP_CTRL_RIGHT;
		if (SDL_GameControllerGetButton(sdl_controller, SDL_CONTROLLER_BUTTON_X))
			buttons |= PSP_CTRL_SQUARE;
		if (SDL_GameControllerGetButton(sdl_controller, SDL_CONTROLLER_BUTTON_A))
			buttons |= PSP_CTRL_CROSS;
		if (SDL_GameControllerGetButton(sdl_controller, SDL_CONTROLLER_BUTTON_Y))
			buttons |= PSP_CTRL_TRIANGLE;
		if (SDL_GameControllerGetButton(sdl_controller, SDL_CONTROLLER_BUTTON_B))
			buttons |= PSP_CTRL_CIRCLE;
		if (SDL_GameControllerGetButton(sdl_controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER))
			buttons |= PSP_CTRL_LTRIGGER;
		if (SDL_GameControllerGetButton(sdl_controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER))
			buttons |= PSP_CTRL_RTRIGGER;
		if (SDL_GameControllerGetButton(sdl_controller, SDL_CONTROLLER_BUTTON_GUIDE))
			buttons |= PSP_CTRL_HOME;

		// Analog stick to D-pad
		Sint16 axis_x = SDL_GameControllerGetAxis(sdl_controller, SDL_CONTROLLER_AXIS_LEFTX);
		Sint16 axis_y = SDL_GameControllerGetAxis(sdl_controller, SDL_CONTROLLER_AXIS_LEFTY);

		if (axis_y < -16384) buttons |= PSP_CTRL_UP;
		if (axis_y >  16384) buttons |= PSP_CTRL_DOWN;
		if (axis_x < -16384) buttons |= PSP_CTRL_LEFT;
		if (axis_x >  16384) buttons |= PSP_CTRL_RIGHT;
	}

	return buttons;
}


#if (EMU_SYSTEM == MVS)
/*--------------------------------------------------------
	Poll gamepad (MVS / fatfursp mode)
--------------------------------------------------------*/

UINT32 poll_gamepad_fatfursp(void)
{
	// For now, just use the regular polling
	return poll_gamepad();
}


/*--------------------------------------------------------
	Poll gamepad with analog data
--------------------------------------------------------*/

UINT32 poll_gamepad_analog(void)
{
	UINT32 data = poll_gamepad() & 0xffff;

	// Add analog stick data if controller is present
	if (sdl_controller)
	{
		Sint16 axis_x = SDL_GameControllerGetAxis(sdl_controller, SDL_CONTROLLER_AXIS_LEFTX);
		Sint16 axis_y = SDL_GameControllerGetAxis(sdl_controller, SDL_CONTROLLER_AXIS_LEFTY);

		// Convert from -32768..32767 to 0..255 range like PSP
		UINT8 lx = ((axis_x + 32768) >> 8) & 0xff;
		UINT8 ly = ((axis_y + 32768) >> 8) & 0xff;

		data |= lx << 16;
		data |= ly << 24;
	}
	else
	{
		// Default center position
		data |= 0x80 << 16;
		data |= 0x80 << 24;
	}

	return data;
}
#endif


/*--------------------------------------------------------
	Update pad state with repeat logic
--------------------------------------------------------*/

void pad_update(void)
{
	UINT32 data = poll_gamepad();

	if (data)
	{
		if (!pressed_check)
		{
			pressed_check = 1;
			pressed_count = 0;
			pressed_delay = 8;
			prev_time = ticker();
		}
		else
		{
			int count;

			curr_time = ticker();
			count = (int)((curr_time - prev_time) / (TICKS_PER_SEC / 60));
			prev_time = curr_time;

			pressed_count += count;

			if (pressed_count > pressed_delay)
			{
				pressed_count = 0;
				if (pressed_delay > 2) pressed_delay -= 2;
			}
			else data = 0;
		}
	}
	else pressed_check = 0;

	pad = data;
}


/*--------------------------------------------------------
	Check if button is pressed
--------------------------------------------------------*/

int pad_pressed(UINT32 code)
{
	return (pad & code) != 0;
}


/*--------------------------------------------------------
	Check if any button (except disabled ones) is pressed
--------------------------------------------------------*/

int pad_pressed_any(UINT32 disable_code)
{
	return (pad & (PSP_CTRL_ANY ^ disable_code)) != 0;
}


/*--------------------------------------------------------
	Wait until all buttons are released
--------------------------------------------------------*/

void pad_wait_clear(void)
{
	while (poll_gamepad())
	{
		video_wait_vsync();
		if (!Loop) break;
	}

	pad = 0;
	pressed_check = 0;
}


/*--------------------------------------------------------
	Wait for button press (with timeout)
--------------------------------------------------------*/

void pad_wait_press(int msec)
{
	TICKER start_time = ticker();
	int timeout = (msec == PAD_WAIT_INFINITY) ? 0 : (msec * 1000);

	while (!poll_gamepad())
	{
		video_wait_vsync();

		if (!Loop) break;

		if (timeout && (ticker() - start_time > timeout))
			break;
	}

	pad = 0;
	pressed_check = 0;
}
