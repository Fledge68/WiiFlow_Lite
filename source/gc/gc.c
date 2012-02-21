#include <gccore.h>
#include "gc.h"

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

	if(VIDEO_HaveComponentCable())
		sram->flags |= 0x80; //set progressive flag
	else
		sram->flags &= 0x7F; //clear progressive flag

	if (!i)
	{
		rmode = &TVNtsc480IntDf;
		sram->flags &= 0xFE; // Clear bit 0 to set the video mode to NTSC
		sram->ntd &= 0xBF; //clear pal60 flag
	}
	else
	{
		rmode = &TVPal528IntDf;
		sram->flags |= 0x01; // Set bit 0 to set the video mode to PAL
		sram->ntd |= 0x40; //set pal60 flag
	}

	__SYS_UnlockSram(1); // 1 -> write changes
	while(!__SYS_SyncSram());

	/* Set video mode to PAL or NTSC */
	*(vu32*)0x800000CC = i;
	DCFlushRange((void *)(0x800000CC), 1);
	ICInvalidateRange((void *)(0x800000CC), 1);
	
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