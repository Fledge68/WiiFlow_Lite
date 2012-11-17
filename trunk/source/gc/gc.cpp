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

// for directory parsing and low-level file I/O
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#include "gc/gc.hpp"
#include "gui/text.hpp"
#include "devicemounter/DeviceHandler.hpp"
#include "gecko/gecko.h"
#include "fileOps/fileOps.h"
#include "loader/utils.h"
#include "loader/disc.h"
#include "loader/sys.h"
#include "memory/memory.h"

// DIOS-MIOS
DML_CFG DMLCfg;

void DML_New_SetOptions(const char *GamePath, char *CheatPath, const char *NewCheatPath, 
		const char *partition, bool cheats, bool debugger, u8 NMM, u8 nodisc, u8 DMLvideoMode, 
		u8 videoSetting, bool widescreen, bool new_dm_cfg, bool activity_led)
{
	gprintf("DIOS-MIOS: Launch game '%s' through memory (new method)\n", GamePath);
	memset(&DMLCfg, 0, sizeof(DML_CFG));

	DMLCfg.Magicbytes = 0xD1050CF6;
	if(new_dm_cfg)
		DMLCfg.CfgVersion = 0x00000002;
	else
		DMLCfg.CfgVersion = 0x00000001;

	if(videoSetting == 0)
		DMLCfg.VideoMode |= DML_VID_NONE;
	else if(videoSetting == 1)
		DMLCfg.VideoMode |= DML_VID_DML_AUTO;
	else
		DMLCfg.VideoMode |= DML_VID_FORCE;

	DMLCfg.Config |= DML_CFG_PADHOOK; //Makes life easier, l+z+b+digital down...

	if(GamePath != NULL)
	{
		strncpy(DMLCfg.GamePath, GamePath, sizeof(DMLCfg.GamePath));
		DMLCfg.Config |= DML_CFG_GAME_PATH;
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
		strncpy(DMLCfg.CheatPath, ptr, sizeof(DMLCfg.CheatPath));
		gprintf("DIOS-MIOS: Cheat Path %s\n", ptr);
		DMLCfg.Config |= DML_CFG_CHEAT_PATH;
	}
	if(activity_led)
		DMLCfg.Config |= DML_CFG_ACTIVITY_LED;
	if(cheats)
		DMLCfg.Config |= DML_CFG_CHEATS;
	if(debugger)
		DMLCfg.Config |= DML_CFG_DEBUGGER;
	if(NMM)
		DMLCfg.Config |= DML_CFG_NMM;
	if(NMM > 1)
		DMLCfg.Config |= DML_CFG_NMM_DEBUG;
	if(nodisc)
	{
		if(new_dm_cfg)
			DMLCfg.Config |= DML_CFG_NODISC_CFG2;
		else
			DMLCfg.Config |= DML_CFG_NODISC_CFG1;
	}
	if(widescreen && new_dm_cfg)
		DMLCfg.Config |= DML_CFG_FORCE_WIDE;

	if(DMLvideoMode > 3)
		DMLCfg.VideoMode |= DML_VID_PROG_PATCH;
}

void DML_Old_SetOptions(const char *GamePath)
{
	gprintf("DIOS-MIOS: Launch game '%s' through boot.bin (old method)\n", GamePath);
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
	gprintf("DIOS-MIOS: Booting Disc in Drive\n");
	memset(&DMLCfg, 0, sizeof(DML_CFG));

	DMLCfg.Magicbytes = 0xD1050CF6;
	if(new_dm_cfg)
		DMLCfg.CfgVersion = 0x00000002;
	else
		DMLCfg.CfgVersion = 0x00000001;
	DMLCfg.VideoMode |= DML_VID_DML_AUTO;

	DMLCfg.Config |= DML_CFG_BOOT_DISC;
}

void DML_New_WriteOptions()
{
	//Write options into memory
	memcpy((void *)0x80001700, &DMLCfg, sizeof(DML_CFG));
	DCFlushRange((void *)(0x80001700), sizeof(DML_CFG));

	//DML v1.2+
	memcpy((void *)0x81200000, &DMLCfg, sizeof(DML_CFG));
	DCFlushRange((void *)(0x81200000), sizeof(DML_CFG));
}


// Devolution
u8 *loader_bin = NULL;
extern "C" { extern void __exception_closeall(); }
static gconfig *DEVO_CONFIG = (gconfig*)0x80000020;
#define DEVO_Entry() ((void(*)(void))loader_bin)()

bool DEVO_Installed(const char *path)
{
	bool devo = false;
	const char *loader_path = fmt("%s/loader.bin", path);
	FILE *f = fopen(loader_path, "rb");
	if(f != NULL)
	{
		fseek(f, 0, SEEK_END);
		if(ftell(f) > 0x80) //Size should be more than 128b
		{
			gprintf("Devolution: Found %s\n", loader_path);
			devo = true;
		}
		rewind(f);
		fclose(f);
	}
	return devo;
}

