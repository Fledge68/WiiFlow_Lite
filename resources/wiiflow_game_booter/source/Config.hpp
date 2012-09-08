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
#ifndef _CFG_HPP_
#define _CFG_HPP_

#include "cios.h"

typedef struct _the_CFG {
	u8 vidMode;
	bool vipatch;
	bool countryString;
	u8 patchVidMode;
	int aspectRatio;
	u32 returnTo;
	u8 configbytes[2];
	IOS_Info IOS;
	void *codelist;
	u8 *codelistend;
	u8 *cheats;
	u32 cheatSize;
	u32 hooktype;
	u8 debugger;
	u32 *gameconf;
	u32 gameconfsize;
	u8 BootType;
	/* needed for channels */
	u64 title;
} the_CFG;

static the_CFG *conf = (the_CFG*)0x90000000;

#endif /* _CFG_HPP_ */
