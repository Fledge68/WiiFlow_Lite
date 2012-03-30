#include <gccore.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include "gc.h"
#include "gecko.h"
#include "fileOps.h"
#include "utils.h"
#include "memory/mem2.hpp"

#define SRAM_ENGLISH 0
#define SRAM_GERMAN 1
#define SRAM_FRENCH 2
#define SRAM_SPANISH 3
#define SRAM_ITALIAN 4
#define SRAM_DUTCH 5

syssram* __SYS_LockSram();
u32 __SYS_UnlockSram(u32 write);
u32 __SYS_SyncSram(void);

void GC_SetVideoMode(u8 videomode)
{
	syssram *sram;
	sram = __SYS_LockSram();
	void *m_frameBuf;
	static GXRModeObj *rmode;
	int memflag = 0;

	if((VIDEO_HaveComponentCable() && (CONF_GetProgressiveScan() > 0)) || videomode > 3)
		sram->flags |= 0x80; //set progressive flag
	else
		sram->flags &= 0x7F; //clear progressive flag

	if(videomode == 1 || videomode == 3 || videomode == 5)
	{
		memflag = 1;
		sram->flags |= 0x01; // Set bit 0 to set the video mode to PAL
		sram->ntd |= 0x40; //set pal60 flag
	}
	else
	{
		sram->flags &= 0xFE; // Clear bit 0 to set the video mode to NTSC
		sram->ntd &= 0xBF; //clear pal60 flag
	}

	if(videomode == 1)
		rmode = &TVPal528IntDf;
	else if(videomode == 2)
		rmode = &TVNtsc480IntDf;
	else if(videomode == 3)
	{
		rmode = &TVEurgb60Hz480IntDf;
		memflag = 5;
	}
	else if(videomode == 4)
		rmode = &TVNtsc480Prog;
	else if(videomode == 5)
	{
		rmode = &TVNtsc480Prog; //TVEurgb60Hz480Prog codedumps
		memflag = 5;
	}

	__SYS_UnlockSram(1); // 1 -> write changes
	while(!__SYS_SyncSram());

	/* Set video mode to PAL or NTSC */
	*(vu32*)0x800000CC = memflag;
	DCFlushRange((void *)(0x800000CC), 4);
	ICInvalidateRange((void *)(0x800000CC), 4);

	VIDEO_Configure(rmode);
	m_frameBuf = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

	VIDEO_ClearFrameBuffer(rmode, m_frameBuf, COLOR_BLACK);
	VIDEO_SetNextFramebuffer(m_frameBuf);

	VIDEO_SetBlack(TRUE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) 
		VIDEO_WaitVSync();
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

void GC_SetLanguage(u8 lang)
{
	if (lang == 0)
		lang = get_wii_language();
	else
		lang--;

	syssram *sram;
	sram = __SYS_LockSram();
	sram->lang = lang;

	__SYS_UnlockSram(1); // 1 -> write changes
	while(!__SYS_SyncSram());
}

bool GC_GameIsInstalled(char *discid, const char* partition, const char* dmlgamedir) 
{
	char folder[50];
	char source[300];
	snprintf(folder, sizeof(folder), dmlgamedir, partition);
	snprintf(source, sizeof(source), "%s/%s/game.iso", folder, discid);
	
	FILE *f = fopen(source, "r");
	if (f) 
	{
		gprintf("Found on %s: %s\n", partition, source);
		fclose(f);
		return true;
	}
	gprintf("Not found on %s: %s\n", partition, source);
	return false;
}

void DML_New_SetOptions(char *GamePath, char *CheatPath, char *NewCheatPath, bool cheats, bool debugger, u8 NMM, u8 nodisc) //, u8 DMLvideoMode)
{
	gprintf("Wiiflow DML: Launch game 'sd:/games/%s/game.iso' through memory (new method)\n", GamePath);

	DML_CFG *DMLCfg = (DML_CFG*)MEM2_alloc(sizeof(DML_CFG));
	memset(DMLCfg, 0, sizeof(DML_CFG));

	DMLCfg->Magicbytes = 0xD1050CF6;
	DMLCfg->CfgVersion = 0x00000001;
	//DMLCfg->VideoMode |= DML_VID_FORCE;
	DMLCfg->VideoMode |= DML_VID_NONE;

	DMLCfg->Config |= DML_CFG_ACTIVITY_LED; //Sorry but I like it lol, option will may follow
	DMLCfg->Config |= DML_CFG_PADHOOK; //Makes life easier, l+z+b+digital down...

	if(GamePath != NULL)
	{
		snprintf(DMLCfg->GamePath, sizeof(DMLCfg->GamePath), "/games/%s/game.iso", GamePath);
		DMLCfg->Config |= DML_CFG_GAME_PATH;
	}

	if(CheatPath != NULL && NewCheatPath != NULL && cheats)
	{
		char *ptr;
		if(strstr(CheatPath, "sd:/") == NULL)
		{
			fsop_CopyFile(CheatPath, NewCheatPath, NULL, NULL);
			ptr = &NewCheatPath[3];
		}
		else
			ptr = &CheatPath[3];
		strncpy(DMLCfg->CheatPath, ptr, sizeof(DMLCfg->CheatPath));
		DMLCfg->Config |= DML_CFG_CHEAT_PATH;
	}

	if(cheats)
		DMLCfg->Config |= DML_CFG_CHEATS;
	if(debugger)
		DMLCfg->Config |= DML_CFG_DEBUGGER;
	if(NMM > 0)
		DMLCfg->Config |= DML_CFG_NMM;
	if(NMM > 1)
		DMLCfg->Config |= DML_CFG_NMM_DEBUG;
	if(nodisc > 0)
		DMLCfg->Config |= DML_CFG_NODISC;

	/*
	if(DMLvideoMode == 1)
		DMLCfg->VideoMode |= DML_VID_FORCE_PAL50;
	else if(DMLvideoMode == 2)
		DMLCfg->VideoMode |= DML_VID_FORCE_NTSC;
	else if(DMLvideoMode == 3)
		DMLCfg->VideoMode |= DML_VID_FORCE_PAL60;
	else
		DMLCfg->VideoMode |= DML_VID_FORCE_PROG;
	*/

	//Write options into memory
	memcpy((void *)0xC0001700, DMLCfg, sizeof(DML_CFG));
	MEM2_free(DMLCfg);
}

void DML_Old_SetOptions(char *GamePath, char *CheatPath, char *NewCheatPath, bool cheats)
{
	gprintf("Wiiflow DML: Launch game 'sd:/games/%s/game.iso' through boot.bin (old method)\n", GamePath);
	FILE *f;
	f = fopen("sd:/games/boot.bin", "wb");
	fwrite(GamePath, 1, strlen(GamePath) + 1, f);
	fclose(f);

	if(cheats && strstr(CheatPath, NewCheatPath) == NULL)
		fsop_CopyFile(CheatPath, NewCheatPath, NULL, NULL);

	//Tell DML to boot the game from sd card
	*(vu32*)0x80001800 = 0xB002D105;
	DCFlushRange((void *)(0x80001800), 4);
	ICInvalidateRange((void *)(0x80001800), 4);

	*(vu32*)0xCC003024 |= 7;
}
