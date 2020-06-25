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
#include <string.h>
#include <ogc/machine/processor.h>
#include <ogc/lwp_threads.h>
#include <ogc/cache.h>
#include "external_booter.hpp"
#include "Config.h"
#include "channel/nand.hpp"
#include "devicemounter/DeviceHandler.hpp"
#include "fileOps/fileOps.h"
#include "gecko/wifi_gecko.hpp"
#include "gui/text.hpp"
#include "loader/fst.h"
#include "loader/mload.h"
#include "loader/wdvd.h"
#include "loader/sys.h"
#include "homebrew/homebrew.h"
#include "memory/mem2.hpp"
//#include "network/FTP_Dir.hpp"
#include "network/https.h"
#include "plugin/crc32.h"

/* External WiiFlow Game Booter */
the_CFG normalCFG;
#define EXT_ADDR_CFG	((vu32*)0x90100000)
#define EXT_ADDR		((u8*)0x90110000) //later for 0x80A80000
#define LDR_ADDR		((u8*)0x93300000)
#define LDR_ENTRY		((entry)LDR_ADDR)

u8 *extldr_ptr = NULL;
u32 extldr_size = 0;

extern "C" {
u8 configbytes[2];
u32 hooktype;
};

extern u8 *code_buf;
extern u32 code_size;
extern void *codelist;
extern u8 *codelistend;
extern u32 gameconfsize;
extern u32 *gameconf;

u8 *booter_ptr = NULL;
u32 booter_size = 0;

void WiiFlow_ExternalBooter(u8 vidMode, bool vipatch, bool countryString, u8 patchVidMode, 
	int aspectRatio, u32 returnTo, u8 BootType, bool use_led)
{
	normalCFG.vidMode = vidMode;
	normalCFG.vipatch = vipatch;
	normalCFG.countryString = countryString;
	normalCFG.patchVidMode = patchVidMode;
	normalCFG.aspectRatio = aspectRatio;
	normalCFG.returnTo = returnTo;
	normalCFG.configbytes[0] = configbytes[0];
	normalCFG.configbytes[1] = configbytes[1];
	normalCFG.IOS = CurrentIOS;
	normalCFG.codelist = codelist;
	normalCFG.codelistend = codelistend;
	normalCFG.cheats = code_buf;
	normalCFG.cheatSize = code_size;
	normalCFG.hooktype = hooktype;
	normalCFG.debugger = debuggerselect;
	normalCFG.gameconf = gameconf;
	normalCFG.gameconfsize = gameconfsize;
	normalCFG.BootType = BootType;
	normalCFG.use_led = use_led;
	normalCFG.wip_list = get_wip_list();
	normalCFG.wip_count = get_wip_count();
	/* Copy CFG Into lower MEM1 so it doesnt get destroyed */
	DCFlushRange(&normalCFG, sizeof(the_CFG));
	the_CFG *lowCFG = (the_CFG*)MEM1_lo_alloc(sizeof(the_CFG));
	memcpy(lowCFG, &normalCFG, sizeof(the_CFG));
	DCFlushRange(&lowCFG, sizeof(the_CFG));
	*EXT_ADDR_CFG = ((u32)lowCFG);
	/* Unmount devices etc */
	ShutdownBeforeExit();
	/* Set proper time */
	settime(secs_to_ticks(time(NULL) - 946684800));
	/* Copy in booter */
	memcpy(EXT_ADDR, booter_ptr, booter_size);
	DCFlushRange(EXT_ADDR, booter_size);
	/* Loader just for the booter */
	memcpy(LDR_ADDR, extldr_ptr, extldr_size);
	DCFlushRange(LDR_ADDR, extldr_size);
	/* Boot it */
	JumpToEntry(LDR_ENTRY);
}

bool ExternalBooter_LoadBins(const char *binDir)
{
	extldr_ptr = fsop_ReadFile(fmt("%s/ext_loader.bin", binDir), &extldr_size);
	if(extldr_size == 0 || extldr_ptr == NULL)
		return false;

	booter_ptr = fsop_ReadFile(fmt("%s/ext_booter.bin", binDir), &booter_size);
	if(booter_size > 0 && booter_ptr != NULL)
		return true;

	MEM2_free(extldr_ptr);
	extldr_ptr = NULL;
	extldr_size = 0;
	return false;
}

extern FragList *frag_list;
extern s32 wbfsDev;
extern u32 wbfs_part_idx;
void ExternalBooter_WiiGameSetup(bool wbfs, bool dvd, bool patchregion, bool private_server, bool patchFix480p, const char *ID)
{
	memset(&normalCFG, 0, sizeof(the_CFG));
	normalCFG.GameBootType = dvd ? TYPE_WII_DISC : (wbfs ? TYPE_WII_WBFS : TYPE_WII_WBFS_EXT);
	strncpy(normalCFG.gameID, ID, 6);
	normalCFG.fragments = frag_list;
	normalCFG.wbfsDevice = wbfsDev;
	normalCFG.wbfsPart = wbfs_part_idx;
	normalCFG.patchregion = patchregion;
	normalCFG.private_server = private_server;
	normalCFG.patchFix480p = patchFix480p;
}

void ExternalBooter_ChannelSetup(u64 title, bool dol)
{
	memset(&normalCFG, 0, sizeof(the_CFG));
	memcpy(&normalCFG.title, &title, sizeof(u64));
	normalCFG.use_dol = dol;
}

void ShutdownBeforeExit(void)
{
	DeviceHandle.UnMountAll();
	NandHandle.DeInit_ISFS();
	WDVD_Close();
	Close_Inputs();
	/* Deinit network */
	if(networkInit == true)
	{
		while(net_get_status() == -EBUSY)
			usleep(50);
		WiFiDebugger.Close();
		//ftp_endTread();
		wolfSSL_Cleanup();
		net_wc24cleanup();
		net_deinit();
		networkInit = false;
	}
}
