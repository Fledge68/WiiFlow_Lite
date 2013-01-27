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
#include "booter.h"
#include "Config.h"
#include "channel/nand.hpp"
#include "devicemounter/DeviceHandler.hpp"
#include "gecko/wifi_gecko.hpp"
#include "gui/text.hpp"
#include "loader/fst.h"
#include "loader/mload.h"
#include "loader/wdvd.h"
#include "loader/sys.h"
#include "homebrew/homebrew.h"
#include "memory/mem2.hpp"
#include "network/http.h"
#include "plugin/crc32.h"

/* External WiiFlow Game Booter */
static the_CFG *BooterConfig = (the_CFG*)0x93100000;
#define EXT_ADDR	((u8*)0x80B00000)
#define EXT_ENTRY	((entry)EXT_ADDR)

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

the_CFG normalCFG;
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

	ShutdownBeforeExit();
	/* Copy CFG into new memory region */
	memcpy(BooterConfig, &normalCFG, sizeof(the_CFG));
	DCFlushRange(BooterConfig, sizeof(the_CFG));
	/* Copy in booter */
	memcpy(EXT_ADDR, booter, booter_size);
	DCFlushRange(EXT_ADDR, booter_size);
	/* Wii Games will need it */
	net_wc24cleanup();
	/* Boot it */
	JumpToEntry(EXT_ENTRY);
}

extern FragList *frag_list;
extern s32 wbfsDev;
extern u32 wbfs_part_idx;
void ExternalBooter_WiiGameSetup(bool wbfs, bool dvd, const char *ID)
{
	memset(&normalCFG, 0, sizeof(the_CFG));
	normalCFG.GameBootType = dvd ? TYPE_WII_DISC : (wbfs ? TYPE_WII_WBFS : TYPE_WII_WBFS_EXT);
	strncpy(normalCFG.gameID, ID, 6);
	normalCFG.fragments = frag_list;
	normalCFG.wbfsDevice = wbfsDev;
	normalCFG.wbfsPart = wbfs_part_idx;
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
		net_deinit();
		networkInit = false;
	}
}
