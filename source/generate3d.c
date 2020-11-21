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
#include "engine3d.h"
#include "generate3d.h"

object3d object[32];

extern point2d spts[MAXDATA];
unsigned char addtimes[MAXDATA];

extern unsigned char teapot[];
extern unsigned char face3do[];

void CalcNorms(object3d *obj, int neg)
{
    vector3d v1,v2;
    int i;

    obj->normal = malloc(obj->npls * sizeof(vector3d));

    for (i=0; i<obj->npls; i++)
    {
        v1.x = obj->point[obj->poly[i].p2].x - obj->point[obj->poly[i].p1].x;
        v1.y = obj->point[obj->poly[i].p2].y - obj->point[obj->poly[i].p1].y;
        v1.z = obj->point[obj->poly[i].p2].z - obj->point[obj->poly[i].p1].z;

        v2.x = obj->point[obj->poly[i].p1].x - obj->point[obj->poly[i].p0].x;
        v2.y = obj->point[obj->poly[i].p1].y - obj->point[obj->poly[i].p0].y;
        v2.z = obj->point[obj->poly[i].p1].z - obj->point[obj->poly[i].p0].z;

        obj->normal[i] = Normalize(CrossProduct(v1,v2));
        if (neg==1) obj->normal[i] = NegVec(obj->normal[i]);
    }
}

void CalcPtNorms(object3d *obj)
{
    obj->pt_normal = malloc(obj->npts * sizeof(vector3d));

    int i;
    for (i=0; i<MAXDATA; i++)
        addtimes[i] = 0;

    for (i=0; i<obj->npts; i++)
    {
        obj->pt_normal[i].x = 0;
        obj->pt_normal[i].y = 0;
        obj->pt_normal[i].z = 0;
    }

    for (i=0; i<obj->npls; i++)
    {
        obj->pt_normal[obj->poly[i].p0].x += obj->normal[i].x;
        obj->pt_normal[obj->poly[i].p0].y += obj->normal[i].y;
        obj->pt_normal[obj->poly[i].p0].z += obj->normal[i].z;
        obj->pt_normal[obj->poly[i].p1].x += obj->normal[i].x;
        obj->pt_normal[obj->poly[i].p1].y += obj->normal[i].y;
        obj->pt_normal[obj->poly[i].p1].z += obj->normal[i].z;
        obj->pt_normal[obj->poly[i].p2].x += obj->normal[i].x;
        obj->pt_normal[obj->poly[i].p2].y += obj->normal[i].y;
        obj->pt_normal[obj->poly[i].p2].z += obj->normal[i].z;
        addtimes[obj->poly[i].p0]++;
        addtimes[obj->poly[i].p1]++;
        addtimes[obj->poly[i].p2]++;
    }

    for (i=0; i<obj->npts; i++)
    {
        obj->pt_normal[i].x /= addtimes[i];
        obj->pt_normal[i].y /= addtimes[i]; 
        obj->pt_normal[i].z /= addtimes[i];
        obj->pt_normal[i] = Normalize(obj->pt_normal[i]);
    }
}

void Load16bit3do(object3d *obj, unsigned char* objdata, int neg)
{
    int i;

    obj->npts = *objdata + (*(objdata+1)<<8);
    obj->nlns = *(objdata+2) + (*(objdata+3)<<8);
    obj->npls = *(objdata+4) + (*(objdata+5)<<8);

    obj->point = malloc(obj->npts * sizeof(point3d));
    obj->line = malloc(obj->nlns * sizeof(line2d));
    obj->poly = malloc(obj->npls * sizeof(poly2d));

    objdata+=6;
    for (i=0; i<obj->npts; i++)
    {
        obj->point[i].x = (*(objdata++) - 128) >> 1;
        obj->point[i].y = (*(objdata++) - 128) >> 1;
        obj->point[i].z = (*(objdata++) - 128) >> 1;
        obj->point[i].c = 0xFFFF;
    }

    for (i=0; i<obj->nlns; i++)
    {
        obj->line[i].p0 = *objdata + (*(objdata+1)<<8);
        obj->line[i].p1 = *(objdata+2) + (*(objdata+3)<<8);
        objdata+=4;
        obj->line[i].c = 0xFFFF;
    }

    for (i=0; i<obj->npls; i++)
    {
        obj->poly[i].p0 = *objdata + (*(objdata+1)<<8);
        obj->poly[i].p1 = *(objdata+2) + (*(objdata+3)<<8);
        obj->poly[i].p2 = *(objdata+4) + (*(objdata+5)<<8);
        objdata+=6;
    }

    CalcNorms(obj, neg);
    CalcPtNorms(obj);
}

