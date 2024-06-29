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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL.h>

#include "gp32_functions.h"

#if !defined(LITTLE_ENDIAN) && !defined(BIG_ENDIAN)

#if defined(__i386__) || defined(__ia64__) || defined(WIN32) || \
    (defined(__alpha__) || defined(__alpha)) || \
     defined(__arm__) || \
    (defined(__mips__) && defined(__MIPSEL__)) || \
     defined(__SYMBIAN32__) || \
     defined(__x86_64__) || \
     defined(__LITTLE_ENDIAN__)
	
/* little endian */
#define LITTLE_ENDIAN

#else
/* big endian */	
#define BIG_ENDIAN

#endif	/* endian check */
#endif	/* !defined(LITTLE_ENDIAN) && !defined(BIG_ENDIAN) */


SDL_Surface *screen;
unsigned short *GpScreen = NULL;

int scale;
unsigned int gp_palette[256];

unsigned int keys[512];
extern int quit;

int cur_bpp;
int fullscreen = 0;
int double_buffer = 0;

void KeyCommands()
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		switch (event.type) 
		{
		case SDL_QUIT:
			quit=1;
			break;

		case SDL_KEYDOWN:
			keys[event.key.keysym.sym]=1;
			if (event.key.keysym.sym==SDLK_ESCAPE) quit=1;
			break;

		case SDL_KEYUP:
			keys[event.key.keysym.sym]=0;
			break;

			default:
				break;
		}
	}
}


int GpKeyGet()
{
    KeyCommands();

    int buttons;
    buttons =   keys[SDLK_l] * GPC_VK_FL | 
                keys[SDLK_r] * GPC_VK_FR | 
                keys[SDLK_s] * GPC_VK_START | 
                keys[SDLK_d] * GPC_VK_SELECT | 
                keys[SDLK_a] * GPC_VK_FA | 
                keys[SDLK_b] * GPC_VK_FB | 
                (keys[SDLK_UP] | keys[SDLK_KP8]) * GPC_VK_UP | 
                (keys[SDLK_DOWN] | keys[SDLK_KP2])  * GPC_VK_DOWN | 
                (keys[SDLK_RIGHT] | keys[SDLK_KP6])  * GPC_VK_RIGHT | 
                (keys[SDLK_LEFT] | keys[SDLK_KP4])  * GPC_VK_LEFT;

    return buttons;
}


int GpGraphicModeSet(int gd_bpp)
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	free(GpScreen);

	screen = SDL_SetVideoMode(LCD_WIDTH << scale, LCD_HEIGHT << scale, gd_bpp, SDL_HWSURFACE | fullscreen | double_buffer);
	if ( screen == NULL ) {
		fprintf(stderr, "Unable to set video mode: %s\n", SDL_GetError());
		return(1);
	}
	atexit(SDL_Quit);

	GpScreen = (unsigned short*)malloc(sizeof(*GpScreen) * (LCD_WIDTH * LCD_HEIGHT));
	cur_bpp = gd_bpp;

	return 0;
}


int GpLcdSurfaceGet(GPDRAWSURFACE * ptgpds)
{
	ptgpds->bpp = cur_bpp;
	ptgpds->buf_w = LCD_WIDTH;
	ptgpds->buf_h = LCD_HEIGHT;
	ptgpds->ox = 0;
	ptgpds->oy = 0;
	ptgpds->ptbuffer = (unsigned char*)GpScreen;
	return 0;
}

#if defined(LITTLE_ENDIAN)
#define RSH		19
#define GSH		11
#define BSH		3
#elif defined(BIG_ENDIAN)
#define RSH		3
#define GSH		11
#define BSH		19
#else
#error "failed to detect byteorder, define LITTLE_ENDIAN or BIG_ENDIAN"
#endif

void GP32toPC(unsigned short *gp32v16, int bpp, SDL_Surface *screen)
{
    if (bpp != 16)
        return;

    int x, y;
    unsigned short c;
    unsigned short *vram;
    vram=(unsigned short*)screen->pixels;
    const size_t lcd_real_width = screen->pitch / screen->format->BytesPerPixel;
    unsigned short *gp32vram = (unsigned short*)gp32v16;

    const int native565 =
    	(screen->format->Rmask == 0xF800)
    	&& (screen->format->Gmask == 0x07E0)
    	&& (screen->format->Bmask == 0x001F);

    switch(scale)
	{
	case 0:
            vram += (LCD_HEIGHT - 1) * lcd_real_width;
            for (x=0; x<LCD_WIDTH; x++)
            {
                unsigned short *y_vram = vram++;
                for (y=LCD_HEIGHT - 1; y>=0; y--)
                {
                    *y_vram = native565 ? *gp32vram++ : __builtin_bswap16(*gp32vram++);
                    y_vram-=lcd_real_width;
                }
            }
            break;

	case 1:
            vram += (LCD_HEIGHT - 1) * 2 * lcd_real_width;
            for (x=0; x<LCD_WIDTH; x++)
            {
                uint32_t *y_vram = (uint32_t*)vram;
                vram += 2;
                for (y=LCD_HEIGHT - 1; y>=0; y--)
                {
                    c = native565 ? *gp32vram++ : __builtin_bswap16(*gp32vram++);
                    uint32_t c32 = (c<<16) | c;

                    *y_vram = c32;
                    *(y_vram + lcd_real_width/2) = c32;

                    y_vram-=lcd_real_width;
                }
            }
            break;

        default:
            break;
    }    
}


void GpSurfaceFlip(GPDRAWSURFACE* ptgpds)
{
	if (SDL_MUSTLOCK(screen))
		if (SDL_LockSurface(screen) < 0)
			return;

	GP32toPC((unsigned short*)ptgpds->ptbuffer, ptgpds->bpp, screen);

	if (SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);

	SDL_Flip(screen);
}


GP_HPALETTE GpPaletteSelect(GP_HPALETTE h_new)
{
    int i;
    unsigned int r, g, b;
    unsigned int c;
    for (i=0; i<256; i++)
    {
        c = h_new[i];
        r = ((c >> 11) & 31)<<RSH;
        g = ((c >> 6) & 31)<<GSH;
        b = ((c >> 1) & 31)<<BSH;
        gp_palette[i] = r | g | b;
    }    
	return 0;
}

GP_HPALETTE GpPaletteCreate (int entry_num, GP_PALETTEENTRY * pal_entry)
{
    GP_HPALETTE pal = (GP_HPALETTE)malloc(sizeof(GP_HPALETTE) * 256);

    int i;
    for (i=0; i<entry_num; i++) {
		pal[i] = pal_entry[i];
	}
        
    return pal;
}

unsigned int GpPaletteRealize (void){ return 0; }


unsigned int GpTickCountGet()
{
    return SDL_GetTicks();
}        

