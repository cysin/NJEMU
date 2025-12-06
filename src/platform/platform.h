/******************************************************************************

    platform.h

    SDL/desktop scaffolding to replace PSP headers for initial porting.

******************************************************************************/

#ifndef PLATFORM_H
#define PLATFORM_H

#define PLATFORM_SDL 1

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <malloc.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <strings.h>
#include <stddef.h>

#include "emucfg.h"
#include "include/osd_cpu.h"

#ifndef MAX_PATH
#define MAX_PATH 512
#endif

#ifndef TICKS_PER_SEC
#define TICKS_PER_SEC 1000000
#endif

/* Screen constants (mirror PSP defaults) */
#define SCR_WIDTH   480
#define SCR_HEIGHT  272
#define BUF_WIDTH   512
#define FRAMESIZE   (BUF_WIDTH * SCR_HEIGHT * sizeof(UINT16))
#define FRAMESIZE32 (BUF_WIDTH * SCR_HEIGHT * sizeof(UINT32))

/* UI placeholders */
#define TEXT(id) ""
#define FONTSIZE 14
#define UI_COLOR(x) (0)
enum { UI_PAL_TITLE, UI_PAL_SELECT, UI_PAL_NORMAL, UI_PAL_INFO, UI_PAL_WARNING, UI_PAL_BG1, UI_PAL_BG2, UI_PAL_FRAME, UI_PAL_FILESEL1, UI_PAL_FILESEL2, UI_PAL_MAX };
enum { ICON_SYSTEM = 0, WP_LOGO = 0 };
#define LANG_ENGLISH             0
#define LANG_JAPANESE            1
#define LANG_CHINESE_SIMPLIFIED  2
#define LANG_CHINESE_TRADITIONAL 3
#define COLOR_BLACK 0, 0, 0
#define COLOR_WHITE 255, 255, 255
extern UINT16 make_col_15(int r, int g, int b);
#define MAKECOL15(r, g, b) make_col_15(r, g, b)

/* Video helper macros/structs mirrored from psp/video.h */
#define SWIZZLED_8x8(tex, idx)        &(tex[(idx) << 6])
#define SWIZZLED_16x16(tex, idx)      &(tex[((idx & ~31) << 8) | ((idx & 31) << 7)])
#define SWIZZLED_32x32(tex, idx)      &(tex[((idx & ~15) << 10) | ((idx & 15) << 8)])
#define NONE_SWIZZLED_8x8(tex, idx)   &(tex[((idx & ~63) << 6) | ((idx & 63) << 3)])
#define NONE_SWIZZLED_16x16(tex, idx) &(tex[((idx & ~31) << 8) | ((idx & 31) << 4)])
#define NONE_SWIZZLED_32x32(tex, idx) &(tex[((idx & ~15) << 10) | ((idx & 15) << 5)])
#define SWIZZLED8_8x8(tex, idx)       ((UINT8 *)SWIZZLED_8x8(tex, idx))
#define SWIZZLED8_16x16(tex, idx)     ((UINT8 *)SWIZZLED_16x16(tex, idx))
#define SWIZZLED8_32x32(tex, idx)     ((UINT8 *)SWIZZLED_32x32(tex, idx))

struct Vertex
{
    UINT16 u, v;
    UINT16 color;
    INT16 x, y, z;
};

typedef struct rectangle
{
    INT16 left;
    INT16 top;
    INT16 right;
    INT16 bottom;
} RECT;

