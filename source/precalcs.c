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
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "gp32_functions.h"
#include "effects.h"
#include "precalcs.h"
#include "script.h"
#include "engine3d.h"

// Precalc screen variables
// ======================

int prec_vsync = 0;

int prec_flip = 0;
unsigned short *prec_framebuffer[2];
static char textbuffer[40];

extern GPDRAWSURFACE GP32Surface[2];
extern unsigned char *framebuffer[2];

int prec_ybar = 0;
int ybar_height = 12;
int total_precs = 6;

extern unsigned char ball[];
extern unsigned char ball_c[256];
extern unsigned short ball_p[256];

float star3dx[nstars3d];
float star3dy[nstars3d];
float star3dz[nstars3d];
float star3dv[nstars3d];

int scsize = 4096, starfar = 4096;


// Effect precalc tables
// ====================

// Texture Plasma
unsigned int fsin6[2048], fsin7[2048], fsin8[2048];

// Polar/Tunnel
unsigned int fsin4[2048], fsin5[2048];
const float d2b = (rang * d2r) / 360.0;
const float d2b2 = (rang2 * d2r) / 360.0;
unsigned int fsin1[2048], fsin2[2048], fsin3[2048];
unsigned short dist_angle[Scr_Size*2];

// Water
unsigned char uberclamp[512];

// Blob

unsigned char blob[blob_width * blob_height];

unsigned short shades[MAXSHADES][256];
unsigned short sshades[MAXSHADES][256];
unsigned short bshade[256], wshade[256];

int div_tbl[4096];

GP_HPALETTE	blurpal;

// Distort

int dsin1[1024], dsin2[1024];
unsigned char dclampx[256], dclampy[128];
unsigned char yscan[92];
unsigned char xscan[192];

extern unsigned char fonts16[];
unsigned char fnt16[32768];

void DistortInit()
{
    unsigned int i;

    for (i=0; i<256; i++)
    {
        if ((i<32) || (i>223)) dclampx[i] = 0;
            else dclampx[i] = i - 32;
        }
    for (i=0; i<128; i++)
        if ((i<22) || (i>105)) dclampy[i] = 0;
            else dclampy[i] = i - 22;

    for (i=0; i<1024; i++)
    {
        dsin1[i] = sin(i/12.0) * 4;
        dsin2[i] = sin(i/16.0) * 6;
    }
    
    for (i=0; i<92; i++)
        yscan[i] = 0;
    for (i=0; i<192; i++)
        xscan[i] = 1;
}

void Stars3dInit()
{
    int i;
    for (i=0; i<nstars3d; i++)
    {
        star3dx[i] = (rand() % scsize) - scsize/2;
        star3dy[i] = (rand() % scsize) - scsize/2;
        star3dz[i] = (rand() % scsize) - scsize/2;
        star3dv[i] = (((float)(rand() % 65536)) / 256.0) + 64;
    }
}


void precdivs()
{
    int fp = 16;
    int i;
    for (i=0; i<4096; i++)
    {
        if ((i-2048)!=0)
            div_tbl[i] = (1<<fp)/(i-2048);
        else
            div_tbl[i] = (1<<fp);
    }
}

// ========== Precalculation Bar Update ===========

void update_precbar(char *prec_msg, int i, int i_full)
{

	unsigned short *vram;
	
	int x, y;
	int x0, x1;
	int y0, y1;
	int halfbar;
	
	int prec_loop = i_full >> 4;
	int prec_percent;

	if ((!(i % prec_loop)) || (i >= i_full-2))
	{
		prec_percent = ((float)i / (float)(i_full-1))*100.0;
		if (i>=i_full-2) prec_percent = 100;

		y0 = GP32_Height - (prec_ybar + 1) * ybar_height;
		y1 = y0 + ybar_height - 1;

		halfbar =  ((GP32_Width/2 -1) * prec_percent) / 100;

		x0 = GP32_Width/2 - 1 - halfbar;
		x1 = GP32_Width/2 + halfbar;

		vram = prec_framebuffer[prec_flip] + x0*GP32_Height + y0;

		for (x=x0; x<x1; x++)
		{
			for (y=y0; y<y1; y++)
			{
				*vram++=0;
			}
			vram+= -y1 + y0  + GP32_Height;
		}

		sprintf(textbuffer, "%s (%d/%d)", prec_msg, prec_ybar+1, total_precs);
		int xc = GP32_Width/2 - ((strlen(textbuffer)>>1)<<3);
		int yc = prec_ybar*ybar_height - 3;
        DrawText(xc, yc+13, strlen(textbuffer), textbuffer, 16, &GP32Surface[prec_flip]);

		GpSurfaceFlip(&GP32Surface[prec_flip], prec_vsync);
        prec_flip = (prec_flip + 1) & 1;
	}

}


