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
#include "gpdef.h"

#define LCD_WIDTH 320
#define LCD_HEIGHT 240

typedef struct tagGPDRAWSURFACE{
	unsigned char * ptbuffer;
	int bpp;	/*reserved*/
	int buf_w;
	int buf_h;
	int ox;
	int oy;
	unsigned char * o_buffer;
} GPDRAWSURFACE;

int GpLcdSurfaceGet(GPDRAWSURFACE * ptgpds);

int GpGraphicModeSet(int gd_bpp);
void GpSurfaceFlip(GPDRAWSURFACE * ptgpds);


typedef unsigned int* GP_HPALETTE;
typedef unsigned short GP_PALETTEENTRY;

GP_HPALETTE GpPaletteCreate (int entry_num, GP_PALETTEENTRY * pal_entry);
GP_HPALETTE GpPaletteSelect(GP_HPALETTE h_new);
unsigned int GpPaletteRealize (void);


void GpTextOut();
void GpTextOut16();

unsigned int GpTickCountGet();

void GpKeyInit();
int GpKeyGet();

void gp_mem_func();

