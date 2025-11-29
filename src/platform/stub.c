/******************************************************************************

    stub.c

    Minimal stubs to let the core/CPS1 build on desktop without PSP SDK.

******************************************************************************/

#include "platform/platform.h"
#include "emumain.h"
#include "sound/sndintrf.h"
#include <SDL.h>
#include <stdarg.h>

volatile int Loop = LOOP_BROWSER;
volatile int Sleep = 0;
char launchDir[MAX_PATH] = ".";
char screenshotDir[MAX_PATH] = ".";
int psp_cpuclock = PSPCLOCK_333;
int devkit_version = 0;
int systembuttons_available = 0;
int njemu_debug = 0;

static struct sound_t sound_info = {0};
struct sound_t *sound = &sound_info;
static UINT16 frame_a[BUF_WIDTH * SCR_HEIGHT];
static UINT16 frame_b[BUF_WIDTH * SCR_HEIGHT];
static UINT16 frame_c[BUF_WIDTH * SCR_HEIGHT];
static UINT16 frame_d[BUF_WIDTH * SCR_HEIGHT];
UINT16 *draw_frame = frame_a;
UINT16 *work_frame = frame_c;
UINT16 *show_frame = frame_a;
UINT16 *tex_frame = frame_d;
static double perf_to_usec = 0.0;
static SDL_GameController *controller = NULL;
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;
static int window_scale = 2;
static SDL_AudioDeviceID audio_dev = 0;
static int audio_active = 0;
static int audio_enable = 0;
static float audio_volume = 1.0f;
static INT16 audio_mixbuf[4096 * 2];
static int audio_channels = 2;

/* input state */
static UINT32 key_buttons = 0;
static UINT32 pad_state = 0;
static UINT8 pressed_check = 0;
static UINT8 pressed_count = 0;
static UINT8 pressed_delay = 0;
static TICKER curr_time = 0;
static TICKER prev_time = 0;
static int analog_x = 0x80;
static int analog_y = 0x80;
static int last_analog_x = 0x80;
static int last_analog_y = 0x80;

void set_cpu_clock(int value)
{
	psp_cpuclock = value;
}

/*----------------------------------------------------------------------
    Timing
----------------------------------------------------------------------*/

void platform_timing_init(void)
{
	if (perf_to_usec == 0.0)
	{
		if ((SDL_WasInit(SDL_INIT_TIMER) & SDL_INIT_TIMER) == 0)
		{
			if (SDL_InitSubSystem(SDL_INIT_TIMER) != 0)
				fprintf(stderr, "[timing] SDL_InitSubSystem(SDL_INIT_TIMER) failed: %s\n", SDL_GetError());
		}

		perf_to_usec = 1000000.0 / (double)SDL_GetPerformanceFrequency();
	}
}

TICKER ticker(void)
{
	platform_timing_init();
	return (TICKER)((double)SDL_GetPerformanceCounter() * perf_to_usec);
}

void sceKernelDelayThread(unsigned int usec)
{
	if (usec == 0) return;
	SDL_Delay((usec + 999) / 1000);
}

void platform_wait_until(TICKER target)
{
	TICKER now = ticker();

	if (target <= now)
		return;

	UINT64 diff = target - now;

	if (diff > 2000)
		SDL_Delay((diff - 1000) / 1000);

	while (ticker() < target)
		SDL_Delay(0);
}

/*----------------------------------------------------------------------
    PSP I/O compatibility (thin POSIX wrappers)
----------------------------------------------------------------------*/

SceUID sceIoOpen(const char *path, int flags, int mode)
{
	return open(path, flags, mode);
}

int sceIoClose(SceUID fd)
{
	return close(fd);
}

int sceIoRead(SceUID fd, void *buf, SceSize size)
{
	return (int)read(fd, buf, (size_t)size);
}

int sceIoWrite(SceUID fd, const void *buf, SceSize size)
{
	return (int)write(fd, buf, (size_t)size);
}

int sceIoLseek(SceUID fd, int offset, int whence)
{
	return (int)lseek(fd, offset, whence);
}

int sceIoRemove(const char *path)
{
	return unlink(path);
}

