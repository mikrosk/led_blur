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
#include <math.h>
#include <stdlib.h>

#include "engine3d.h"
#include "effects.h"
#include "render3d.h"
#include "generate3d.h"

point3d fpts[MAXDATA];
point3d norms[MAXDATA];
point3d pt_norms[MAXDATA];
point2d spts[MAXDATA];
point3d spls[MAXDATA];

unsigned short swp[MAXDATA];

const float proj = 256;
extern unsigned int RenderMode;
int lightcalc = LIGHTVIEW;

extern int div_tbl[4096];

extern object3d object[32];
extern int prticks;

extern unsigned short env1[];

extern unsigned short redbull[];
extern unsigned short redbull_top[];
extern unsigned short redbull_bottom[];

extern int signal3d;
int objnlns, objnpls;

void Init3d()
{
	InitTestObject(&object[0],0);
	InitTestObject(&object[1],2);
	InitTestObject(&object[2],3);
    InitTestObject(&object[4],1);

	InitTestObject(&object[16],16);
	InitTestObject(&object[20],20);
	InitTestObject(&object[24],24);
	
	objnlns = object[1].nlns;
	objnpls = object[1].npls;
}


#define fp_mul 256
#define fp_shr 8

#define proj_shr 8


vector3d CrossProduct(vector3d v1, vector3d v2)
{
	vector3d v;
	v.x=v1.y*v2.z-v1.z*v2.y;
	v.y=v1.z*v2.x-v1.x*v2.z;
	v.z=v1.x*v2.y-v1.y*v2.x;
	return v;
}


int DotProduct(vector3d v1, vector3d v2)
{
	return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}


vector3d Normalize(vector3d v)
{
	int d=sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
	if (d!=0)
	{
	   v.x=(v.x<<fp_shr)/d;
	   v.y=(v.y<<fp_shr)/d;
	   v.z=(v.z<<fp_shr)/d;
    }
    else
    {
        v.x = 0;
        v.y = 0;
        v.z = 0;
    }    
	return v;
}

vector3d NegVec(vector3d v)
{
	v.x=-v.x;
	v.y=-v.y;
	v.z=-v.z;
	return v;
}

void PrintVector(vector3d v)
{
    printf("(%d, %d, %d)\n", v.x, v.y, v.z);
}


void translate3d (object3d *obj)
{
	int i;
	int objposx = obj->pos.x * fp_mul;
	int objposy = obj->pos.y * fp_mul;
	int objposz = obj->pos.z * fp_mul;

    for (i=0; i<obj->npts; i++)
    {
       fpts[i].x = fpts[i].x + objposx;
       fpts[i].y = fpts[i].y + objposy;
       fpts[i].z = fpts[i].z + objposz;
    }
}

void rotate3d (object3d *obj)
{
float cosxr = cos(obj->rot.x); float cosyr = cos(obj->rot.y); float coszr = cos(obj->rot.z);
float sinxr = sin(obj->rot.x); float sinyr = sin(obj->rot.y); float sinzr = sin(obj->rot.z);

int xvx = (cosyr * coszr) * fp_mul; int xvy = (sinxr * sinyr * coszr - cosxr * sinzr) * fp_mul; int xvz = (cosxr * sinyr * coszr + sinxr * sinzr) * fp_mul;
int yvx = (cosyr * sinzr) * fp_mul; int yvy = (cosxr * coszr + sinxr * sinyr * sinzr) * fp_mul; int yvz = (-sinxr * coszr + cosxr * sinyr * sinzr) * fp_mul;
int zvx = (-sinyr) * fp_mul; int zvy = (sinxr * cosyr) * fp_mul; int zvz = (cosxr * cosyr) * fp_mul;

    int i;
    for (i=0; i<obj->npts; i++)
    {
        fpts[i].x = (obj->point[i].x * xvx + obj->point[i].y * xvy + obj->point[i].z * xvz);
        fpts[i].y = (obj->point[i].x * yvx + obj->point[i].y * yvy + obj->point[i].z * yvz);
        fpts[i].z = (obj->point[i].x * zvx + obj->point[i].y * zvy + obj->point[i].z * zvz);
    }
}


