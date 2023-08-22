#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "fb.h"
#include "input.h"
#include "rc.h"
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <segui.h>
#include <unistd.h>
#include <user_syscall.h>

struct fb fb;

static window_t* win;

static int vmode[3] = { 0, 0, 32 };

rcvar_t vid_exports[] =
{
	RCV_VECTOR("vmode", &vmode, 3, "video mode: w h bpp"),
	RCV_END
};

extern int keymap[][2];

static int mapscancode(int scan)
{
	int i;
	for (i = 0; keymap[i][0]; i++)
		if (keymap[i][0] == scan)
			return keymap[i][1];
	return 0;
}

void handle_keys(char c, int isCTRL, int isALT, uint8_t keycode)
{
    event_t e;
    if(keycode < 0x80)
    {
        e.type = EV_PRESS;
        e.code = mapscancode(keycode);
    }
    else
    {
        e.type = EV_RELEASE;
        e.code = mapscancode((keycode - 0x80));
    }

    ev_postevent(&e);
}

void sys_sanitize(char *s)
{
}


void sys_checkdir(char *path, int wr)
{
}

void sys_initpath(char *exe)
{
	char *buf, *home, *p;

	home = strdup(exe);
	p = strrchr(home, '/');
	if (p) *p = 0;
	else
	{
		buf = ".";
		rc_setvar("rcpath", 1, &buf);
		rc_setvar("savedir", 1, &buf);
		return;
	}
	buf = malloc(strlen(home) + 8);
	sprintf(buf, ".;%s/", home);
	rc_setvar("rcpath", 1, &buf);
	sprintf(buf, ".");
	rc_setvar("savedir", 1, &buf);
	free(buf);
}

int overticks = 0;
void sys_sleep(uint32_t ms)
{
	if(ms > 20000)
		overticks++;
	if(overticks > 3)
		SYSCALL_SLEEP(abs(ms)/1000);
}

void *sys_timer()
{
	struct timeval *tv;
	
	tv = malloc(sizeof(struct timeval));
	gettimeofday(tv, NULL);
	return tv;
}

#pragma GCC push_options
#pragma GCC optimize ("O0")
int sys_elapsed(struct timeval *prev)
{
	struct timeval tv;
	int secs, usecs;
	
	gettimeofday(&tv, NULL);
	secs = tv.tv_sec - prev->tv_sec;
	usecs = tv.tv_usec - prev->tv_usec;
	*prev = tv;
	if (!secs) return usecs;
	return 1000000 + usecs;
}
#pragma GCC pop_options

void vid_begin()
{
	window_display(win, fb.ptr);
}

void vid_init()
{
	window_info_t* info = malloc(sizeof(window_info_t));
	memset(info, 0, sizeof(window_info_t));

	int scale = rc_getint("scale");

	if (!vmode[0] || !vmode[1])
	{
		if (scale < 1) scale = 1;
		vmode[0] = 160 * scale;
		vmode[1] = 144 * scale;
	}

	info->width = vmode[0];
	info->height = vmode[1];
	info->bpp = 32;
	info->x = 160;
	info->y = 144;
	strcpy(info->title, "SEGnuboy");
	info->fb_ptr = fb.ptr = malloc(32*144*160);
	win = create_window(info);

    fb.delegate_scaling = scale;
	fb.w = win->width;
	fb.h = win->height;
	fb.pelsize = 4;
	fb.dirty = 0;
	fb.pitch = fb.w*fb.pelsize;
	fb.indexed = 0;

    fb.cc[0].r=0;
    fb.cc[0].l=16;
    fb.cc[1].r=0;
    fb.cc[1].l=8;
    fb.cc[2].r=0;
    fb.cc[2].l=0;
    fb.cc[3].r=0;
    fb.cc[3].l=24;

    fb.enabled = 1;

    SYSCALL_ATTACH(handle_keys);
}

void vid_preinit()
{
}

void vid_setpal(int i, int r, int g, int b)
{
	printf("I: 0x%6x, R: 0x%6x, G: 0x%6x, B: 0x%6x\n", i, r, g, b);
}

void vid_close()
{
}

void vid_settitle(char *title)
{
    window_change_title(win, title);
}

void vid_end()
{
}

void ev_poll(int wait)
{
}
