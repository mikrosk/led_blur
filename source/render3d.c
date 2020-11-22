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
#include <math.h>
#include <stdlib.h>
#include "engine3d.h"
#include "precalcs.h"
#include "render3d.h"
#include "effects.h"

unsigned char ball_c[256];
unsigned short ball_p[256];

extern unsigned short swp[MAXDATA];
int zdata[MAXDATA];

extern point3d fpts[MAXDATA];
extern point3d norms[MAXDATA];
extern point3d pt_norms[MAXDATA];
extern point2d spts[MAXDATA];
extern point3d spls[MAXDATA];
tcord point_tc[MAXDATA];

unsigned int RenderMode;

unsigned int zbuffer[GP32_Width*GP32_Height];

extern unsigned short shades[MAXSHADES][256];
extern int div_tbl[4096];

extern unsigned int fsin6[2048], fsin7[2048], fsin8[2048];
extern unsigned char draculf[];

extern unsigned char buffer1[Scr_Size], buffer2[Scr_Size];

int tm=0;
extern int prticks;

void drawpoint(point2d point, unsigned short *vram)
{
    if (point.x>=0 && point.x<GP32_Width && point.y>=0 && point.y<GP32_Height)
		*(vram + point.y + point.x * GP32_Height) = point.c;
}

void drawball(point2d point, float zf, unsigned short *vram)
{
    int x, y;
    int x0 = 0, y0 = 0;
    int x1 = ball_width * zf - 1, y1 = ball_height * zf - 1;

    int xi = point.x - (ball_width >> 1) * zf;
    int yi = point.y - (ball_height >> 1) * zf;

    if ((xi < GP32_Width) && (xi > -ball_width * zf) && (yi < GP32_Height) && (yi > -ball_height * zf))
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
        if (xi > GP32_Width - ball_width * zf) x1 = GP32_Width + x0 - xi - 1;
        if (yi > GP32_Height - ball_height * zf) y1 = GP32_Height + y0 - yi - 1;

        int vofs= yi + xi * GP32_Height;
        vram+=vofs;
        unsigned int c;

        float di = mulfp/zf;
        int u, v, up;

        u = su;
        for (x = x0; x<=x1; x++)
        {
            v = sv;
            up = (u>>fp) * ball_width;
            for (y = y0; y<=y1; y++)
            {
                c = ball_c[up + (v>>fp)];
                if (c!=0)
                    *(vram+y) = ball_p[c + point.c];
                v+=di;
            }
            u+=di;
            vram += GP32_Height;
        }
    }
}


void drawline8bit (line2d line, unsigned char *buffer)
{

	int x1 = spts[line.p0].x;
	int y1 = spts[line.p0].y;
	int x2 = spts[line.p1].x;
	int y2 = spts[line.p1].y;
	int c = 63;

	int dx, dy, n, l;
	int x00, y00;
	int fp = 12;
	int vramofs;

	int x, y;

	dx = x2 - x1;
	dy = y2 - y1;

	if (abs(dy) < abs(dx))
	{
		if (x1>x2)
		{
			n = x1; x1 = x2; x2 = n;
			n = y1; y1 = y2; y2 = n;
		}

        l = ((dy<<fp)*div_tbl[dx+2048])>>16;
        y00 = y1<<fp;
		for (x=x1; x<x2; x++)
		{
			vramofs = ((y00 += l)>>fp) + x * GP32_Height;
            if (vramofs>=0 && vramofs<Scr_Size) * (buffer + vramofs) = c;
		}
	}
	else
	{

		if (y1>y2)
		{
			n = y1; y1 = y2; y2 = n;
			n = x1; x1 = x2; x2 = n;
		}

        l = ((dx<<fp)*div_tbl[dy+2048])>>16;
        x00 = x1<<fp;

		for (y=y1; y<y2; y++)
		{
			vramofs = y + ((x00 += l)>>fp) * GP32_Height;
            if (vramofs>=0 && vramofs<Scr_Size)  * (buffer + vramofs) = c;
		}

	}
}


void drawline16bit (line2d line, unsigned short *buffer)
{

	int x1 = spts[line.p0].x+1;
	int y1 = spts[line.p0].y;
	int x2 = spts[line.p1].x+1;
	int y2 = spts[line.p1].y;
	int c =  (31<<11) | (63<<5) | (31<<0);

	int dx, dy, n, l;
	int x00, y00;
	int fp = 12;
	int vramofs;

	int x, y;

	dx = x2 - x1;
	dy = y2 - y1;

	if (abs(dy) < abs(dx))
	{
		if (x1>x2)
		{
			n = x1; x1 = x2; x2 = n;
			n = y1; y1 = y2; y2 = n;
		}

        l = ((dy<<fp)*div_tbl[dx+2048])>>16;
        y00 = y1<<fp;
		for (x=x1; x<x2; x++)
		{
			vramofs = ((y00 += l)>>fp) + x * GP32_Height;
            if (vramofs>=0 && vramofs<Scr_Size) * (buffer + vramofs) = c;
		}
	}
	else
	{

		if (y1>y2)
		{
			n = y1; y1 = y2; y2 = n;
			n = x1; x1 = x2; x2 = n;
		}

        l = ((dx<<fp)*div_tbl[dy+2048])>>16;
        x00 = x1<<fp;

		for (y=y1; y<y2; y++)
		{
			vramofs = y + ((x00 += l)>>fp) * GP32_Height;
            if (vramofs>=0 && vramofs<Scr_Size)  * (buffer + vramofs) = c;
		}

	}
}


void drawline(line2d line, unsigned short *buffer, int bpp)
{
    if (bpp==8) drawline8bit(line, (unsigned char*)buffer);
        else if (bpp==16) drawline16bit(line, (unsigned short*)buffer);
}

void DrawFlatTriangle (poly2d poly, unsigned short *vram, unsigned short shade[])
{
int x0 =spts[poly.p0].x; int y0 =spts[poly.p0].y;
int x1 =spts[poly.p1].x; int y1 =spts[poly.p1].y;
int x2 =spts[poly.p2].x; int y2 =spts[poly.p2].y;
int c = shade[poly.c];

// ===== Sort =====

int temp;
if (x1<x0)
{
    temp = x0; x0 = x1; x1 = temp;
    temp = y0; y0 = y1; y1 = temp;
}
if (x2<x0)
{
    temp = x0; x0 = x2; x2 = temp;
    temp = y0; y0 = y2; y2 = temp;
}
if (x2<x1)
{
    temp = x1; x1 = x2; x2 = temp;
    temp = y1; y1 = y2; y2 = temp;
}

// ===== Interpolation variables =====

int fp = 8;
int ly01=0, ly12=0, ly02=0, n;

int dx01 = x1 - x0;
int dy01 = y1 - y0;
ly01 = ((dy01<<fp)*div_tbl[dx01+2048])>>16;

int dx12 = x2 - x1;
int dy12 = y2 - y1;
ly12 = ((dy12<<fp)*div_tbl[dx12+2048])>>16;

int dx02 = x2 - x0;
int dy02 = y2 - y0;
ly02 = ((dy02<<fp)*div_tbl[dx02+2048])>>16;

int vramofs;
int x, y;

int y01 = y0<<fp;
int y02 = y01;

int sy1, sy2;

    int xp = x0 * GP32_Height;
    for (x = x0; x<x1; x++)
    {
        sy1 = y01>>fp;
        sy2 = y02>>fp;
        if (sy1>sy2) {temp = sy1; sy1 = sy2; sy2 = temp;}

        xp+=GP32_Height;
        vramofs = xp + sy1;
        for (y = sy1; y<sy2; y++)
        {
            if (vramofs>=0 && vramofs<GP32_Width*GP32_Height) *(vram+vramofs)=c;
            vramofs++;
        }
        y01+=ly01;
        y02+=ly02;
    }

    y01 = y1<<fp;

    xp = x1 * GP32_Height;
    for (x = x1; x<x2; x++)
    {
        sy1 = y01>>fp;
        sy2 = y02>>fp;
        if (sy1>sy2) {temp = sy1; sy1 = sy2; sy2 = temp;}

        xp+=GP32_Height;
        vramofs = xp + sy1;
        for (y = sy1; y<sy2; y++)
        {
            if (vramofs>=0 && vramofs<GP32_Width * GP32_Height) *(vram + vramofs)=c;
            vramofs++;
        }
        y01+=ly12;
        y02+=ly02;
    }
}