void GenerateCylinder(object3d *obj)
{
    float pi = 3.14151693;
    float d2r = 180.0/pi;

    int cx = 64, cy = 2, rx = 32, ry = 128;
    
    obj->npts = cx * cy + 2;
    obj->npls = 2 * cx * (cy-1) + 2 * cx;

    obj->point = malloc(obj->npts * sizeof(point3d));
    obj->poly = malloc(obj->npls * sizeof(poly2d));    

    unsigned int x, y;
    float xangl;
    unsigned int i = 0;
    for (y=0; y<cy; y++)
    {
        for (xangl = 0; xangl<360; xangl += (360.0/cx))
        {
            obj->point[i].x = cos(xangl/d2r) * rx;
            obj->point[i].z = sin(xangl/d2r) * rx;
            obj->point[i].y = ((float)y/(float)(cy-1)) * ry - (ry>>1);
            i++;
        }
    }

    obj->point[i].x = 0; obj->point[i].y = -(ry>>1); obj->point[i].z = 0; int i0 = i; i++;
    obj->point[i].x = 0; obj->point[i].y = ry>>1; obj->point[i].z = 0; int i1 = i; i++;

    i = 0;
    int u1 = 255, v1 = 255;
    for (y=0; y<cy-1; y++)
    {
        for (x=0; x<cx; x++)
        {
            obj->poly[i].p0 = (x%cx) + (y%cy)*cx;
            obj->poly[i].p1 = ((x+1)%cx) + (y%cy)*cx;
            obj->poly[i].p2 = (x%cx) + ((y+1)%cy)*cx;

            obj->poly[i].tc0.u = (x * u1) / cx; obj->poly[i].tc0.v = ((y%cy) * v1) / (cy-1);
            obj->poly[i].tc1.u = ((x+1) * u1) / cx; obj->poly[i].tc1.v = ((y%cy) * v1) / (cy-1);
            obj->poly[i].tc2.u = (x * u1) / cx; obj->poly[i].tc2.v = (((y+1)%cy) * v1) / (cy-1);
            i++;

            obj->poly[i].p0 = (x%cx) + ((y+1)%cy)*cx;
            obj->poly[i].p1 = ((x+1)%cx) + (y%cy)*cx;
            obj->poly[i].p2 = ((x+1)%cx) + ((y+1)%cy)*cx;

            obj->poly[i].tc0.u = (x * u1) / cx; obj->poly[i].tc0.v = (((y+1)%cy) * v1) / (cy-1);
            obj->poly[i].tc1.u = ((x+1) * u1) / cx; obj->poly[i].tc1.v = ((y%cy) * v1) / (cy-1);
            obj->poly[i].tc2.u = ((x+1) * u1) / cx; obj->poly[i].tc2.v = (((y+1)%cy) * v1) / (cy-1);
            i++;
        }
    }

    float dg = 2 * pi;
    for (x=0; x<cx; x++)
    {
        obj->poly[i].p2 = x;
        obj->poly[i].p1 = ((x+1)%cx);
        obj->poly[i].p0 = i0;
        obj->poly[i].tc2.u = cos(((dg*x)/(float)cx))*123+126;   obj->poly[i].tc2.v = sin(((dg*x)/(float)cx))*123+126;
        obj->poly[i].tc1.u = cos(((dg*x)/(float)cx))*123+126;   obj->poly[i].tc1.v = sin(((dg*x)/(float)cx))*123+126;
        obj->poly[i].tc0.u = 128;                        obj->poly[i].tc0.v = 128;
        i++;
    }

    for (x=0; x<cx; x++)
    {
        obj->poly[i].p0 = (cy-1)*cx + x;
        obj->poly[i].p1 = (cy-1)*cx + ((x+1)%cx);
        obj->poly[i].p2 = i1;
        obj->poly[i].tc0.u = cos(((dg*x)/(float)cx))*123+126;   obj->poly[i].tc0.v = sin(((dg*x)/(float)cx))*123+126;
        obj->poly[i].tc1.u = cos(((dg*x)/(float)cx))*123+126;   obj->poly[i].tc1.v = sin(((dg*x)/(float)cx))*123+126;
        obj->poly[i].tc2.u = 128;                        obj->poly[i].tc2.v = 128;
        i++;
    }

    CalcNorms(obj, 1);
    CalcPtNorms(obj);
}