/* PSP-style button masks */
#define PSP_CTRL_SELECT    0x00000001
#define PSP_CTRL_START     0x00000008
#define PSP_CTRL_UP        0x00000010
#define PSP_CTRL_RIGHT     0x00000020
#define PSP_CTRL_DOWN      0x00000040
#define PSP_CTRL_LEFT      0x00000080
#define PSP_CTRL_LTRIGGER  0x00000100
#define PSP_CTRL_RTRIGGER  0x00000200
#define PSP_CTRL_TRIANGLE  0x00001000
#define PSP_CTRL_CIRCLE    0x00002000
#define PSP_CTRL_CROSS     0x00004000
#define PSP_CTRL_SQUARE    0x00008000
#define PSP_CTRL_ANY       (PSP_CTRL_SELECT|PSP_CTRL_START|PSP_CTRL_UP|PSP_CTRL_RIGHT|PSP_CTRL_DOWN|PSP_CTRL_LEFT|PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|PSP_CTRL_TRIANGLE|PSP_CTRL_CIRCLE|PSP_CTRL_CROSS|PSP_CTRL_SQUARE)
#define PAD_WAIT_INFINITY  (-1)

/* PSP I/O compatibility */
typedef int SceUID;
typedef int SceSize;
#define PSP_O_RDONLY O_RDONLY
#define PSP_O_WRONLY O_WRONLY
#define PSP_O_CREAT  O_CREAT
#define PSP_O_TRUNC  O_TRUNC
#define PSP_SEEK_SET SEEK_SET
#define PSP_SEEK_CUR SEEK_CUR
#define PSP_SEEK_END SEEK_END

#ifndef HAVE_SWAB
static inline void swab(const void *src, void *dst, ssize_t n)
{
	const unsigned char *s = (const unsigned char *)src;
	unsigned char *d = (unsigned char *)dst;
	for (ssize_t i = 0; i + 1 < n; i += 2)
	{
		d[i] = s[i + 1];
		d[i + 1] = s[i];
	}
}
#endif

/* PSP GU compatibility (no-op stubs for now) */
#define GU_PSM_5551 0
#define GU_PSM_T8 0
#define GU_TEXTURE_16BIT 0
#define GU_COLOR_5551 0
#define GU_VERTEX_16BIT 0
#define GU_TRANSFORM_2D 0
#define GU_COLOR_8888 0
#define GU_PSM_8888 0
#define GU_TRUE 1
#define GU_FALSE 0
#define GU_SYNC_FINISH 0
#define GU_DIRECT 0
#define GU_COLOR_BUFFER_BIT 0
#define GU_DEPTH_BUFFER_BIT 0
#define GU_FAST_CLEAR_BIT 0
#define GU_SPRITES 0
#define GU_POINTS 0
#define GU_ALPHA_TEST 0
#define GU_BLEND 0
#define GU_FOG 0
#define GU_SCISSOR_TEST 0
#define GU_TEXTURE_2D 0
#define GU_COLOR_5650 0

static inline void *sceGuGetMemory(size_t sz) { return malloc(sz); }
#define sceGuStart(...)
#define sceGuDrawBufferList(...)
#define sceGuDepthBuffer(...)
#define sceGuScissor(...)
#define sceGuEnable(...)
#define sceGuClear(...)
#define sceGuTexMode(...)
#define sceGuTexImage(...)
#define sceGuClutLoad(...)
#define sceGuFinish(...)
#define sceGuSync(...)
#define sceGuDisable(...)
#define sceGuDepthMask(...)
#define sceGuDrawArray(...)
#define sceGuClearColor(...)
#define sceGuTexFilter(...)

/* Global state mirrors */
enum { LOOP_EXIT = 0, LOOP_BROWSER, LOOP_RESTART, LOOP_RESET, LOOP_EXEC };
enum { PSPCLOCK_222 = 0, PSPCLOCK_266, PSPCLOCK_300, PSPCLOCK_333, PSPCLOCK_MAX };

extern volatile int Loop;
extern volatile int Sleep;
extern char launchDir[MAX_PATH];
extern char screenshotDir[MAX_PATH];
extern int psp_cpuclock;
extern int devkit_version;
extern int systembuttons_available;
extern int njemu_debug;

void set_cpu_clock(int value);

