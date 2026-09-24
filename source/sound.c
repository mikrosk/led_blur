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
#include <stdlib.h>

#include <libxmp-lite/xmp.h>
#include <SDL.h>

#define MOD_FILENAME	"retroatt.mod"
#define SAMPLE_RATE		24585

int play_music = 1;
extern int scale;

static xmp_context c;

static void callback(void *userdata, Uint8 *stream, int len) {
	xmp_play_buffer(c, stream, len, 0);
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

	SDL_AudioSpec desired = {0};
	desired.freq = SAMPLE_RATE;
	desired.format = AUDIO_S16MSB;
	desired.channels = 2;
	desired.samples = 2048;	// 2048/24585 = 83ms
	desired.callback = callback;

	if (scale)
		desired.samples *= 2;

	SDL_AudioSpec obtained;
	if (SDL_OpenAudio(&desired, &obtained) < 0) {
		exit(EXIT_FAILURE);
	}

	int format = 0;
	if (obtained.format == AUDIO_S8
		|| obtained.format == AUDIO_U8) {
		format |= XMP_FORMAT_8BIT;
	}

	if (obtained.format == AUDIO_U8
		|| obtained.format == AUDIO_U16LSB
		|| obtained.format == AUDIO_U16MSB) {
		format |= XMP_FORMAT_UNSIGNED;
	}

	if (obtained.channels == 1) {
		format |= XMP_FORMAT_MONO;
	}

	if (obtained.format == AUDIO_S16LSB
		|| obtained.format == AUDIO_U16LSB) {
		format |= XMP_FORMAT_BYTESWAP;
	}

	xmp_start_player(c, obtained.freq, format);
}

void PlaySong(void) {
	if (!play_music)
		return;

	SDL_PauseAudio(0);
}

void SoundEnd(void) {
	if (!play_music)
		return;

	SDL_CloseAudio();

	xmp_stop_module(c);
	xmp_end_player(c);
	xmp_release_module(c);
}