int sceIoRename(const char *oldpath, const char *newpath)
{
	return rename(oldpath, newpath);
}

/*----------------------------------------------------------------------
    UI / video placeholders
----------------------------------------------------------------------*/

static void vlogf(const char *prefix, const char *fmt, va_list ap)
{
	fprintf(stderr, "[%s] ", prefix);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
}

void msg_printf(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vlogf("msg", fmt, ap);
	va_end(ap);
}

void msg_screen_clear(void) { }
void msg_screen_init(int background, int icon, const char *title) { (void)background; (void)icon; (void)title; }
void show_background(void) { }
void load_background(int id) { (void)id; }
static void ensure_video(void)
{
	if (window)
		return;

	int w = SCR_WIDTH * window_scale;
	int h = SCR_HEIGHT * window_scale;

	if (SDL_CreateWindowAndRenderer(w, h, SDL_WINDOW_RESIZABLE, &window, &renderer) != 0)
	{
		fprintf(stderr, "[video] SDL_CreateWindowAndRenderer failed: %s\n", SDL_GetError());
		return;
	}

	SDL_SetWindowTitle(window, "NJEMU (SDL)");
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, BUF_WIDTH, SCR_HEIGHT);
	if (!texture)
		fprintf(stderr, "[video] SDL_CreateTexture failed: %s\n", SDL_GetError());
}

static void audio_callback(void *userdata, Uint8 *stream, int len)
{
	(void)userdata;

	if (!audio_active || !audio_dev)
	{
		memset(stream, 0, len);
		return;
	}

	if (!audio_enable || Sleep)
	{
		memset(stream, 0, len);
		return;
	}

	int16_t *out = (int16_t *)stream;
	int frames = len / (int)(sizeof(int16_t) * audio_channels);
	int offset = 0;

	while (frames > 0)
	{
		int chunk = sound->samples;
		if (chunk > frames) chunk = frames;

		(*sound->update)(audio_mixbuf);

		if (audio_volume < 0.999f || audio_volume > 1.001f)
		{
			for (int i = 0; i < chunk * audio_channels; i++)
			{
				int sample = (int)(audio_mixbuf[i] * audio_volume);
				if (sample > 32767) sample = 32767;
				else if (sample < -32768) sample = -32768;
				out[offset + i] = (int16_t)sample;
			}
		}
		else
		{
			memcpy(&out[offset], audio_mixbuf, chunk * audio_channels * sizeof(int16_t));
		}

		offset += chunk * audio_channels;
		frames -= chunk;
	}
}

void video_init(void)
{
	ensure_video();
	video_clear_screen();
}

void video_exit(void)
{
	if (texture) { SDL_DestroyTexture(texture); texture = NULL; }
	if (renderer) { SDL_DestroyRenderer(renderer); renderer = NULL; }
	if (window) { SDL_DestroyWindow(window); window = NULL; }
}

void video_clear_screen(void)
{
	video_clear_frame(draw_frame);
	video_clear_frame(show_frame);
	video_clear_frame(work_frame);
	video_clear_frame(tex_frame);
}

void video_set_mode(int bpp) { (void)bpp; }

void video_flip_screen(int wait_vsync)
{
	(void)wait_vsync;
	ensure_video();

	UINT16 *display = draw_frame;

	if (renderer && texture)
	{
		SDL_UpdateTexture(texture, NULL, display, BUF_WIDTH * sizeof(UINT16));
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
	}

	if (draw_frame == frame_a)
		draw_frame = frame_b;
	else
		draw_frame = frame_a;

	show_frame = display;
}

void *video_frame_addr(void *frame, int x, int y)
{
	UINT16 *base = (UINT16 *)frame;
	return base + y * BUF_WIDTH + x;
}

static inline int rect_w(const RECT *r) { return r->right - r->left; }
static inline int rect_h(const RECT *r) { return r->bottom - r->top; }

