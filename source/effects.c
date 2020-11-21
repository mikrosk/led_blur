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
#include <math.h>
#include "effects.h"
#include "precalcs.h"
#include "engine3d.h"
#include "string.h"

// Polar/Tunnel variables

extern unsigned short dist_angle[Scr_Size*2];
extern unsigned int fsin4[2048], fsin5[2048];
extern unsigned int fsin6[2048], fsin7[2048], fsin8[2048];
extern unsigned int fsin1[2048], fsin2[2048], fsin3[2048];

// Water variables

extern unsigned char uberclamp[512];
extern unsigned short jlh0[];

unsigned char buffer1[Scr_Size], buffer2[Scr_Size];
unsigned char *ba = buffer1 + GP32_Height, *bb = buffer2 + GP32_Height;

unsigned int wtime = 0;

// ===================

static float ra, zm;

extern int nfrm;
extern int prticks;

extern unsigned char rotbitmap[];
extern unsigned char blob[blob_width * blob_height];
unsigned char blobbuffer[GP32_Width * GP32_Height];

extern unsigned short shades[MAXSHADES][256];

extern float star3dx[nstars3d];
extern float star3dy[nstars3d];
extern float star3dz[nstars3d];
extern float star3dv[nstars3d];
extern int scsize, starfar;
int sx[nstars3d], sy[nstars3d];
float sz[nstars3d];

extern unsigned short mindlapse[];
extern unsigned short presents[];

extern int mo;

#define fp_mul 256
#define fp_shr 8
#define n2 4*fp_mul

extern int dsin1[1024], dsin2[1024];
extern unsigned char dclampx[256], dclampy[128];
extern unsigned char yscan[92];
extern unsigned char xscan[192];

extern unsigned char fonts16[];
extern unsigned char fnt16[32768];

void PlasmaFade(unsigned short *vram, unsigned short shade[], unsigned int area)
{
    unsigned int k = (prticks>>4) % 438;
    unsigned int x, y;
    unsigned char c;
    for (x=0; x<320; x++)
        for (y=0; y<240; y++)
        {
            c = (fsin4[x] + fsin6[y] + fsin8[x+y+k]) & 255;
            if (c<area)
                *vram++ = shade[c];
            else
                *vram++ = shades[16][0];
        }
}

void PlasmaFade2(unsigned short *vram, unsigned short shade[], unsigned int area)
{
    unsigned int k = ((49152+4096 + prticks)>>4) % 438;
    unsigned int x, y;
    unsigned char c;
    for (x=0; x<320; x++)
        for (y=0; y<240; y++)
        {
            c = (fsin4[x] + fsin6[y] + fsin8[x+y+k]) & 255;
            if (c<area)
                *vram++ = shade[c];
            else
                vram++;
        }
}


void Plasma(unsigned short *vram, unsigned short shade[])
{
    unsigned int *vram32 = (unsigned int*)vram;
    unsigned int k = (prticks>>4)%438;
    unsigned int x, y;
    for (x=0; x<320; x++)
        for (y=0; y<240; y+=8)
        {
            *vram32++ = shade[(fsin4[x] + fsin6[y] + fsin8[x+y+k]) & 255] | (shade[(fsin4[x] + fsin6[y+1] + fsin8[x+y+k+1]) & 255]<<16);
            *vram32++ = shade[(fsin4[x] + fsin6[y+2] + fsin8[x+y+k+2]) & 255] | (shade[(fsin4[x] + fsin6[y+3] + fsin8[x+y+k+3]) & 255]<<16);
            *vram32++ = shade[(fsin4[x] + fsin6[y+4] + fsin8[x+y+k+4]) & 255] | (shade[(fsin4[x] + fsin6[y+5] + fsin8[x+y+k+5]) & 255]<<16);
            *vram32++ = shade[(fsin4[x] + fsin6[y+6] + fsin8[x+y+k+6]) & 255] | (shade[(fsin4[x] + fsin6[y+7] + fsin8[x+y+k+7]) & 255]<<16);
        }
}


