/****************************************************************************
 * Snes9x Nintendo Wii/Gamecube Port
 *
 * softdev July 2006
 * Tantric 2008-2010
 *
 * audio.cpp
 *
 * Audio driver
 * Audio is fixed to 32Khz/16bit/Stereo
 ***************************************************************************/

#include <gccore.h>
#include <ogcsys.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <asndlib.h>

#include "video.h"

#include "snes9x/snes9x.h"
#include "snes9x/memmap.h"
#include "snes9x/cpuexec.h"
#include "snes9x/ppu.h"
#include "snes9x/apu/apu.h"
#include "snes9x/display.h"
#include "snes9x/gfx.h"
#include "snes9x/spc7110.h"
#include "snes9x/controls.h"

extern int ConfigRequested;

/*** Double buffered audio ***/
#define AUDIOBUFFER 2048
static unsigned char soundbuffer[2][AUDIOBUFFER] __attribute__ ((__aligned__ (32)));
static int whichab = 0;	/*** Audio buffer flip switch ***/

#define AUDIOSTACK 16384
static lwpq_t audioqueue;
static lwp_t athread;
static uint8 astack[AUDIOSTACK];
static mutex_t audiomutex = LWP_MUTEX_NULL;

/****************************************************************************
 * Audio Threading
 ***************************************************************************/
static void *
AudioThread (void *arg)
{
	LWP_InitQueue (&audioqueue);

	while (1)
	{
		if (ConfigRequested)
			memset (soundbuffer[whichab], 0, AUDIOBUFFER);
		else
		{
			LWP_MutexLock(audiomutex);
			S9xMixSamples (soundbuffer[whichab], AUDIOBUFFER >> 1);
			LWP_MutexUnlock(audiomutex);
		}
		DCFlushRange (soundbuffer[whichab], AUDIOBUFFER);
		LWP_ThreadSleep (audioqueue);
	}

	return NULL;
}

/****************************************************************************
 * MixSamples
 * This continually calls S9xMixSamples On each DMA Completion
 ***************************************************************************/
static void
GCMixSamples ()
{
	if (!ConfigRequested)
	{
		whichab ^= 1;
		AUDIO_InitDMA ((u32) soundbuffer[whichab], AUDIOBUFFER);
		LWP_ThreadSignal (audioqueue);
	}
}

static void FinalizeSamplesCallback (void *data)
{ 
	LWP_MutexLock(audiomutex);
	S9xFinalizeSamples();
	LWP_MutexUnlock(audiomutex);
}

/****************************************************************************
 * InitAudio
 ***************************************************************************/
void
InitAudio ()
{
	#ifdef NO_SOUND
	AUDIO_Init (NULL);
	AUDIO_SetDSPSampleRate(AI_SAMPLERATE_32KHZ);
	AUDIO_RegisterDMACallback(GCMixSamples);
	#else
	ASND_Init();
	#endif
	LWP_MutexInit(&audiomutex, false);
	LWP_CreateThread (&athread, AudioThread, NULL, astack, AUDIOSTACK, 70);
}

/****************************************************************************
 * SwitchAudioMode
 *
 * Switches between menu sound and emulator sound
 ***************************************************************************/
void
SwitchAudioMode(int mode)
{
	if(mode == 0) // emulator
	{
		#ifndef NO_SOUND
		ASND_Pause(1);
		AUDIO_StopDMA();
		AUDIO_SetDSPSampleRate(AI_SAMPLERATE_32KHZ);
		AUDIO_RegisterDMACallback(GCMixSamples);
		#endif
		memset(soundbuffer[0],0,AUDIOBUFFER);
		memset(soundbuffer[1],0,AUDIOBUFFER);
		DCFlushRange(soundbuffer[0],AUDIOBUFFER);
		DCFlushRange(soundbuffer[1],AUDIOBUFFER);
		AUDIO_InitDMA((u32)soundbuffer[whichab],AUDIOBUFFER);
		AUDIO_StartDMA();

		S9xSetSamplesAvailableCallback(FinalizeSamplesCallback, NULL);
	}
	else // menu
	{
		S9xSetSamplesAvailableCallback(NULL, NULL);
		#ifndef NO_SOUND
		ASND_Init();
		ASND_Pause(0);
		#else
		AUDIO_StopDMA();
		#endif
	}
}

/****************************************************************************
 * ShutdownAudio
 *
 * Shuts down audio subsystem. Useful to avoid unpleasant sounds if a
 * crash occurs during shutdown.
 ***************************************************************************/
void ShutdownAudio()
{
	AUDIO_StopDMA();
}

/****************************************************************************
 * AudioStart
 *
 * Called to kick off the Audio Queue
 ***************************************************************************/
void
AudioStart ()
{
	GCMixSamples ();
}
