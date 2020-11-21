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
#define ball_width 16
#define ball_height 16

void Render(object3d *obj, unsigned int shadenum1, unsigned int shadenum2, unsigned short *vram, unsigned int texshr, unsigned short texture1[], unsigned short texture2[], unsigned short texture3[], unsigned int bpp);
void RenderSpecial(object3d *obj, unsigned int shadenum1, unsigned int shadenum2, float perc, unsigned short *vram);

void drawpoint(point2d point, unsigned short *vram);
void drawball(point2d point, float zf, unsigned short *vram);
void drawline(line2d line, unsigned short *buffer, int bpp);
void drawtriangle (poly2d poly, unsigned short *vram);

