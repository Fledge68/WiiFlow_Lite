#include <gccore.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include "gc.h"
#include "gecko.h"

#define MAX_FAT_PATH 1024

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

void DML_RemoveGame(const char *discid)
{
	int num = 6;
	const char *fl[6] = {"sd:/games/%s/game.iso","sd:/games/%s/sys/boot.bin","sd:/games/%s/sys/bi2.bin",
	"sd:/games/%s/sys/apploader.img","sd:/games/%s/sys","sd:/games/%s"};
	char fname[MAX_FAT_PATH];
	FILE *f;
	DIR *dir;
	int i;
	for(i = 0; i < num; i++)
	{
		sprintf(fname, fl[i], discid);
		f = fopen((char*)fname, "r");
		if(f) 
		{
			gprintf("Deleting %s...\n",fname);
			fclose(f);
			remove(fname);
		}
		dir = opendir((char*)fname);
		if(dir)
		{
			gprintf("Deleting %s...\n",fname);
			closedir(dir);
			unlink((char*)fname);
		}
	}
}

bool DML_GameIsInstalled(char *discid) 
{
	char filepath[64];
	sprintf(filepath, "sd:/games/%s/game.iso", discid);
	
	gprintf("Filepath on SD: %s\n", filepath);
	
	FILE *f = fopen(filepath, "r");
	if (f) 
	{
		fclose(f);
		return true;
	}
	gprintf("Not found\n");
	return false;
}
