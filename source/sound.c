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

// see https://github.com/mikrosk/atari_sound_setup
#include "../../atari_sound_setup.git/atari_sound_setup.h"

#define MOD_FILENAME	"retroatt.mod"
#define SAMPLE_RATE		24585

int play_music = 1;

static xmp_context c;

static char* pPhysical;
static char* pLogical;
static char* pBuffer;
static char* pTempBuffer;

static AudioSpec obtained_format;

static void loadBuffer(char* pBuffer) {
	if (pTempBuffer) {
		xmp_play_buffer(c, pTempBuffer, obtained_format.size, 0);

		short* s = (short*)pTempBuffer;
		short* d = (short*)pBuffer;
		size_t size = obtained_format.size;

		while (size--) {
			*d++ = __builtin_bswap16(*s++);
		}
	} else {
		xmp_play_buffer(c, pBuffer, obtained_format.size, 0);
	}
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

	if (!AtariSoundSetupInitXbios(&desired, &obtained_format)) {
		exit(EXIT_FAILURE);
	}

	int format = 0;
	if (obtained_format.format == AudioFormatSigned8
		|| obtained_format.format == AudioFormatUnsigned8) {
		format |= XMP_FORMAT_8BIT;
	}

	if (obtained_format.format == AudioFormatUnsigned8
		|| obtained_format.format == AudioFormatUnsigned16LSB
		|| obtained_format.format == AudioFormatUnsigned16MSB) {
		format |= XMP_FORMAT_UNSIGNED;
	}

	if (obtained_format.channels == 1) {
		format |= XMP_FORMAT_MONO;
	}

	xmp_start_player(c, obtained_format.frequency, format);

	if (obtained_format.format == AudioFormatSigned16LSB
		|| obtained_format.format == AudioFormatUnsigned16LSB) {
		pTempBuffer = (char*)Mxalloc(obtained_format.size, MX_PREFTTRAM);
		if (pTempBuffer == NULL) {
			exit(EXIT_FAILURE);
		}
	}

	pBuffer = (char*)Mxalloc(2 * obtained_format.size, MX_STRAM);
	if (pBuffer == NULL) {
		exit(EXIT_FAILURE);
	}
	pPhysical = pBuffer;
	pLogical = pBuffer + obtained_format.size;

	loadBuffer(pPhysical);

	Setbuffer(SR_PLAY, pBuffer, pBuffer + 2*obtained_format.size);
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
	Mfree(pTempBuffer);
	pTempBuffer = NULL;
}