void DrawFlatTriangleZB (poly2d poly, unsigned short *vram, unsigned short shade[])
{
int x0 =spts[poly.p0].x; int y0 =spts[poly.p0].y;
int x1 =spts[poly.p1].x; int y1 =spts[poly.p1].y;
int x2 =spts[poly.p2].x; int y2 =spts[poly.p2].y;
int c = shade[poly.c];

int zfp = 0;
int z0 = fpts[poly.p0].z >> zfp;
int z1 = fpts[poly.p1].z >> zfp;
int z2 = fpts[poly.p2].z >> zfp;

// ===== Sort =====

int temp;
if (x1<x0)
{
    temp = x0; x0 = x1; x1 = temp;
    temp = y0; y0 = y1; y1 = temp;
    temp = z0; z0 = z1; z1 = temp;
}
if (x2<x0)
{
    temp = x0; x0 = x2; x2 = temp;
    temp = y0; y0 = y2; y2 = temp;
    temp = z0; z0 = z2; z2 = temp;
}
if (x2<x1)
{
    temp = x1; x1 = x2; x2 = temp;
    temp = y1; y1 = y2; y2 = temp;
    temp = z1; z1 = z2; z2 = temp;
}

// ===== Interpolation variables =====

int n, fp = 8;
int ly01=0, ly12=0, ly02=0;
int lz01=0, lz12=0, lz02=0;


int dx01 = x1 - x0;
int dy01 = y1 - y0;
int dz01 = z1 - z0;

    ly01 = ((dy01<<fp)*div_tbl[dx01+2048])>>16;
    lz01 = (dz01*div_tbl[dx01+2048])>>16;


int dx12 = x2 - x1;
int dy12 = y2 - y1;
int dz12 = z2 - z1;

    ly12 = ((dy12<<fp)*div_tbl[dx12+2048])>>16;
    lz12 = (dz12*div_tbl[dx12+2048])>>16;


int dx02 = x2 - x0;
int dy02 = y2 - y0;
int dz02 = z2 - z0;

    ly02 = ((dy02<<fp)*div_tbl[dx02+2048])>>16;
    lz02 = (dz02*div_tbl[dx02+2048])>>16;


int vramofs;
int x, y;

int y01 = y0<<fp;
int y02 = y01;
int z01 = z0;
int z02 = z01;

int dz;
int sy1, sy2;
int sz1, sz2;

    int xp = x0 * GP32_Height;
    for (x = x0; x<x1; x++)
    {
        sy1 = y01>>fp;
        sy2 = y02>>fp;
        sz1 = z01;
        sz2 = z02;

        if (sy1>sy2)
        {
            temp = sy1; sy1 = sy2; sy2 = temp;
            temp = sz1; sz1 = sz2; sz2 = temp;
        }
        dz = ((sz2 - sz1)*div_tbl[sy2-sy1+2048])>>16;

        xp+=GP32_Height;
        vramofs = xp + sy1;
        for (y = sy1; y<sy2; y++)
        {
            sz1+=dz;
            if ((vramofs>=0 && vramofs<GP32_Width*GP32_Height) && sz1<zbuffer[vramofs])
            {
                zbuffer[vramofs] = sz1;
                *(vram+vramofs)=c;
            }


            vramofs++;
        }
        y01+=ly01;
        y02+=ly02;
        z01+=lz01;
        z02+=lz02;
    }

    y01 = y1<<fp;
    z01 = z1;

    xp = x1 * GP32_Height;
    for (x = x1; x<x2; x++)
    {
        sy1 = y01>>fp;
        sy2 = y02>>fp;
        sz1 = z01;
        sz2 = z02;

        if (sy1>sy2)
        {
            temp = sy1; sy1 = sy2; sy2 = temp;
            temp = sz1; sz1 = sz2; sz2 = temp;
        }
        dz = ((sz2 - sz1)*div_tbl[sy2-sy1+2048])>>16;

        xp+=GP32_Height;
        vramofs = xp + sy1;
        for (y = sy1; y<sy2; y++)
        {
            sz1+=dz;
            if ((vramofs>=0 && vramofs<GP32_Width*GP32_Height) && sz1<zbuffer[vramofs])
            {
                zbuffer[vramofs] = sz1;
                *(vram+vramofs)=c;
            }
            vramofs++;
        }
        y01+=ly12;
        y02+=ly02;
        z01+=lz12;
        z02+=lz02;
    }
}


void DrawFlatWaterTriangle (poly2d poly, unsigned short *vram, unsigned short shade1[], unsigned short shade2[])
{
int x0 =spts[poly.p0].x; int y0 =spts[poly.p0].y;
int x1 =spts[poly.p1].x; int y1 =spts[poly.p1].y;
int x2 =spts[poly.p2].x; int y2 =spts[poly.p2].y;
unsigned int c1 = shade1[poly.c];
unsigned int c2 = shade2[poly.c];

int zfp = 0;
int z0 = fpts[poly.p0].z >> zfp;
int z1 = fpts[poly.p1].z >> zfp;
int z2 = fpts[poly.p2].z >> zfp;

unsigned char *b1 = (unsigned char*)buffer1;

// ===== Sort =====

int temp;
if (x1<x0)
{
    temp = x0; x0 = x1; x1 = temp;
    temp = y0; y0 = y1; y1 = temp;
    temp = z0; z0 = z1; z1 = temp;
}
if (x2<x0)
{
    temp = x0; x0 = x2; x2 = temp;
    temp = y0; y0 = y2; y2 = temp;
    temp = z0; z0 = z2; z2 = temp;
}
if (x2<x1)
{
    temp = x1; x1 = x2; x2 = temp;
    temp = y1; y1 = y2; y2 = temp;
    temp = z1; z1 = z2; z2 = temp;
}

// ===== Interpolation variables =====

int n, fp = 8;
int ly01=0, ly12=0, ly02=0;
int lz01=0, lz12=0, lz02=0;


int dx01 = x1 - x0;
int dy01 = y1 - y0;
int dz01 = z1 - z0;

    ly01 = ((dy01<<fp)*div_tbl[dx01+2048])>>16;
    lz01 = (dz01*div_tbl[dx01+2048])>>16;


int dx12 = x2 - x1;
int dy12 = y2 - y1;
int dz12 = z2 - z1;

    ly12 = ((dy12<<fp)*div_tbl[dx12+2048])>>16;
    lz12 = (dz12*div_tbl[dx12+2048])>>16;


int dx02 = x2 - x0;
int dy02 = y2 - y0;
int dz02 = z2 - z0;

    ly02 = ((dy02<<fp)*div_tbl[dx02+2048])>>16;
    lz02 = (dz02*div_tbl[dx02+2048])>>16;


int vramofs;
int x, y;

int y01 = y0<<fp;
int y02 = y01;
int z01 = z0;
int z02 = z01;

int dz;
int sy1, sy2;
int sz1, sz2;
int zclip1 = 128000, zclip2 = 130000;
unsigned int c;

    int xp = x0 * GP32_Height;
    for (x = x0; x<x1; x++)
    {
        sy1 = y01>>fp;
        sy2 = y02>>fp;
        sz1 = z01;
        sz2 = z02;

        if (sy1>sy2)
        {
            temp = sy1; sy1 = sy2; sy2 = temp;
            temp = sz1; sz1 = sz2; sz2 = temp;
        }
        dz = ((sz2 - sz1)*div_tbl[sy2-sy1+2048])>>16;

        xp+=GP32_Height;
        vramofs = xp + sy1;
        for (y = sy1; y<sy2; y++)
        {
            sz1+=dz;
            if (vramofs>=0 && vramofs<GP32_Width*GP32_Height)
            {
                if (sz1>zclip1)
                {
                    if (sz1<zclip2) *(b1+vramofs)=sz1 - zclip1;
                    c = c2;
                }
                else c = c1;
                *(vram+vramofs)=c;
            }


            vramofs++;
        }
        y01+=ly01;
        y02+=ly02;
        z01+=lz01;
        z02+=lz02;
    }

    y01 = y1<<fp;
    z01 = z1;

    xp = x1 * GP32_Height;
    for (x = x1; x<x2; x++)
    {
        sy1 = y01>>fp;
        sy2 = y02>>fp;
        sz1 = z01;
        sz2 = z02;

        if (sy1>sy2)
        {
            temp = sy1; sy1 = sy2; sy2 = temp;
            temp = sz1; sz1 = sz2; sz2 = temp;
        }
        dz = ((sz2 - sz1)*div_tbl[sy2-sy1+2048])>>16;

        xp+=GP32_Height;
        vramofs = xp + sy1;
        for (y = sy1; y<sy2; y++)
        {
            sz1+=dz;
            if (vramofs>=0 && vramofs<GP32_Width*GP32_Height)
            {
                if (sz1>zclip1)
                {
                    if (sz1<zclip2) *(b1+vramofs)=sz1 - zclip1;
                    c = c2;
                }
                else c = c1;
                *(vram+vramofs)=c;
            }
            vramofs++;
        }
        y01+=ly12;
        y02+=ly02;
        z01+=lz12;
        z02+=lz02;
    }
}


void DrawFlatWaterTriangleZB (poly2d poly, unsigned short *vram, unsigned short shade1[], unsigned short shade2[])
{
int x0 =spts[poly.p0].x; int y0 =spts[poly.p0].y;
int x1 =spts[poly.p1].x; int y1 =spts[poly.p1].y;
int x2 =spts[poly.p2].x; int y2 =spts[poly.p2].y;
unsigned int c1 = shade1[poly.c];
unsigned int c2 = shade2[poly.c];

int zfp = 0;
int z0 = fpts[poly.p0].z >> zfp;
int z1 = fpts[poly.p1].z >> zfp;
int z2 = fpts[poly.p2].z >> zfp;

unsigned char *b1 = (unsigned char*)buffer1;

// ===== Sort =====

int temp;
if (x1<x0)
{
    temp = x0; x0 = x1; x1 = temp;
    temp = y0; y0 = y1; y1 = temp;
    temp = z0; z0 = z1; z1 = temp;
}
if (x2<x0)
{
    temp = x0; x0 = x2; x2 = temp;
    temp = y0; y0 = y2; y2 = temp;
    temp = z0; z0 = z2; z2 = temp;
}
if (x2<x1)
{
    temp = x1; x1 = x2; x2 = temp;
    temp = y1; y1 = y2; y2 = temp;
    temp = z1; z1 = z2; z2 = temp;
}

// ===== Interpolation variables =====

int n, fp = 8;
int ly01=0, ly12=0, ly02=0;
int lz01=0, lz12=0, lz02=0;


int dx01 = x1 - x0;
int dy01 = y1 - y0;
int dz01 = z1 - z0;

    ly01 = ((dy01<<fp)*div_tbl[dx01+2048])>>16;
    lz01 = (dz01*div_tbl[dx01+2048])>>16;


int dx12 = x2 - x1;
int dy12 = y2 - y1;
int dz12 = z2 - z1;

    ly12 = ((dy12<<fp)*div_tbl[dx12+2048])>>16;
    lz12 = (dz12*div_tbl[dx12+2048])>>16;


int dx02 = x2 - x0;
int dy02 = y2 - y0;
int dz02 = z2 - z0;

    ly02 = ((dy02<<fp)*div_tbl[dx02+2048])>>16;
    lz02 = (dz02*div_tbl[dx02+2048])>>16;


int vramofs;
int x, y;

int y01 = y0<<fp;
int y02 = y01;
int z01 = z0;
int z02 = z01;

int dz;
int sy1, sy2;
int sz1, sz2;
int zclip = 128000;
unsigned int c;

    int xp = x0 * GP32_Height;
    for (x = x0; x<x1; x++)
    {
        sy1 = y01>>fp;
        sy2 = y02>>fp;
        sz1 = z01;
        sz2 = z02;

        if (sy1>sy2)
        {
            temp = sy1; sy1 = sy2; sy2 = temp;
            temp = sz1; sz1 = sz2; sz2 = temp;
        }
        dz = ((sz2 - sz1)*div_tbl[sy2-sy1+2048])>>16;

        xp+=GP32_Height;
        vramofs = xp + sy1;
        for (y = sy1; y<sy2; y++)
        {
            sz1+=dz;
            if ((vramofs>=0 && vramofs<GP32_Width*GP32_Height) && sz1<zbuffer[vramofs])
            {
                zbuffer[vramofs] = sz1;
                if (zbuffer[vramofs]>zclip)
                {
                    *(b1+vramofs)=zbuffer[vramofs] - zclip;
                    c = c2;
                }
                else c = c1;
                *(vram+vramofs)=c;
            }


            vramofs++;
        }
        y01+=ly01;
        y02+=ly02;
        z01+=lz01;
        z02+=lz02;
    }

    y01 = y1<<fp;
    z01 = z1;

    xp = x1 * GP32_Height;
    for (x = x1; x<x2; x++)
    {
        sy1 = y01>>fp;
        sy2 = y02>>fp;
        sz1 = z01;
        sz2 = z02;

        if (sy1>sy2)
        {
            temp = sy1; sy1 = sy2; sy2 = temp;
            temp = sz1; sz1 = sz2; sz2 = temp;
        }
        dz = ((sz2 - sz1)*div_tbl[sy2-sy1+2048])>>16;

        xp+=GP32_Height;
        vramofs = xp + sy1;
        for (y = sy1; y<sy2; y++)
        {
            sz1+=dz;
            if ((vramofs>=0 && vramofs<GP32_Width*GP32_Height) && sz1<zbuffer[vramofs])
            {
                zbuffer[vramofs] = sz1;
                if (zbuffer[vramofs]>zclip)
                {
                    *(b1+vramofs)=zbuffer[vramofs] - zclip;
                    c = c2;
                }
                else c = c1;
                *(vram+vramofs)=c;
            }
            vramofs++;
        }
        y01+=ly12;
        y02+=ly02;
        z01+=lz12;
        z02+=lz02;
    }
}