// ====== MakeColors subroutine ======

void MakeColors(unsigned short cols[], ColorRGB c0, ColorRGB c1, int n0, int n1)
{
	int i;
	float dr,dg,db;
	float cr,cg,cb;

	cr=c0.r; cg=c0.g; cb=c0.b;

	dr=((float)c1.r - (float)c0.r)/(float)(n1 - n0 + 1);
	dg=((float)c1.g - (float)c0.g)/(float)(n1 - n0 + 1);
	db=((float)c1.b - (float)c0.b)/(float)(n1 - n0 + 1);

	for (i=n0; i<=n1; i++)
	{
		cr+=dr;	cg+=dg;	cb+=db;
		cols[i]= ((int)cr<<11) | ((int)cg<<6) | ((int)cb<<1);
	}
}

void CloneColors()
{
    unsigned int j, i;
    for (j=0; j<MAXSHADES; j++)
        for (i=0; i<256; i++)
            sshades[j][i] = shades[j][i];
}

void MakeFadeColors()
{
    unsigned int i;
    for (i=0; i<256; i++)
    {
        bshade[i] = 0;
        wshade[i] = 0xFFFE;
    }
}

void colortest()
{
	ColorRGB c0[4]={{15,0,31}, {31,15,0}, {31,31,0}, {31,31,31}};
	ColorRGB c1[4]={{7,0,15}, {15,7,31}, {7,15,31}, {15,31,31}};
	ColorRGB c2[4]={{15,7,0}, {15,15,0}, {7,31,7}, {0,31,15}};
	ColorRGB toon[4]={{7,7,7}, {15,15,15}, {15,26,31}, {31,31,31}};
	ColorRGB plsm1[4]={{0,0,15}, {31,0,0}, {31,23,0}, {31,31,31}};
	ColorRGB plsm2[3]={{15,0,0}, {0,0,31}, {0,31,31}};
	ColorRGB plsm3[3]={{0,15,0}, {31,31,0}, {0,31,31}};
	ColorRGB plsm4[3]={{15,0,0}, {31,31,15}, {15,15,0}};
	ColorRGB plsm5[3]={{31,0,0}, {15,0,31}, {31,31,31}};
	ColorRGB plsm6[3]={{0,0,0}, {31,31,31}, {0,0,0}};
	ColorRGB backmask[4]={{15,15,15}, {15,0,15}, {31,0,15}, {31,31,31}};
	ColorRGB polar[4]={{0,0,15}, {7,15,7}, {31,15,0}, {31,31,31}};
	ColorRGB plasma[4]={{15,0,31}, {15,15,31}, {31,31,31}, {15,31,31}};
    ColorRGB blobcol[3] = {{0,0,0}, {15,7,31}, {23,31,31}};
    ColorRGB plasmablue[4] = {{15,7,7}, {0,0,31}, {15,31,31}, {15,7,31}};

    MakeColors (shades[34], plasmablue[0], plasmablue[1], 0, 63);
    MakeColors (shades[34], plasmablue[1], plasmablue[2], 64, 127);
    MakeColors (shades[34], plasmablue[2], plasmablue[3], 128, 191);
    MakeColors (shades[34], plasmablue[3], plasmablue[0], 192, 255);

    MakeColors (shades[32], blobcol[0], blobcol[1], 0, 127);
    MakeColors (shades[32], blobcol[1], blobcol[2], 128, 255);

	MakeColors(shades[33], plsm1[0], plsm1[1], 0, 63);
	MakeColors(shades[33], plsm1[1], plsm1[3], 64, 191);
	MakeColors(shades[33], plsm1[3], plsm1[3], 192, 255);

	MakeColors(shades[0], c0[0], c0[1], 0, 159);
	MakeColors(shades[0], c0[1], c0[2], 160, 191);
	MakeColors(shades[0], c0[2], c0[3], 192, 255);

	MakeColors(shades[1], c1[0], c1[1], 0, 127);
	MakeColors(shades[1], c1[1], c1[2], 128, 191);
	MakeColors(shades[1], c1[2], c1[3], 192, 255);

	MakeColors(shades[2], c2[0], c2[1], 0, 63);
	MakeColors(shades[2], c2[1], c2[2], 64, 191);
	MakeColors(shades[2], c2[2], c2[3], 192, 255);

	MakeColors(shades[3], toon[0], toon[1], 0, 63);
	MakeColors(shades[3], toon[1], toon[2], 64, 191);
	MakeColors(shades[3], toon[2], toon[3], 192, 255);

	MakeColors(shades[4], plsm1[0], plsm1[1], 0, 63);
	MakeColors(shades[4], plsm1[1], plsm1[2], 64, 127);
	MakeColors(shades[4], plsm1[2], plsm1[1], 128, 191);
	MakeColors(shades[4], plsm1[1], plsm1[0], 192, 255);

	MakeColors(shades[5], plsm2[0], plsm2[1], 0, 63);
	MakeColors(shades[5], plsm2[1], plsm2[2], 64, 127);
	MakeColors(shades[5], plsm2[2], plsm2[1], 128, 191);
	MakeColors(shades[5], plsm2[1], plsm2[0], 192, 255);

	MakeColors(shades[6], plsm3[0], plsm3[1], 0, 63);
	MakeColors(shades[6], plsm3[1], plsm3[2], 64, 127);
	MakeColors(shades[6], plsm3[2], plsm3[1], 128, 191);
	MakeColors(shades[6], plsm3[1], plsm3[0], 192, 255);

	MakeColors(shades[7], plsm4[0], plsm4[1], 0, 63);
	MakeColors(shades[7], plsm4[1], plsm4[2], 64, 127);
	MakeColors(shades[7], plsm4[2], plsm4[1], 128, 191);
	MakeColors(shades[7], plsm4[1], plsm4[0], 192, 255);

	MakeColors(shades[8], plsm5[0], plsm5[1], 0, 63);
	MakeColors(shades[8], plsm5[1], plsm5[2], 64, 127);
	MakeColors(shades[8], plsm5[2], plsm5[1], 128, 191);
	MakeColors(shades[8], plsm5[1], plsm5[0], 192, 255);

	MakeColors(shades[9], plsm6[0], plsm6[1], 0, 63);
	MakeColors(shades[9], plsm6[1], plsm6[2], 64, 127);
	MakeColors(shades[9], plsm6[2], plsm6[1], 128, 191);
	MakeColors(shades[9], plsm6[1], plsm6[0], 192, 255);

	MakeColors(shades[10], backmask[0], backmask[1], 0, 127);
	MakeColors(shades[10], backmask[1], backmask[2], 128, 191);
	MakeColors(shades[10], backmask[2], backmask[3], 192, 255);

	MakeColors(shades[11], polar[0], polar[1], 0, 127);
	MakeColors(shades[11], polar[1], polar[2], 128, 191);
	MakeColors(shades[11], polar[2], polar[3], 192, 255);

	MakeColors(shades[12], plasma[0], plasma[1], 128, 159);
	MakeColors(shades[12], plasma[1], plasma[2], 160, 191);
	MakeColors(shades[12], plasma[2], plasma[3], 192, 223);
	MakeColors(shades[12], plasma[3], plasma[0], 224, 255);

	MakeColors(shades[16], plsm1[0], plsm1[1], 0, 127);
	MakeColors(shades[16], plsm1[1], plsm1[2], 128, 191);
	MakeColors(shades[16], plsm1[2], plsm1[3], 192, 255);
}


