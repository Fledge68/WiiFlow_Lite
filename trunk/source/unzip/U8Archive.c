/***************************************************************************
 * Copyright (C) 2011
 * by r-win
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * U8Archive.c
 *
 * for Wiiflow 2011
 ***************************************************************************/

#include <string.h>
#include "U8Archive.h"

static char *u8Filename(const struct U8Entry *fst, int i)
{
	return (char *)(fst + fst[0].numEntries) + fst[i].nameOffset;
}

static struct U8Header *open_u8_archive(const u8 *u8_archive)
{
	struct U8Header *arcHdr = (struct U8Header *)u8_archive;
	if (arcHdr->fcc != 0x55AA382D)
	{
		return NULL;
	}
	return arcHdr;
}

const u8 *u8_get_file_by_index(const u8 *archive, u32 index, u32 *size)
{
	struct U8Header *arcHdr = open_u8_archive(archive);
	if (arcHdr == NULL) return NULL;

	const struct U8Entry *fst = (const struct U8Entry *)(archive + arcHdr->rootNodeOffset);
	if (index < 1 || index >= fst[0].numEntries) return NULL;
	if (fst[index].fileType != 0) return NULL; // Not a file, but a directory entry
	
	*size = fst[index].fileLength;
	return archive + fst[index].fileOffset;
}

const u8 *u8_get_file(const u8 *archive, const char *filename, u32 *size)
{
	u32 i = 0;
	struct U8Header *arcHdr = open_u8_archive(archive);
	if (arcHdr == NULL) return NULL;
	
	const struct U8Entry *fst = (const struct U8Entry *)(archive + arcHdr->rootNodeOffset);
	for (i = 1; i < fst[0].numEntries; ++i)
		if (fst[i].fileType == 0 && strcasecmp(u8Filename(fst, i), filename) == 0)
			break;
	if (i >= fst[0].numEntries)
		return NULL;
		
	*size = fst[i].fileLength;
	return archive + fst[i].fileOffset;
}