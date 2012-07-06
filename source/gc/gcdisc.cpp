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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "gcdisc.hpp"
#include "loader/gc_disc_dump.hpp"
#include "mem2.hpp"
#include "gecko.h"
#include "gc.h"

using namespace std;

void GC_Disc::init(char *path)
{
	opening_bnr = NULL;
	strncpy(GamePath, path, sizeof(GamePath));
	FILE *f;
	if(strcasestr(GamePath, "boot.bin") != NULL)
	{
		GameType = TYPE_FST;
		string fst(GamePath);
		fst.erase(fst.end() - 8, fst.end());
		fst.append("fst.bin");
		f = fopen(fst.c_str(), "rb");
		fseek(f, 0, SEEK_END);
		u32 size = ftell(f);
		fseek(f, 0, SEEK_SET);
		Read_FST(f, size);
		fclose(f);
	}
	else
	{
		GameType = TYPE_ISO;
		f = fopen(GamePath, "rb");
		u8 *ReadBuffer = (u8*)MEM2_alloc(0x440);
		fread(ReadBuffer, 1, 0x440, f);
		u32 FSTOffset = *(vu32*)(ReadBuffer+0x424);
		u32 FSTSize = *(vu32*)(ReadBuffer+0x428);
		MEM2_free(ReadBuffer);
		fseek(f, FSTOffset, SEEK_SET);
		Read_FST(f, FSTSize);
		fclose(f);
	}
}

void GC_Disc::clear()
{
	if(opening_bnr)
	{
		MEM2_free(opening_bnr);
		opening_bnr = NULL;
	}
	if(FSTable)
	{
		MEM2_free(FSTable);
		FSTable = NULL;
	}
}

void GC_Disc::Read_FST(FILE *f, u32 FST_size)
{
	FSTable = (u8*)MEM2_alloc(FST_size);
	fread(FSTable, 1, FST_size, f);

	FSTEnt = *(vu32*)(FSTable+0x08);
	FSTNameOff = (char*)(FSTable + FSTEnt * 0x0C);
}

u8 *GC_Disc::GetGameCubeBanner()
{
	FILE *f = NULL;
	FST *fst = (FST *)(FSTable);
	for(u32 i = 1; i < FSTEnt; ++i)
	{
		if(fst[i].Type) //Folder
			continue;
		else if(strcasecmp(FSTNameOff + fst[i].NameOffset, "opening.bnr") == 0)
		{
			if(GameType == TYPE_FST)
			{
				string path(GamePath);
				path.erase(path.end() - 12, path.end());
				path.append("root/opening.bnr");
				f = fopen(path.c_str(), "rb");
			}
			else
			{
				f = fopen(GamePath, "rb");
				fseek(f, fst[i].FileOffset, SEEK_SET);
			}
			if(f)
			{
				opening_bnr = (u8*)MEM2_alloc(fst[i].FileLength);
				fread(opening_bnr, 1, fst[i].FileLength, f);
				fclose(f);
			}
		}
	}
	return opening_bnr;
}
