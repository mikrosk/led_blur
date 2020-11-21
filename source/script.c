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
#include <string.h>
#include <math.h>
#include "gp32_functions.h"
#include "engine3d.h"
#include "effects.h"
#include "precalcs.h"

extern unsigned char *framebuffer[2];
extern GPDRAWSURFACE GP32Surface[2];

//extern MODPlay mymod;

extern int clockspeed;
extern int vsync;

extern int flip;
int modplay=0;

int buttons0 = 0, buttons1 = 0;

int fps=0, mo=0, atime=0, ptime=0;
int fps_show=0;
char sbuffer[64];

int quit=0;
int part=0;
#define nparts 9
int chpart=1;
int bpp[nparts];

int nfrm=0;
int partime, prticks;

extern unsigned char bitfonts[];
unsigned char fonts[59*64];

unsigned char rotbitmap[128*128];
unsigned char blurbuffer1[Scr_Size];
unsigned char *blur1 = blurbuffer1;

extern GP_HPALETTE	blurpal;

extern unsigned short shades[MAXSHADES][256];
extern unsigned short sshades[MAXSHADES][256];
extern unsigned short bshade[256], wshade[256];

extern unsigned char yscan[92];
int yss = 0, ypp = 1;
extern unsigned char xscan[192];
int xss = 0, xpp = 1;

int signal3d = 0;

void InitFonts()
{
	int x, y, i =0;
	int n, c;

	for (n=0; n<59; n++)
	{
		for (y=0; y<8; y++)
		{
			c = bitfonts[i++];
			for (x=0; x<8; x++)
			{
				fonts[(n << 6) + (x << 3) + (7 - y)] = ((c >>  (7 - x)) & 1) * 255;
			}
		}
	}
}


static inline void DrawFont(int xp, int yp, int ch, int bpp, unsigned short* gp32v16)
{
    unsigned char *vram8;
    unsigned short *vram16;
    int cp = ch << 6;
    int x, y, xc, xi;

    switch(bpp)
    {
        case 8:

        vram8 = (unsigned char*)gp32v16 + xp * GP32_Height + ((GP32_Height - 1) - yp);
        for (x=0; x<8; x++)
        {
            xc = xp + x;
            if ((xc>=1) && (xc<GP32_Width - 1))
            {
                xi = x << 3;
                for (y=0; y<8; y++)
                {
                    *vram8++ |= fonts[cp + xi + y];
                }
                vram8-=8;
            }
            vram8+=GP32_Height;
        }
        break;
        
        case 16:

        vram16 = (unsigned short*)gp32v16 + xp * GP32_Height + ((GP32_Height - 1) - yp);
        for (x=0; x<8; x++)
        {
            xc = xp + x;
            if ((xc>=1) && (xc<GP32_Width - 1))
            {
                xi = x << 3;
                for (y=0; y<8; y++)
                {
                    *vram16++ |= fonts[cp + xi + y];
                }
                vram16-=8;
            }
            vram16+=GP32_Height;
        }
        break;
        
        default:
            break;
    }    
}


void DrawText(int xtp, int ytp, int cn, char *text, int bpp, GPDRAWSURFACE * ptgpds)
{
	int n;
	char c;

	for (n = 0; n<cn; n++)
	{
		c = *text++;
		if (c>31 && c<92) DrawFont(xtp, ytp, c - 32, bpp, (unsigned short*)ptgpds->ptbuffer);
			else if (c==0) n = cn;
		xtp+=8; if (xtp>GP32_Width - 1) n = cn;
	}
}


int Ticks()
{
	return (int)(GpTickCountGet() * (66.0/clockspeed));
}


void CountFps(int show)
{
	char tmp_string[64];

	if (Ticks()-atime>=1000)
	{
		atime = Ticks();
		mo=(nfrm-fps);
		fps=nfrm;
	}

	if (show==1)
	{
		sprintf(sbuffer, "FPS = %d", mo);
        DrawText(8, 16, 16, sbuffer, bpp[part], &GP32Surface[flip]);
	}
}