void DrawGouraudTriangle (poly2d poly, unsigned short *vram, unsigned short shade[])
{
int x0 =spts[poly.p0].x; int y0 =spts[poly.p0].y;
int x1 =spts[poly.p1].x; int y1 =spts[poly.p1].y;
int x2 =spts[poly.p2].x; int y2 =spts[poly.p2].y;

int c;
int c0 = spts[poly.p0].c;
int c1 = spts[poly.p1].c;
int c2 = spts[poly.p2].c;

// ===== Sort =====

int temp;
if (x1<x0)
{
    temp = c0; c0 = c1; c1 = temp;
    temp = x0; x0 = x1; x1 = temp;
    temp = y0; y0 = y1; y1 = temp;
}
if (x2<x0)
{
    temp = c0; c0 = c2; c2 = temp;
    temp = x0; x0 = x2; x2 = temp;
    temp = y0; y0 = y2; y2 = temp;
}
if (x2<x1)
{
    temp = c1; c1 = c2; c2 = temp;
    temp = x1; x1 = x2; x2 = temp;
    temp = y1; y1 = y2; y2 = temp;
}

// ===== Interpolation variables =====

int n;
int fp = 8;
int ly01=0, ly12=0, ly02=0;
int lc01=0, lc12=0, lc02=0;

int dx01 = x1 - x0;
int dy01 = y1 - y0;
int dc01 = c1 - c0;
ly01 = ((dy01<<fp)*div_tbl[dx01+2048])>>16;
lc01 = ((dc01<<fp)*div_tbl[dx01+2048])>>16;

int dx12 = x2 - x1;
int dy12 = y2 - y1;
int dc12 = c2 - c1;
ly12 = ((dy12<<fp)*div_tbl[dx12+2048])>>16;
lc12 = ((dc12<<fp)*div_tbl[dx12+2048])>>16;

int dx02 = x2 - x0;
int dy02 = y2 - y0;
int dc02 = c2 - c0;
ly02 = ((dy02<<fp)*div_tbl[dx02+2048])>>16;
lc02 = ((dc02<<fp)*div_tbl[dx02+2048])>>16;

int vramofs;
int x, y;

int y01 = y0<<fp;
int y02 = y01;
int c01 = c0<<fp;
int c02 = c01;

int dc = 0;
int sc1, sc2;

int sy1, sy2;

    int xp = x0 * GP32_Height;
    for (x = x0; x<x1; x++)
    {
        sy1 = y01>>fp;
        sy2 = y02>>fp;
        sc1 = c01;
        sc2 = c02;

        if (sy1>sy2)
        {
            temp = sy1; sy1 = sy2; sy2 = temp;
            temp = sc1; sc1 = sc2; sc2 = temp;
        }
        dc = ((sc2 - sc1)*div_tbl[sy2-sy1+2048])>>16;

        xp+=GP32_Height;
        vramofs = xp + sy1;
        for (y = sy1; y<sy2; y++)
        {
            sc1+=dc;
            c = sc1>>fp;
            if (vramofs>=0 && vramofs<GP32_Width*GP32_Height) *(vram+vramofs) = shade[c];
            vramofs++;
        }
        y01+=ly01;
        y02+=ly02;
        c01+=lc01;
        c02+=lc02;
    }

    y01 = y1<<fp;
    c01 = c1<<fp;

    xp = x1 * GP32_Height;
    for (x = x1; x<x2; x++)
    {
        sy1 = y01>>fp;
        sy2 = y02>>fp;
        sc1 = c01;
        sc2 = c02;

        if (sy1>sy2)
        {
            temp = sy1; sy1 = sy2; sy2 = temp;
            temp = sc1; sc1 = sc2; sc2 = temp;
        }
        dc = ((sc2 - sc1)*div_tbl[sy2-sy1+2048])>>16;

        xp+=GP32_Height;
        vramofs = xp + sy1;
        for (y = sy1; y<sy2; y++)
        {
            sc1+=dc;
            c = sc1>>fp;
            if (vramofs>=0 && vramofs<GP32_Width*GP32_Height) *(vram+vramofs) = shade[c];
            vramofs++;
        }
        y01+=ly12;
        y02+=ly02;
        c01+=lc12;
        c02+=lc02;
    }
}


void DrawGouraudTriangleZB (poly2d poly, unsigned short *vram, unsigned short shade[])
{
int x0 =spts[poly.p0].x; int y0 =spts[poly.p0].y;
int x1 =spts[poly.p1].x; int y1 =spts[poly.p1].y;
int x2 =spts[poly.p2].x; int y2 =spts[poly.p2].y;

int c;
int c0 = spts[poly.p0].c;
int c1 = spts[poly.p1].c;
int c2 = spts[poly.p2].c;

int zfp = 0;
int z0 = fpts[poly.p0].z >> zfp;
int z1 = fpts[poly.p1].z >> zfp;
int z2 = fpts[poly.p2].z >> zfp;

// ===== Sort =====

int temp;
if (x1<x0)
{
    temp = c0; c0 = c1; c1 = temp;
    temp = z0; z0 = z1; z1 = temp;
    temp = x0; x0 = x1; x1 = temp;
    temp = y0; y0 = y1; y1 = temp;
}
if (x2<x0)
{
    temp = c0; c0 = c2; c2 = temp;
    temp = z0; z0 = z2; z2 = temp;
    temp = x0; x0 = x2; x2 = temp;
    temp = y0; y0 = y2; y2 = temp;
}
if (x2<x1)
{
    temp = c1; c1 = c2; c2 = temp;
    temp = z1; z1 = z2; z2 = temp;
    temp = x1; x1 = x2; x2 = temp;
    temp = y1; y1 = y2; y2 = temp;
}

// ===== Interpolation variables =====

int n;
int fp = 8;
int ly01=0, ly12=0, ly02=0;
int lc01=0, lc12=0, lc02=0;
int lz01=0, lz12=0, lz02=0;

int dx01 = x1 - x0;
int dy01 = y1 - y0;
int dc01 = c1 - c0;
int dz01 = z1 - z0;

    ly01 = ((dy01<<fp)*div_tbl[dx01+2048])>>16;
    lc01 = ((dc01<<fp)*div_tbl[dx01+2048])>>16;
    lz01 = (dz01*div_tbl[dx01+2048])>>16;

int dx12 = x2 - x1;
int dy12 = y2 - y1;
int dc12 = c2 - c1;
int dz12 = z2 - z1;

    ly12 = ((dy12<<fp)*div_tbl[dx12+2048])>>16;
    lc12 = ((dc12<<fp)*div_tbl[dx12+2048])>>16;
    lz12 = (dz12*div_tbl[dx12+2048])>>16;

int dx02 = x2 - x0;
int dy02 = y2 - y0;
int dc02 = c2 - c0;
int dz02 = z2 - z0;

    ly02 = ((dy02<<fp)*div_tbl[dx02+2048])>>16;
    lc02 = ((dc02<<fp)*div_tbl[dx02+2048])>>16;
    lz02 = (dz02*div_tbl[dx02+2048])>>16;

int vramofs;
int x, y;

int y01 = y0<<fp;
int y02 = y01;
int c01 = c0<<fp;
int c02 = c01;
int z01 = z0;
int z02 = z01;

int dc, dz;
int sc1, sc2;
int sy1, sy2;
int sz1, sz2;

    int xp = x0 * GP32_Height;
    for (x = x0; x<x1; x++)
    {
        sy1 = y01>>fp;
        sy2 = y02>>fp;
        sc1 = c01;
        sc2 = c02;
        sz1 = z01;
        sz2 = z02;

        if (sy1>sy2)
        {
            temp = sy1; sy1 = sy2; sy2 = temp;
            temp = sc1; sc1 = sc2; sc2 = temp;
            temp = sz1; sz1 = sz2; sz2 = temp;
        }

            dc = ((sc2 - sc1)*div_tbl[sy2-sy1+2048])>>16;
            dz = ((sz2 - sz1)*div_tbl[sy2-sy1+2048])>>16;

        xp+=GP32_Height;
        vramofs = xp + sy1;
        for (y = sy1; y<sy2; y++)
        {
            sc1+=dc;
            sz1+=dz;
            if ((vramofs>=0 && vramofs<GP32_Width*GP32_Height) && sz1<zbuffer[vramofs])
            {
                c = sc1>>fp;
                if (c<0) c=0;
                zbuffer[vramofs] = sz1;
                *(vram+vramofs)=shade[c];
            }
            vramofs++;
        }
        y01+=ly01;
        y02+=ly02;
        c01+=lc01;
        c02+=lc02;
        z01+=lz01;
        z02+=lz02;
    }

    y01 = y1<<fp;
    c01 = c1<<fp;
    z01 = z1;

    xp = x1 * GP32_Height;
    for (x = x1; x<x2; x++)
    {
        sy1 = y01>>fp;
        sy2 = y02>>fp;
        sc1 = c01;
        sc2 = c02;
        sz1 = z01;
        sz2 = z02;

        if (sy1>sy2)
        {
            temp = sy1; sy1 = sy2; sy2 = temp;
            temp = sc1; sc1 = sc2; sc2 = temp;
            temp = sz1; sz1 = sz2; sz2 = temp;
        }

            dc = ((sc2 - sc1)*div_tbl[sy2-sy1+2048])>>16;
            dz = ((sz2 - sz1)*div_tbl[sy2-sy1+2048])>>16;

        xp+=GP32_Height;
        vramofs = xp + sy1;
        for (y = sy1; y<sy2; y++)
        {
            sc1+=dc;
            sz1+=dz;
            if ((vramofs>=0 && vramofs<GP32_Width*GP32_Height) && sz1<zbuffer[vramofs])
            {
                c = sc1>>fp;
                if (c<0) c=0;
                zbuffer[vramofs] = sz1;
                *(vram+vramofs)=shade[c];
            }
            vramofs++;
        }
        y01+=ly12;
        y02+=ly02;
        c01+=lc12;
        c02+=lc02;
        z01+=lz12;
        z02+=lz02;
    }
}