void LogoZoom(unsigned short *vram, int xp, int yp, float zf)
{
    unsigned int x, y;
    unsigned int c;
    vram += (yp + xp * GP32_Height);

    int u = ((128>>1) * (1 - 1/zf)) * 65536;
    int v;
    int du = 65536/zf;
    int dv = du;

    int zu, zv;    
    for (x=0; x<128; x++)
    {
        v = ((32>>1) * (1 - 1/zf)) * 65536;
        for (y=0; y<32; y++)
        {
            zu = u>>16; zv = v>>16;
            if (zu<0) zu = 0; if (zu>127) zu = 127;
            if (zv<0) zv = 0; if (zv>31) zv = 31;
            c = presents[zu*32 + zv];
            if (c!=0) *(vram+y) = c;
            v+=dv;
        }
        u+=du;
        vram+=GP32_Height;
    }
}

void LogoDistort(unsigned short *vram, int xp, int yp)
{
    int x, y, i;
    unsigned int c;
    vram += (yp + xp * GP32_Height);

    i = 0;
    int xx = 0, yy = 4;
    unsigned int xd, yd, disp;
    unsigned int tm1 = (prticks>>4) % 75, tm2 = (prticks>>4) % 100;
    for (x=0-xx; x<192+xx; x++)
    {
        for (y=0-yy; y<84+yy; y++)
        {
            disp = dclampx[32 + x + dsin1[64+y+tm1]] * 84 + dclampy[22 + y + dsin2[64+x+tm2]];
            c = mindlapse[disp];
            if ((c!=0) && (yscan[y+yy]!=0) && (xscan[x+xx]!=0)) *(vram+y) = c;
        }
        vram+=GP32_Height;
    }
}