void video_copy_rect(void *src, void *dst, RECT *src_r, RECT *dst_r)
{
	UINT16 *src16 = (UINT16 *)src;
	UINT16 *dst16 = (UINT16 *)dst;
	int sw = rect_w(src_r);
	int sh = rect_h(src_r);
	int dw = rect_w(dst_r);
	int dh = rect_h(dst_r);
	int y;

	if (sw <= 0 || sh <= 0 || dw <= 0 || dh <= 0)
		return;

	for (y = 0; y < dh; y++)
	{
		int sy = src_r->top + (sh * y) / dh;
		if (sy < 0 || sy >= SCR_HEIGHT) continue;
		UINT16 *src_row = src16 + sy * BUF_WIDTH;
		UINT16 *dst_row = dst16 + (dst_r->top + y) * BUF_WIDTH + dst_r->left;

		for (int x = 0; x < dw; x++)
		{
			int sx = src_r->left + (sw * x) / dw;
			if (sx < 0 || sx >= BUF_WIDTH) continue;
			dst_row[x] = src_row[sx];
		}
	}
}

void video_copy_rect_flip(void *src, void *dst, RECT *src_r, RECT *dst_r)
{
	UINT16 *src16 = (UINT16 *)src;
	UINT16 *dst16 = (UINT16 *)dst;
	int sw = rect_w(src_r);
	int sh = rect_h(src_r);
	int dw = rect_w(dst_r);
	int dh = rect_h(dst_r);

	if (sw <= 0 || sh <= 0 || dw <= 0 || dh <= 0)
		return;

	for (int y = 0; y < dh; y++)
	{
		int sy = src_r->top + (sh - 1 - (sh * y) / dh);
		if (sy < 0 || sy >= SCR_HEIGHT) continue;
		UINT16 *src_row = src16 + sy * BUF_WIDTH;
		UINT16 *dst_row = dst16 + (dst_r->top + y) * BUF_WIDTH + dst_r->left;

		for (int x = 0; x < dw; x++)
		{
			int sx = src_r->left + (sw - 1 - (sw * x) / dw);
			if (sx < 0 || sx >= BUF_WIDTH) continue;
			dst_row[x] = src_row[sx];
		}
	}
}

void video_copy_rect_rotate(void *src, void *dst, RECT *src_r, RECT *dst_r)
{
	UINT16 *src16 = (UINT16 *)src;
	UINT16 *dst16 = (UINT16 *)dst;
	int sw = rect_w(src_r);
	int sh = rect_h(src_r);
	int dw = rect_w(dst_r);
	int dh = rect_h(dst_r);

	if (sw <= 0 || sh <= 0 || dw <= 0 || dh <= 0)
		return;

	for (int y = 0; y < dh; y++)
	{
		for (int x = 0; x < dw; x++)
		{
			int sx = src_r->left + (sw - 1 - (sw * y) / dh);
			int sy = src_r->top + (sh * x) / dw;

			if (sx < 0 || sx >= BUF_WIDTH || sy < 0 || sy >= SCR_HEIGHT)
				continue;

			UINT16 *dst_px = dst16 + (dst_r->top + y) * BUF_WIDTH + (dst_r->left + x);
			*dst_px = src16[sy * BUF_WIDTH + sx];
		}
	}
}

