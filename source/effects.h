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
#define pi 3.14151693

#define GP32_Width 320
#define GP32_Height 240
#define Scr_Size GP32_Width*GP32_Height

#define font_width 16
#define font_height 16

void precalcs();
void update_precbar(char *prec_msg, int i, int i_full);

void SetPlasmaPal();

void Polar(unsigned short *vram, unsigned short shade1[], unsigned short shade2[]);
void Rotozoomer(unsigned short *vram, unsigned short shade[], float ra, float zm);
void Juhlia128(unsigned char *vram);
void Juhlia(unsigned short *buffer, unsigned short shade[], float xpf, float ypf);

void Plasma(unsigned short *vram, unsigned short shade[]);
void PlasmaFade(unsigned short *vram, unsigned short shade[], unsigned int area);
void BlobStars3D(unsigned short *vram);
void Blur(unsigned char *buffer, unsigned char *vram);

void LogoDistort(unsigned short *vram, int xp, int yp);
void LogoZoom(unsigned short *vram, int xp, int yp, float zf);
void drawfont16(int xf, int yf, float zf, int ch, unsigned short shade[], unsigned short *vram);
void OpenJLH(unsigned short *vram, int ffp);
void Close(unsigned short *vram, int xclose);

void Water(unsigned short *vram);
void PlasmaFade2(unsigned short *vram, unsigned short shade[], unsigned int area);
