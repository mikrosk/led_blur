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

#define MAXSHADES 64

#define Twidth 128
#define Theight 128
#define shl_theight 7

#define blob_width 32
#define blob_height 32

#define nstars3d 512

#define rang 128
#define rang2 256
#define d2r 180.0/pi

typedef struct ColorRGB
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} ColorRGB;

void precalcs();
void MakeColors(unsigned short cols[], ColorRGB c0, ColorRGB c1, int n0, int n1);