void Water(unsigned short *vram)
{
	vram+=GP32_Height;

	int i, x, y;
	int xp, yp = 0;
	unsigned char *b1 = ba, *b2 = bb, *bc;

	unsigned int xd, yd;

	if ((prticks - wtime) >= 128)
	{
		wtime = prticks;

		xd = (rand() % ((unsigned int)(GP32_Width * 0.8))+(unsigned int)(GP32_Width * 0.1)) * GP32_Height;
		yd = rand() % GP32_Height;
		*(b1 + xd + yd) = 255;
		*(b1 + xd + yd - 1) = 192;
		*(b1 + xd + yd + 1) = 192;
		*(b1 + xd + yd - GP32_Height) = 192;
		*(b1 + xd + yd + GP32_Height) = 192;
		*(b1 + xd + yd - GP32_Height - 1) = 64;
		*(b1 + xd + yd - GP32_Height + 1) = 64;
		*(b1 + xd + yd + GP32_Height - 1) = 64;
		*(b1 + xd + yd + GP32_Height + 1) = 64;
	}


/*
    if (prticks<8192)
    {
     	xd = (GP32_Width>>1) + sin(prticks/1024.0) * (GP32_Width/3 - 1);
     	yd = (GP32_Height>>1) + sin(prticks/768.0) * (GP32_Height/3 - 1);
    	*(b1 + xd * GP32_Height + yd) = 128;
    	*(b1 + (xd - 1) * GP32_Height + yd) = 64;
    	*(b1 + (xd + 1) * GP32_Height + yd) = 64;
    	*(b1 + xd * GP32_Height + yd - 1) = 64;
    	*(b1 + xd * GP32_Height + yd + 1) = 64;
    }
*/

	bc = bb; bb = ba; ba = bc;

	unsigned int v0;
    unsigned int *vram32 = (unsigned int*)vram;
    unsigned int *bram32 = (unsigned int*)b2;
    unsigned char c0, c1, c2, c3;

	for (x=1; x<GP32_Width - 1; x++)
	{
        c3 = *(b2-1);
		for (y=0; y<GP32_Height; y+=8)
		{
                c0 = uberclamp[256 + (( *(b1-1) + *(b1+1) + *(b1-GP32_Height) + *(b1+GP32_Height))>>1) - *b2];
//		        *b2 = c0;
		        xp = x + ((c3 - *(b2+1))>>2);
                yp = y + ((*(b2-GP32_Height) - *(b2+GP32_Height))>>2);
		        v0 = jlh0[yp + xp*GP32_Height];

                c1 = uberclamp[256 + (( *b1 + *(b1+2) + *(b1-GP32_Height+1) + *(b1+GP32_Height+1))>>1) - *(b2+1)];
//		        *(b2+1) = c1;
		        xp = x + ((c0 - *(b2+2))>>2);
                yp = y + 1 + ((*(b2-GP32_Height+1) - *(b2+GP32_Height+1))>>2);
		        *vram32 = (jlh0[yp + xp*GP32_Height]<<16) | v0;

                c2 = uberclamp[256 + (( *(b1+1) + *(b1+3) + *(b1-GP32_Height+2) + *(b1+GP32_Height+2))>>1) - *(b2+2)];
//		        *(b2+2) = c2;
		        xp = x + ((c1 - *(b2+3))>>2);
                yp = y + 2 + ((*(b2-GP32_Height+2) - *(b2+GP32_Height+2))>>2);
		        v0 = jlh0[yp + xp*GP32_Height];

                c3 = uberclamp[256 + (( *(b1+2) + *(b1+4) + *(b1-GP32_Height+3) + *(b1+GP32_Height+3))>>1) - *(b2+3)];
//		        *(b2+3) = c3;
		        xp = x + ((c2 - *(b2+4))>>2);
                yp = y + 3 + ((*(b2-GP32_Height+3) - *(b2+GP32_Height+3))>>2);
		        *(vram32+1) = (jlh0[yp + xp*GP32_Height]<<16) | v0;

                *bram32 = (c3<<24) | (c2<<16) | (c1<<8) | c0;

                c0 = uberclamp[256 + (( *(b1+3) + *(b1+5) + *(b1-GP32_Height+4) + *(b1+GP32_Height+4))>>1) - *(b2+4)];
//		        *(b2+4) = c0;
		        xp = x + ((c3 - *(b2+5))>>2);
                yp = y + 4 + ((*(b2-GP32_Height+4) - *(b2+GP32_Height+4))>>2);
		        v0 = jlh0[yp + xp*GP32_Height];

                c1 = uberclamp[256 + (( *(b1+4) + *(b1+6) + *(b1-GP32_Height+5) + *(b1+GP32_Height+5))>>1) - *(b2+5)];
//		        *(b2+5) = c1;
		        xp = x + ((c0 - *(b2+6))>>2);
                yp = y + 5 + ((*(b2-GP32_Height+5) - *(b2+GP32_Height+5))>>2);
		        *(vram32+2) = (jlh0[yp + xp*GP32_Height]<<16) | v0;

                c2 = uberclamp[256 + (( *(b1+5) + *(b1+7) + *(b1-GP32_Height+6) + *(b1+GP32_Height+6))>>1) - *(b2+6)];
//		        *(b2+6) = c2;
		        xp = x + ((c1 - *(b2+7))>>2);
                yp = y + 6 + ((*(b2-GP32_Height+6) - *(b2+GP32_Height+6))>>2);
		        v0 = jlh0[yp + xp*GP32_Height];

                c3 = uberclamp[256 + (( *(b1+6) + *(b1+8) + *(b1-GP32_Height+7) + *(b1+GP32_Height+7))>>1) - *(b2+7)];
//		        *(b2+7) = c3;
		        xp = x + ((c2 - *(b2+8))>>2);
                yp = y + 7 + ((*(b2-GP32_Height+7) - *(b2+GP32_Height+7))>>2);
		        *(vram32+3) = (jlh0[yp + xp*GP32_Height]<<16) | v0;

                *(bram32+1) = (c3<<24) | (c2<<16) | (c1<<8) | c0;

                vram32+=4;
                bram32+=2;
		        b1+=8; b2+=8;
		}
	}

    for (i=0; i<GP32_Height; i++)
    {
        *(ba-i) = 0;
        *(ba+(GP32_Width-2)*GP32_Height-i) = 0;
        *(bb-i) = 0;
        *(bb+(GP32_Width-2)*GP32_Height-i) = 0;
        *(vram-i) = 0;
    }
}