void Keys()
{
	unsigned short buttons_event=0;
	unsigned short buttons_down=0;
	unsigned short buttons_up=0;

	buttons1 = GpKeyGet();
	buttons_event = buttons0 ^ buttons1;

	if (buttons_event)
	{
		buttons_down = buttons_event & buttons1;
		buttons_up = buttons_event ^ buttons1;
	}

	buttons0 = buttons1;


	if (buttons_down & GPC_VK_SELECT)
		vsync=(vsync+1)&1;

/*
	if (buttons_down & GPC_VK_FL)
	{
		part--;
		if (part<0) part = nparts - 1;
        partime = Ticks();
		chpart=1;
	}

	if (buttons_down & GPC_VK_FR)
	{
		part=(part+1)%nparts;
        partime = Ticks();
		chpart=1;
	}
*/
	if (buttons_down & GPC_VK_FB)
		fps_show=(fps_show+1)&1;
}

	
void InitGraphics(int depth);

void checkmode()
{

	if (chpart)
	{
		chpart=0;
	   if (bpp[part]==16)
	   {
		framebuffer[0] = (unsigned char*)GP32Surface[0].ptbuffer;
		framebuffer[1] = (unsigned char*)GP32Surface[1].ptbuffer;
	   }
	   else
	   {
		framebuffer[0] = (unsigned char*)GP32Surface[0].ptbuffer;
		framebuffer[1] = (unsigned char*)GP32Surface[1].ptbuffer;
	   }
	   InitGraphics(bpp[part]);
	}
}


void ClearScreen(unsigned short *vram, unsigned short c)
{
    int i;
    for (i=0; i<Scr_Size; i++)
		*(vram+i) = c;
}

void Fade2Shade(unsigned short shade1[], unsigned short shade2[], float pc, unsigned short shade[])
{
    unsigned int i;
    unsigned int r1, g1, b1, r2, g2, b2;
    for (i=0; i<256; i++)
    {
        r1 = (shade1[i]>>11) & 0x1F; g1 = (shade1[i]>>6) & 0x1F; b1 = (shade1[i]>>1) & 0x1F;
        r2 = (shade2[i]>>11) & 0x1F; g2 = (shade2[i]>>6) & 0x1F; b2 = (shade2[i]>>1) & 0x1F;
        r2 = r1 + (r2-r1) * pc; g2 = g1 + (g2-g1) * pc; b2 = b1 + (b2-b1) * pc;
        shade[i] = (r2<<11) | (g2<<6) | (b2<<1);
    }
}

void DrawText16(int xp, int yp, char *text, unsigned short *vram)
{
    int n = strlen(text);
    int i;
    for (i=0; i<n; i++)
        drawfont16(xp + (i<<4), yp, 1.5*(sin(i/2.0+prticks/128.0)*0.35 + 0.95), *(text+i), shades[16], vram);
}

void WaterPart()
{
    int cc;
    if (prticks<1024)
        OpenJLH((unsigned short*)framebuffer[flip], (1023 - prticks)>>6);
    else 
    {
        Water((unsigned short*)framebuffer[flip]);
        RunScene3d((unsigned short*)framebuffer[flip], 16);
    }

    if (prticks>8191)
    {
        if (prticks<16384)
            DrawText16(72, 224, "Code:Optimus", (unsigned short*)framebuffer[flip]);
        else if (prticks<16384+8192)
            DrawText16(20, 12, "Music:The Hardliner", (unsigned short*)framebuffer[flip]);
    }

    if (prticks>24576+2048+320 && prticks<24576+2048+512+320)
    {
        cc = (int)(((float)(prticks-24576-2048-320)/511.0) * 160.0);
        if (cc>=155)
            cc = 160;
        Close ((unsigned short*)framebuffer[flip], cc);
    }
    if (prticks>24576+2048+512+320 && prticks<24576+2048+512+1640)
    {
        ClearScreen((unsigned short*)framebuffer[flip],0);
    }
    if (prticks>24576+2048+512+1640)
    {
        ClearScreen((unsigned short*)framebuffer[flip],0);
        partime = Ticks();
        part = 7;
        quit = 1;
    }
}

