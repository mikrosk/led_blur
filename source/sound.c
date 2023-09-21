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

#include <libxmp-lite/xmp.h>
#include <SDL.h>

#define MOD_FILENAME	"retroatt.mod"
#define SAMPLE_RATE		24585

int play_music = 1;

static xmp_context c;

static void callback(void *userdata, Uint8 *stream, int len) {
	SDL_LockAudio();

	xmp_play_buffer(c, stream, len, 0);

	SDL_UnlockAudio();
}

void SoundInit(void) {
	if (!play_music)
		return;

	c = xmp_create_context();

	struct xmp_test_info ti;
	xmp_test_module(MOD_FILENAME, &ti);

	if (xmp_load_module(c, MOD_FILENAME) != 0) {
		play_music = 0;
		return;
	}

	xmp_start_player(c, SAMPLE_RATE, 0);	// 0: stereo 16bit signed (default)

	SDL_AudioSpec desired = {0};
	desired.freq = SAMPLE_RATE;
	desired.format = AUDIO_S16MSB;
	desired.channels = 2;
	desired.samples = 2048 * 4;	// 83ms * 4
	desired.callback = callback;

	if (SDL_OpenAudio(&desired, NULL) < 0 ) {
		play_music = 0;
		return;
	}
}

void PlaySong(void) {
	if (!play_music)
		return;

	SDL_PauseAudio(0);
}

void SoundEnd(void) {
	if (!play_music)
		return;

	SDL_PauseAudio(1);
	SDL_CloseAudio();

	xmp_stop_module(c);
	xmp_end_player(c);
	xmp_release_module(c);
}