void DrawTextureTriangle(poly2d poly, unsigned short *vram, int tshr, unsigned short texture[])
{
int x0 =spts[poly.p0].x; int y0 =spts[poly.p0].y;
int x1 =spts[poly.p1].x; int y1 =spts[poly.p1].y;
int x2 =spts[poly.p2].x; int y2 =spts[poly.p2].y;

int c;

int u0 = poly.tc0.u; int v0 = poly.tc0.v;
int u1 = poly.tc1.u; int v1 = poly.tc1.v;
int u2 = poly.tc2.u; int v2 = poly.tc2.v;

// ===== Sort =====

int temp;
if (x1<x0)
{
    temp = u0; u0 = u1; u1 = temp;
    temp = v0; v0 = v1; v1 = temp;
    temp = x0; x0 = x1; x1 = temp;
    temp = y0; y0 = y1; y1 = temp;
}
if (x2<x0)
{
    temp = u0; u0 = u2; u2 = temp;
    temp = v0; v0 = v2; v2 = temp;
    temp = x0; x0 = x2; x2 = temp;
    temp = y0; y0 = y2; y2 = temp;
}
if (x2<x1)
{
    temp = u1; u1 = u2; u2 = temp;
    temp = v1; v1 = v2; v2 = temp;
    temp = x1; x1 = x2; x2 = temp;
    temp = y1; y1 = y2; y2 = temp;
}

// ===== Interpolation variables =====

int n;
int fp = 8;
int ly01=0, ly12=0, ly02=0;
int lu01=0, lu12=0, lu02=0;
int lv01=0, lv12=0, lv02=0;

int dx01 = x1 - x0;
int dy01 = y1 - y0;
int du01 = u1 - u0;
int dv01 = v1 - v0;

    ly01 = ((dy01<<fp)*div_tbl[dx01+2048])>>16;
    lu01 = ((du01<<fp)*div_tbl[dx01+2048])>>16;
    lv01 = ((dv01<<fp)*div_tbl[dx01+2048])>>16;

int dx12 = x2 - x1;
int dy12 = y2 - y1;
int du12 = u2 - u1;
int dv12 = v2 - v1;

    ly12 = ((dy12<<fp)*div_tbl[dx12+2048])>>16;
    lu12 = ((du12<<fp)*div_tbl[dx12+2048])>>16;
    lv12 = ((dv12<<fp)*div_tbl[dx12+2048])>>16;

int dx02 = x2 - x0;
int dy02 = y2 - y0;
int du02 = u2 - u0;
int dv02 = v2 - v0;

    ly02 = ((dy02<<fp)*div_tbl[dx02+2048])>>16;
    lu02 = ((du02<<fp)*div_tbl[dx02+2048])>>16;
    lv02 = ((dv02<<fp)*div_tbl[dx02+2048])>>16;

int vramofs;
int x, y;

int y01 = y0<<fp;
int y02 = y01;
int u01 = u0<<fp;
int u02 = u01;
int v01 = v0<<fp;
int v02 = v01;

int du, dv;
int su1, su2;
int sv1, sv2;
int sy1, sy2;

int px, py;

    int xp = x0 * GP32_Height;
    for (x = x0; x<x1; x++)
    {
        sy1 = y01>>fp;
        sy2 = y02>>fp;
        su1 = u01;
        su2 = u02;
        sv1 = v01;
        sv2 = v02;

        if (sy1>sy2)
        {
            temp = sy1; sy1 = sy2; sy2 = temp;
            temp = su1; su1 = su2; su2 = temp;
            temp = sv1; sv1 = sv2; sv2 = temp;
        }

            du = ((su2 - su1)*div_tbl[sy2-sy1+2048])>>16;
            dv = ((sv2 - sv1)*div_tbl[sy2-sy1+2048])>>16;

        xp+=GP32_Height;
        vramofs = xp + sy1;
        for (y = sy1; y<sy2; y++)
        {
            su1+=du;
            sv1+=dv;
            if (vramofs>=0 && vramofs<GP32_Width*GP32_Height)
            {
                c = texture[((su1>>fp)>>tshr) + (((sv1>>fp)>>tshr)<<(8-tshr))];
                *(vram+vramofs) = c;
            }
            vramofs++;
        }
        y01+=ly01;
        y02+=ly02;
        u01+=lu01;
        u02+=lu02;
        v01+=lv01;
        v02+=lv02;
    }

    y01 = y1<<fp;
    u01 = u1<<fp;
    v01 = v1<<fp;

    xp = x1 * GP32_Height;
    for (x = x1; x<x2; x++)
    {
        sy1 = y01>>fp;
        sy2 = y02>>fp;
        su1 = u01;
        su2 = u02;
        sv1 = v01;
        sv2 = v02;

        if (sy1>sy2)
        {
            temp = sy1; sy1 = sy2; sy2 = temp;
            temp = su1; su1 = su2; su2 = temp;
            temp = sv1; sv1 = sv2; sv2 = temp;
        }

            du = ((su2 - su1)*div_tbl[sy2-sy1+2048])>>16;
            dv = ((sv2 - sv1)*div_tbl[sy2-sy1+2048])>>16;

        xp+=GP32_Height;
        vramofs = xp + sy1;
        for (y = sy1; y<sy2; y++)
        {
            su1+=du;
            sv1+=dv;
            if (vramofs>=0 && vramofs<GP32_Width*GP32_Height)
            {
                c = texture[((su1>>fp)>>tshr) + (((sv1>>fp)>>tshr)<<(8-tshr))];
                *(vram+vramofs) = c;
            }
            vramofs++;
        }
        y01+=ly12;
        y02+=ly02;
        u01+=lu12;
        u02+=lu02;
        v01+=lv12;
        v02+=lv02;
    }
}

void DrawTextureTriangleZB(poly2d poly, unsigned short *vram, int tshr, unsigned short texture[])
{
int x0 =spts[poly.p0].x; int y0 =spts[poly.p0].y;
int x1 =spts[poly.p1].x; int y1 =spts[poly.p1].y;
int x2 =spts[poly.p2].x; int y2 =spts[poly.p2].y;

int c;

int u0 = poly.tc0.u; int v0 = poly.tc0.v;
int u1 = poly.tc1.u; int v1 = poly.tc1.v;
int u2 = poly.tc2.u; int v2 = poly.tc2.v;

int zfp = 0;
int z0 = fpts[poly.p0].z >> zfp;
int z1 = fpts[poly.p1].z >> zfp;
int z2 = fpts[poly.p2].z >> zfp;

// ===== Sort =====

int temp;
if (x1<x0)
{
    temp = u0; u0 = u1; u1 = temp;
    temp = v0; v0 = v1; v1 = temp;
    temp = z0; z0 = z1; z1 = temp;
    temp = x0; x0 = x1; x1 = temp;
    temp = y0; y0 = y1; y1 = temp;
}
if (x2<x0)
{
    temp = u0; u0 = u2; u2 = temp;
    temp = v0; v0 = v2; v2 = temp;
    temp = z0; z0 = z2; z2 = temp;
    temp = x0; x0 = x2; x2 = temp;
    temp = y0; y0 = y2; y2 = temp;
}
if (x2<x1)
{
    temp = u1; u1 = u2; u2 = temp;
    temp = v1; v1 = v2; v2 = temp;
    temp = z1; z1 = z2; z2 = temp;
    temp = x1; x1 = x2; x2 = temp;
    temp = y1; y1 = y2; y2 = temp;
}

// ===== Interpolation variables =====

int n;
int fp = 8;
int ly01=0, ly12=0, ly02=0;
int lu01=0, lu12=0, lu02=0;
int lv01=0, lv12=0, lv02=0;
int lz01=0, lz12=0, lz02=0;

int dx01 = x1 - x0;
int dy01 = y1 - y0;
int du01 = u1 - u0;
int dv01 = v1 - v0;
int dz01 = z1 - z0;

    ly01 = ((dy01<<fp)*div_tbl[dx01+2048])>>16;
    lu01 = ((du01<<fp)*div_tbl[dx01+2048])>>16;
    lv01 = ((dv01<<fp)*div_tbl[dx01+2048])>>16;
    lz01 = (dz01*div_tbl[dx01+2048])>>16;

int dx12 = x2 - x1;
int dy12 = y2 - y1;
int du12 = u2 - u1;
int dv12 = v2 - v1;
int dz12 = z2 - z1;

    ly12 = ((dy12<<fp)*div_tbl[dx12+2048])>>16;
    lu12 = ((du12<<fp)*div_tbl[dx12+2048])>>16;
    lv12 = ((dv12<<fp)*div_tbl[dx12+2048])>>16;
    lz12 = (dz12*div_tbl[dx12+2048])>>16;

int dx02 = x2 - x0;
int dy02 = y2 - y0;
int du02 = u2 - u0;
int dv02 = v2 - v0;
int dz02 = z2 - z0;

    ly02 = ((dy02<<fp)*div_tbl[dx02+2048])>>16;
    lu02 = ((du02<<fp)*div_tbl[dx02+2048])>>16;
    lv02 = ((dv02<<fp)*div_tbl[dx02+2048])>>16;
    lz02 = (dz02*div_tbl[dx02+2048])>>16;

int vramofs;
int x, y;

int y01 = y0<<fp;
int y02 = y01;
int u01 = u0<<fp;
int u02 = u01;
int v01 = v0<<fp;
int v02 = v01;
int z01 = z0;
int z02 = z01;

int du, dv, dz;
int su1, su2;
int sv1, sv2;
int sy1, sy2;
int sz1, sz2;

int px, py;

    int xp = x0 * GP32_Height;
    for (x = x0; x<x1; x++)
    {
        sy1 = y01>>fp;
        sy2 = y02>>fp;
        su1 = u01;
        su2 = u02;
        sv1 = v01;
        sv2 = v02;
        sz1 = z01;
        sz2 = z02;

        if (sy1>sy2)
        {
            temp = sy1; sy1 = sy2; sy2 = temp;
            temp = su1; su1 = su2; su2 = temp;
            temp = sv1; sv1 = sv2; sv2 = temp;
            temp = sz1; sz1 = sz2; sz2 = temp;
        }

            du = ((su2 - su1)*div_tbl[sy2-sy1+2048])>>16;
            dv = ((sv2 - sv1)*div_tbl[sy2-sy1+2048])>>16;
            dz = ((sz2 - sz1)*div_tbl[sy2-sy1+2048])>>16;

        xp+=GP32_Height;
        vramofs = xp + sy1;
        for (y = sy1; y<sy2; y++)
        {
            su1+=du;
            sv1+=dv;
            sz1+=dz;
            if ((vramofs>=0 && vramofs<GP32_Width*GP32_Height) && sz1<zbuffer[vramofs])
            {
                zbuffer[vramofs] = sz1;
                c = texture[((su1>>fp)>>tshr) + (((sv1>>fp)>>tshr)<<(8-tshr))];
                *(vram+vramofs) = c;
            }
            vramofs++;
        }
        y01+=ly01;
        y02+=ly02;
        u01+=lu01;
        u02+=lu02;
        v01+=lv01;
        v02+=lv02;
        z01+=lz01;
        z02+=lz02;
    }

    y01 = y1<<fp;
    u01 = u1<<fp;
    v01 = v1<<fp;
    z01 = z1;

    xp = x1 * GP32_Height;
    for (x = x1; x<x2; x++)
    {
        sy1 = y01>>fp;
        sy2 = y02>>fp;
        su1 = u01;
        su2 = u02;
        sv1 = v01;
        sv2 = v02;
        sz1 = z01;
        sz2 = z02;

        if (sy1>sy2)
        {
            temp = sy1; sy1 = sy2; sy2 = temp;
            temp = su1; su1 = su2; su2 = temp;
            temp = sv1; sv1 = sv2; sv2 = temp;
            temp = sz1; sz1 = sz2; sz2 = temp;
        }

            du = ((su2 - su1)*div_tbl[sy2-sy1+2048])>>16;
            dv = ((sv2 - sv1)*div_tbl[sy2-sy1+2048])>>16;
            dz = ((sz2 - sz1)*div_tbl[sy2-sy1+2048])>>16;

        xp+=GP32_Height;
        vramofs = xp + sy1;
        for (y = sy1; y<sy2; y++)
        {
            su1+=du;
            sv1+=dv;
            sz1+=dz;
            if ((vramofs>=0 && vramofs<GP32_Width*GP32_Height) && sz1<zbuffer[vramofs])
            {
                zbuffer[vramofs] = sz1;
                c = texture[((su1>>fp)>>tshr) + (((sv1>>fp)>>tshr)<<(8-tshr))];
                *(vram+vramofs) = c;
            }
            vramofs++;
        }
        y01+=ly12;
        y02+=ly02;
        u01+=lu12;
        u02+=lu02;
        v01+=lv12;
        v02+=lv02;
        z01+=lz12;
        z02+=lz02;
    }
}


