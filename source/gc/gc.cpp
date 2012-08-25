/****************************************************************************
 * Copyright (C) 2012 FIX94
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include <gccore.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <ogc/machine/processor.h>
#include <sdcard/wiisd_io.h>

// for directory parsing and low-level file I/O
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#include "gc/gc.hpp"
#include "fat.h"
#include "devicemounter/sdhc.h"
#include "devicemounter/usbstorage_libogc.h"
#include "gecko/gecko.h"
#include "fileOps/fileOps.h"
#include "loader/utils.h"
#include "loader/disc.h"
#include "memory/memory.h"

// DIOS-MIOS
DML_CFG *DMLCfg = NULL;

void DML_New_SetOptions(const char *GamePath, char *CheatPath, const char *NewCheatPath, const char *partition, bool cheats, bool debugger, u8 NMM, u8 nodisc, u8 DMLvideoMode, u8 videoSetting, bool widescreen, bool new_dm_cfg)
{
	gprintf("Wiiflow GC: Launch game '%s' through memory (new method)\n", GamePath);

	DMLCfg = (DML_CFG*)malloc(sizeof(DML_CFG));
	if(DMLCfg == NULL)
		return;
	memset(DMLCfg, 0, sizeof(DML_CFG));

	DMLCfg->Magicbytes = 0xD1050CF6;
	if(new_dm_cfg)
		DMLCfg->CfgVersion = 0x00000002;
	else
		DMLCfg->CfgVersion = 0x00000001;

	if(videoSetting == 0)
		DMLCfg->VideoMode |= DML_VID_NONE;
	else if(videoSetting == 1)
		DMLCfg->VideoMode |= DML_VID_DML_AUTO;
	else
		DMLCfg->VideoMode |= DML_VID_FORCE;

	DMLCfg->Config |= DML_CFG_ACTIVITY_LED; //Sorry but I like it lol, option will may follow
	DMLCfg->Config |= DML_CFG_PADHOOK; //Makes life easier, l+z+b+digital down...

	if(GamePath != NULL)
	{
		strncpy(DMLCfg->GamePath, GamePath, sizeof(DMLCfg->GamePath));
		DMLCfg->Config |= DML_CFG_GAME_PATH;
	}

	if(CheatPath != NULL && NewCheatPath != NULL && cheats)
	{
		char *ptr;
		if(strstr(CheatPath, partition) == NULL)
		{
			fsop_CopyFile(CheatPath, (char*)NewCheatPath, NULL, NULL);
			ptr = strstr(NewCheatPath, ":/") + 1;
		}
		else
			ptr = strstr(CheatPath, ":/") + 1;
		strncpy(DMLCfg->CheatPath, ptr, sizeof(DMLCfg->CheatPath));
		gprintf("Cheat Path: %s\n", ptr);
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
	{
		if(new_dm_cfg)
			DMLCfg->Config |= DML_CFG_NODISC_CFG2;
		else
			DMLCfg->Config |= DML_CFG_NODISC_CFG1;
	}
	if(widescreen && new_dm_cfg)
		DMLCfg->Config |= DML_CFG_FORCE_WIDE;

	if(DMLvideoMode > 3)
		DMLCfg->VideoMode |= DML_VID_PROG_PATCH;
}

void DML_Old_SetOptions(const char *GamePath)
{
	gprintf("Wiiflow GC: Launch game '%s' through boot.bin (old method)\n", GamePath);
	FILE *f;
	f = fopen("sd:/games/boot.bin", "wb");
	fwrite(GamePath, 1, strlen(GamePath) + 1, f);
	fclose(f);

	//Tell DML to boot the game from sd card
	*(vu32*)0x80001800 = 0xB002D105;
	DCFlushRange((void *)(0x80001800), 4);
	ICInvalidateRange((void *)(0x80001800), 4);

	*(vu32*)0xCC003024 |= 7;
}

void DML_New_SetBootDiscOption(bool new_dm_cfg)
{
	gprintf("Booting GC game\n");

	DMLCfg = (DML_CFG*)malloc(sizeof(DML_CFG));
	if(DMLCfg == NULL)
		return;
	memset(DMLCfg, 0, sizeof(DML_CFG));

	DMLCfg->Magicbytes = 0xD1050CF6;
	if(new_dm_cfg)
		DMLCfg->CfgVersion = 0x00000002;
	else
		DMLCfg->CfgVersion = 0x00000001;
	DMLCfg->VideoMode |= DML_VID_DML_AUTO;

	DMLCfg->Config |= DML_CFG_BOOT_DISC;
}

void DML_New_WriteOptions()
{
	if(DMLCfg == NULL)
		return;

	//Write options into memory
	memcpy((void *)0x80001700, DMLCfg, sizeof(DML_CFG));
	DCFlushRange((void *)(0x80001700), sizeof(DML_CFG));

	//DML v1.2+
	memcpy((void *)0x81200000, DMLCfg, sizeof(DML_CFG));
	DCFlushRange((void *)(0x81200000), sizeof(DML_CFG));

	free(DMLCfg);
}


// Devolution
u8 *loader_bin = NULL;
extern "C" { extern void __exception_closeall(); }
static gconfig *DEVO_CONFIG = (gconfig*)0x80000020;
#define DEVO_Entry() ((void(*)(void))loader_bin)()

bool DEVO_Installed(const char* path)
{
	bool devo = false;
	char loader_path[256];
	snprintf(loader_path, sizeof(loader_path), "%s/loader.bin", path);
	FILE *f = fopen(loader_path, "rb");
	if(f)
	{
		u8 *tbuf = (u8 *)malloc(0x04);
		fread(tbuf, 1, 4, f);
		if(*(vu32*)tbuf == 0x4800004c)
			devo = true;
			
		free(tbuf);
		fclose(f);
	}
	return devo;
}

void DEVO_GetLoader(const char *loader)
{
	//Read in loader.bin
	char loader_path[256];
	snprintf(loader_path, sizeof(loader_path), "%s/loader.bin", loader);
	FILE *f = fopen(loader_path, "rb");
	if(f)
	{
		gprintf("Read Devolution Loader: \"%s\"\n", loader_path);
		fseek(f, 0, SEEK_END);
		u32 size = ftell(f);
		rewind(f);
		loader_bin = (u8*)memalign(32, size);
		fread(loader_bin, 1, size, f);
		DCFlushRange(loader_bin, ALIGN32(size));
		fclose(f);
	}
	else
	{
		gprintf("Devolution loader.bin not found!\n");
		return;
	}
	gprintf("%s\n", (char*)loader_bin + 4);
}

void DEVO_SetOptions(const char *isopath, const char *partition, const char *gameID, bool memcard_emu)
{
	// re-mount device we need
	fatMountSimple(partition, strncasecmp(partition, "sd", 2) ? &__io_usbstorage_ogc : &__io_wiisd);

	//start writing cfg to mem
	struct stat st;
	int data_fd;
	char iso2path[256];

	stat(isopath, &st);
	FILE *f = fopen(isopath, "rb");
	gprintf("Read ISO File: \"%s\"\n", isopath);
	fread((u8*)0x80000000, 1, 32, f);
	fclose(f);

	// fill out the Devolution config struct
	memset(DEVO_CONFIG, 0, sizeof(gconfig));
	DEVO_CONFIG->signature = 0x3EF9DB23;
	DEVO_CONFIG->version = 0x00000100;
	DEVO_CONFIG->device_signature = st.st_dev;
	DEVO_CONFIG->disc1_cluster = st.st_ino;
	
	// If 2nd iso file tell Devo about it
	strcpy(iso2path, isopath);
	char *ptz = (char *)NULL;
	ptz = strstr(iso2path, "game.iso");
	if(ptz != NULL)
		strncpy(ptz, "gam1.iso", 8);

	f = fopen(iso2path, "rb");
	if(f)
	{
		gprintf("Found 2nd iso file for multi DVD game: \"%s\"\n", iso2path);
		stat(iso2path, &st);
		DEVO_CONFIG->disc2_cluster = st.st_ino;
		fclose(f);
	}

	// make sure these directories exist, they are required for Devolution to function correctly
	char full_path[256];
	snprintf(full_path, sizeof(full_path), "%s:/apps", partition);
	fsop_MakeFolder(full_path);
	snprintf(full_path, sizeof(full_path), "%s:/apps/gc_devo", partition);
	fsop_MakeFolder(full_path);

	if(memcard_emu)
	{
		// find or create a 16MB memcard file for emulation
		// this file can be located anywhere since it's passed by cluster, not name
		// it must be at least 16MB though
		if(gameID[3] == 'J') //Japanese Memory Card
			snprintf(full_path, sizeof(full_path), "%s:/apps/gc_devo/memcard_jap.bin", partition);
		else
			snprintf(full_path, sizeof(full_path), "%s:/apps/gc_devo/memcard.bin", partition);

		// check if file doesn't exist or is less than 16MB
		if(stat(full_path, &st) == -1 || st.st_size < 16<<20)
		{
			// need to enlarge or create it
			data_fd = open(full_path, O_WRONLY|O_CREAT);
			if(data_fd >= 0)
			{
				// make it 16MB
				gprintf("Resizing memcard file...\n");
				ftruncate(data_fd, 16<<20);
				if(fstat(data_fd, &st) == -1 || st.st_size < 16<<20)
				{
					// it still isn't big enough. Give up.
					st.st_ino = 0;
				}
				close(data_fd);
			}
			else
			{
				// couldn't open or create the memory card file
				st.st_ino = 0;
			}
		}
	}
	else
		st.st_ino = 0;

	// set FAT cluster for start of memory card file
	// if this is zero memory card emulation will not be used
	DEVO_CONFIG->memcard_cluster = st.st_ino;

	// flush disc ID and Devolution config out to memory
	DCFlushRange((void*)0x80000000, 64);

	fatUnmount(partition);
}

void DEVO_Boot()
{
	u32 cookie;

	/* cleaning up and load dol */
	SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);
	_CPU_ISR_Disable(cookie);
	__exception_closeall();
	DEVO_Entry();
	_CPU_ISR_Restore(cookie);
}


