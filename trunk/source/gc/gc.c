#include <stdint.h>
#include <string.h>
#include <gccore.h>
#include <ogc/es.h>
#include <ogc/video_types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>
#include <math.h>
#include <ogcsys.h>
#include <unistd.h>
#include <sys/statvfs.h>
#include "gc.h"
#define SEP 0xFF

#define BC 0x0000000100000100ULL
#define MIOS 0x0000000100000101ULL

/** Base address for video registers. */
#define MEM_VIDEO_BASE (0xCC002000)

#define VIDEO_MODE_NTSC 0
#define VIDEO_MODE_PAL 1
#define VIDEO_MODE_PAL60 2
#define VIDEO_MODE_NTSC480P 3
#define VIDEO_MODE_PAL480P 4

#define SRAM_ENGLISH 0
#define SRAM_GERMAN 1
#define SRAM_FRENCH 2
#define SRAM_SPANISH 3
#define SRAM_ITALIAN 4
#define SRAM_DUTCH 5

syssram* __SYS_LockSram();
u32 __SYS_UnlockSram(u32 write);
u32 __SYS_SyncSram(void);

void set_video_mode(int i)
{
	syssram *sram;
	sram = __SYS_LockSram();
	void *m_frameBuf;
	static GXRModeObj *rmode;
	if (i == VIDEO_MODE_NTSC)
	{
		rmode = &TVNtsc480IntDf;
		sram->flags = sram->flags & ~(1 << 0);	// Clear bit 0 to set the video mode to NTSC
	}
	else
	{
		rmode = &TVPal528IntDf;
		sram->flags = sram->flags |  (1 << 0);	// Set bit 0 to set the video mode to PAL
	}
	
	__SYS_UnlockSram(1); // 1 -> write changes
	while(!__SYS_SyncSram());

	/* Set video mode to PAL or NTSC */
	*(u32*)0x800000CC = i;
	
	VIDEO_Configure(rmode);
	m_frameBuf = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	
	VIDEO_ClearFrameBuffer(rmode, m_frameBuf, COLOR_BLACK);
	VIDEO_SetNextFramebuffer(m_frameBuf);
}

u8 get_wii_language()
{
	switch (CONF_GetLanguage())
	{
		case CONF_LANG_GERMAN:
			return SRAM_GERMAN;
		case CONF_LANG_FRENCH:
			return SRAM_FRENCH;
		case CONF_LANG_SPANISH:
			return SRAM_SPANISH;
		case CONF_LANG_ITALIAN:
			return SRAM_ITALIAN;
		case CONF_LANG_DUTCH:
			return SRAM_DUTCH;
		default:
			return SRAM_ENGLISH;
	}
}

void set_language(u8 lang)
{
	if (lang == 0)
	{
		lang = get_wii_language();
	}
	else
		lang--;

	syssram *sram;
	sram = __SYS_LockSram();
	sram->lang = lang;

	__SYS_UnlockSram(1); // 1 -> write changes
	while(!__SYS_SyncSram());
}