void DEVO_GetLoader(const char *path)
{
	//Read in loader.bin
	const char *loader_path = fmt("%s/loader.bin", path);
	FILE *f = fopen(loader_path, "rb");
	if(f != NULL)
	{
		gprintf("Devolution: Reading %s\n", loader_path);
		fseek(f, 0, SEEK_END);
		u32 size = ftell(f);
		rewind(f);
		loader_bin = (u8*)memalign(32, size);
		fread(loader_bin, 1, size, f);
		DCFlushRange(loader_bin, size);
		fclose(f);
	}
	else
	{
		gprintf("Devolution: Loader not found!\n");
		return;
	}
	gprintf("%s\n", (u8*)loader_bin + 4);
}

void DEVO_SetOptions(const char *isopath, const char *gameID, bool memcard_emu)
{
	// re-mount device we need
	DeviceHandle.MountDevolution();

	//start writing cfg to mem
	struct stat st;
	int data_fd;
	char iso2path[256];
	iso2path[255] = '\0';

	stat(isopath, &st);
	FILE *f = fopen(isopath, "rb");
	gprintf("Devolution: ISO Header %s\n", isopath);
	fread((u8*)0x80000000, 1, 32, f);
	fclose(f);

	// fill out the Devolution config struct
	memset(DEVO_CONFIG, 0, sizeof(gconfig));
	DEVO_CONFIG->signature = 0x3EF9DB23;
	DEVO_CONFIG->version = 0x00000100;
	DEVO_CONFIG->device_signature = st.st_dev;
	DEVO_CONFIG->disc1_cluster = st.st_ino;
	
	// If 2nd iso file tell Devo about it
	strncpy(iso2path, isopath, 255);
	char *ptz = strstr(iso2path, "game.iso");
	if(ptz != NULL)
		strncpy(ptz, "gam1.iso", 8);

	f = fopen(iso2path, "rb");
	if(f != NULL)
	{
		gprintf("Devolution: 2nd ISO File for Multi DVD Game %s\n", iso2path);
		stat(iso2path, &st);
		DEVO_CONFIG->disc2_cluster = st.st_ino;
		fclose(f);
	}

	// make sure these directories exist, they are required for Devolution to function correctly
	fsop_MakeFolder(fmt("%s:/apps", DeviceName[currentPartition]));
	fsop_MakeFolder(fmt("%s:/apps/gc_devo", DeviceName[currentPartition]));

	if(memcard_emu)
	{
		const char *memcard_dir = NULL;
		// find or create a 16MB memcard file for emulation
		// this file can be located anywhere since it's passed by cluster, not name
		// it must be at least 512KB (smallest possible memcard = 59 blocks)
		if(gameID[3] == 'J') //Japanese Memory Card
			memcard_dir = fmt("%s:/apps/gc_devo/memcard_jap.bin", DeviceName[currentPartition]);
		else
			memcard_dir = fmt("%s:/apps/gc_devo/memcard.bin", DeviceName[currentPartition]);
		gprintf("Devolution: Memory Card File %s\n", memcard_dir);
		// check if file doesn't exist
		if(stat(memcard_dir, &st) == -1 || st.st_size < (1<<19))
		{
			// need to enlarge or create it
			data_fd = open(memcard_dir, O_WRONLY|O_CREAT);
			if(data_fd >= 0)
			{
				// try to make it 16MB (largest possible memcard = 2043 blocks)
				gprintf("Devolution: Resizing Memory Card File...\n");
				ftruncate(data_fd, 16<<20);
				if(fstat(data_fd, &st) == -1 || st.st_size < (1<<19))
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
		gprintf("Devolution: Memory Card at %08x\n", st.st_ino);
	}
	else
		st.st_ino = 0;

	// set FAT cluster for start of memory card file
	// if this is zero memory card emulation will not be used
	DEVO_CONFIG->memcard_cluster = st.st_ino;

	// flush disc ID and Devolution config out to memory
	DCFlushRange((void*)0x80000000, 64);

	DeviceHandle.UnMountDevolution();
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

void GC_SetVideoMode(u8 videomode, u8 videoSetting, bool DIOSMIOS)
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
		if(DIOSMIOS && videoSetting == 2)
			DMLCfg.VideoMode |= DML_VID_FORCE_PAL50;
		vmode = &TVPal528IntDf;
	}
	else if(videomode == 2)
	{
		if(DIOSMIOS && videoSetting == 2)
			DMLCfg.VideoMode |= DML_VID_FORCE_NTSC;
		vmode = &TVNtsc480IntDf;
	}
	else if(videomode == 3)
	{
		if(DIOSMIOS && videoSetting == 2)
			DMLCfg.VideoMode |= DML_VID_FORCE_PAL60;
		vmode = &TVEurgb60Hz480IntDf;
		vmode_reg = 5;
	}
	else if(videomode == 4 ||videomode == 6)
	{
		if(DIOSMIOS && videoSetting == 2)
			DMLCfg.VideoMode |= DML_VID_FORCE_PROG;
		vmode = &TVNtsc480Prog;
	}
	else if(videomode == 5 || videomode == 7)
	{
		if(DIOSMIOS && videoSetting == 2)
			DMLCfg.VideoMode |= DML_VID_FORCE_PROG;
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
