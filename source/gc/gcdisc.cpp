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
#include <string.h>

#include "gc.hpp"
#include "gcdisc.hpp"
#include "fileOps/fileOps.h"
#include "loader/gc_disc_dump.hpp"
#include "gecko/gecko.hpp"
#include "memory/mem2.hpp"
#include "gui/fmt.h"

GC_Disc GC_Disc_Reader;

void GC_Disc::init(const char *path)
{
	strncpy(GamePath, path, MAX_FAT_PATH);
	opening_bnr = NULL;
	FSTable = NULL;

	FILE *f = NULL;
	u32 FSTSize = 0;
	if(strstr(GamePath, "boot.bin") != NULL)
	{
		GameType = TYPE_FST;
		if(strchr(GamePath, '/') != NULL) //boot.bin
			*strrchr(GamePath, '/') = '\0';
		if(strchr(GamePath, '/') != NULL) //sys
			*strrchr(GamePath, '/') = '\0';
		char *FstPath = fmt_malloc("%s/sys/fst.bin", GamePath);
		if(FstPath != NULL)
		{
			fsop_GetFileSizeBytes(FstPath, &FSTSize);
			f = fopen(FstPath, "rb");
			MEM2_free(FstPath);
		}
	}
	else
	{
		GameType = TYPE_ISO;
		f = fopen(GamePath, "rb");
		if(f == NULL)
			return;
		u8 *ReadBuffer = (u8*)MEM2_alloc(0x440);
		if(ReadBuffer == NULL)
			return;
		fread(ReadBuffer, 1, 0x440, f);
		u32 FSTOffset = *(u32*)(ReadBuffer + 0x424);
		FSTSize = *(u32*)(ReadBuffer + 0x428);
		MEM2_free(ReadBuffer);
		fseek(f, FSTOffset, SEEK_SET);
	}
	if(f != NULL)
	{
		Read_FST(f, FSTSize);
		fclose(f);
	}
}

void GC_Disc::clear()
{
	if(opening_bnr != NULL)
	{
		MEM2_free(opening_bnr);
		opening_bnr = NULL;
	}
	if(FSTable != NULL)
	{
		MEM2_free(FSTable);
		FSTable = NULL;
	}
}

void GC_Disc::Read_FST(FILE *f, u32 FST_size)
{
	if(f == NULL)
		return;
	FSTable = (u8*)MEM2_alloc(FST_size);
	if(FSTable == NULL)
		return;
	fread(FSTable, 1, FST_size, f);

	FSTEnt = *(u32*)(FSTable + 0x08);
	FSTNameOff = (char*)(FSTable + FSTEnt * 0x0C);
}

u8 *GC_Disc::GetGameCubeBanner()
{
	if(FSTable == NULL || GamePath == NULL)
		return NULL;

	FILE *bnr_fp = NULL;
	u32 BnrSize = 0;

	FST *fst = (FST*)FSTable;
	for(u32 i = 1; i < FSTEnt; ++i)
	{
		if(fst[i].Type) //Folder
			continue;
		else if(strcmp(FSTNameOff + fst[i].NameOffset, "opening.bnr") == 0)
		{
			if(GameType == TYPE_FST)
				bnr_fp = fopen(fmt("%s/root/opening.bnr", GamePath), "rb");
			else
			{
				bnr_fp = fopen(GamePath, "rb");
				if(bnr_fp != NULL)
					fseek(bnr_fp, fst[i].FileOffset, SEEK_SET);
			}
			BnrSize = fst[i].FileLength;
			break;
		}
	}

	if(bnr_fp != NULL)
	{
		opening_bnr = (u8*)MEM2_alloc(BnrSize);
		if(opening_bnr != NULL && fread(opening_bnr, 1, BnrSize, bnr_fp) != BnrSize)
			MEM2_free(opening_bnr);
		fclose(bnr_fp);
	}
	return opening_bnr;
}
