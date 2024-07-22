/*
 * This file is part of the ledblur/mindlapse demo.
 * Copyright (c) 2006 Michael Kargas <optimus6128@yahoo.gr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/* UNIX port by John Tsiombikas <nuclear@siggraph.org> */
#include <stdio.h>
#include <stdlib.h>

#include <libxmp-lite/xmp.h>

#include <mint/falcon.h>
#include <mint/osbind.h>
#include <usound.h>	// see https://github.com/mikrosk/usound

#define MOD_FILENAME	"retroatt.mod"
#define SAMPLE_RATE		24585

int play_music = 1;
extern int scale;

static xmp_context c;

static char* pPhysical;
static char* pLogical;
static char* pBuffer;

static AudioSpec obtained;

static void loadBuffer(char* pBuffer) {
	xmp_play_buffer(c, pBuffer, obtained.size, 0);
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

	AudioSpec desired;
	desired.frequency = SAMPLE_RATE;
	desired.channels = 2;
	desired.format = AudioFormatSigned16MSB;
	desired.samples = 2048;	// 2048/24585 = 83ms

	if (scale)
		desired.samples *= 2;

	if (!AtariSoundSetupInitXbios(&desired, &obtained)) {
		exit(EXIT_FAILURE);
	}

	int format = 0;
	if (obtained.format == AudioFormatSigned8
		|| obtained.format == AudioFormatUnsigned8) {
		format |= XMP_FORMAT_8BIT;
	}

	if (obtained.format == AudioFormatUnsigned8
		|| obtained.format == AudioFormatUnsigned16LSB
		|| obtained.format == AudioFormatUnsigned16MSB) {
		format |= XMP_FORMAT_UNSIGNED;
	}

	if (obtained.channels == 1) {
		format |= XMP_FORMAT_MONO;
	}

	if (obtained.format == AudioFormatSigned16LSB
		|| obtained.format == AudioFormatUnsigned16LSB) {
		format |= XMP_FORMAT_BYTESWAP;
	}

	xmp_start_player(c, obtained.frequency, format);

	pBuffer = (char*)Mxalloc(2 * obtained.size, MX_STRAM);
	if (pBuffer == NULL) {
		exit(EXIT_FAILURE);
	}
	pPhysical = pBuffer;
	pLogical = pBuffer + obtained.size;

	loadBuffer(pPhysical);

	Setbuffer(SR_PLAY, pBuffer, pBuffer + 2*obtained.size);
}

void PlaySong(void) {
	if (!play_music)
		return;

	Buffoper (SB_PLA_ENA | SB_PLA_RPT);
}

void UpdateSong(void) {
	if (!play_music)
		return;

	static int loadSampleFlag;

	SndBufPtr sPtr;
	if (Buffptr(&sPtr) != 0) {
		exit(EXIT_FAILURE);
	}

	if (loadSampleFlag == 0) {
		// we play from pPhysical (1st buffer)
		if (sPtr.play < pLogical)
		{
			loadBuffer(pLogical);
			loadSampleFlag = !loadSampleFlag;
		}
	} else {
		// we play from pLogical (2nd buffer)
		if (sPtr.play >= pLogical) {
			loadBuffer(pPhysical);
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

	AtariSoundSetupDeinitXbios();

	Mfree(pBuffer);
	pBuffer = NULL;
}