void Juhlia(unsigned short *buffer, unsigned short shade[], float xpf, float ypf)
{
    int xp = xpf * fp_mul;
    int yp = ypf * fp_mul;
	int x, y, n0;

	int x0, y0, x1, y1;
	int xk, yk, yl;
	int mj2;

	unsigned short c;

	int xofs = (GP32_Width - 1) * GP32_Height;
	buffer+=(GP32_Height>>1);

	int di = (int)(0.016f * fp_mul);

	yl = -di * (-(GP32_Height>>1));
	xk = di * (-(GP32_Width>>1));

	for (x=0; x<GP32_Width; x++)
	{
		yk = yl;
		for (y=(GP32_Height>>1) - 1; y>=0; y--)
		{
			x0 = xk;
			y0 = yk;
			n0 = 0;
			
				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) goto end;


				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) {n0 = 1; goto end;}

				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) {n0 = 2; goto end;}
				
				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) {n0 = 3; goto end;}

				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) {n0 = 4; goto end;}

				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) {n0 = 5; goto end;}

				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) {n0 = 6; goto end;}
				
				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) {n0 = 7; goto end;}

				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) {n0 = 8; goto end;}


				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) {n0 = 9; goto end;}

				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) {n0 = 10; goto end;}
				
				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) {n0 = 11; goto end;}

				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) {n0 = 12; goto end;}

				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) {n0 = 13; goto end;}

				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) {n0 = 14; goto end;}
				
				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) {n0 = 15; goto end;}
				
				n0 = 15;
		end:

    	c = n0<<4;
    	*(buffer + y) = shade[c];
    	*(buffer - y - 1 + xofs) = shade[c];

		yk-=di;
		}
    	buffer+=GP32_Height;
        xofs-=(GP32_Height<<1);
        xk+=di;
	}
}


void Blur(unsigned char *buffer, unsigned char *vram)
{
    unsigned int i, c;
    unsigned int *vram32 = (unsigned int*)(vram+GP32_Height);
    unsigned int *buffer32 = (unsigned int*)(buffer+GP32_Height);
    for (i=GP32_Height; i<Scr_Size-GP32_Height; i+=4)
    {
        c = ((c + *(buffer32+1) + *(buffer32+(GP32_Height>>2)) + *(buffer32-(GP32_Height>>2)))>>2) & 0x3F3F3F3F;
        *buffer32++ = c;
        *vram32++ = c<<2;
    }
}


void BlurX2(unsigned char *buffer, unsigned char *vram)
{
    unsigned int i, c;
    unsigned int *vram32 = (unsigned int*)(vram+GP32_Height);
    unsigned int *buffer32 = (unsigned int*)(buffer+GP32_Height);
    for (i=GP32_Height; i<Scr_Size-GP32_Height; i+=4)
    {
        c = ((c + *(buffer32+1))>>1) & 0x3F3F3F3F;
        *buffer32++ = c;
        *vram32++ = c<<2;
    }
}

void BlurY2(unsigned char *buffer, unsigned char *vram)
{
    unsigned int i, c;
    unsigned int *vram32 = (unsigned int*)(vram+GP32_Height);
    unsigned int *buffer32 = (unsigned int*)(buffer+GP32_Height);
    for (i=GP32_Height; i<Scr_Size-GP32_Height; i+=4)
    {
        c = ((*(buffer32+(GP32_Height>>2)) + *(buffer32-(GP32_Height>>2)))>>1) & 0x3F3F3F3F;
        *buffer32++ = c;
        *vram32++ = c<<2;
    }
}

void Zoom(unsigned char *buffer1, unsigned char *buffer2, float zf)
{
    unsigned int fpshr = 16, fpmul = 65536;

    int u, v;
    int du = fpmul / zf, dv = fpmul / zf;
    u = 0; v = 0;
    unsigned int x, y;    
    for (x=0; x<GP32_Width; x++)
    {
        u = 0;
        for (y=0; y<GP32_Height; y++)
        {
            *buffer2++ = *(buffer1 + (u>>fpshr) * GP32_Height + (v>>fpshr));
            u+=du;
        }
        v+=dv;
    }
}