void GenerateSpherical(object3d *obj, int pln, float f1, float f2, float k1, float k2, float kk)
{
    float ro, phi, theta;
    int spx = 24, spy = spx;

    float pi = 3.14151693;
    float d2r = 180.0/pi;

    unsigned int i=0;

    obj->npts = spx * spy;
    obj->npls = 2 * spx * spy - 2*spy;

    obj->point = malloc(obj->npts * sizeof(point3d));
    obj->poly = malloc(obj->npls * sizeof(poly2d));

    for (theta=0.0; theta<360.0; theta+=(360.0/spx))
    {
        float ro0 = sin((theta*f1)/d2r*4)*k1;
        float cth = cos(theta/d2r);
        float sth = sin(theta/d2r);
        for (phi=0.0; phi<180.0; phi+=(180.0/spy))
        {
            ro = ro0 + sin((phi*f2)/d2r*4)*k2 + kk;
            obj->point[i].x = ro * sin(phi/d2r) * cth;
            obj->point[i].y = ro * sin(phi/d2r) * sth;
            obj->point[i].z = ro * cos(phi/d2r);
            obj->point[i].c = 0xFFFF;
            i++;
        }
    }

    int x, y;
    i = 0;
    int j = 0;
    for (x=0; x<spx; x++)
    {
        for (y=0; y<spy-1; y++)
        {
            obj->poly[i].p0 = (y%spy) + (x%spx)*spy;
            obj->poly[i].p1 = (y%spy)+1 + (x%spx)*spy;
            obj->poly[i].p2 = (y%spy) + ((x+1)%spx)*spy;
            i++;

            obj->poly[i].p0 = (y%spy) + ((x+1)%spx)*spy;
            obj->poly[i].p1 = ((y+1)%spy) + (x%spx)*spy;
            obj->poly[i].p2 = ((y+1)%spy) + ((x+1)%spx)*spy;
            i++;
        }
    }

    CalcNorms(obj, 0);
    CalcPtNorms(obj);

    obj->pos.x = 0; obj->pos.y = 0; obj->pos.z = 320;
    obj->rot.x = 0; obj->rot.y = 0; obj->rot.z = 0;

}

void ReversePolygonOrder(object3d *obj)
{
    int i, a, b, c;
    for (i=0; i<obj->npls; i++)
    {
        a = obj->poly[i].p0;
        b = obj->poly[i].p1;
        c = obj->poly[i].p2;
        obj->poly[i].p0 = c;
        obj->poly[i].p1 = b;
        obj->poly[i].p2 = a;
    }
}

