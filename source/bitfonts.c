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
unsigned char bitfonts[] = {0,0,0,0,0,0,0,0,4,12,8,24,16,0,32,0,10,18,20,0,0,0,0,0,0,20,126,40,252,80,
0,0,6,25,124,32,248,34,28,0,4,12,72,24,18,48,32,0,14,18,20,8,21,34,29,0,32,32,64,0,0,0,
0,0,16,32,96,64,64,64,32,0,4,2,2,2,6,4,8,0,8,42,28,127,28,42,8,0,0,4,12,62,24,16,
0,0,0,0,0,0,0,0,32,64,0,0,0,60,0,0,0,0,0,0,0,0,0,0,32,0,4,12,8,24,16,48,
32,0,14,17,35,77,113,66,60,0,12,28,12,8,24,16,16,0,30,50,4,24,48,96,124,0,28,50,6,4,2,98,
60,0,2,18,36,100,126,8,8,0,15,16,24,4,2,50,28,0,14,17,32,76,66,98,60,0,126,6,12,24,16,48,
32,0,56,36,24,100,66,98,60,0,14,17,17,9,2,34,28,0,0,0,16,0,0,16,0,0,0,0,16,0,16,32,
0,0,0,0,0,0,0,0,0,0,0,0,30,0,60,0,0,0,0,0,0,0,0,0,0,0,28,50,6,12,8,0,
16,0,0,0,0,0,0,0,0,0,14,27,51,63,99,65,65,0,28,18,57,38,65,65,62,0,14,25,32,96,64,98,
60,0,12,18,49,33,65,66,60,0,30,32,32,120,64,64,60,0,31,48,32,60,96,64,64,0,14,25,32,96,68,98,
60,0,17,17,50,46,100,68,68,0,8,8,24,16,48,32,32,0,2,2,2,6,68,68,56,0,16,17,54,60,120,76,
66,0,16,48,32,96,64,64,60,0,10,21,49,33,99,66,66,0,17,41,37,101,67,66,66,0,28,50,33,97,67,66,
60,0,28,50,34,36,120,64,64,0,28,50,33,97,77,66,61,0,28,50,34,36,124,70,66,0,14,25,16,12,2,70,
60,0,126,24,16,16,48,32,32,0,17,49,35,98,70,68,56,0,66,102,36,44,40,56,48,0,33,97,67,66,86,84,
40,0,67,36,24,28,36,66,66,0,34,18,22,12,12,8,24,0,31,2,4,4,8,24,62,0};