void PlasmaCube()
{
//    ClearScreen((unsigned short*)framebuffer[flip],0);
    OpenJLH((unsigned short*)framebuffer[flip], 31);
	RunScene3d((unsigned short*)framebuffer[flip], 4);

    if (prticks<2048)
        PlasmaFade2((unsigned short*)framebuffer[flip],shades[34], 255 - (prticks>>3));
    if (prticks>16384-4096)
    {
        partime = Ticks();
        part = 8;
    }
}

void PlasmaFace()
{
    if (prticks<4096)
    	PlasmaFade((unsigned short*)framebuffer[flip],shades[34],prticks>>4);
    else
        Plasma((unsigned short*)framebuffer[flip],shades[34]);

    int adj3 = -1536;
    if (prticks>8191+adj3)
        RunScene3d((unsigned short*)framebuffer[flip], 3);
    if (prticks>49152+4096)
    {
        partime = Ticks();
        part = 6;
    }
}

void JuhliaBig()
{
    if (prticks<256)
        Fade2Shade(wshade, sshades[16], (prticks/256.0), shades[16]);
    else if (prticks<1024)
        Fade2Shade(sshades[16], sshades[16], 1, shades[16]);

    float xp, yp;
    if (prticks<24576)
    {
    	xp = sin(prticks/512.0);
    	yp = sin(prticks/768.0);
    }
    else if (prticks<24576+2048)
    {
    	xp = sin((24576)/512.0) * (1+((prticks-24576)/192.0));
    	yp = sin((24576)/768.0) * (1+((prticks-24576)/192.0));
    }
    else
    {
    	xp = 6128;
    	yp = 6510;
    	ClearScreen((unsigned short*)framebuffer[flip],shades[16][0]);
    	ClearScreen((unsigned short*)framebuffer[(flip+1)&1],shades[16][0]);
        partime = Ticks();
        part = 5;
    }

    if (prticks<24576+2048)
    {
        Juhlia((unsigned short*)framebuffer[flip], shades[16], xp, yp);
    	RunScene3d((unsigned short*)framebuffer[flip],0);
    }
}

void JuhliaIn()
{
    if (prticks<1024)
        Fade2Shade(bshade, sshades[33], (prticks/1024.0), shades[33]);

    float fcuk3 = 32768-6144;
    float ra, zm;
    if (prticks<fcuk3)
    {
        ra=sin(prticks/fcuk3)*6128.0;
    	zm=sin(prticks/1900.0)+1.03;
    }
    else if (prticks<fcuk3+192)
    {
        ra=sin(prticks/fcuk3)*6128.0;
    	zm=sin(fcuk3/1900.0)+1.03 - ((prticks-fcuk3+1)/192.0);
        Fade2Shade(sshades[33], wshade, ((prticks-fcuk3)/192.0), shades[33]);
    }
    else
    {
        partime = Ticks();
        part = 4;
    }

    Juhlia128(rotbitmap);
    Rotozoomer((unsigned short*)framebuffer[flip],shades[33], ra, zm);
}

void PolarTunnel()
{
    if (prticks<128)
    {
        Fade2Shade(wshade, sshades[11], (prticks/128.0), shades[11]);
        Fade2Shade(wshade, sshades[12], (prticks/128.0), shades[12]);
    }
    else if (prticks<1024)
    {
        Fade2Shade(sshades[11], sshades[11], 1, shades[11]);
        Fade2Shade(sshades[12], sshades[12], 1, shades[12]);
    }

    if (prticks>24576-1024+2048 && prticks<24576+2048)
    {
        Fade2Shade(sshades[11], bshade, ((prticks-(24576-1024+2048))/1024.0), shades[11]);
        Fade2Shade(sshades[12], bshade, ((prticks-(24576-1024+2048))/1024.0), shades[12]);
    }
    if (prticks>=24576+2048)
    {
        partime = Ticks();
        part = 3;
    }

    Polar((unsigned short*)framebuffer[flip],shades[11],shades[12]);
}

