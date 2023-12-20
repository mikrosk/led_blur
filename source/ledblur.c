/*
This file is part of the ledblur/mindlapse demo.
Copyright (c) 2006 Michael Kargas <optimus6128@yahoo.gr>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
/* UNIX port by John Tsiombikas <nuclear@siggraph.org> */
#include <stdlib.h>
#include <SDL.h>
#include "precalcs.h"
#include "gp32_functions.h"
#include "script.h"
#include "sound.h"
#undef main

int clockspeed = 66;

unsigned char *framebuffer;
GPDRAWSURFACE GP32Surface;

extern int quit;
extern int partime;

extern int scale;
extern int fullscreen;
extern int play_music;
extern int fps_show;
extern int double_buffer;

static void ClearScreen(int c)
{
	unsigned short *vram = (unsigned short*)framebuffer;
	int i;

	for (i=0; i<LCD_WIDTH*LCD_HEIGHT; i++)
		*vram++=c;
}


void InitGraphics(int depth)
{
	GpGraphicModeSet(depth);

	GpLcdSurfaceGet(&GP32Surface);
	framebuffer = (unsigned char*)GP32Surface.ptbuffer;
}


static void InitMusic()
{
    SoundInit();
    PlaySong();
}


static void Init()
{
	InitGraphics(16);
	SDL_WM_SetCaption ("Led Blur PC by Optimus/Mindlapse", 0);
	SDL_ShowCursor(SDL_DISABLE);

	ClearScreen(0x80a0);

	InitFonts();
	precalcs();

	ClearScreen(0);

	InitMusic();
	partime = Ticks();
}

const char *usage_str = "Command line options:\n"
	"-d\tdouble buffering\n"
	"-f\tfullscreen\n"
	"-m\tno music\n"
	"-p\tprint fps\n"
	"-s\tscale to 640x480\n"
	"-h\thelp (this usage info)\n";

int main(int argc, char *argv[])
{
	int i;
	scale = 0;
	fullscreen = 0;
		
	for(i=1; i<argc; i++) {
		if(argv[i][0] == '-' && argv[i][2] == 0) {
			switch(argv[i][1]) {
			case 'f':
				fullscreen = SDL_FULLSCREEN;
				break;

			case 's':
				scale = 1;
				break;

			case 'm':
				play_music = 0;
				break;

			case 'p':
				fps_show = 1;
				break;

			case 'd':
				double_buffer = SDL_DOUBLEBUF;
				break;

			case 'h':
				printf("%s\n", usage_str);
				return 0;

			default:
				fprintf(stderr, "unknown option: %s. Try %s -h for usage help.\n", argv[i], argv[0]);
				return EXIT_FAILURE;
			}
		} else {
			fprintf(stderr, "unexpected argument: %s. Try %s -h for usage help.\n", argv[i], argv[0]);
			return EXIT_FAILURE;
		}
	}
	Init();

	while (!quit)
	{
		Run();
		UpdateSong();
		GpSurfaceFlip(&GP32Surface);
		UpdateSong();
	}
    SoundEnd();
    return 0;
}