void Juhlia128(unsigned char *vram)
{
	int xp = sin(prticks/640.0) * fp_mul;
	int yp = cos(prticks/768.0) * fp_mul;

	int x, y, n0;

	int x0, y0, x1, y1;
	int xk, yk, yl;
	int mj2;

	unsigned short c;

	int xofs = (128 - 1) * 128;
	vram+=(128>>1);

	int di = (int)(0.028f * fp_mul);

	yl = -di * (-(128>>1));
	xk = di * (-(128>>1));

	for (x=0; x<128; x++)
	{
		yk = yl;
		for (y=(128>>1) - 1; y>=0; y--)
		{
			x0 = xk;
			y0 = yk;
			n0 = 0;
			
				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) goto end;


				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) {n0 = 1; goto end;}

				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) {n0 = 2; goto end;}
				
				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) {n0 = 3; goto end;}

				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) {n0 = 4; goto end;}

				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) {n0 = 5; goto end;}

				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) {n0 = 6; goto end;}
				
				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) {n0 = 7; goto end;}

				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) {n0 = 8; goto end;}


				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) {n0 = 9; goto end;}

				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) {n0 = 10; goto end;}
				
				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) {n0 = 11; goto end;}

				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) {n0 = 12; goto end;}

				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) {n0 = 13; goto end;}

				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) {n0 = 14; goto end;}
				
				x1 = ((x0 * x0)>>fp_shr) - ((y0 * y0)>>fp_shr) + xp;
				y1 = (((x0 * y0)<<1)>>fp_shr) + yp;
				mj2 = ((x1 * x1)>>fp_shr) + ((y1 * y1)>>fp_shr);
				x0 = x1; y0 = y1;
				if (mj2 > n2) {n0 = 15; goto end;}
				
				n0 = 15;
		end:

		c = n0<<4;
		*(vram + y) = c;
		*(vram - y - 1 + xofs) = c;
		yk-=di;
		}
    	vram+=128;
        xofs-=(128<<1);
        xk+=di;
	}
}


// ========== Rotozoomer ===========

void Rotozoomer(unsigned short *vram, unsigned short shade[], float ra, float zm)
{
	int x,y;
	int fp=24;

	int mx=0,my=0,mmx=10<<fp,mmy=-15<<fp;
	int dx=(cos(ra/d2r)*zm)*pow(2,fp);
	int dy=(sin(ra/d2r)*zm)*pow(2,fp);

    unsigned int c0;
    unsigned int *vram32 = (unsigned int*)vram;
	for (x=0; x<GP32_Width; x++)
	{
		mx = (mmx-=dy);
		my = (mmy+=dx);

		for (y=0; y<GP32_Height; y+=8)
		{
			c0 = shade[rotbitmap[((((int)((mx+=dx)>>fp))) & 127) + (((((int)((my+=dy)>>fp))&(Theight-1)) & 127)<<7)]];
			*vram32++ = (shade[rotbitmap[((((int)((mx+=dx)>>fp))) & 127) + (((((int)((my+=dy)>>fp))&(Theight-1)) & 127)<<7)]]<<16) | c0;
			c0 = shade[rotbitmap[((((int)((mx+=dx)>>fp))) & 127) + (((((int)((my+=dy)>>fp))&(Theight-1)) & 127)<<7)]];
			*vram32++ = (shade[rotbitmap[((((int)((mx+=dx)>>fp))) & 127) + (((((int)((my+=dy)>>fp))&(Theight-1)) & 127)<<7)]]<<16) | c0;
			c0 = shade[rotbitmap[((((int)((mx+=dx)>>fp))) & 127) + (((((int)((my+=dy)>>fp))&(Theight-1)) & 127)<<7)]];
			*vram32++ = (shade[rotbitmap[((((int)((mx+=dx)>>fp))) & 127) + (((((int)((my+=dy)>>fp))&(Theight-1)) & 127)<<7)]]<<16) | c0;
			c0 = shade[rotbitmap[((((int)((mx+=dx)>>fp))) & 127) + (((((int)((my+=dy)>>fp))&(Theight-1)) & 127)<<7)]];
			*vram32++ = (shade[rotbitmap[((((int)((mx+=dx)>>fp))) & 127) + (((((int)((my+=dy)>>fp))&(Theight-1)) & 127)<<7)]]<<16) | c0;
		}
	}
}