// ============= Effects Precalculations ============


void BlobInit()
{
    int x, y, i = 0;
    int xc, yc, c;
    float dist;

    for (y = 0; y<blob_height; y++)
    {
        for (x = 0; x<blob_width; x++)
        {
            xc = x - (blob_width >> 1);
            yc = y - (blob_height >> 1);
            
            dist = xc * xc + yc * yc;
            if (dist==0) dist = 1;
            c = (int)((blob_width * blob_height * 512) / (dist * dist));
            if (c>255) c = 255;
//            if (c==0) c = 191;
            blob[i] = c;
            i++;
        }
    }
}

void WaterInit()
{
	int i;
	for (i=0; i<512; i++)
	{
		if (i<256) uberclamp[i] = 0;
		  else uberclamp[i] = i - 256;
		update_precbar("SLOW THE EGGS.", i, 512);
	}
	prec_ybar++;
}


void Show3dInit()
{
	Init3d();

	int i;

	for (i=0; i<256; i++)
	{
		if (i>0 && i<255) ball_p[i] = ((ball[(i<<1) + 2] + (ball[(i<<1) + 3]<<8))<<1);
			else ball_p[i] = 0;
		ball_c[i] = ball[512+i];
			if (ball_c[i]>15) ball_c[i]=15;
	}

	for (i=0; i<4096; i++)
	{
		update_precbar("TRICKY TEST..", i, 4096);
	}
	prec_ybar++;
}


