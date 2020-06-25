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

// Languages
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

// Nintendont
NIN_CFG NinCfg;
bool slippi;

/* since Nintendont v1.98 argsboot is supported.
since v3.324 '$$Version:' string was added for loaders to detect.
since wiiflow lite doesn't support versions less than v3.358
we will use argsboot and version detection every time. */

void Nintendont_SetOptions(const char *gamePath, const char *gameID, const char *CheatPath, u8 lang, 
							u32 n_cfg, u32 n_vm, s8 vidscale, s8 vidoffset, u8 netprofile)
{
	memset(&NinCfg, 0, sizeof(NIN_CFG));
	NinCfg.Magicbytes = 0x01070CF6;
	
	NinCfg.MaxPads = 4;

	/* check nintendont version so we can set the proper config version */
	u32 NIN_cfg_version = NIN_CFG_VERSION;
	char NINVersion[7]= "";
	u32 NINRev = 0;
	const char *dol_path = NULL;
	for(u8 i = SD; i < MAXDEVICES; ++i)
	{
		if(slippi)
			dol_path = fmt(NIN_SLIPPI_PATH, DeviceName[i]);
		else
			dol_path = fmt(NIN_LOADER_PATH, DeviceName[i]);
		if(!fsop_FileExist(dol_path))
			continue;
		u8 *buffer = NULL;
		u32 filesize = 0;
		const char *str = "$$Version:";
		buffer = fsop_ReadFile(dol_path, &filesize);
		// if buffer is NULL then out of memory - hopefully that doesn't happen and shouldn't
		for(u32 i = 0; i < filesize; i += 32)
		{
			if(memcmp(buffer+i, str, strlen(str)) == 0)
			{
				// get nintendont version (NINVersion) from current position in nintendont boot.dol (buffer)
				snprintf(NINVersion, sizeof(NINVersion), "%s", buffer+i+strlen(str));
				// get revision from version and use it to get NinCfg version
				NINRev = atoi(strchr(NINVersion, '.')+1);
				break;
			}
		}
		MEM2_free(buffer);
		break;
	}
	if(NINRev == 0 || NINRev < 358)
		NIN_cfg_version = 2;// nintendont not found or revision is less than 358 thus too old for wiiflow lite
		
	else if(NINRev >= 358 && NINRev < 368)
		NIN_cfg_version = 5;
	else if(NINRev >= 368 && NINRev < 424)
		NIN_cfg_version = 6;
	else if(NINRev >= 424 && NINRev < 431)
		NIN_cfg_version = 7;
	else if(NINRev >= 431 && NINRev < 487)
		NIN_cfg_version = 8;
	
	NinCfg.Version = NIN_cfg_version;

	/* set config options */
	NinCfg.Config = n_cfg;

	/* VideoMode setup */
	NinCfg.VideoMode = n_vm;
	
	NinCfg.VideoScale = vidscale;
	NinCfg.VideoOffset = vidoffset;
	
	/* bba network profile */
	NinCfg.NetworkProfile = netprofile;

	/* language setup */
	if(lang == 0)
		lang = get_wii_language();
	else
		lang--;

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

	/* MemCard Blocks Setup */
	if(NinCfg.Config & NIN_CFG_MC_MULTI)
		NinCfg.MemCardBlocks = 0x4; //1019 blocks (8MB)
	else
		NinCfg.MemCardBlocks = 0x2; //251 blocks (2MB)

	/* CheatPath Setup */
	if(CheatPath != NULL && (NinCfg.Config & NIN_CFG_CHEATS))
		snprintf(NinCfg.CheatPath, sizeof(NinCfg.CheatPath), strchr(CheatPath, '/'));
	
	/* GamePath Setup */
	if(strlen(gamePath) == 2 && strcmp(gamePath, "di") == 0)
		strcpy(NinCfg.GamePath, gamePath);
	else
	{
		strncpy(NinCfg.GamePath, strchr(gamePath, '/'), 254);
		if(strstr(NinCfg.GamePath, "boot.bin") != NULL)
		{
			*strrchr(NinCfg.GamePath, '/') = '\0'; //boot.bin
			*(strrchr(NinCfg.GamePath, '/')+1) = '\0'; //sys
		}
	}
	
	/* GameID Setup */
	memcpy(&NinCfg.GameID, gameID, 4);
	gprintf("Nintendont Game Path: %s, ID: %08x\n", NinCfg.GamePath, NinCfg.GameID);
	
	gprintf("Writing Arguments\n");
	AddBootArgument((char*)&NinCfg, sizeof(NIN_CFG));
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

bool Nintendont_GetLoader(bool use_slippi)
{
	slippi = use_slippi;
	bool ret = false;
	const char *dol_path = NULL;
	for(u8 i = SD; i < MAXDEVICES; ++i)
	{
		if(slippi)
			dol_path = fmt(NIN_SLIPPI_PATH, DeviceName[i]);
		else
			dol_path = fmt(NIN_LOADER_PATH, DeviceName[i]);
		ret = (LoadHomebrew(dol_path) == 1);
		if(ret == true)
		{
			gprintf("Nintendont loaded: %s\n", dol_path);
			AddBootArgument(dol_path);
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

void DEVO_SetOptions(const char *isopath, const char *gameID, u8 videomode, u8 lang, 
						bool memcard_emu, bool widescreen, bool activity_led, bool wifi)
{
	struct stat st;

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
	// if wiiflow is using a cIOS the custom disc interface device usbstorage.c will cause st.st_dev to return 'WUMS' instead of 'WUSB'.
	// if wiiflow is using IOS58 usbstorage_libogc interface and st.st_dev will return the proper result 'WUSB'.
	// for sd it always return 'WISD'.
	// only last two letters are returned by DevkitPro 'SD', 'SB' or 'MS'.
	// so we use this little trick to make sure device signature is always set to the proper 2 letter ID.
	DEVO_CONFIG->device_signature = st.st_dev == 'SD' ? 'SD' : 'SB';
	DEVO_CONFIG->disc1_cluster = st.st_ino;

	// Pergame options
	if(wifi)
		DEVO_CONFIG->options |= DEVO_CONFIG_WIFILOG;
	if(widescreen)
		DEVO_CONFIG->options |= DEVO_CONFIG_WIDE;
	if(!activity_led)
		DEVO_CONFIG->options |= DEVO_CONFIG_NOLED;

	// If 2nd iso file tell Devo about it
	char iso2path[256];
	memset(iso2path, 0, sizeof(iso2path));
	strncpy(iso2path, isopath, 255);
	char *ptz = strstr(iso2path, "game.iso");
	if(ptz != NULL)
	{
		memcpy(ptz, "gam1.iso", 8);
		f = fopen(iso2path, "rb");
		if(f == NULL)
		{
			memcpy(ptz, "gam2.iso", 8);
			f = fopen(iso2path, "rb");
			if(f == NULL)
			{
				memcpy(ptz, "disc2.iso", 9);
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

	// setup memcard
	int data_fd;
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

	// GX Render Mode (rmode) and register (rmode_reg)
	GXRModeObj *rmode = VIDEO_GetPreferredMode(0);
	int rmode_reg = 0;// VI_NTSC
	
	switch (videomode)
	{
		case 1:// PAL50
			rmode = &TVPal528IntDf;
			rmode_reg = 1;// VI_PAL
			break;
		case 2:// PAL60 480i
			rmode = &TVEurgb60Hz480IntDf;
			rmode_reg = 5;// VI_EURGB60
			break;
		case 3:// NTSC 480i
		default:
			rmode = &TVNtsc480IntDf;
			break;
	}
	
	/* Set video mode register */
	*Video_Mode = rmode_reg;
	DCFlushRange((void*)Video_Mode, 4);

	/* Set video mode */
	if(rmode != NULL)
		VIDEO_Configure(rmode);

	/* Setup video */
	VIDEO_SetBlack(TRUE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();
	else while(VIDEO_GetNextField())
		VIDEO_WaitVSync();

	/* language setup */
	if(lang == 0)
		lang = get_wii_language();
	else
		lang--;

	// sram settins for devo language only
	syssram *sram;
	sram = __SYS_LockSram();
	sram->lang = lang;
	__SYS_UnlockSram(1); // 1 -> write changes
	while(!__SYS_SyncSram())
		usleep(100);
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