void rotate3d_normals (object3d *obj)
{
float cosxr = cos(obj->rot.x); float cosyr = cos(obj->rot.y); float coszr = cos(obj->rot.z);
float sinxr = sin(obj->rot.x); float sinyr = sin(obj->rot.y); float sinzr = sin(obj->rot.z);

int xvx = (cosyr * coszr) * fp_mul; int xvy = (sinxr * sinyr * coszr - cosxr * sinzr) * fp_mul; int xvz = (cosxr * sinyr * coszr + sinxr * sinzr) * fp_mul;
int yvx = (cosyr * sinzr) * fp_mul; int yvy = (cosxr * coszr + sinxr * sinyr * sinzr) * fp_mul; int yvz = (-sinxr * coszr + cosxr * sinyr * sinzr) * fp_mul;
int zvx = (-sinyr) * fp_mul; int zvy = (sinxr * cosyr) * fp_mul; int zvz = (cosxr * cosyr) * fp_mul;

    int i;
    for (i=0; i<obj->npls; i++)
    {
        norms[i].x = obj->normal[i].x * xvx + obj->normal[i].y * xvy + obj->normal[i].z * xvz;
        norms[i].y = obj->normal[i].x * yvx + obj->normal[i].y * yvy + obj->normal[i].z * yvz;
        norms[i].z = obj->normal[i].x * zvx + obj->normal[i].y * zvy + obj->normal[i].z * zvz;
    }
}

void rotate3d_pt_normals (object3d *obj)
{
float cosxr = cos(obj->rot.x); float cosyr = cos(obj->rot.y); float coszr = cos(obj->rot.z);
float sinxr = sin(obj->rot.x); float sinyr = sin(obj->rot.y); float sinzr = sin(obj->rot.z);

int xvx = (cosyr * coszr) * fp_mul; int xvy = (sinxr * sinyr * coszr - cosxr * sinzr) * fp_mul; int xvz = (cosxr * sinyr * coszr + sinxr * sinzr) * fp_mul;
int yvx = (cosyr * sinzr) * fp_mul; int yvy = (cosxr * coszr + sinxr * sinyr * sinzr) * fp_mul; int yvz = (-sinxr * coszr + cosxr * sinyr * sinzr) * fp_mul;
int zvx = (-sinyr) * fp_mul; int zvy = (sinxr * cosyr) * fp_mul; int zvz = (cosxr * cosyr) * fp_mul;

    int i;
    for (i=0; i<obj->npts; i++)
    {
        pt_norms[i].x = obj->pt_normal[i].x * xvx + obj->pt_normal[i].y * xvy + obj->pt_normal[i].z * xvz;
        pt_norms[i].y = obj->pt_normal[i].x * yvx + obj->pt_normal[i].y * yvy + obj->pt_normal[i].z * yvz;
        pt_norms[i].z = obj->pt_normal[i].x * zvx + obj->pt_normal[i].y * zvy + obj->pt_normal[i].z * zvz;
    }
}

void project3d (object3d *obj)
{
	int i;

	for (i=0; i<obj->npts; i++)
		if (fpts[i].z > 0)
		{
            spts[i].x = ((fpts[i].x << proj_shr) / fpts[i].z) + (GP32_Width>>1);
            spts[i].y = ((fpts[i].y << proj_shr) / fpts[i].z) + (GP32_Height>>1);
		}
}

void CalcPolyColorStatic(object3d *obj)
{
    int i, c;
    for (i=0; i<obj->npls; i++)
    {
        c = norms[i].z>>8;
        if (c<0) c=0;
        if (c>255) c=255;
        spls[i].c = c;
    }
}

void CalcPointColorStatic(object3d *obj)
{
    int i, c;
    for (i=0; i<obj->npts; i++)
    {
        c = pt_norms[i].z>>8;
        if (c<0) c=0;
        if (c>255) c=255;
        spts[i].c = c;
    }
}

void CalcPolyColorDynamic(object3d *obj)
{
    vector3d light, v;
    light.x = 0;
    light.y = 0;
    light.z = 1;
    float c;

    int i;
    for (i=0; i<obj->npls; i++)
    {
        v.x = norms[i].x;
        v.y = norms[i].y;
        v.z = norms[i].z;
        c = DotProduct(v,light) >> 8;
        if (c<0) c=0;
        if (c>255) c=255;
        spls[i].c = c;
    }
}

void CalcPointColorDynamic(object3d *obj)
{
    vector3d light, v;
    light.x = 0;
    light.y = 0;
    light.z = 1;
    float c;

    int i;
    for (i=0; i<obj->npts; i++)
    {
        v.x = pt_norms[i].x;
        v.y = pt_norms[i].y;
        v.z = pt_norms[i].z;
        c = DotProduct(v,light) >> 8;
        if (c<0) c = 0;
        if (c>255) c = 255;
        spts[i].c = c;
    }
}

void CalcPointColor(object3d *obj)
{
    if (lightcalc==LIGHTVIEW) CalcPointColorStatic(obj);
    if (lightcalc==LIGHTMOVE) CalcPointColorDynamic(obj);
}

