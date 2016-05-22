#include "snes9x.h"
#include "memmap.h"
#include "s9xdebug.h"
#include "cpuexec.h"
#include "ppu.h"
#include "apu.h"
#include "display.h"
#include "gfx.h"
#include "soundux.h"
#include "spc700.h"
#include "spc7110.h"
#include "controls.h"

#include "video.h"


#include <time/time.h>
#include <ppc/timebase.h>
#include <xenon_sound/sound.h>

int FrameTimer, timerstyle = 1;
unsigned long long prev, now;

unsigned long long gettime(void)
{
	return mftb();
}

int diff_usec(unsigned long long prev, unsigned long long now)
{
	return (now - prev) /(PPC_TIMEBASE_FREQ/1000000);
}


extern void S9xSetInfoString (const char *string);

/*** Miscellaneous Functions ***/
void S9xMessage(int /*type */, int /*number */, const char *message)
{
#define MAX_MESSAGE_LEN (36 * 3)

	static char buffer[MAX_MESSAGE_LEN + 1];
	strncpy(buffer, message, MAX_MESSAGE_LEN);
	buffer[MAX_MESSAGE_LEN] = 0;
	S9xSetInfoString(buffer);
}

void S9xExit()
{
	printf(" *** EXIT\n");
	while (1);
}

/*** File based functions ***/
const char *
S9xChooseFilename(bool8 read_only)
{
	return NULL;
}

const char *
S9xChooseMovieFilename(bool8 read_only)
{
	return NULL;
}

const char *
S9xGetDirectory(enum s9x_getdirtype dirtype)
{
	return NULL;
}

const char *
S9xGetFilename(const char *ex, enum s9x_getdirtype dirtype)
{
	return NULL;
}

const char *
S9xGetFilenameInc(const char *e, enum s9x_getdirtype dirtype)
{
	return NULL;
}

/*** Memory based functions ***/
void S9xAutoSaveSRAM()
{

}

/*** Sound based functions ***/
void S9xToggleSoundChannel(int c)
{
	if (c == 8)
		so.sound_switch = 255;
	else
		so.sound_switch ^= 1 << c;
	S9xSetSoundControl(so.sound_switch);
}

/****************************************************************************
 * OpenSoundDevice
 *
 * Main initialisation for NGC sound system
 ***************************************************************************/
bool8 S9xOpenSoundDevice(int mode, bool8 stereo, int buffer_size)
{
	so.stereo = TRUE;
	so.playback_rate = 48000;
	so.sixteen_bit = TRUE;
	so.encoded = 0;
	so.buffer_size = 4096;
	so.sound_switch = 255;
	S9xSetPlaybackRate(so.playback_rate);

// init audio

	xenon_sound_init();

	return TRUE;
}

/*** Deprecated function. NGC uses threaded sound ***/
void S9xGenerateSound()
{
}

/* eke-eke */
void S9xInitSync()
{
	FrameTimer = 0;
	prev = gettime();
}

/*** Synchronisation ***/

void S9xSyncSpeed ()
{
	uint32 skipFrms = Settings.SkipFrames;

	if (Settings.TurboMode)
		skipFrms = Settings.TurboSkipFrames;

	if (timerstyle == 0) /* use NGC vertical sync (VSYNC) with NTSC roms */
	{
		while (FrameTimer == 0)
		{
			udelay(50);
		}

		if (FrameTimer > skipFrms)
			FrameTimer = skipFrms;

		if ((FrameTimer > 1) && (IPPU.SkippedFrames < skipFrms))
		{
			IPPU.SkippedFrames++;
			IPPU.RenderThisFrame = FALSE;
		}
		else
		{
			IPPU.SkippedFrames = 0;
			IPPU.RenderThisFrame = TRUE;
		}
	}
	else /* use internal timer for PAL roms */
	{
		unsigned int timediffallowed = Settings.TurboMode ? 0 : Settings.FrameTime;
		now = gettime();
		
		if (diff_usec(prev, now) > timediffallowed)
		{
			printf("L\n");
			/* Timer has already expired */
			if (IPPU.SkippedFrames < skipFrms)
			{
				IPPU.SkippedFrames++;
				IPPU.RenderThisFrame = FALSE;
			}
			else
			{
				IPPU.SkippedFrames = 0;
				IPPU.RenderThisFrame = TRUE;
			}
		}
		else
		{
			/*** Ahead - so hold up ***/
			while (diff_usec(prev, now) < timediffallowed)
			{
				now = gettime();
				udelay(50);
			}
			IPPU.RenderThisFrame = TRUE;
			IPPU.SkippedFrames = 0;
		}

		prev = now;
	}

	if (!Settings.TurboMode)
		FrameTimer--;
	return;
}

/*** Video / Display related functions ***/
bool8 S9xInitUpdate()
{
	return (TRUE);
}

bool8 S9xDeinitUpdate(int Width, int Height)
{
//	update_video(Width, Height);
	videoBlit(Width, Height);
	return (TRUE);
}

bool8 S9xContinueUpdate(int Width, int Height)
{
	return (TRUE);
}

void S9xSetPalette()
{
	return;
}

/*** Input functions ***/
void S9xHandlePortCommand(s9xcommand_t cmd, int16 data1, int16 data2)
{
	return;
}

bool S9xPollButton(uint32 id, bool * pressed)
{
	return 0;
}

bool S9xPollAxis(uint32 id, int16 * value)
{
	return 0;
}

bool S9xPollPointer(uint32 id, int16 * x, int16 * y)
{
	return 0;
}

void S9xLoadSDD1Data()
{
	Memory.FreeSDD1Data();

	Settings.SDD1Pack = FALSE;

	if (strncmp(Memory.ROMName, "Star Ocean", 10) == 0)
		Settings.SDD1Pack = TRUE;

	if (strncmp(Memory.ROMName, "STREET FIGHTER ALPHA2", 21) == 0)
		Settings.SDD1Pack = TRUE;

	return;
}

/****************************************************************************
 * Note that these are DUMMY functions, and only allow Snes9x to
 * compile. Where possible, they will return an error signal.
 ***************************************************************************/

/*** splitpath function ***/
void _splitpath(char const *buffer, char *drive, char *dir, char *fname,
		char *ext)
{
	return; // Do nothing - NGC code should NEVER call a function which relies on it
}

void _makepath(char *filename, const char *drive, const char *dir,
		const char *fname, const char *ext)
{
	return; // Do nothing - NGC code should NEVER call a function which relies on it
}

char *
S9xBasename(char *name)
{
	return name;
}

unsigned long SNESROMSize = 0;