void video_clear_frame(void *frame)
{
	memset(frame, 0, FRAMESIZE);
}
void small_font_print(int x, int y, const char *buf, int shadow) { (void)x; (void)y; (void)buf; (void)shadow; }
void small_icon_shadow(int x, int y, int pal, int icon) { (void)x; (void)y; (void)pal; (void)icon; }
int uifont_get_string_width(const char *text) { return text ? (int)strlen(text) * 8 : 0; }
void uifont_print_center(int x, int pal, const char *text) { (void)x; (void)pal; (void)text; }
void uifont_print_shadow(int x, int y, int pal, const char *text) { (void)x; (void)y; (void)pal; (void)text; }
void uifont_print_shadow_center(int y, int pal, const char *text) { (void)y; (void)pal; (void)text; }
void boxfill_alpha(int sx, int sy, int ex, int ey, int r, int g, int b, int a) { (void)sx; (void)sy; (void)ex; (void)ey; (void)r; (void)g; (void)b; (void)a; }
void draw_dialog(int sx, int sy, int ex, int ey) { (void)sx; (void)sy; (void)ex; (void)ey; }
int draw_volume_status(int force) { (void)force; return 0; }
int draw_battery_status(int force) { (void)force; return 0; }
void ui_show_popup(int force) { (void)force; }
void ui_popup(const char *fmt, ...) { (void)fmt; }
void ui_popup_reset(void) { }
int showmenu(void)
{
	msg_printf("Menu: F5 speedlimit %s, F6 vsync %s, F7 stretch=%d, F8 fps %s, +/- volume=%d",
		option_speedlimit ? "on" : "off",
		option_vsync ? "on" : "off",
		option_stretch,
		option_showfps ? "on" : "off",
		option_sound_volume);
	pad_wait_clear();
	return 0;
}
int save_png(const char *path) { (void)path; return 0; }
int ui_text_get_language(void) { return LANG_ENGLISH; }
int readHomeButton(void) { return 0; }
int commandlist(int show) { (void)show; return 0; }
void load_gamecfg(const char *name) { (void)name; }
void save_gamecfg(const char *name) { (void)name; }
void show_exit_screen(void) { }

/*----------------------------------------------------------------------
    Input stubs
----------------------------------------------------------------------*/
static void set_button(UINT32 mask, int pressed)
{
	if (pressed)
		key_buttons |= mask;
	else
		key_buttons &= ~mask;
}

static int axis_to_analog(Sint16 v)
{
	int a = (int)((v + 32768) * 255LL / 65535LL);
	if (a < 0) a = 0;
	if (a > 255) a = 255;
	return a;
}

static void open_controller(void)
{
	if (controller) return;

	if (SDL_NumJoysticks() > 0)
		controller = SDL_GameControllerOpen(0);
}