void DrawTextureTriangle_plasma(poly2d poly, unsigned short *vram, unsigned short shade[])
{
int x0 =spts[poly.p0].x; int y0 =spts[poly.p0].y;
int x1 =spts[poly.p1].x; int y1 =spts[poly.p1].y;
int x2 =spts[poly.p2].x; int y2 =spts[poly.p2].y;

int c;

int u0 = poly.tc0.u; int v0 = poly.tc0.v;
int u1 = poly.tc1.u; int v1 = poly.tc1.v;
int u2 = poly.tc2.u; int v2 = poly.tc2.v;

// ===== Sort =====

int temp;
if (x1<x0)
{
    temp = u0; u0 = u1; u1 = temp;
    temp = v0; v0 = v1; v1 = temp;
    temp = x0; x0 = x1; x1 = temp;
    temp = y0; y0 = y1; y1 = temp;
}
if (x2<x0)
{
    temp = u0; u0 = u2; u2 = temp;
    temp = v0; v0 = v2; v2 = temp;
    temp = x0; x0 = x2; x2 = temp;
    temp = y0; y0 = y2; y2 = temp;
}
if (x2<x1)
{
    temp = u1; u1 = u2; u2 = temp;
    temp = v1; v1 = v2; v2 = temp;
    temp = x1; x1 = x2; x2 = temp;
    temp = y1; y1 = y2; y2 = temp;
}

// ===== Interpolation variables =====

int n;
int fp = 8;
int ly01=0, ly12=0, ly02=0;
int lu01=0, lu12=0, lu02=0;
int lv01=0, lv12=0, lv02=0;

int dx01 = x1 - x0;
int dy01 = y1 - y0;
int du01 = u1 - u0;
int dv01 = v1 - v0;

    ly01 = ((dy01<<fp)*div_tbl[dx01+2048])>>16;
    lu01 = ((du01<<fp)*div_tbl[dx01+2048])>>16;
    lv01 = ((dv01<<fp)*div_tbl[dx01+2048])>>16;

int dx12 = x2 - x1;
int dy12 = y2 - y1;
int du12 = u2 - u1;
int dv12 = v2 - v1;

    ly12 = ((dy12<<fp)*div_tbl[dx12+2048])>>16;
    lu12 = ((du12<<fp)*div_tbl[dx12+2048])>>16;
    lv12 = ((dv12<<fp)*div_tbl[dx12+2048])>>16;

int dx02 = x2 - x0;
int dy02 = y2 - y0;
int du02 = u2 - u0;
int dv02 = v2 - v0;

    ly02 = ((dy02<<fp)*div_tbl[dx02+2048])>>16;
    lu02 = ((du02<<fp)*div_tbl[dx02+2048])>>16;
    lv02 = ((dv02<<fp)*div_tbl[dx02+2048])>>16;

int vramofs;
int x, y;

int y01 = y0<<fp;
int y02 = y01;
int u01 = u0<<fp;
int u02 = u01;
int v01 = v0<<fp;
int v02 = v01;

int du, dv;
int su1, su2;
int sv1, sv2;
int sy1, sy2;

int px, py;

    int xp = x0 * GP32_Height;
    for (x = x0; x<x1; x++)
    {
        sy1 = y01>>fp;
        sy2 = y02>>fp;
        su1 = u01;
        su2 = u02;
        sv1 = v01;
        sv2 = v02;

        if (sy1>sy2)
        {
            temp = sy1; sy1 = sy2; sy2 = temp;
            temp = su1; su1 = su2; su2 = temp;
            temp = sv1; sv1 = sv2; sv2 = temp;
        }

            du = ((su2 - su1)*div_tbl[sy2-sy1+2048])>>16;
            dv = ((sv2 - sv1)*div_tbl[sy2-sy1+2048])>>16;

        xp+=GP32_Height;
        vramofs = xp + sy1;
        for (y = sy1; y<sy2; y++)
        {
            su1+=du;
            sv1+=dv;
            if (vramofs>=0 && vramofs<GP32_Width*GP32_Height)
            {
                px = su1>>fp; py = sv1>>fp;
                if (px<0) px = 0; if (py<0) py = 0;
                c = (fsin6[px] + fsin7[py] + fsin8[px + py+tm] + fsin7[fsin6[px] + fsin8[px+py+tm]]) & 255;
                *(vram+vramofs)=shade[c];
            }
            vramofs++;
        }
        y01+=ly01;
        y02+=ly02;
        u01+=lu01;
        u02+=lu02;
        v01+=lv01;
        v02+=lv02;
    }

    y01 = y1<<fp;
    u01 = u1<<fp;
    v01 = v1<<fp;

    xp = x1 * GP32_Height;
    for (x = x1; x<x2; x++)
    {
        sy1 = y01>>fp;
        sy2 = y02>>fp;
        su1 = u01;
        su2 = u02;
        sv1 = v01;
        sv2 = v02;

        if (sy1>sy2)
        {
            temp = sy1; sy1 = sy2; sy2 = temp;
            temp = su1; su1 = su2; su2 = temp;
            temp = sv1; sv1 = sv2; sv2 = temp;
        }

            du = ((su2 - su1)*div_tbl[sy2-sy1+2048])>>16;
            dv = ((sv2 - sv1)*div_tbl[sy2-sy1+2048])>>16;

        xp+=GP32_Height;
        vramofs = xp + sy1;
        for (y = sy1; y<sy2; y++)
        {
            su1+=du;
            sv1+=dv;
            if (vramofs>=0 && vramofs<GP32_Width*GP32_Height)
            {
                px = su1>>fp; py = sv1>>fp;
                if (px<0) px = 0; if (py<0) py = 0;
                c = (fsin6[px] + fsin7[py] + fsin8[px + py+tm] + fsin7[fsin6[px] + fsin8[px+py+tm]]) & 255;
                *(vram+vramofs)=shade[c];
            }
            vramofs++;
        }
        y01+=ly12;
        y02+=ly02;
        u01+=lu12;
        u02+=lu02;
        v01+=lv12;
        v02+=lv02;
    }
}


