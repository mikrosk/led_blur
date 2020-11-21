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

#include <libxmp-lite/xmp.h>

#include <mint/falcon.h>
#include <mint/osbind.h>

#define MOD_FILENAME	"retroattivo.mod"
#define SAMPLE_RATE	49170

int play_music = 1;

static xmp_context c;

static char* pPhysical;
static char* pLogical;
static size_t bufferSize;	// size of one buffer
static char* pBuffer;

static void loadBuffer(char* pBuffer, size_t bufferSize) {
    xmp_play_buffer(c, pBuffer, bufferSize, 0);
}

void SoundInit(void) {
    if (!play_music)
        return;

    c = xmp_create_context();
    
    struct xmp_test_info ti;
    xmp_test_module(MOD_FILENAME, &ti);

    if (xmp_load_module(c, MOD_FILENAME) != 0) {
	exit(EXIT_FAILURE);
    }
    
    xmp_start_player(c, SAMPLE_RATE, 0);	// 0: stereo 16bit signed (default)
    
    bufferSize = 2 * 2 * SAMPLE_RATE * 1;	// 2 channels * 16 bit * 49170 Hz * 1 second

    pBuffer = (char*)Mxalloc(2 * bufferSize, MX_STRAM);
    if (pBuffer == NULL) {
    	exit(EXIT_FAILURE);
    }
    pPhysical = pBuffer;
    pLogical = pBuffer + bufferSize;

    loadBuffer(pPhysical, bufferSize);
    loadBuffer(pLogical, bufferSize);
    
    Sndstatus(SND_RESET);

    Devconnect(DMAPLAY, DAC, CLK25M, CLK50K, NO_SHAKE);

    Setmode(MODE_STEREO16);
    Soundcmd(ADDERIN, MATIN);
    Setbuffer(SR_PLAY, pBuffer, pBuffer + 2*bufferSize);
}

void PlaySong(void) {
    if (!play_music)
        return;

    Buffoper (SB_PLA_ENA | SB_PLA_RPT);
}

void UpdateSong(void) {
    if (!play_music)
        return;

    static int loadSampleFlag = 1;

    SndBufPtr sPtr;
    if (Buffptr(&sPtr) != 0) {
	exit(EXIT_FAILURE);
    }

    if (loadSampleFlag == 0) {
	// we play from pPhysical (1st buffer)
	if (sPtr.play < pLogical)
	{
            loadBuffer(pLogical, bufferSize);
            loadSampleFlag = !loadSampleFlag;
	}
    } else {
        // we play from pLogical (2nd buffer)
        if (sPtr.play >= pLogical) {
            loadBuffer(pPhysical, bufferSize);
            loadSampleFlag = !loadSampleFlag;
        }
    }
}

void SoundEnd(void) {
    if (!play_music)
        return;

    Buffoper(0x00);

    xmp_stop_module(c);
    xmp_end_player(c);
    xmp_release_module(c);

    Mfree(pBuffer);
    pBuffer = NULL;
}