static void handle_event(const SDL_Event *e)
{
	switch (e->type)
	{
	case SDL_QUIT:
		Loop = LOOP_EXIT;
		break;
	case SDL_KEYDOWN:
	case SDL_KEYUP:
	{
		int down = (e->type == SDL_KEYDOWN);
		switch (e->key.keysym.scancode)
		{
		case SDL_SCANCODE_ESCAPE: if (down) Loop = LOOP_EXIT; break;
		case SDL_SCANCODE_LEFT: set_button(PSP_CTRL_LEFT, down); break;
		case SDL_SCANCODE_RIGHT: set_button(PSP_CTRL_RIGHT, down); break;
		case SDL_SCANCODE_UP: set_button(PSP_CTRL_UP, down); break;
		case SDL_SCANCODE_DOWN: set_button(PSP_CTRL_DOWN, down); break;
		case SDL_SCANCODE_RETURN: set_button(PSP_CTRL_START, down); break;
		case SDL_SCANCODE_BACKSPACE: set_button(PSP_CTRL_SELECT, down); break;
		case SDL_SCANCODE_Z: set_button(PSP_CTRL_CROSS, down); break;
		case SDL_SCANCODE_X: set_button(PSP_CTRL_CIRCLE, down); break;
		case SDL_SCANCODE_A: set_button(PSP_CTRL_SQUARE, down); break;
		case SDL_SCANCODE_S: set_button(PSP_CTRL_TRIANGLE, down); break;
		case SDL_SCANCODE_Q: set_button(PSP_CTRL_LTRIGGER, down); break;
		case SDL_SCANCODE_W: set_button(PSP_CTRL_RTRIGGER, down); break;
		case SDL_SCANCODE_F5:
			if (down) { option_speedlimit = !option_speedlimit; msg_printf("speedlimit %s", option_speedlimit ? "on" : "off"); }
			break;
		case SDL_SCANCODE_F6:
			if (down) { option_vsync = !option_vsync; msg_printf("vsync %s", option_vsync ? "on" : "off"); }
			break;
		case SDL_SCANCODE_F7:
			if (down) { option_stretch = (option_stretch + 1) % 5; msg_printf("stretch mode %d", option_stretch); }
			break;
		case SDL_SCANCODE_F8:
			if (down) { option_showfps = !option_showfps; msg_printf("showfps %s", option_showfps ? "on" : "off"); }
			break;
		case SDL_SCANCODE_EQUALS:
		case SDL_SCANCODE_KP_PLUS:
			if (down && option_sound_volume < 10) { option_sound_volume++; sound_thread_set_volume(); msg_printf("volume %d", option_sound_volume); }
			break;
		case SDL_SCANCODE_MINUS:
		case SDL_SCANCODE_KP_MINUS:
			if (down && option_sound_volume > 0) { option_sound_volume--; sound_thread_set_volume(); msg_printf("volume %d", option_sound_volume); }
			break;
		default: break;
		}
		break;
	}
	case SDL_CONTROLLERBUTTONDOWN:
	case SDL_CONTROLLERBUTTONUP:
	{
		int down = (e->type == SDL_CONTROLLERBUTTONDOWN);
		switch (e->cbutton.button)
		{
		case SDL_CONTROLLER_BUTTON_A: set_button(PSP_CTRL_CROSS, down); break;
		case SDL_CONTROLLER_BUTTON_B: set_button(PSP_CTRL_CIRCLE, down); break;
		case SDL_CONTROLLER_BUTTON_X: set_button(PSP_CTRL_SQUARE, down); break;
		case SDL_CONTROLLER_BUTTON_Y: set_button(PSP_CTRL_TRIANGLE, down); break;
		case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: set_button(PSP_CTRL_LTRIGGER, down); break;
		case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: set_button(PSP_CTRL_RTRIGGER, down); break;
		case SDL_CONTROLLER_BUTTON_START: set_button(PSP_CTRL_START, down); break;
		case SDL_CONTROLLER_BUTTON_BACK: set_button(PSP_CTRL_SELECT, down); break;
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT: set_button(PSP_CTRL_LEFT, down); break;
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: set_button(PSP_CTRL_RIGHT, down); break;
		case SDL_CONTROLLER_BUTTON_DPAD_UP: set_button(PSP_CTRL_UP, down); break;
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN: set_button(PSP_CTRL_DOWN, down); break;
		default: break;
		}
		break;
	}
	case SDL_CONTROLLERAXISMOTION:
		if (e->caxis.axis == SDL_CONTROLLER_AXIS_LEFTX)
			analog_x = axis_to_analog(e->caxis.value);
		else if (e->caxis.axis == SDL_CONTROLLER_AXIS_LEFTY)
			analog_y = axis_to_analog(e->caxis.value);
		break;
	case SDL_CONTROLLERDEVICEADDED:
		open_controller();
		break;
	case SDL_CONTROLLERDEVICEREMOVED:
		if (controller && SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller)) == e->cdevice.which)
		{
			SDL_GameControllerClose(controller);
			controller = NULL;
		}
		break;
	default:
		break;
	}
}

static void pump_events(void)
{
	SDL_Event e;

	while (SDL_PollEvent(&e))
		handle_event(&e);
}

static UINT32 synth_buttons(int fatfursp_mode)
{
	pump_events();

	UINT32 buttons = key_buttons & PSP_CTRL_ANY;
	int ax = analog_x;
	int ay = analog_y;

	/* mirror digital presses into analog extremes */
	if (buttons & PSP_CTRL_LEFT) ax = 0x00;
	else if (buttons & PSP_CTRL_RIGHT) ax = 0xff;
	else ax = 0x80;

	if (buttons & PSP_CTRL_UP) ay = 0x00;
	else if (buttons & PSP_CTRL_DOWN) ay = 0xff;
	else ay = 0x80;

	if (fatfursp_mode)
	{
		if (!(buttons & PSP_CTRL_UP)    && ay <= 0x30) buttons |= PSP_CTRL_UP;
		if (!(buttons & PSP_CTRL_DOWN)  && ay >= 0xd0) buttons |= PSP_CTRL_DOWN;
		if (!(buttons & PSP_CTRL_LEFT)  && ax <= 0x30) buttons |= PSP_CTRL_LEFT;
		if (!(buttons & PSP_CTRL_RIGHT) && ax >= 0xd0) buttons |= PSP_CTRL_RIGHT;
	}
	else
	{
		if (ay >= 0xd0) buttons |= PSP_CTRL_DOWN;
		if (ay <= 0x30) buttons |= PSP_CTRL_UP;
		if (ax <= 0x30) buttons |= PSP_CTRL_LEFT;
		if (ax >= 0xd0) buttons |= PSP_CTRL_RIGHT;
	}

	last_analog_x = ax;
	last_analog_y = ay;

	return buttons;
}