#define RNG(x, a, b)	((x) < (a) ? (a) : ((x) >= (b) ? ((b) - 1) : (x)))

void Polar(unsigned short *vram, unsigned short shade1[], unsigned short shade2[])
{
	int k1, k2, k3, sz;
	int c;
	k1 = (int)(prticks/32.0) & 127;
	k3 = (int)(prticks/32.0) & 255;
	
	sz = fsin3[(int)(prticks/32.0)];

	ra = sin(prticks/2048.0)*96.0;
	zm = prticks/1024.0;
	int zm128 = (int)(zm * 128) & 255;
	int rai = (int)ra & 255;

	int u,v;
	int i, x, y, dist, angl;
	unsigned int c0, c1, c2;
    unsigned int *vram32 = (unsigned int*)vram;
    unsigned int *vram33 = (unsigned int*)(vram + GP32_Height - 2);

    i = 0;
    unsigned int xp = ((GP32_Width - 1) * GP32_Height)>>1;
	for (x=0; x<(GP32_Width>>1); x++)
	{
        for (y=0; y<(GP32_Height>>1); y+=2)
        {
            dist = dist_angle[i];
            angl = dist_angle[i+1];

            u = dist + zm128;
            v = angl + rai;
            c = (fsin1[u] + fsin2[v+k3] + fsin3[u+v] + fsin1[u+fsin1[v]]) & 255;

            if (c<(128 + (dist>>1)))
            {
                c = dist + fsin4[angl + fsin5[angl + k1]]-sz;
                if (c>255) c = 255;
                if (c<0) c = 0;
                c0 = shade1[c];
            }
            else
                c0 = shade2[c];

            dist = dist_angle[i+2];
            angl = dist_angle[i+3];

            u = dist + zm128;
            v = angl + rai;
            c = (fsin1[u] + fsin2[v+k3] + fsin3[u+v] + fsin1[u+fsin1[v]]) & 255;


            if (c<(128 + (dist>>1)))
            {
                c = dist + fsin4[angl + fsin5[angl + k1]]-sz;
                if (c>255) c = 255;
                if (c<0) c = 0;

                c1 = (shade1[c]<<16) | c0;
                c2 = (c0<<16) | shade1[c];
                *vram32 = c1;
                *vram33 = c2;
                *(vram32+xp) = c1;
                *(vram33+xp) = c2;
            }
            else
            {
                c1 = (shade2[c]<<16) | c0;
                c2 = (c0<<16) | shade2[c];
            }
                *vram32 = c1;
                *vram33 = c2;
                *(vram32+xp) = c1;
                *(vram33+xp) = c2;
                vram32++;
                vram33--;
            i+=4;
    	}
        xp -= GP32_Height;
    	vram32+=GP32_Height/4;
    	vram33+=(3*GP32_Height/4);
    	i+=GP32_Height;
    }
}