void DrawTextureTriangleZB_plasma(poly2d poly, unsigned short *vram, unsigned short shade[])
{
int x0 =spts[poly.p0].x; int y0 =spts[poly.p0].y;
int x1 =spts[poly.p1].x; int y1 =spts[poly.p1].y;
int x2 =spts[poly.p2].x; int y2 =spts[poly.p2].y;

int c;

int u0 = poly.tc0.u; int v0 = poly.tc0.v;
int u1 = poly.tc1.u; int v1 = poly.tc1.v;
int u2 = poly.tc2.u; int v2 = poly.tc2.v;

int zfp = 0;
int z0 = fpts[poly.p0].z >> zfp;
int z1 = fpts[poly.p1].z >> zfp;
int z2 = fpts[poly.p2].z >> zfp;

// ===== Sort =====

int temp;
if (x1<x0)
{
    temp = u0; u0 = u1; u1 = temp;
    temp = v0; v0 = v1; v1 = temp;
    temp = z0; z0 = z1; z1 = temp;
    temp = x0; x0 = x1; x1 = temp;
    temp = y0; y0 = y1; y1 = temp;
}
if (x2<x0)
{
    temp = u0; u0 = u2; u2 = temp;
    temp = v0; v0 = v2; v2 = temp;
    temp = z0; z0 = z2; z2 = temp;
    temp = x0; x0 = x2; x2 = temp;
    temp = y0; y0 = y2; y2 = temp;
}
if (x2<x1)
{
    temp = u1; u1 = u2; u2 = temp;
    temp = v1; v1 = v2; v2 = temp;
    temp = z1; z1 = z2; z2 = temp;
    temp = x1; x1 = x2; x2 = temp;
    temp = y1; y1 = y2; y2 = temp;
}

// ===== Interpolation variables =====

int n;
int fp = 8;
int ly01=0, ly12=0, ly02=0;
int lu01=0, lu12=0, lu02=0;
int lv01=0, lv12=0, lv02=0;
int lz01=0, lz12=0, lz02=0;

int dx01 = x1 - x0;
int dy01 = y1 - y0;
int du01 = u1 - u0;
int dv01 = v1 - v0;
int dz01 = z1 - z0;

    ly01 = ((dy01<<fp)*div_tbl[dx01+2048])>>16;
    lu01 = ((du01<<fp)*div_tbl[dx01+2048])>>16;
    lv01 = ((dv01<<fp)*div_tbl[dx01+2048])>>16;
    lz01 = (dz01*div_tbl[dx01+2048])>>16;

int dx12 = x2 - x1;
int dy12 = y2 - y1;
int du12 = u2 - u1;
int dv12 = v2 - v1;
int dz12 = z2 - z1;

    ly12 = ((dy12<<fp)*div_tbl[dx12+2048])>>16;
    lu12 = ((du12<<fp)*div_tbl[dx12+2048])>>16;
    lv12 = ((dv12<<fp)*div_tbl[dx12+2048])>>16;
    lz12 = (dz12*div_tbl[dx12+2048])>>16;

int dx02 = x2 - x0;
int dy02 = y2 - y0;
int du02 = u2 - u0;
int dv02 = v2 - v0;
int dz02 = z2 - z0;

    ly02 = ((dy02<<fp)*div_tbl[dx02+2048])>>16;
    lu02 = ((du02<<fp)*div_tbl[dx02+2048])>>16;
    lv02 = ((dv02<<fp)*div_tbl[dx02+2048])>>16;
    lz02 = (dz02*div_tbl[dx02+2048])>>16;

int vramofs;
int x, y;

int y01 = y0<<fp;
int y02 = y01;
int u01 = u0<<fp;
int u02 = u01;
int v01 = v0<<fp;
int v02 = v01;
int z01 = z0;
int z02 = z01;

int du, dv, dz;
int su1, su2;
int sv1, sv2;
int sy1, sy2;
int sz1, sz2;

int px, py;

    int xp = x0 * GP32_Height;
    for (x = x0; x<x1; x++)
    {
        sy1 = y01>>fp;
        sy2 = y02>>fp;
        su1 = u01;
        su2 = u02;
        sv1 = v01;
        sv2 = v02;
        sz1 = z01;
        sz2 = z02;

        if (sy1>sy2)
        {
            temp = sy1; sy1 = sy2; sy2 = temp;
            temp = su1; su1 = su2; su2 = temp;
            temp = sv1; sv1 = sv2; sv2 = temp;
            temp = sz1; sz1 = sz2; sz2 = temp;
        }

            du = ((su2 - su1)*div_tbl[sy2-sy1+2048])>>16;
            dv = ((sv2 - sv1)*div_tbl[sy2-sy1+2048])>>16;
            dz = ((sz2 - sz1)*div_tbl[sy2-sy1+2048])>>16;

        xp+=GP32_Height;
        vramofs = xp + sy1;
        for (y = sy1; y<sy2; y++)
        {
            su1+=du;
            sv1+=dv;
            sz1+=dz;
            if ((vramofs>=0 && vramofs<GP32_Width*GP32_Height) && sz1<zbuffer[vramofs])
            {
                px = su1>>fp; py = sv1>>fp;
                if (px<0) px = 0; if (py<0) py = 0;
                c = (fsin6[px] + fsin7[py+tm] + fsin8[px + py+tm+tm] + fsin7[fsin6[px] + fsin8[px+py+tm] + tm]) & 255;
                zbuffer[vramofs] = sz1;
                *(vram+vramofs)=shade[c];
            }
            vramofs++;
        }
        y01+=ly01;
        y02+=ly02;
        u01+=lu01;
        u02+=lu02;
        v01+=lv01;
        v02+=lv02;
        z01+=lz01;
        z02+=lz02;
    }

    y01 = y1<<fp;
    u01 = u1<<fp;
    v01 = v1<<fp;
    z01 = z1;

    xp = x1 * GP32_Height;
    for (x = x1; x<x2; x++)
    {
        sy1 = y01>>fp;
        sy2 = y02>>fp;
        su1 = u01;
        su2 = u02;
        sv1 = v01;
        sv2 = v02;
        sz1 = z01;
        sz2 = z02;

        if (sy1>sy2)
        {
            temp = sy1; sy1 = sy2; sy2 = temp;
            temp = su1; su1 = su2; su2 = temp;
            temp = sv1; sv1 = sv2; sv2 = temp;
            temp = sz1; sz1 = sz2; sz2 = temp;
        }

            du = ((su2 - su1)*div_tbl[sy2-sy1+2048])>>16;
            dv = ((sv2 - sv1)*div_tbl[sy2-sy1+2048])>>16;
            dz = ((sz2 - sz1)*div_tbl[sy2-sy1+2048])>>16;

        xp+=GP32_Height;
        vramofs = xp + sy1;
        for (y = sy1; y<sy2; y++)
        {
            su1+=du;
            sv1+=dv;
            sz1+=dz;
            if ((vramofs>=0 && vramofs<GP32_Width*GP32_Height) && sz1<zbuffer[vramofs])
            {
                px = su1>>fp; py = sv1>>fp;
                if (px<0) px = 0; if (py<0) py = 0;
                c = (fsin6[px] + fsin7[py+tm] + fsin8[px + py+tm+tm] + fsin7[fsin6[px] + fsin8[px+py+tm] + tm]) & 255;
                zbuffer[vramofs] = sz1;
                *(vram+vramofs)=shade[c];
            }
            vramofs++;
        }
        y01+=ly12;
        y02+=ly02;
        u01+=lu12;
        u02+=lu02;
        v01+=lv12;
        v02+=lv02;
        z01+=lz12;
        z02+=lz02;
    }
}


void DrawEnvmappedTriangle(poly2d poly, unsigned short *vram, int tshr, unsigned short texture[])
{
int x0 =spts[poly.p0].x; int y0 =spts[poly.p0].y;
int x1 =spts[poly.p1].x; int y1 =spts[poly.p1].y;
int x2 =spts[poly.p2].x; int y2 =spts[poly.p2].y;

int c;

int u0 = point_tc[poly.p0].u; int v0 = point_tc[poly.p0].v;
int u1 = point_tc[poly.p1].u; int v1 = point_tc[poly.p1].v;
int u2 = point_tc[poly.p2].u; int v2 = point_tc[poly.p2].v;

// ===== Sort =====

int temp;
if (x1<x0)
{
    temp = u0; u0 = u1; u1 = temp;
    temp = v0; v0 = v1; v1 = temp;
    temp = x0; x0 = x1; x1 = temp;
    temp = y0; y0 = y1; y1 = temp;
}
if (x2<x0)
{
    temp = u0; u0 = u2; u2 = temp;
    temp = v0; v0 = v2; v2 = temp;
    temp = x0; x0 = x2; x2 = temp;
    temp = y0; y0 = y2; y2 = temp;
}
if (x2<x1)
{
    temp = u1; u1 = u2; u2 = temp;
    temp = v1; v1 = v2; v2 = temp;
    temp = x1; x1 = x2; x2 = temp;
    temp = y1; y1 = y2; y2 = temp;
}

// ===== Interpolation variables =====

int n;
int fp = 8;
int ly01=0, ly12=0, ly02=0;
int lu01=0, lu12=0, lu02=0;
int lv01=0, lv12=0, lv02=0;

int dx01 = x1 - x0;
int dy01 = y1 - y0;
int du01 = u1 - u0;
int dv01 = v1 - v0;

    ly01 = ((dy01<<fp)*div_tbl[dx01+2048])>>16;
    lu01 = ((du01<<fp)*div_tbl[dx01+2048])>>16;
    lv01 = ((dv01<<fp)*div_tbl[dx01+2048])>>16;

int dx12 = x2 - x1;
int dy12 = y2 - y1;
int du12 = u2 - u1;
int dv12 = v2 - v1;

    ly12 = ((dy12<<fp)*div_tbl[dx12+2048])>>16;
    lu12 = ((du12<<fp)*div_tbl[dx12+2048])>>16;
    lv12 = ((dv12<<fp)*div_tbl[dx12+2048])>>16;

int dx02 = x2 - x0;
int dy02 = y2 - y0;
int du02 = u2 - u0;
int dv02 = v2 - v0;

    ly02 = ((dy02<<fp)*div_tbl[dx02+2048])>>16;
    lu02 = ((du02<<fp)*div_tbl[dx02+2048])>>16;
    lv02 = ((dv02<<fp)*div_tbl[dx02+2048])>>16;

int vramofs;
int x, y;

int y01 = y0<<fp;
int y02 = y01;
int u01 = u0<<fp;
int u02 = u01;
int v01 = v0<<fp;
int v02 = v01;

int du, dv;
int su1, su2;
int sv1, sv2;
int sy1, sy2;

int px, py;

int tclamp = (256>>tshr) - 1;

    int xp = x0 * GP32_Height;
    for (x = x0; x<x1; x++)
    {
        sy1 = y01>>fp;
        sy2 = y02>>fp;
        su1 = u01;
        su2 = u02;
        sv1 = v01;
        sv2 = v02;

        if (sy1>sy2)
        {
            temp = sy1; sy1 = sy2; sy2 = temp;
            temp = su1; su1 = su2; su2 = temp;
            temp = sv1; sv1 = sv2; sv2 = temp;
        }

            du = ((su2 - su1)*div_tbl[sy2-sy1+2048])>>16;
            dv = ((sv2 - sv1)*div_tbl[sy2-sy1+2048])>>16;

        xp+=GP32_Height;
        vramofs = xp + sy1;
        for (y = sy1; y<sy2; y++)
        {
            su1+=du;
            sv1+=dv;
            if (vramofs>=0 && vramofs<GP32_Width*GP32_Height)
            {
                c = texture[(((su1>>fp)>>tshr) & tclamp) + ((((sv1>>fp)>>tshr) & tclamp)<<(8-tshr))];
                *(vram+vramofs) = c;
            }
            vramofs++;
        }
        y01+=ly01;
        y02+=ly02;
        u01+=lu01;
        u02+=lu02;
        v01+=lv01;
        v02+=lv02;
    }

    y01 = y1<<fp;
    u01 = u1<<fp;
    v01 = v1<<fp;

    xp = x1 * GP32_Height;
    for (x = x1; x<x2; x++)
    {
        sy1 = y01>>fp;
        sy2 = y02>>fp;
        su1 = u01;
        su2 = u02;
        sv1 = v01;
        sv2 = v02;

        if (sy1>sy2)
        {
            temp = sy1; sy1 = sy2; sy2 = temp;
            temp = su1; su1 = su2; su2 = temp;
            temp = sv1; sv1 = sv2; sv2 = temp;
        }

            du = ((su2 - su1)*div_tbl[sy2-sy1+2048])>>16;
            dv = ((sv2 - sv1)*div_tbl[sy2-sy1+2048])>>16;

        xp+=GP32_Height;
        vramofs = xp + sy1;
        for (y = sy1; y<sy2; y++)
        {
            su1+=du;
            sv1+=dv;
            if (vramofs>=0 && vramofs<GP32_Width*GP32_Height)
            {
                c = texture[(((su1>>fp)>>tshr) & tclamp) + ((((sv1>>fp)>>tshr) & tclamp)<<(8-tshr))];
                *(vram+vramofs) = c;
            }
            vramofs++;
        }
        y01+=ly12;
        y02+=ly02;
        u01+=lu12;
        u02+=lu02;
        v01+=lv12;
        v02+=lv02;
    }
}



