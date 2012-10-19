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
#ifndef _CFG_H_
#define _CFG_H_

#include "loader/cios.h"
#include "loader/frag.h"
#include "loader/wip.h"

typedef struct _the_CFG {
	/* needed for wii games */
	char gameID[7];
	FragList *fragments;
	s32 wbfsDevice;
	u32 wbfsPart;
	u8 GameBootType;
	u8 mload_rev;
	WIP_Code *wip_list;
	u32 wip_count;
	/* needed for channels */
	u64 title;
	/* General Stuff */
	IOS_Info IOS;
	u8 BootType;
	u8 vidMode;
	u8 patchVidMode;
	u8 configbytes[2];
	u8 debugger;
	u8 vipatch;
	u8 countryString;
	int aspectRatio;
	void *codelist;
	u8 *codelistend;
	u8 *cheats;
	u32 cheatSize;
	u32 hooktype;
	u32 *gameconf;
	u32 gameconfsize;
	u32 returnTo;
} the_CFG;

#endif /* _CFG_HPP_ */