void CalcPolyColor(object3d *obj)
{
    if (lightcalc==LIGHTVIEW) CalcPolyColorStatic(obj);
    if (lightcalc==LIGHTMOVE) CalcPolyColorDynamic(obj);
}



void Calc3d(object3d *obj)
{
	rotate3d(obj);
	translate3d(obj);
	project3d(obj);
}

void quicksort (int lo, int hi, int data[])
{
	int m1 = lo;
	int m2 = hi;
	int temp0;
	unsigned short temp1;

	int mp = data[(lo + hi)>>1];

	while (m1<=m2)
	{
		while (data[m1] < mp) m1++;
		while (mp < data[m2]) m2--;

		if (m1<=m2)
		{
			temp0 = data[m1]; data[m1] = data[m2]; data[m2] = temp0;
			temp1 = swp[m1]; swp[m1] = swp[m2]; swp[m2] = temp1;
			m1++;
			m2--;
		}
	}

	if (m2>lo) quicksort(lo, m2, data);
	if (m1<hi) quicksort(m1, hi, data);
}

void RunScene3d(unsigned short *vram, int sn)
{
    int i;
    int objn;

    switch(sn)
    {
        case 0:
            objn = 0;
            RenderMode = VBALLS;

        	object[objn].rot.x = ((prticks/2.0) / D2R);
            object[objn].rot.y = ((prticks/4.0) / D2R);
            object[objn].rot.z = ((prticks/1.0) / D2R);

            object[objn].pos.x = -320;
            if (prticks>8192+3144+256)
            {
                object[objn].pos.x = -320 + (prticks-8192-3144-256)/12.0;
                if (object[objn].pos.x>0) object[objn].pos.x = 0;
            }
            if (prticks>24576-1024)
            {
                object[objn].pos.x = (prticks-24576+1024)/8.0;
            }

        	Calc3d(&object[objn]);
            Render(&object[objn], 0, 0, vram, 0, 0 ,0, 0, 16);
        break;

        case 1:
            objn = 2;
            RenderMode = WIRE;

        	object[objn].rot.x = 0;
            object[objn].rot.y = ((prticks/1.0) / D2R);
            object[objn].rot.z = ((prticks/2.0) / D2R);

            Calc3d(&object[objn]);
            Render(&object[objn], 0, 0, vram, 0, 0, 0, 0, 8);
        break;

        case 2:
            objn = 2;
        	object[objn].rot.x = ((prticks/1.0) / D2R);
            object[objn].rot.y = ((prticks/3.0) / D2R);
            object[objn].rot.z = ((prticks/2.0) / D2R);
            Calc3d(&object[objn]);

            RenderMode = FLAT;
            Render(&object[objn], 1, 0, vram, 0, 0, 0, 0, 16);
        break;

        case 3:
            objn = 1;
            object[objn].pos.z = 128;
            object[objn].pos.y = 12;
        	object[objn].rot.x = sin(prticks/768.0)*0.5;
        	object[objn].rot.y = PI/2 + 0.25 + sin(prticks/512.0)*1.0;
        	object[objn].rot.z = PI/2 + sin(prticks/1024.0)*0.5;
            Calc3d(&object[objn]);

            int adj3 = -1536;
            int adj4 = -1536-128;
            int adj5 = -3144;
            if (prticks<8192+4096+adj3)
                object[objn].nlns = ((prticks-(8192+adj3))/4096.0) * objnlns;
                    else object[objn].nlns = objnlns;
            if ((prticks>16384+adj3+adj4) && (prticks<24576+adj3+adj4))
                object[objn].npls = ((prticks-(16384+adj3+adj4))/8192.0) * objnpls;
                    else object[objn].npls = objnpls;

            if (prticks<=16384+8192+adj3+adj4)
            {
                RenderMode = WIRE;
                Render(&object[objn], 0, 0, vram, 0, 0, 0, 0, 16);
            }
            if ((prticks>16383+adj3+adj4) && (prticks<32768+adj3+adj4+adj5))
            {
                RenderMode = FLAT;
                Render(&object[objn], 3, 0, vram, 0, 0, 0, 0, 16);
            }

            if ((prticks>32767+adj3+adj4+adj5) && (prticks<40960+adj3+adj4+adj5))
                RenderSpecial(&object[objn], 3, 0, 1 - (prticks-(32768+adj3+adj4+adj5))/8192.0, vram);
                    else if ((prticks>40959+adj3+adj4+adj5) && (prticks<40959+adj3+adj4+adj5+1024))
                    {
                        object[objn].npls = objnpls;
                        RenderSpecial(&object[objn], 3, 0, 0, vram);
                    }

            if ((prticks>40959+adj3+adj4+adj5+1024+2048+8192) && (object[objn].npls!=0))
            {
                object[objn].npls = (1 - (prticks-(40959+adj3+adj4+adj5+1024+2048+8192))/2048.0) * objnpls;
                if (object[objn].npls<0) object[objn].npls = 0;
            }
            if ((prticks>=40959+adj3+adj4+adj5+1024) && (object[objn].npls!=0))
            {
                RenderMode = GOURAUD;
                Render(&object[objn], 0, 0, vram, 0, 0, 0, 0, 16);
            }
        break;

        case 4:
            objn = 4;
        	object[objn].rot.x = ((prticks/4.0) / D2R);
            object[objn].rot.y = ((prticks/2.0) / D2R);
            object[objn].rot.z = ((prticks/3.0) / D2R);

            if (prticks>16384-4096-2048)
                object[objn].pos.x = (prticks-16384+4096+2048)/6.0;

            Calc3d(&object[objn]);

            RenderMode = TEXTUREPLASMA;
            Render(&object[objn], 0, 0, vram, 0, 0, 0, 0, 16);
        break;

        case 6:
            objn = 5;
        	object[objn].rot.x = PI;
            object[objn].rot.y = ((prticks/2.0) / D2R);

            Calc3d(&object[objn]);

            RenderMode = ENVMAP | ZBUFFER;
            Render(&object[objn], 0, 0, vram, 2, env1, 0, 0, 16);
        break;

        case 16:
            objn = 16;
            object[objn].rot.x = ((prticks/1.0) / D2R);
            object[objn].rot.y = ((prticks/2.0) / D2R);

            if (prticks<6144)
            {
                object[objn].pos.y = 0;
                object[objn].pos.x = -760 + (prticks/6144.0)*760;
                object[objn].pos.z = 768;
            }
            else if (prticks<8192)
            {
                object[objn].pos.z = 768 - ((prticks-6144)/2048.0) * 384;
            }
            else
            {
                object[objn].pos.y = sin(((8192-prticks)/4.0)/128.0) * 24;
                object[objn].pos.x = sin(((8192-prticks)/4.0)/144.0) * 64; 
                object[objn].pos.z = sin(((8192-prticks)/4.0)/192.0) * 96 + 384;
            }

            Calc3d(&object[objn]);
            RenderMode = FLATWATER;
            Render(&object[objn], 3, 1, vram, 0, 0, 0, 0, 16);
        break;

        case 20:
            if (prticks<16384)
            {
                objn = 20;
            	object[objn].rot.x = ((prticks/1.0) / D2R);
                object[objn].rot.y = ((prticks/2.0) / D2R);

                object[objn].pos.y = sin((prticks/4.0)/128.0) * 24;
                object[objn].pos.x = sin((prticks/4.0)/144.0) * 64; 
                object[objn].pos.z = sin((prticks/4.0)/192.0) * 96 + 384;

                if (prticks>8192)
                {
                object[objn].pos.z += (prticks-8192)/((16384+64-prticks)/64.0);
                }

                Calc3d(&object[objn]);

                RenderMode = ENVMAP;
                Render(&object[objn], 3, 1, vram, 2, env1, 0, 0, 16);
            }
        break;

        case 24:
            objn = 24;

            int fcuk = 8192;
            if (prticks<2048)
            {
                object[objn].rot.x = -PI/2;
                object[objn].rot.z = (prticks/2047.0) * 2*PI;
                object[objn].pos.z = 2047-prticks/1.1;
            }
            else if (prticks<4096+fcuk)
            {
                object[objn].rot.x = -PI/2;
                object[objn].rot.z = (prticks/2047.0) * 2*PI;
            }
            else if (prticks<6144+fcuk)
            {
                object[objn].rot.x = -PI/2 + ((prticks-4096-fcuk)/2048.0)*PI/2;
            }
            else if (prticks<8192+fcuk)
            {
                object[objn].rot.y = -(prticks-6144-fcuk)/1360.0;
            }
            else if (prticks<16384+fcuk)
            {
                object[objn].rot.x = sin(((8192+fcuk-prticks)/4096.0)*2*PI);
                object[objn].rot.y = -(8192+fcuk-6144-fcuk)/1360.0 + sin(((8192+fcuk-prticks)/2048.0)*2*PI);
            }
            else if (prticks<16384+fcuk+16384-512)
            {
                object[objn].rot.x = (prticks-16384-fcuk)/512.0;
                object[objn].pos.z = 2047-2047/1.1 + (prticks-16384-fcuk)/2.5;
            }
            else
                signal3d = 1;


            Calc3d(&object[objn]);
            RenderMode = TEXTURE;
            Render(&object[objn], 0, 0, vram, 1, redbull, redbull_bottom, redbull_top, 16);
        break;

        default:
        break;
    }
}