// General
#define SRAM_ENGLISH 0
#define SRAM_GERMAN 1
#define SRAM_FRENCH 2
#define SRAM_SPANISH 3
#define SRAM_ITALIAN 4
#define SRAM_DUTCH 5

extern "C" {
syssram* __SYS_LockSram();
u32 __SYS_UnlockSram(u32 write);
u32 __SYS_SyncSram(void);
}

void GC_SetVideoMode(u8 videomode, u8 videoSetting)
{
	syssram *sram;
	sram = __SYS_LockSram();
	GXRModeObj *vmode = VIDEO_GetPreferredMode(0);
	int vmode_reg = 0;

	if((VIDEO_HaveComponentCable() && (CONF_GetProgressiveScan() > 0)) || videomode > 3)
		sram->flags |= 0x80; //set progressive flag
	else
		sram->flags &= 0x7F; //clear progressive flag

	if(videomode == 1 || videomode == 3 || videomode == 5)
	{
		vmode_reg = 1;
		sram->flags |= 0x01; // Set bit 0 to set the video mode to PAL
		sram->ntd |= 0x40; //set pal60 flag
	}
	else
	{
		sram->flags &= 0xFE; // Clear bit 0 to set the video mode to NTSC
		sram->ntd &= 0xBF; //clear pal60 flag
	}

	if(videomode == 1)
	{
		if(DMLCfg != NULL && videoSetting == 2)
			DMLCfg->VideoMode |= DML_VID_FORCE_PAL50;
		vmode = &TVPal528IntDf;
	}
	else if(videomode == 2)
	{
		if(DMLCfg != NULL && videoSetting == 2)
			DMLCfg->VideoMode |= DML_VID_FORCE_NTSC;
		vmode = &TVNtsc480IntDf;
	}
	else if(videomode == 3)
	{
		if(DMLCfg != NULL && videoSetting == 2)
			DMLCfg->VideoMode |= DML_VID_FORCE_PAL60;
		vmode = &TVEurgb60Hz480IntDf;
		vmode_reg = 5;
	}
	else if(videomode == 4 ||videomode == 6)
	{
		if(DMLCfg != NULL && videoSetting == 2)
			DMLCfg->VideoMode |= DML_VID_FORCE_PROG;
		vmode = &TVNtsc480Prog;
	}
	else if(videomode == 5 || videomode == 7)
	{
		if(DMLCfg != NULL && videoSetting == 2)
			DMLCfg->VideoMode |= DML_VID_FORCE_PROG;
		vmode = &TVNtsc480Prog;
		vmode_reg = 5;
	}

	__SYS_UnlockSram(1); // 1 -> write changes
	while(!__SYS_SyncSram());

	/* Set video mode register */
	*Video_Mode = vmode_reg;
	DCFlushRange((void*)Video_Mode, 4);

	/* Set video mode */
	if(vmode != 0)
		VIDEO_Configure(vmode);

	/* Setup video */
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(vmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();
	else while(VIDEO_GetNextField())
		VIDEO_WaitVSync();

	/* Set black and flush */
	VIDEO_SetBlack(TRUE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(vmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();
	else while(VIDEO_GetNextField())
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

int GC_GameIsInstalled(char *discid, const char* partition, const char* dmlgamedir) 
{
	char folder[50];
	char source[300];
	snprintf(folder, sizeof(folder), dmlgamedir, partition);
	snprintf(source, sizeof(source), "%s/%s/game.iso", folder, discid);
	
	FILE *f = fopen(source, "rb");
	if(f) 
	{
		gprintf("Found on %s: %s\n", partition, source);
		fclose(f);
		return 1;
	}
	else
	{
		snprintf(source, sizeof(source), "%s/%s/sys/boot.bin", folder, discid);
		f = fopen(source, "rb");
		if(f) 
		{
			gprintf("Found on %s: %s\n", partition, source);
			fclose(f);
			return 2;
		}
	}
	return 0;
}
