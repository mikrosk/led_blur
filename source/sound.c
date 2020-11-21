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

#define MOD_FILENAME	"retroattivo.mod"

int play_music = 1;

#if defined(WIN32) || defined(__WIN32__)
#include <windows.h>
#include <conio.h>

#include <fmod.h>
#include <fmod_errors.h>

FMUSIC_MODULE *mod = NULL;
FSOUND_SAMPLE *song = 0;
FSOUND_DSPUNIT    *DrySFXUnit = NULL;

void SoundInit(void)
{
	if(!play_music) return;
	
	if (FSOUND_GetVersion() < FMOD_VERSION)
	{
		printf("Error : You are using the wrong DLL version!  You should be using FMOD %.02f\n", FMOD_VERSION);
		exit(1);
	}

	if (!FSOUND_Init(44100, 64, 0))
	{
		printf("%s\n", FMOD_ErrorString(FSOUND_GetError()));
		exit(1);
	}
}

void PlaySong(void)
{
	if(!play_music) return;
	mod = FMUSIC_LoadSong(MOD_FILENAME);
	if (!mod)
	{
		printf("%s\n", FMOD_ErrorString(FSOUND_GetError()));
		exit(1);
	}
	FMUSIC_PlaySong(mod);
}


void SoundEnd(void)
{
	if(play_music) {
		FMUSIC_FreeSong(mod);
		FSOUND_Close();
	}
}

#else /* use mikmod instead of fmod on unix */
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <mikmod.h>

MODULE *mod;
pid_t plr_pid;


static void sig_handler(int s) {
	switch(s) {
	case SIGUSR1:
		exit(0);

	case SIGCHLD:
		wait(0);
		break;

	default:
		break;
	}
}


void SoundInit(void) {
	if(!play_music) return;

	MikMod_RegisterAllDrivers();
	MikMod_RegisterAllLoaders();

	md_mode |= DMODE_SOFT_MUSIC;
	if(MikMod_Init("") != 0) {
		fprintf(stderr, "mikmod init failed: %s\n", MikMod_strerror(MikMod_errno));
		exit(1);
	}

	if(!(mod = Player_Load(MOD_FILENAME, 64, 0))) {
		fprintf(stderr, "failed to load %s: %s\n", MOD_FILENAME, MikMod_strerror(MikMod_errno));
		exit(1);
	}
}

void PlaySong(void) {
	if(!play_music) return;
	
	Player_Start(mod);
	
	if(!(plr_pid = fork())) {
		atexit(Player_Stop);

		while(Player_Active()) {
			usleep(10000);
			MikMod_Update();
		}
		exit(0);
	}
	
	signal(SIGUSR1, sig_handler);
	signal(SIGCHLD, sig_handler);
}

void SoundEnd(void) {
	if(!play_music) return;

	if(plr_pid) {
		kill(plr_pid, SIGUSR1);
	}
	MikMod_Exit();
}

#endif