void RedBull()
{
    BlobStars3D((unsigned short*)framebuffer[flip]);
    RunScene3d((unsigned short*)framebuffer[flip], 24);

    int fcuk = 8192;
    if (prticks>16384+fcuk+16384-512 - 128)
    {
        Fade2Shade(sshades[32], wshade, (prticks-(16384+fcuk+16384-512 - 128))/128.0, shades[32]);
    }

    if (signal3d==1)
    {
        partime = Ticks();
        part = 2;
    }
}

void Space()
{
    if (prticks<2048)
        Fade2Shade(bshade, sshades[32], (prticks/2048.0), shades[32]);

    if (ypp!=0)
    {
        int j;
        if (ypp==1)
            for (j=0; j<=yss; j++)
               yscan[2*j] = 1;
        if (ypp==-1)
            for (j=46; j>=yss; j--)
               yscan[2*j+1] = 1;

        if (ypp==1) yss = (prticks/1024.0)*46;
        if (ypp==-1) yss = ((1-(prticks-1024)/1024.0))*46;
        if (yss>45) ypp = -1;
        if (yss<0) ypp = 0;
    }

    unsigned int tmlogo = 8192+1024-384;
    if (xpp!=0)
    {
        if (prticks>tmlogo-2048)
        {
            int j;
            for (j=0; j<=xss; j++)
            {
                xscan[2*j] = 0;
                xscan[191 - 2*j] = 0;
            }
            xss = (prticks-tmlogo+2048)/16.0;
            if (xss>95)
            {
                for (j=0; j<96; j++)
                {
                    xscan[2*j] = 0;
                    xscan[191 - 2*j] = 0;
                }
            xpp = 0;
            }
        }
    }

    BlobStars3D((unsigned short*)framebuffer[flip]);

    if (prticks<tmlogo) LogoDistort((unsigned short*)framebuffer[flip], 64, 78);
        else if (prticks<tmlogo+1024) LogoZoom((unsigned short*)framebuffer[flip], 96, 108, ((float)(prticks-tmlogo)/1024.0));
            else if (prticks<tmlogo+3072) LogoZoom((unsigned short*)framebuffer[flip], 96, 108, 1);
                else if (prticks<tmlogo+4096) LogoZoom((unsigned short*)framebuffer[flip], 96, 108, 1.0 - ((float)(prticks-tmlogo-3072)/1024.0));
                    else
                    {
                        partime = Ticks();
                        part = 1;
                    }

}

void Script()
{
    int i;
    unsigned short *vram;

    prticks = Ticks() - partime;

	switch(part)
	{

		case 0:
            bpp[part] = 16;
            checkmode();
            Space();
		break;

        case 1:
            bpp[part] = 16;
            checkmode();
            RedBull();
        break;

		case 2:
			bpp[part]=16;
			checkmode();
			PolarTunnel();
			RunScene3d((unsigned short*)framebuffer[flip], 20);
		break;

		case 3:
			bpp[part]=16;
			checkmode();
			JuhliaIn();
		break;

		case 4:
			bpp[part] = 16;
			checkmode();
			JuhliaBig();
		break;

		case 5:
			bpp[part] = 16;
			checkmode();
			PlasmaFace();
		break;

		case 6:
			bpp[part] = 16;
			checkmode();
			PlasmaCube();
		break;

		case 7:
			bpp[part] = 16;
			checkmode();
            ClearScreen((unsigned short*)framebuffer[flip],0);
            quit = 1;
//			RunScene3d((unsigned short*)framebuffer[flip],2);
//            GpPaletteSelect(blurpal);
//			RunScene3d(blur1,1);
//			Blur(blur1, framebuffer[flip]);
		break;

		case 8:
			bpp[part] = 16;
			checkmode();
			WaterPart();
		break;

		default:
		break;
	}
}

void Run()
{
	Keys();
	Script();
	CountFps(fps_show);
	nfrm++;
}