/* Timing */
typedef UINT64 TICKER;
void platform_timing_init(void);
TICKER ticker(void);
void sceKernelDelayThread(unsigned int usec);
void platform_wait_until(TICKER target);
static inline void sceDisplayWaitVblankStart(void) { sceKernelDelayThread(16000); }
static inline void video_wait_vsync(void) { sceKernelDelayThread(16000); }
static inline int scePowerIsBatteryCharging(void) { return 1; }
static inline int scePowerGetBatteryLifePercent(void) { return 100; }

/* I/O compatibility */
SceUID sceIoOpen(const char *path, int flags, int mode);
int sceIoClose(SceUID fd);
int sceIoRead(SceUID fd, void *buf, SceSize size);
int sceIoWrite(SceUID fd, const void *buf, SceSize size);
int sceIoLseek(SceUID fd, int offset, int whence);
int sceIoRemove(const char *path);
int sceIoRename(const char *oldpath, const char *newpath);

/* UI / video stubs */
void msg_printf(const char *fmt, ...);
void msg_screen_clear(void);
void msg_screen_init(int background, int icon, const char *title);
void show_background(void);
void load_background(int id);
void video_init(void);
void video_exit(void);
void video_clear_screen(void);
void video_set_mode(int bpp);
void video_flip_screen(int wait_vsync);
void *video_frame_addr(void *frame, int x, int y);
void video_copy_rect(void *src, void *dst, RECT *src_r, RECT *dst_r);
void video_copy_rect_flip(void *src, void *dst, RECT *src_r, RECT *dst_r);
void video_copy_rect_rotate(void *src, void *dst, RECT *src_r, RECT *dst_r);
void video_clear_frame(void *frame);
extern UINT16 *draw_frame;
extern UINT16 *work_frame;
extern UINT16 *show_frame;
extern UINT16 *tex_frame;
void small_font_print(int x, int y, const char *buf, int shadow);
void small_icon_shadow(int x, int y, int pal, int icon);
int uifont_get_string_width(const char *text);
void uifont_print_center(int x, int pal, const char *text);
void uifont_print_shadow(int x, int y, int pal, const char *text);
void uifont_print_shadow_center(int y, int pal, const char *text);
void boxfill_alpha(int sx, int sy, int ex, int ey, int r, int g, int b, int a);
void draw_dialog(int sx, int sy, int ex, int ey);
int draw_volume_status(int force);
int draw_battery_status(int force);
void ui_show_popup(int force);
void ui_popup(const char *fmt, ...);
void ui_popup_reset(void);
int showmenu(void);
int save_png(const char *path);
int ui_text_get_language(void);
int readHomeButton(void);
int commandlist(int show);
void load_gamecfg(const char *name);
void save_gamecfg(const char *name);
void show_exit_screen(void);

/* Input stubs */
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

/* Sound structures/macros (from psp/sound.h) */
#define SOUND_SAMPLES_24000 (400*2)
#define SOUND_SAMPLES_44100 (736*2)
#define SOUND_SAMPLES_48000 (800)

#if (EMU_SYSTEM == CPS2)
#define SOUND_BUFFER_SIZE ((400*2)*2)
#else
#define SOUND_BUFFER_SIZE ((736*2)*2)
#endif

struct sound_t
{
	int stack;
	int channels;
	int frequency;
	int samples;
	void (*update)(INT16 *buffer);
	void (*callback)(INT32 **buffer, int length);
};

#define MAXOUT (+32767)
#define MINOUT (-32768)
#define Limit(val, max, min)           \
{                                      \
	if (val > max) val = max;          \
	else if (val < min) val = min;     \
}

extern struct sound_t *sound;
void sound_thread_init(void);
void sound_thread_exit(void);
void sound_thread_enable(int enable);
void sound_thread_set_volume(void);
int sound_thread_start(void);
void sound_thread_stop(void);
void sound_mute(int mute);

#endif /* PLATFORM_H */
