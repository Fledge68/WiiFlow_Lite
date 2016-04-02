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
#include <ogc/machine/processor.h>
#include <ogc/lwp_threads.h>
// for directory parsing and low-level file I/O
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include "menu/menu.hpp"
#include "gc/gc.hpp"
#include "gui/text.hpp"
#include "devicemounter/DeviceHandler.hpp"
#include "gecko/gecko.hpp"
#include "fileOps/fileOps.h"
#include "homebrew/homebrew.h"
#include "loader/utils.h"
#include "loader/disc.h"
#include "loader/sys.h"
#include "memory/memory.h"
#include "memory/mem2.hpp"

// DIOS-MIOS
DML_CFG DMLCfg;

void DML_New_SetOptions(const char *GamePath, char *CheatPath, const char *NewCheatPath, 
		const char *partition, bool cheats, bool debugger, u8 NMM, u8 nodisc, u8 DMLvideoMode, 
		u8 videoSetting, bool widescreen, bool new_dm_cfg, bool activity_led, bool screenshot)
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
		const char *ptr = NULL;
		if(strncasecmp(CheatPath, partition, strlen(partition)) != 0)
		{
			fsop_CopyFile(CheatPath, NewCheatPath, NULL, NULL);
			ptr = strchr(NewCheatPath, '/');
		}
		else
			ptr = strchr(CheatPath, '/');
		strncpy(DMLCfg.CheatPath, ptr, sizeof(DMLCfg.CheatPath));
		gprintf("DIOS-MIOS: Cheat Path %s\n", ptr);
		DMLCfg.Config |= DML_CFG_CHEAT_PATH;
	}
	if(screenshot)
		DMLCfg.Config |= DML_CFG_SCREENSHOT;
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
	fsop_WriteFile(DML_BOOT_PATH, GamePath, strlen(GamePath)+1);

	//Tell DML to boot the game from sd card
	*(vu32*)0x80001800 = 0xB002D105;
	DCFlushRange((void *)(0x80001800), 4);
	ICInvalidateRange((void *)(0x80001800), 4);

	*(vu32*)0xCC003024 |= 7;
}