void InitTestObject(object3d *obj, int objn)
{
    int i, k;
    int x, y, z;

    switch(objn)
    {
        case 0:
            obj->npts = 64;
            obj->nlns = 0;
            obj->npls = 0;

            obj->point = malloc(obj->npts * sizeof(point3d));
            obj->line = malloc(obj->nlns * sizeof(line2d));
            obj->poly = malloc(obj->npls * sizeof(poly2d));


            i = 0;
            for (z = 0; z<4; z++)
            {
                for (y = 0; y<4; y++)
                {
                    for (x = 0; x<4; x++)
                    {
                        obj->point[i].x = ((float)x - 1.5) * 32.0;
                        obj->point[i].y = ((float)y - 1.5) * 32.0;
                        obj->point[i].z = ((float)z - 1.5) * 32.0;
                        obj->point[i].c = (i&15)<<4;
                        i++;
                    }
                }
            }

        obj->pos.x = 0; obj->pos.y = 0; obj->pos.z = 256;
        obj->rot.x = 0; obj->rot.y = 0; obj->rot.z = 0;
        break;

        case 1:

            obj->npts = 8;
            obj->nlns = 12;
            obj->npls = 12;

            obj->point = malloc(obj->npts * sizeof(point3d));
            obj->line = malloc(obj->nlns * sizeof(line2d));
            obj->poly = malloc(obj->npls * sizeof(poly2d));

            obj->point[0].x = -64; obj->point[0].y = 64; obj->point[0].z = -64;
            obj->point[1].x = -64; obj->point[1].y = -64; obj->point[1].z = -64;
            obj->point[2].x = 64; obj->point[2].y = -64; obj->point[2].z = -64;
            obj->point[3].x = 64; obj->point[3].y = 64; obj->point[3].z = -64;
            obj->point[4].x = -64; obj->point[4].y = 64; obj->point[4].z = 64;
            obj->point[5].x = -64; obj->point[5].y = -64; obj->point[5].z = 64;
            obj->point[6].x = 64; obj->point[6].y = -64; obj->point[6].z = 64;
            obj->point[7].x = 64; obj->point[7].y = 64; obj->point[7].z = 64;

            obj->line[0].p0 = 0; obj->line[0].p1 = 1;
            obj->line[1].p0 = 1; obj->line[1].p1 = 2;
            obj->line[2].p0 = 2; obj->line[2].p1 = 3;
            obj->line[3].p0 = 3; obj->line[3].p1 = 0;
            obj->line[4].p0 = 4; obj->line[4].p1 = 5;
            obj->line[5].p0 = 5; obj->line[5].p1 = 6;
            obj->line[6].p0 = 6; obj->line[6].p1 = 7;
            obj->line[7].p0 = 7; obj->line[7].p1 = 4;
            obj->line[8].p0 = 0; obj->line[8].p1 = 4;
            obj->line[9].p0 = 1; obj->line[9].p1 = 5;
            obj->line[10].p0 = 2; obj->line[10].p1 = 6;
            obj->line[11].p0 = 3; obj->line[11].p1 = 7;

            obj->poly[0].p0 = 0; obj->poly[0].p1 = 1; obj->poly[0].p2 = 2;
            obj->poly[1].p0 = 0; obj->poly[1].p1 = 2; obj->poly[1].p2 = 3;
            obj->poly[2].p0 = 7; obj->poly[2].p1 = 6; obj->poly[2].p2 = 5;
            obj->poly[3].p0 = 7; obj->poly[3].p1 = 5; obj->poly[3].p2 = 4;
            obj->poly[4].p0 = 3; obj->poly[4].p1 = 2; obj->poly[4].p2 = 6;
            obj->poly[5].p0 = 3; obj->poly[5].p1 = 6; obj->poly[5].p2 = 7;
            obj->poly[6].p0 = 4; obj->poly[6].p1 = 5; obj->poly[6].p2 = 1;
            obj->poly[7].p0 = 4; obj->poly[7].p1 = 1; obj->poly[7].p2 = 0;
            obj->poly[8].p0 = 4; obj->poly[8].p1 = 0; obj->poly[8].p2 = 3;
            obj->poly[9].p0 = 4; obj->poly[9].p1 = 3; obj->poly[9].p2 = 7;
            obj->poly[10].p0 = 1; obj->poly[10].p1 = 5; obj->poly[10].p2 = 6;
            obj->poly[11].p0 = 1; obj->poly[11].p1 = 6; obj->poly[11].p2 = 2;


            int tl = 8, tr = 248;

            for (i=0; i<6; i++)
            {
                obj->poly[2*i].tc0.u = tl; obj->poly[2*i].tc0.v = tl; obj->poly[2*i].tc1.u = tl; obj->poly[2*i].tc1.v = tr; obj->poly[2*i].tc2.u = tr; obj->poly[2*i].tc2.v = tr;
                obj->poly[2*i+1].tc0.u = tl; obj->poly[2*i+1].tc0.v = tl; obj->poly[2*i+1].tc1.u = tr; obj->poly[2*i+1].tc1.v = tr; obj->poly[2*i+1].tc2.u = tr; obj->poly[2*i+1].tc2.v = tl;
            }

            for (i=0; i<obj->nlns; i++)
                obj->line[i].c = 0xFFFF;

            obj->pos.x = 0; obj->pos.y = 0; obj->pos.z = 320;
            obj->rot.x = 0; obj->rot.y = 0; obj->rot.z = 0;

            CalcNorms(obj, 1);
            CalcPtNorms(obj);
        break;

        case 2:
            Load16bit3do(obj, face3do, 0);
            ReversePolygonOrder(obj);
            obj->pos.x = 0; obj->pos.y = 14; obj->pos.z = 128;
            obj->rot.x = 0; obj->rot.y = -PI/2; obj->rot.z = PI/2;
        break;

        case 3:
            Load16bit3do(obj, teapot,1);
            obj->pos.x = 0; obj->pos.y = 0; obj->pos.z = 160;
            obj->rot.x = 0; obj->rot.y = 0; obj->rot.z = PI;
        break;

        case 16:
            GenerateSpherical(obj, 24, 1.5, 2.0, 24.0, 24.0, 48.0);
            ReversePolygonOrder(obj);
        break;

        case 20:
            GenerateSpherical(obj, 24, 1.5, 2.0, 16.0, 16.0, 64.0);
            ReversePolygonOrder(obj);
        break;

        case 24:
            GenerateCylinder(obj);
        break;

        default:
        break;
    }    
}


