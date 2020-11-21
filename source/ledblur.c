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
//#include "chn_functions.h"
#include "sound.h"
#undef main

int clockspeed = 66;

unsigned char *framebuffer[2];
GPDRAWSURFACE GP32Surface[2];

int flip=0;
int vsync=0;

//extern unsigned char modfile1[];
//MODPlay mymod;

extern int quit;
extern int partime;

extern int scale;
extern int fullscreen;
extern int play_music;

static void ClearScreen(int page, int c)
{
	unsigned short *vram = (unsigned short*)framebuffer[page];
	int i;

	for (i=0; i<LCD_WIDTH*LCD_HEIGHT; i++)
		*vram++=c;
}


void InitGraphics(int depth)
{
	GpGraphicModeSet(depth, 0);

	GpLcdSurfaceGet(&GP32Surface[0], 0);
	GpLcdSurfaceGet(&GP32Surface[1], 1);
//	GpSurfaceSet(&GP32Surface[0]);

	framebuffer[0] = (unsigned char*)GP32Surface[0].ptbuffer;
	framebuffer[1] = (unsigned char*)GP32Surface[1].ptbuffer;
}


static void InitMusic()
{
    SoundInit();
    PlaySong();
//	MODPlay_Init (&mymod);
//	MODPlay_SetMOD (&mymod, &modfile1);
//	MODPlay_Start (&mymod);
}


static void Init()
{
	InitGraphics(16);
    SDL_WM_SetCaption ("Led Blur PC by Optimus/Mindlapse", 0);
    SDL_ShowCursor(SDL_DISABLE);

	ClearScreen(0, 0x80a0);
	ClearScreen(1, 0x80a0);

    InitFonts();
	precalcs();

	ClearScreen(0, 0);
	ClearScreen(1, 0);

	InitMusic();
    partime = Ticks();	
}

const char *usage_str = "Command line options:\n"
	"-f\tfullscreen\n"
	"-s\tsmall, do not scale to 640x480\n"
	"-m\tno music\n"
	"-h\thelp (this usage info)\n";

int main(int argc, char *argv[])
{
	int i;
	scale = 1;
	fullscreen = 0;
	vsync = 1;
		
	for(i=1; i<argc; i++) {
		if(argv[i][0] == '-' && argv[i][2] == 0) {
			switch(argv[i][1]) {
			case 'f':
				fullscreen = SDL_FULLSCREEN;
				vsync = 0;
				break;

			case 's':
				scale = 0;
				break;

			case 'm':
				play_music = 0;
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
        GpSurfaceFlip(&GP32Surface[flip], vsync);
		flip=(flip+1)&1;
	}
    SoundEnd();
    return 0;
}