void pad_init(void)
{
	if ((SDL_WasInit(SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER) & (SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER)) != (SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER))
	{
		if (SDL_InitSubSystem(SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER) != 0)
			fprintf(stderr, "[input] SDL_InitSubSystem failed: %s\n", SDL_GetError());
	}

	open_controller();

	key_buttons = 0;
	pad_state = 0;
	pressed_check = 0;
	pressed_count = 0;
	pressed_delay = 0;
	curr_time = prev_time = 0;
	analog_x = analog_y = 0x80;
	last_analog_x = last_analog_y = 0x80;
}

UINT32 poll_gamepad(void)
{
	return synth_buttons(0);
}

#if (EMU_SYSTEM == MVS)
UINT32 poll_gamepad_fatfursp(void)
{
	return synth_buttons(1);
}

UINT32 poll_gamepad_analog(void)
{
	UINT32 buttons = synth_buttons(0) & 0xffff;
	return buttons | ((UINT32)last_analog_x << 16) | ((UINT32)last_analog_y << 24);
}
#endif

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

	pad_state = data;
}

int pad_pressed(UINT32 code) { return (pad_state & code) == code; }
int pad_pressed_any(UINT32 disable_code) { return (pad_state & (PSP_CTRL_ANY ^ disable_code)) != 0; }

void pad_wait_clear(void)
{
	while (poll_gamepad())
	{
		video_wait_vsync();
		if (Loop == LOOP_EXIT) break;
	}

	pad_state = 0;
	pressed_check = 0;
}

void pad_wait_press(int msec)
{
	pad_wait_clear();

	if (msec == PAD_WAIT_INFINITY)
	{
		while (!poll_gamepad())
		{
			video_wait_vsync();
			if (Loop == LOOP_EXIT) break;
		}
	}
	else
	{
		TICKER target = ticker() + msec * (TICKS_PER_SEC / 1000);

		while (ticker() < target)
		{
			video_wait_vsync();
			if (poll_gamepad()) break;
			if (Loop == LOOP_EXIT) break;
		}
	}

	pad_wait_clear();
}

/*----------------------------------------------------------------------
    Sound thread (SDL audio)
----------------------------------------------------------------------*/

void sound_thread_init(void)
{
	audio_active = 0;
	audio_enable = 0;
	audio_volume = 1.0f;
	audio_dev = 0;
}

void sound_thread_exit(void)
{
	sound_thread_stop();
}

void sound_thread_enable(int enable)
{
	audio_enable = enable;
}

void sound_thread_set_volume(void)
{
	float vol = (float)(option_sound_volume * 10) / 100.0f;
	if (vol < 0.0f) vol = 0.0f;
	if (vol > 1.0f) vol = 1.0f;
	audio_volume = vol;
}

int sound_thread_start(void)
{
	SDL_AudioSpec want, have;

	if (audio_dev)
		sound_thread_stop();

	memset(&want, 0, sizeof(want));
	want.freq = sound->frequency;
	want.format = AUDIO_S16SYS;
	want.channels = 2;
	audio_channels = 2;
	want.samples = (Uint16)sound->samples;
	want.callback = audio_callback;

	audio_dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
	if (audio_dev == 0)
	{
		fprintf(stderr, "[audio] SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
		Loop = LOOP_EXIT;
		return 0;
	}

	audio_channels = have.channels;
	sound_thread_set_volume();
	audio_active = 1;
	audio_enable = option_sound_enable;

	SDL_PauseAudioDevice(audio_dev, 0);
	return 1;
}

void sound_thread_stop(void)
{
	if (audio_dev)
	{
		SDL_PauseAudioDevice(audio_dev, 1);
		SDL_CloseAudioDevice(audio_dev);
		audio_dev = 0;
	}
	audio_active = 0;
}
