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
#include "external_booter.hpp"
#include "Config.hpp"
#include "fst.h"
#include "mload.h"
#include "wdvd.h"
#include "channel/nand.hpp"
#include "devicemounter/DeviceHandler.hpp"
#include "homebrew/homebrew.h"

/* External WiiFlow Game Booter */
#define EXECUTE_ADDR	((u8 *)0x92000000)

extern const u8 wii_game_booter_dol[];
extern const u32 wii_game_booter_dol_size;

extern "C" {
u8 configbytes[2];
u32 hooktype;
};

the_CFG normalCFG;

extern u8 *code_buf;
extern u32 code_size;
extern void *codelist;
extern u8 *codelistend;
extern u32 gameconfsize;
extern u32 *gameconf;

void WiiFlow_ExternalBooter(u8 vidMode, bool vipatch, bool countryString, u8 patchVidMode, int aspectRatio, u32 returnTo, u8 BootType)
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
	memcpy((void *)0x90000000, &normalCFG, sizeof(the_CFG));
	DCFlushRange((void *)(0x90000000), sizeof(the_CFG));

	ShutdownBeforeExit(true);
	memcpy(EXECUTE_ADDR, wii_game_booter_dol, wii_game_booter_dol_size);
	DCFlushRange(EXECUTE_ADDR, wii_game_booter_dol_size);
	BootHomebrew();
}

extern FragList *frag_list;
extern s32 wbfsDev;
extern u32 wbfs_part_idx;
void ExternalBooter_WiiGameSetup(bool wbfs, bool dvd, const char *ID)
{
	normalCFG.GameBootType = dvd ? TYPE_WII_DISC : (wbfs ? TYPE_WII_WBFS : TYPE_WII_WBFS_EXT);
	strncpy(normalCFG.gameID, ID, 6);
	normalCFG.fragments = frag_list;
	normalCFG.wbfsDevice = wbfsDev;
	normalCFG.wbfsPart = wbfs_part_idx;
	normalCFG.mload_rev = mload_get_version();
}

void ExternalBooter_ChannelSetup(u64 title)
{
	normalCFG.title = title;
}

void ShutdownBeforeExit(bool KeepPatches)
{
	DeviceHandle.UnMountAll();
	Nand::Instance()->DeInit_ISFS(KeepPatches);
	WDVD_Close();
}