// ========== Polar ===========
/*
void Polar(unsigned short *vram, unsigned short shade1[], unsigned short shade2[])
{
	unsigned int k1, k2, k3, sz;
	int c;
	k1 = (unsigned int)(prticks/32.0) & 127;
	k3 = (unsigned int)(prticks/32.0) & 255;
	
	sz = fsin3[(unsigned int)(prticks/32.0)];

	ra=sin(prticks/2048.0)*96.0;
	zm=prticks/1024.0;
	unsigned int zm128 = zm*128;
	unsigned int rai = ra;

	unsigned int u,v;
	unsigned int i, dist, angl;
	unsigned int c0;
    unsigned int *vram32 = (unsigned int*)vram;

	for (i=0; i<Scr_Size*2; i+=4)
	{
        dist = dist_angle[i];
        angl = dist_angle[i+1];

        u = dist + zm128;
        v = angl + rai;
        c = (fsin1[u] + fsin2[v+k3] + fsin3[u+v] + fsin1[u+fsin1[v]]) & 255;

        if (c<(128 + (dist>>1)))
        {
            c = dist + fsin4[angl + fsin5[angl + k1]]-sz;
            if (c>255) c = 255;
            if (c<0) c = 0;
            c0 = shade1[c];
        }
        else
            c0 = shade2[c];

        dist = dist_angle[i+2];
        angl = dist_angle[i+3];

        u = dist + zm128;
        v = angl + rai;
        c = (fsin1[u] + fsin2[v+k3] + fsin3[u+v] + fsin1[u+fsin1[v]]) & 255;

        if (c<(128 + (dist>>1)))
        {
            c = dist + fsin4[angl + fsin5[angl + k1]]-sz;
            if (c>255) c = 255;
            if (c<0) c = 0;
            *vram32++ = (shade1[c]<<16) | c0;
        }
        else
            *vram32++ = (shade2[c]<<16) | c0;
	}

	*(vram32-((160*240-120)>>1))=0xFFFFFFFF;
}
*/

void Blobs2vram(unsigned short *vram, unsigned short shade[])
{
    unsigned int i;
    unsigned int *vram32 = (unsigned int*)vram;
    for (i=0; i<Scr_Size; i+=2)
        *vram32++ = shade[blobbuffer[i]] | (shade[blobbuffer[i+1]]<<16);
}


void draw_blob(int xc, int yc, float zf)
{
    int x, y;
    int x0 = 0, y0 = 0;
    int x1 = blob_width * zf - 1, y1 = blob_height * zf - 1;

    int xi = xc - (blob_width >> 1) * zf;
    int yi = yc - (blob_height >> 1) * zf;

    if ((xi < GP32_Width) && (xi > -blob_width * zf) && (yi < GP32_Height) && (yi > -blob_height * zf))
    {
    	int fp = 16, mulfp = 65536;
        int su = 0, sv = 0;

        if (xi<0)
        {
            x0 = - xi; xi = 0;
            su = (x0<<fp)/zf;
        }
        if (yi<0)
        {
            y0 = - yi; yi = 0;
            sv = (y0<<fp)/zf;
        }
        if (xi > GP32_Width - blob_width * zf) x1 = GP32_Width + x0 - xi - 1;
        if (yi > GP32_Height - blob_height * zf) y1 = GP32_Height + y0 - yi - 1;

        int vofs= yi + xi * GP32_Height;
        unsigned char *buffer = blobbuffer + vofs;
        unsigned int c;

        int di = mulfp/zf;
        int u, v, up;

        u = su;
        for (x = x0; x<=x1; x++)
        {
            v = sv;
            up = (u>>fp) * blob_width;
            for (y = y0; y<=y1; y++)
            {
                c = *buffer + blob[(v>>fp) + up];
                if (c > 255) c = 255;
                *buffer++ = c;
                v+=di;
            }
            u+=di;
            buffer+= -(y1-y0+1) + GP32_Height;
        }
    }
}

void clean_blob(int xc, int yc, float zf)
{
    int x, y;
    int x0 = 0, y0 = 0;
    int x1 = blob_width * zf - 1, y1 = blob_height * zf - 1;

    int xi = xc - (blob_width >> 1) * zf;
    int yi = yc - (blob_height >> 1) * zf;

    if ((xi < GP32_Width) && (xi > -blob_width * zf) && (yi < GP32_Height) && (yi > -blob_height * zf))
    {
        if (xi<0)
        {
            x0 = - xi; xi = 0;
        }
        if (yi<0)
        {
            y0 = - yi; yi = 0;
        }
        if (xi > GP32_Width - blob_width * zf) x1 = GP32_Width + x0 - xi - 1;
        if (yi > GP32_Height - blob_height * zf) y1 = GP32_Height + y0 - yi - 1;

        int vofs= yi + xi * GP32_Height;
        unsigned char *buffer = blobbuffer + vofs;
        for (x = x0; x<=x1; x++)
        {
            for (y = y0; y<=y1; y++)
            {
                *buffer++ = 0;
            }
            buffer+= -(y1-y0+1) + GP32_Height;
        }
    }
}