void DrawEnvmappedTriangleZB(poly2d poly, unsigned short *vram, int tshr, unsigned short texture[])
{
int x0 =spts[poly.p0].x; int y0 =spts[poly.p0].y;
int x1 =spts[poly.p1].x; int y1 =spts[poly.p1].y;
int x2 =spts[poly.p2].x; int y2 =spts[poly.p2].y;

int c;

int u0 = point_tc[poly.p0].u; int v0 = point_tc[poly.p0].v;
int u1 = point_tc[poly.p1].u; int v1 = point_tc[poly.p1].v;
int u2 = point_tc[poly.p2].u; int v2 = point_tc[poly.p2].v;

int zfp = 0;
int z0 = fpts[poly.p0].z >> zfp;
int z1 = fpts[poly.p1].z >> zfp;
int z2 = fpts[poly.p2].z >> zfp;

// ===== Sort =====

int temp;
if (x1<x0)
{
    temp = u0; u0 = u1; u1 = temp;
    temp = v0; v0 = v1; v1 = temp;
    temp = z0; z0 = z1; z1 = temp;
    temp = x0; x0 = x1; x1 = temp;
    temp = y0; y0 = y1; y1 = temp;
}
if (x2<x0)
{
    temp = u0; u0 = u2; u2 = temp;
    temp = v0; v0 = v2; v2 = temp;
    temp = z0; z0 = z2; z2 = temp;
    temp = x0; x0 = x2; x2 = temp;
    temp = y0; y0 = y2; y2 = temp;
}
if (x2<x1)
{
    temp = u1; u1 = u2; u2 = temp;
    temp = v1; v1 = v2; v2 = temp;
    temp = z1; z1 = z2; z2 = temp;
    temp = x1; x1 = x2; x2 = temp;
    temp = y1; y1 = y2; y2 = temp;
}

// ===== Interpolation variables =====

int n;
int fp = 8;
int ly01=0, ly12=0, ly02=0;
int lu01=0, lu12=0, lu02=0;
int lv01=0, lv12=0, lv02=0;
int lz01=0, lz12=0, lz02=0;

int dx01 = x1 - x0;
int dy01 = y1 - y0;
int du01 = u1 - u0;
int dv01 = v1 - v0;
int dz01 = z1 - z0;

    ly01 = ((dy01<<fp)*div_tbl[dx01+2048])>>16;
    lu01 = ((du01<<fp)*div_tbl[dx01+2048])>>16;
    lv01 = ((dv01<<fp)*div_tbl[dx01+2048])>>16;
    lz01 = (dz01*div_tbl[dx01+2048])>>16;

int dx12 = x2 - x1;
int dy12 = y2 - y1;
int du12 = u2 - u1;
int dv12 = v2 - v1;
int dz12 = z2 - z1;

    ly12 = ((dy12<<fp)*div_tbl[dx12+2048])>>16;
    lu12 = ((du12<<fp)*div_tbl[dx12+2048])>>16;
    lv12 = ((dv12<<fp)*div_tbl[dx12+2048])>>16;
    lz12 = (dz12*div_tbl[dx12+2048])>>16;

int dx02 = x2 - x0;
int dy02 = y2 - y0;
int du02 = u2 - u0;
int dv02 = v2 - v0;
int dz02 = z2 - z0;

    ly02 = ((dy02<<fp)*div_tbl[dx02+2048])>>16;
    lu02 = ((du02<<fp)*div_tbl[dx02+2048])>>16;
    lv02 = ((dv02<<fp)*div_tbl[dx02+2048])>>16;
    lz02 = (dz02*div_tbl[dx02+2048])>>16;

int vramofs;
int x, y;

int y01 = y0<<fp;
int y02 = y01;
int u01 = u0<<fp;
int u02 = u01;
int v01 = v0<<fp;
int v02 = v01;
int z01 = z0;
int z02 = z01;

int du, dv, dz;
int su1, su2;
int sv1, sv2;
int sy1, sy2;
int sz1, sz2;

int px, py;

    int xp = x0 * GP32_Height;
    for (x = x0; x<x1; x++)
    {
        sy1 = y01>>fp;
        sy2 = y02>>fp;
        su1 = u01;
        su2 = u02;
        sv1 = v01;
        sv2 = v02;
        sz1 = z01;
        sz2 = z02;

        if (sy1>sy2)
        {
            temp = sy1; sy1 = sy2; sy2 = temp;
            temp = su1; su1 = su2; su2 = temp;
            temp = sv1; sv1 = sv2; sv2 = temp;
            temp = sz1; sz1 = sz2; sz2 = temp;
        }

            du = ((su2 - su1)*div_tbl[sy2-sy1+2048])>>16;
            dv = ((sv2 - sv1)*div_tbl[sy2-sy1+2048])>>16;
            dz = ((sz2 - sz1)*div_tbl[sy2-sy1+2048])>>16;

        xp+=GP32_Height;
        vramofs = xp + sy1;
        for (y = sy1; y<sy2; y++)
        {
            su1+=du;
            sv1+=dv;
            sz1+=dz;
            if ((vramofs>=0 && vramofs<GP32_Width*GP32_Height) && sz1<zbuffer[vramofs])
            {
                zbuffer[vramofs] = sz1;
                c = texture[((su1>>fp)>>tshr) + (((sv1>>fp)>>tshr)<<(8-tshr))];
                *(vram+vramofs) = c;
            }
            vramofs++;
        }
        y01+=ly01;
        y02+=ly02;
        u01+=lu01;
        u02+=lu02;
        v01+=lv01;
        v02+=lv02;
        z01+=lz01;
        z02+=lz02;
    }

    y01 = y1<<fp;
    u01 = u1<<fp;
    v01 = v1<<fp;
    z01 = z1;

    xp = x1 * GP32_Height;
    for (x = x1; x<x2; x++)
    {
        sy1 = y01>>fp;
        sy2 = y02>>fp;
        su1 = u01;
        su2 = u02;
        sv1 = v01;
        sv2 = v02;
        sz1 = z01;
        sz2 = z02;

        if (sy1>sy2)
        {
            temp = sy1; sy1 = sy2; sy2 = temp;
            temp = su1; su1 = su2; su2 = temp;
            temp = sv1; sv1 = sv2; sv2 = temp;
            temp = sz1; sz1 = sz2; sz2 = temp;
        }

            du = ((su2 - su1)*div_tbl[sy2-sy1+2048])>>16;
            dv = ((sv2 - sv1)*div_tbl[sy2-sy1+2048])>>16;
            dz = ((sz2 - sz1)*div_tbl[sy2-sy1+2048])>>16;

        xp+=GP32_Height;
        vramofs = xp + sy1;
        for (y = sy1; y<sy2; y++)
        {
            su1+=du;
            sv1+=dv;
            sz1+=dz;
            if ((vramofs>=0 && vramofs<GP32_Width*GP32_Height) && sz1<zbuffer[vramofs])
            {
                zbuffer[vramofs] = sz1;
                c = texture[((su1>>fp)>>tshr) + (((sv1>>fp)>>tshr)<<(8-tshr))];
                *(vram+vramofs) = c;
            }
            vramofs++;
        }
        y01+=ly12;
        y02+=ly02;
        u01+=lu12;
        u02+=lu02;
        v01+=lv12;
        v02+=lv02;
        z01+=lz12;
        z02+=lz02;
    }
}


void zsort(int zsortdata[], object3d *obj)
{
    int i, mz;
    for (i=0; i<obj->npls; i++)
    {
        mz = (fpts[obj->poly[i].p0].z + fpts[obj->poly[i].p1].z + fpts[obj->poly[i].p2].z)>>2;
        zsortdata[i] = -mz;
		swp[i] = i;
    }
}


void CleanZbuffer()
{
    int i;
    for (i=0; i<GP32_Width*GP32_Height; i+=2)
        zbuffer[i]= 4294967295;
    for (i=1; i<GP32_Width*GP32_Height; i+=2)
        zbuffer[i]= 4294967295;
}