static void PolarInit()
{
	int i;
	float l=1.00;
	for (i=0;i<2048;i++)
	{
		fsin4[i]=sin(i/(l*d2b))*48.0+64.0;
		fsin5[i]=sin(i/(l*d2b/2))*40.0+48.0;
		update_precbar("..AND MORE SINES", i, 2048);
	}
	prec_ybar++;

	int x, y;

	float w=Twidth/2;
	i=0;
	for (x=-GP32_Width/2; x<GP32_Width/2; x++)
	{
		for (y=-GP32_Height/2; y<GP32_Height/2; y++)
		{
			dist_angle[i++]=(int)((w*Twidth)*(1/sqrt(x*x+y*y)));
			dist_angle[i++]=((int)(2.0 * Twidth * atan2(y,x)/pi)) & 255;
			update_precbar("RADIUS AND ANGLE FOR EACH PIXEL!", i>>1, Scr_Size);
		}
	}
	prec_ybar++;
}

static void PlasmaInit()
{
	int i;

	for (i=0; i<2048; i++)
	{
		fsin1[i] = sin(i/(d2b2/3)) * 96 + 96;
		fsin2[i] = sin(i/(d2b2/4)) * 112 + 112;
		fsin3[i] = sin(i/(d2b2/5)) * 128 + 128;
		fsin6[i] = sin(i/(2*15.0)) * 96 + 96;
		fsin7[i] = sin(i/(2*20.0)) * 112 + 112;
		fsin8[i] = sin(i/(2*35.0)) * 128 + 128;
		update_precbar("PRECALCULATING SINES..", i, 2048);
	}
	prec_ybar++;
}

void Set8bitPals()
{
    unsigned int i;
    unsigned short palblur[256];
    for(i = 0 ; i < 256 ; i++)
        palblur[i] = ((i>>3)<<11) | ((i>>3)<<6) | ((i>>3)<<1) | 0;
    blurpal = GpPaletteCreate(256, palblur);
}

void FontInit()
{
    int x, y, c, i=0;
    for (c=0; c<128; c++)
        for (x=0; x<16; x++)
            for (y=15; y>=0; y--)
                fnt16[i++] = fonts16[(c<<8) + y*16 + x];
}

void precalcs()
{
	prec_framebuffer[0] = (unsigned short*)GP32Surface[0].ptbuffer;
	prec_framebuffer[1] = (unsigned short*)GP32Surface[1].ptbuffer;

    precdivs();
    FontInit();

	PlasmaInit();
	PolarInit();
	Show3dInit();
	WaterInit();
    BlobInit();
    Stars3dInit();
    DistortInit();

    Set8bitPals();
    colortest();
    CloneColors();
    MakeFadeColors();
}