void BlobStars3D(unsigned short *vram)
{
    int i;

    for (i=0; i<nstars3d; i++)
    {
        sx[i] = (star3dx[i]*256)/star3dz[i] + (GP32_Width>>1);
        sy[i] = (star3dy[i]*256)/star3dz[i] + (GP32_Height>>1);
        sz[i] = (starfar - star3dz[i]) / (starfar*1.5);

        draw_blob (sx[i], sy[i], sz[i]);
        
        if (star3dz[i]<16)
        {
            star3dx[i] = (rand() % scsize) - scsize/2;
            star3dy[i] = (rand() % scsize) - scsize/2;
            star3dz[i] = starfar;
            star3dv[i] = (((float)(rand() % 65536)) / 256.0) + 8;
        }
    }

    if ((prticks - wtime) >= 16)
    {
        for (i=0; i<nstars3d; i++)
    	{
    		wtime = prticks;
            star3dz[i] -= star3dv[i];
        }
    }
    Blobs2vram(vram, shades[32]);

    for (i=0; i<nstars3d; i++)
        clean_blob (sx[i], sy[i], sz[i]);
}

void drawfont16(int xf, int yf, float zf, int ch, unsigned short shade[], unsigned short *vram)
{
    int x, y;
    int x0 = 0, y0 = 0;
    int x1 = font_width * zf - 1, y1 = font_height * zf - 1;

    int xi = xf - (font_width >> 1) * zf;
    int yi = yf - (font_height >> 1) * zf;

    if ((xi < GP32_Width) && (xi > -font_width * zf) && (yi < GP32_Height) && (yi > -font_height * zf))
    {
    	int fp = 16, mulfp = 65536;
        int su = 0, sv = 0;

        if (xi<0)
        {
            x0 = - xi; xi = 0;
            su = (x0<<fp)/zf;
        }
        if (yi<0)
        {
            y0 = - yi; yi = 0;
            sv = (y0<<fp)/zf;
        }
        if (xi > GP32_Width - font_width * zf) x1 = GP32_Width + x0 - xi - 1;
        if (yi > GP32_Height - font_height * zf) y1 = GP32_Height + y0 - yi - 1;

        int vofs= yi + xi * GP32_Height;
        vram+=vofs;
        unsigned int c;

        float di = mulfp/zf;
        int u, v, up;

        int chn = fonts16[ch]<<8;

        u = su;
        for (x = x0; x<=x1; x++)
        {
            v = sv;
            up = (u>>fp) * font_width;
            for (y = y0; y<=y1; y++)
            {
                c = fnt16[256 + chn + up + (v>>fp)];
                if (c!=0)
                    *(vram+y) = shade[c<<4];
                v+=di;
            }
            u+=di;
            vram += GP32_Height;
        }
    }
}

void OpenJLH(unsigned short *vram, int ffp)
{
    int x, y;
    unsigned int *vram32 = (unsigned int*)vram;
    unsigned int xp;
    for (x=0; x<GP32_Width; x++)
    {
        xp = (x>>ffp)*GP32_Height;
        for (y=0; y<GP32_Height; y+=2)
        {
            *vram32++ = (jlh0[xp+y+1]<<16) | jlh0[xp+y];
        }
    }
}

void Close(unsigned short *vram, int xclose)
{
    unsigned short *vram1 = vram;
    unsigned short *vram2 = vram + Scr_Size-1;
    int x, y;

    for (x=0; x<xclose; x++)
        for (y=0; y<GP32_Height; y++)
            *vram1++ = 0;

    for (x=GP32_Width-1; x>=GP32_Width-1 - xclose; x--)
        for (y=0; y<GP32_Height; y++)
            *vram2-- = 0;
}