void Render(object3d *obj, unsigned int shadenum1, unsigned int shadenum2, unsigned short *vram, unsigned int texshr, unsigned short texture1[], unsigned short texture2[], unsigned short texture3[], unsigned int bpp)
{
	int i, j;
    int vx0, vy0, vx1, vy1, n;

    tm = (prticks>>3) % 438;

    int tu, tv, s;

	switch (RenderMode)
	{


	case POINTS:
        for (i = 0; i<obj->npts; i++)
            spts[i].c = obj->point[i].c;
		for (i=0; i<obj->npts; i++)
			drawpoint(spts[i], vram);
	break;


	case WIRE:
        for (i = 0; i<obj->npts; i++)
            spts[i].c = obj->point[i].c;
		for (i=0; i<obj->nlns; i++)
            drawline(obj->line[i], vram, bpp);
	break;

	case FLAT:
        rotate3d_normals(obj);
        CalcPolyColor(obj);

        zsort (zdata, obj);
		quicksort(0, obj->npls - 1, zdata);

        for (i=0; i<obj->npls; i++)
        {
            j = swp[i];
            vx0 = spts[obj->poly[j].p0].x - spts[obj->poly[j].p1].x;
            vy0 = spts[obj->poly[j].p0].y - spts[obj->poly[j].p1].y;
            vx1 = spts[obj->poly[j].p2].x - spts[obj->poly[j].p1].x;
            vy1 = spts[obj->poly[j].p2].y - spts[obj->poly[j].p1].y;
            n = vx0 * vy1 - vx1 * vy0;
            if (n<0)
            {
                obj->poly[j].c = spls[j].c;
                DrawFlatTriangle(obj->poly[j], vram, shades[shadenum1]);
            }
        }
	break;

	case (FLAT | ZBUFFER):
        rotate3d_normals(obj);
        CalcPolyColor(obj);

        CleanZbuffer();
        for (i=0; i<obj->npls; i++)
        {
            if (norms[i].z>=0)
            {
                obj->poly[i].c = spls[i].c;
                DrawFlatTriangleZB(obj->poly[i], vram, shades[shadenum1]);
            }
        }
	break;

	case (FLATWATER):
        rotate3d_normals(obj);
        CalcPolyColor(obj);

        zsort (zdata, obj);
		quicksort(0, obj->npls - 1, zdata);

        for (i=0; i<obj->npls; i++)
        {
            j = swp[i];
            vx0 = spts[obj->poly[j].p0].x - spts[obj->poly[j].p1].x;
            vy0 = spts[obj->poly[j].p0].y - spts[obj->poly[j].p1].y;
            vx1 = spts[obj->poly[j].p2].x - spts[obj->poly[j].p1].x;
            vy1 = spts[obj->poly[j].p2].y - spts[obj->poly[j].p1].y;
            n = vx0 * vy1 - vx1 * vy0;
            if (n<0)
            {
                obj->poly[j].c = spls[j].c;
                DrawFlatWaterTriangle(obj->poly[j], vram, shades[shadenum1], shades[shadenum2]);
            }
        }
	break;

	case (FLATWATER | ZBUFFER):
        rotate3d_normals(obj);
        CalcPolyColor(obj);

        CleanZbuffer();
        for (i=0; i<obj->npls; i++)
        {
            obj->poly[i].c = spls[i].c;
            if (norms[i].z>=0)
                DrawFlatWaterTriangleZB(obj->poly[i], vram, shades[shadenum1], shades[shadenum2]);
        }
	break;

	case GOURAUD:
        rotate3d_pt_normals(obj);
        CalcPointColor(obj);

        zsort (zdata, obj);
		quicksort(0, obj->npls - 1, zdata);

        for (i=0; i<obj->npls; i++)
        {
            j = swp[i];
            vx0 = spts[obj->poly[j].p0].x - spts[obj->poly[j].p1].x;
            vy0 = spts[obj->poly[j].p0].y - spts[obj->poly[j].p1].y;
            vx1 = spts[obj->poly[j].p2].x - spts[obj->poly[j].p1].x;
            vy1 = spts[obj->poly[j].p2].y - spts[obj->poly[j].p1].y;
            n = vx0 * vy1 - vx1 * vy0;
            if (n<0)
                DrawGouraudTriangle(obj->poly[j], vram, shades[shadenum1]);
        }
    break;

	case (GOURAUD | ZBUFFER):
        rotate3d_normals(obj);
        rotate3d_pt_normals(obj);
        CalcPointColor(obj);

        CleanZbuffer();
        for (i=0; i<obj->npls; i++)
        {
            if (norms[i].z>=0)
                DrawGouraudTriangleZB(obj->poly[i], vram, shades[shadenum1]);
        }
    break;

	case (TEXTURE):
        zsort (zdata, obj);
		quicksort(0, obj->npls - 1, zdata);

        for (i=0; i<obj->npls; i++)
        {
            j = swp[i];
            vx0 = spts[obj->poly[j].p0].x - spts[obj->poly[j].p1].x;
            vy0 = spts[obj->poly[j].p0].y - spts[obj->poly[j].p1].y;
            vx1 = spts[obj->poly[j].p2].x - spts[obj->poly[j].p1].x;
            vy1 = spts[obj->poly[j].p2].y - spts[obj->poly[j].p1].y;
            n = vx0 * vy1 - vx1 * vy0;
            if (n<0)
            {
                if (j>=obj->npls-64) DrawTextureTriangle(obj->poly[j], vram, texshr, texture3);
                    else if (j>=obj->npls-128) DrawTextureTriangle(obj->poly[j], vram, texshr, texture2);
                        else DrawTextureTriangle(obj->poly[j], vram, texshr, texture1);
            }
        }
    break;

	case (TEXTURE | ZBUFFER):
        rotate3d_normals(obj);

        CleanZbuffer();
        for (i=0; i<obj->npls; i++)
        {
            if (norms[i].z>=0)
            {
                if (i>=obj->npls-64) DrawTextureTriangleZB(obj->poly[i], vram, texshr, texture3);
                    else if (i>=obj->npls-128) DrawTextureTriangleZB(obj->poly[i], vram, texshr, texture2);
                        else DrawTextureTriangleZB(obj->poly[i], vram, texshr, texture1);
            }
        }
    break;

	case (ENVMAP):
        rotate3d_pt_normals(obj);

        zsort (zdata, obj);
		quicksort(0, obj->npls - 1, zdata);

    	for (i=0; i<obj->npts; i++)
    	{
            tu = (pt_norms[i].x>>9) + 127;
            tv = (pt_norms[i].y>>9) + 127;
    		point_tc[i].u = abs(tu) & 255;
    		point_tc[i].v = abs(tv) & 255;
        }

        for (i=0; i<obj->npls; i++)
        {
            j = swp[i];
            vx0 = spts[obj->poly[j].p0].x - spts[obj->poly[j].p1].x;
            vy0 = spts[obj->poly[j].p0].y - spts[obj->poly[j].p1].y;
            vx1 = spts[obj->poly[j].p2].x - spts[obj->poly[j].p1].x;
            vy1 = spts[obj->poly[j].p2].y - spts[obj->poly[j].p1].y;
            n = vx0 * vy1 - vx1 * vy0;
            if (n<0)
                DrawEnvmappedTriangle(obj->poly[j], vram, texshr, texture1);
        }
    break;

	case (ENVMAP | ZBUFFER):
        rotate3d_normals(obj);
        rotate3d_pt_normals(obj);

        CleanZbuffer();
    	for (i=0; i<obj->npts; i++)
    	{
            tu = (pt_norms[i].x>>9) + 127;
            tv = (pt_norms[i].y>>9) + 127;
    		point_tc[i].u = abs(tu) & 255;
    		point_tc[i].v = abs(tv) & 255;
        }

        for (i=0; i<obj->npls; i++)
        {
            if (norms[i].z>=0)
                DrawEnvmappedTriangleZB(obj->poly[i], vram, texshr, texture1);
        }
    break;

	case (TEXTUREPLASMA):

        zsort (zdata, obj);
		quicksort(0, obj->npls - 1, zdata);

        for (i=0; i<obj->npls; i++)
        {
            j = swp[i];
            vx0 = spts[obj->poly[j].p0].x - spts[obj->poly[j].p1].x;
            vy0 = spts[obj->poly[j].p0].y - spts[obj->poly[j].p1].y;
            vx1 = spts[obj->poly[j].p2].x - spts[obj->poly[j].p1].x;
            vy1 = spts[obj->poly[j].p2].y - spts[obj->poly[j].p1].y;
            n = vx0 * vy1 - vx1 * vy0;
            if (n<0)
                DrawTextureTriangle_plasma(obj->poly[j], vram, shades[4+(j>>1)]);
        }
    break;

	case (TEXTUREPLASMA | ZBUFFER):
        rotate3d_normals(obj);

        CleanZbuffer();
        for (i=0; i<obj->npls; i++)
        {
            obj->poly[i].c = spls[i].c;
            if (norms[i].z>=0)
                DrawTextureTriangleZB_plasma(obj->poly[i], vram, shades[4+(i>>1)]);
        }
    break;

	case VBALLS:

        for (i = 0; i<obj->npts; i++)
            spts[i].c = obj->point[i].c;

		for (i=0; i<obj->npts; i++)
		{
			swp[i] = i;
			zdata[i] = -fpts[i].z;
		}

		quicksort(0, obj->npts - 1, zdata);

		for (i=0; i<obj->npts; i++)
			drawball(spts[swp[i]], (93000 - fpts[swp[i]].z)/24576.0, vram);
	break;

	default:
	break;
	}

}


void RenderSpecial(object3d *obj, unsigned int shadenum1, unsigned int shadenum2, float perc, unsigned short *vram)
{
    int vx0, vy0, vx1, vy1, n, j;
    rotate3d_normals(obj);
    rotate3d_pt_normals(obj);
    CalcPointColor(obj);
    CalcPolyColor(obj);

    zsort (zdata, obj);
	quicksort(0, obj->npls - 1, zdata);

    int i;
    int np = obj->npls * perc;
    for (i=obj->npls-1; i>=np; i--)
    {
        j = swp[i];
        vx0 = spts[obj->poly[j].p0].x - spts[obj->poly[j].p1].x;
        vy0 = spts[obj->poly[j].p0].y - spts[obj->poly[j].p1].y;
        vx1 = spts[obj->poly[j].p2].x - spts[obj->poly[j].p1].x;
        vy1 = spts[obj->poly[j].p2].y - spts[obj->poly[j].p1].y;
        n = vx0 * vy1 - vx1 * vy0;
        if (n<0)
            DrawGouraudTriangle(obj->poly[j], vram, shades[shadenum2]);
    }

    for (i=np; i>=0; i--)
    {
        j = swp[i];
        vx0 = spts[obj->poly[j].p0].x - spts[obj->poly[j].p1].x;
        vy0 = spts[obj->poly[j].p0].y - spts[obj->poly[j].p1].y;
        vx1 = spts[obj->poly[j].p2].x - spts[obj->poly[j].p1].x;
        vy1 = spts[obj->poly[j].p2].y - spts[obj->poly[j].p1].y;
        n = vx0 * vy1 - vx1 * vy0;
        if (n<0)
        {
            obj->poly[j].c = spls[j].c;
            DrawFlatTriangle(obj->poly[j], vram, shades[shadenum1]);
        }
    }
}