void DML_New_SetBootDiscOption(bool new_dm_cfg)
{
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

// Nintendont
NIN_CFG NinCfg;
u8 NinDevice = 0;
bool NinArgsboot = false;

void Nintendont_SetOptions(const char *game, const char *gameID, char *CheatPath,char *NewCheatPath, const char *partition,
	bool cheats, u8 NMM, u8 videomode, u8 videoSetting, bool widescreen, bool usb_hid, bool native_ctl, bool deflicker, bool screenshot, bool NIN_Debugger)
{
	NinDevice = DeviceHandle.PathToDriveType(game);
	memset(&NinCfg, 0, sizeof(NIN_CFG));
	NinCfg.Magicbytes = 0x01070CF6;
	NinCfg.MemCardBlocks = 0x2;//251 blocks
	
	//check version
	u32 NIN_cfg_version = NIN_CFG_VERSION;	
	for(u8 i = SD; i < MAXDEVICES; ++i)
	{
		const char *dol_path = fmt(NIN_LOADER_PATH, DeviceName[i]);
		if(!fsop_FileExist(dol_path))
			continue;
		u32 filesize = 0;				
		u8 *buffer = fsop_ReadFile(dol_path, &filesize);
		char NINversion[21];		
		for(u32 i = 0; i < filesize-60; ++i)
		{
			// Nintendont Loader..Built   : %s %s..Jan 10 2014.11:21:01 
			if((*(vu32*)(buffer+i+2)) == 0x6e74656e && (*(vu32*)(buffer+i+6)) == 0x646f6e74 
				&& (*(vu32*)(buffer+i+11)) == 0x4c6f6164) //'nten' 'dont' 'Load'
			{
				for(int k= 30; k <50; ++k)
				{  
					if((*(vu32*)(buffer+i+k)) == 0x4A616E20 || (*(vu32*)(buffer+i+k)) == 0x46656220 ||
						(*(vu32*)(buffer+i+k)) == 0x4D617220 || (*(vu32*)(buffer+i+k)) == 0x41707220 ||
						(*(vu32*)(buffer+i+k)) == 0x4D617920 || (*(vu32*)(buffer+i+k)) == 0x4A756E20 ||
						(*(vu32*)(buffer+i+k)) == 0x4A756C20 || (*(vu32*)(buffer+i+k)) == 0x41756720 ||
						(*(vu32*)(buffer+i+k)) == 0x53657020 || (*(vu32*)(buffer+i+k)) == 0x4F637420 ||
						(*(vu32*)(buffer+i+k)) == 0x4E6F7620 || (*(vu32*)(buffer+i+k)) == 0x44656320 ) // find Month
					{	
						for(int j = 0 ; j < 20 ; j++)
							NINversion[j] = *(u8*)(buffer+i+k+j);
							
						NINversion[11] = ' '; // replace \0 between year and time with a space.
						NINversion[20] = 0;
						struct tm time;
						strptime(NINversion, "%b %d %Y %H:%M:%S", &time);
						
						const time_t NINLoaderTime = mktime(&time);
						const time_t v135time = 1407167999;// v1.135
					
						if(difftime(NINLoaderTime,v135time) > 0)
							NIN_cfg_version = 3;
						else
							NIN_cfg_version = 2;
						break;
					}
				}
				break;
			}
		}
		free(buffer);	
		break;
	}	    
	NinCfg.Version = NIN_cfg_version;
	
	if(memcmp("0x474851",gameID,3)==0)
		NinCfg.MaxPads = 1;
	else
		NinCfg.MaxPads = 4;
	
	NinCfg.Config |= NIN_CFG_AUTO_BOOT;
	
	if(NinDevice != SD)
		NinCfg.Config |= NIN_CFG_USB;

	if(IsOnWiiU() == true)
		NinCfg.Config |= NIN_CFG_MEMCARDEMU;

	videoSetting = 2;
	if(videoSetting == 2)
		NinCfg.VideoMode |= NIN_VID_FORCE;

	if((videomode > 3) && (videomode != 6))
		NinCfg.Config |= NIN_CFG_FORCE_PROG;

	if(usb_hid)
		NinCfg.Config |= NIN_CFG_HID;

	if(NIN_Debugger)
		NinCfg.Config |= NIN_CFG_OSREPORT;

	if(native_ctl)
		NinCfg.Config |= NIN_CFG_NATIVE_SI;

	if(deflicker)
		NinCfg.VideoMode |= NIN_VID_FORCE_DF;

	if(screenshot)
		NinCfg.Config |= NIN_CFG_WIIU_WIDE;

	if(widescreen)
		NinCfg.Config |= NIN_CFG_FORCE_WIDE;

	if(NMM > 0)
		NinCfg.Config |= NIN_CFG_MEMCARDEMU;

	if(NMM > 1)
	{
		NinCfg.Config |= NIN_CFG_MC_MULTI;		
		NinCfg.MemCardBlocks = 0x4;//1019 blocks (8MB)
	}
	if(CheatPath != NULL && NewCheatPath != NULL && cheats)
	{
		const char *ptr = NULL;
		if(strncasecmp(CheatPath, partition, strlen(partition)) != 0)
		{
			fsop_CopyFile(CheatPath, NewCheatPath, NULL, NULL);
			ptr = strchr(NewCheatPath, '/');
		}
		else
			ptr = strchr(CheatPath, '/');
		snprintf(NinCfg.CheatPath,sizeof(NinCfg.CheatPath),ptr);
		NinCfg.Config |= NIN_CFG_CHEAT_PATH;
	}
	if(cheats)
		NinCfg.Config |= NIN_CFG_CHEATS;
	
	strncpy(NinCfg.GamePath, strchr(game, '/'), 254);
	if(strstr(NinCfg.GamePath, "boot.bin") != NULL)
	{
		*strrchr(NinCfg.GamePath, '/') = '\0'; //boot.bin
		*(strrchr(NinCfg.GamePath, '/')+1) = '\0'; //sys
	}
	memcpy(&NinCfg.GameID, gameID, 4);
	gprintf("Nintendont Game Path: %s, ID: %08x\n", NinCfg.GamePath, NinCfg.GameID);
}

void Nintendont_BootDisc(u8 NMM, bool widescreen, bool usb_hid, bool native_ctl, bool deflicker)
{
	memset(&NinCfg, 0, sizeof(NIN_CFG));
	NinCfg.Magicbytes = 0x01070CF6;
	FILE * location = fopen("sd:/nincfg.bin", "r");
	if(location == NULL)
	{
		NinCfg.Config |= NIN_CFG_USB;
		fclose(location);
		location = NULL;
	}
	NinCfg.Version = NIN_CFG_VERSION;
	NinCfg.Config |= NIN_CFG_AUTO_BOOT;
	NinCfg.VideoMode |= NIN_VID_AUTO;

	if(usb_hid)
		NinCfg.Config |= NIN_CFG_HID;	
	if(NMM == 1)
	{
		NinCfg.Config |= NIN_CFG_MEMCARDEMU;
		NinCfg.MemCardBlocks = 0x2;//251 blocks (2MB)
	}
	else if(NMM == 2)
	{
	  NinCfg.Config |= NIN_CFG_MEMCARDEMU;
	  NinCfg.Config |= NIN_CFG_MC_MULTI;		
	  NinCfg.MemCardBlocks = 0x4;//1019 blocks (8MB)

	}
	if(native_ctl)
		NinCfg.Config |= NIN_CFG_NATIVE_SI;		
	if(deflicker)
		NinCfg.VideoMode |= NIN_VID_FORCE_DF;				
	if(widescreen)
		NinCfg.Config |= NIN_CFG_FORCE_WIDE;
	snprintf(NinCfg.GamePath,sizeof(NinCfg.GamePath),"di");
}

void Nintendont_WriteOptions()
{
	/* Newer Nintendont versions */
	if(NinArgsboot == true)
	{
		gprintf("Writing Arguments\n");
		AddBootArgument((char*)&NinCfg, sizeof(NIN_CFG));
		return;
	}
	/* general loader */
	if(DeviceHandle.SD_Inserted())
	{
		gprintf("Writing Nintendont CFG: sd:/%s\n", NIN_CFG_PATH);
		fsop_WriteFile(fmt("sd:/%s", NIN_CFG_PATH), &NinCfg, sizeof(NIN_CFG));
	}
	/* for kernel */
	if(NinDevice != SD)
	{
		gprintf("Writing Nintendont USB Kernel CFG: %s:/%s\n", DeviceName[NinDevice], NIN_CFG_PATH);
		fsop_WriteFile(fmt("%s:/%s", DeviceName[NinDevice], NIN_CFG_PATH), &NinCfg, sizeof(NIN_CFG));
	}
}

bool Nintendont_Installed()
{
	for(u8 i = SD; i < MAXDEVICES; ++i)
	{
		const char *dol_path = fmt(NIN_LOADER_PATH, DeviceName[i]);
		if(fsop_FileExist(dol_path) == true)
		{
			gprintf("Nintendont found\n");
			return true;
		}
	}
	return false;
}

bool Nintendont_GetLoader()
{
	bool ret = false;
	for(u8 i = SD; i < MAXDEVICES; ++i)
	{
		const char *dol_path = fmt(NIN_LOADER_PATH, DeviceName[i]);
		ret = (LoadHomebrew(dol_path) == 1);
		if(ret == true)
		{
			gprintf("Nintendont loaded: %s\n", dol_path);
			AddBootArgument(dol_path);
			//search for argsboot
			u32 size;
			const char *dol_ptr = GetHomebrew(&size);
			for(u32 i = 0; i < size; i += 0x10)
			{
				if(strncmp(dol_ptr + i, "argsboot", 8) == 0)
				{
					gprintf("Nintendont argsboot found at %08x\n", i);
					NinArgsboot = true;
					break;
				}
			}
			break;
		}
	}
	return ret;
}

// Devolution
u8 *tmp_buffer = NULL;
u8 *loader_bin = NULL;
u32 loader_size = 0;
extern "C" { extern void __exception_closeall(); }
static gconfig *DEVO_CONFIG = (gconfig*)0x80000020;
#define DEVO_ENTRY ((entry)loader_bin)

bool DEVO_Installed(const char *path)
{
	loader_size = 0;
	bool devo = false;
	fsop_GetFileSizeBytes(fmt(DEVO_LOADER_PATH, path), &loader_size);
	if(loader_size > 0x80) //Size should be more than 128b
	{
		gprintf("Devolution found\n");
		devo = true;
	}
	return devo;
}

void DEVO_GetLoader(const char *path)
{
	loader_size = 0;
	tmp_buffer = fsop_ReadFile(fmt(DEVO_LOADER_PATH, path), &loader_size);
	if(tmp_buffer == NULL)
		gprintf("Devolution: Loader not found!\n");
}

void DEVO_SetOptions(const char *isopath, const char *gameID, bool memcard_emu,
					bool widescreen, bool activity_led, bool wifi)
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
	fread((u8*)Disc_ID, 1, 32, f);
	fclose(f);
	f = NULL;

	// fill out the Devolution config struct
	memset(DEVO_CONFIG, 0, sizeof(gconfig));
	DEVO_CONFIG->signature = DEVO_CONFIG_SIG;
	DEVO_CONFIG->version = DEVO_CONFIG_VERSION;
	DEVO_CONFIG->device_signature = st.st_dev;
	DEVO_CONFIG->disc1_cluster = st.st_ino;

	// Pergame options
	if(wifi)
		DEVO_CONFIG->options |= DEVO_CONFIG_WIFILOG;
	if(widescreen)
		DEVO_CONFIG->options |= DEVO_CONFIG_WIDE;
	if(!activity_led)
		DEVO_CONFIG->options |= DEVO_CONFIG_NOLED;

	// If 2nd iso file tell Devo about it
	strncpy(iso2path, isopath, 255);
	char *ptz = strstr(iso2path, "game.iso");
	if(ptz != NULL)
	{
		strncpy(ptz, "gam1.iso", 8);
		f = fopen(iso2path, "rb");
		if(f == NULL)
		{
			strncpy(ptz, "gam2.iso", 8);
			f = fopen(iso2path, "rb");
			if(f == NULL)
			{
				strncpy(ptz, "disc2.iso", 9);
				f = fopen(iso2path, "rb");
			}
		}
		if(f != NULL)
		{
			gprintf("Devolution: 2nd ISO File for Multi DVD Game %s\n", iso2path);
			stat(iso2path, &st);
			DEVO_CONFIG->disc2_cluster = st.st_ino;
			fclose(f);
		}
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
	DCFlushRange((void*)Disc_ID, 64);

	DeviceHandle.UnMountDevolution();
}

void DEVO_Boot()
{
	/* Move our loader into low MEM1 */
	loader_bin = (u8*)MEM1_lo_alloc(loader_size);
	memcpy(loader_bin, tmp_buffer, loader_size);
	DCFlushRange(loader_bin, ALIGN32(loader_size));
	MEM2_free(tmp_buffer);
	gprintf("%s\n", (loader_bin+4));
	/* Boot that binary */
	JumpToEntry(DEVO_ENTRY);
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

void GC_SetVideoMode(u8 videomode, u8 videoSetting, u8 loader)
{
	syssram *sram;
	sram = __SYS_LockSram();
	GXRModeObj *vmode = VIDEO_GetPreferredMode(0);
	int vmode_reg = 0;

	if(loader == 2)
		videoSetting = 2;
	
	if((VIDEO_HaveComponentCable() && (CONF_GetProgressiveScan() > 0)) || ((videomode > 3) && (videomode != 6)))
		sram->flags |= 0x80; //set progressive flag
	else
		sram->flags &= 0x7F; //clear progressive flag

	if(videomode == 1 || videomode == 3 || videomode == 5 || videomode == 6 || videomode == 7)
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
		if(videoSetting == 2)
		{
			if(loader == 0)
				DMLCfg.VideoMode |= DML_VID_FORCE_PAL50;
			else if(loader == 2)
				NinCfg.VideoMode |= NIN_VID_FORCE_PAL50;
		}
		vmode = &TVPal528IntDf;
	}
	else if(videomode == 2)
	{
		if(videoSetting == 2)
		{
			if(loader == 0)
				DMLCfg.VideoMode |= DML_VID_FORCE_NTSC;
			else if(loader == 2)
				NinCfg.VideoMode |= NIN_VID_FORCE_NTSC;
		}
		vmode = &TVNtsc480IntDf;
	}
	else if(videomode == 3)
	{
		if(videoSetting == 2)
		{
			if(loader == 0)
				DMLCfg.VideoMode |= DML_VID_FORCE_PAL60;
			else if(loader == 2)
				NinCfg.VideoMode |= NIN_VID_FORCE_PAL60;
		}
		vmode = &TVEurgb60Hz480IntDf;
		vmode_reg = 5;
	}
	else if(videomode == 4)
	{
		if(videoSetting == 2)
		{
			if(loader == 0)
				DMLCfg.VideoMode |= DML_VID_FORCE_PROG;
			else if(loader == 2)
				NinCfg.VideoMode |= NIN_VID_FORCE_NTSC;
		}
		vmode = &TVNtsc480IntDf;// shouldn't this be vmode = &TVNtsc480Prog
	}
	else if(videomode == 5)
	{
		if(videoSetting == 2)
		{
			if(loader == 0)
				DMLCfg.VideoMode |= DML_VID_FORCE_PROG;
			else if(loader == 2)
				NinCfg.VideoMode |= NIN_VID_FORCE_PAL60;
		}
		vmode = &TVEurgb60Hz480IntDf;
		vmode_reg = 5;
	}
	else if(videomode == 6)
	{
		if(videoSetting == 2)
		{
			if(loader == 0)
				DMLCfg.VideoMode |= DML_VID_FORCE_PAL60;
			else if(loader == 2)
				NinCfg.VideoMode |= NIN_VID_FORCE_MPAL;
		}
		vmode = &TVEurgb60Hz480IntDf;
		vmode_reg = 5;
	}
	else if(videomode == 7)
	{
		if(videoSetting == 2)
		{
			if(loader == 0)
				DMLCfg.VideoMode |= DML_VID_FORCE_PROG;
			else if(loader == 2)
				NinCfg.VideoMode |= NIN_VID_FORCE_MPAL;
		}
		vmode = &TVEurgb60Hz480IntDf;
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

void GC_SetLanguage(u8 lang, u8 loader)
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

	/* write language for nintendont */
	if(loader == 2)
	{
		switch(lang)
		{
			case SRAM_GERMAN:
				NinCfg.Language = NIN_LAN_GERMAN;
				break;
			case SRAM_FRENCH:
				NinCfg.Language = NIN_LAN_FRENCH;
				break;
			case SRAM_SPANISH:
				NinCfg.Language = NIN_LAN_SPANISH;
				break;
			case SRAM_ITALIAN:
				NinCfg.Language = NIN_LAN_ITALIAN;
				break;
			case SRAM_DUTCH:
				NinCfg.Language = NIN_LAN_DUTCH;
				break;
			default:
				NinCfg.Language = NIN_LAN_ENGLISH;
				break;
		}
	}
}
