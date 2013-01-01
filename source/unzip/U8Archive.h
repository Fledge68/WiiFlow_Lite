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

#ifndef _U8ARCHIVE_H_
#define _U8ARCHIVE_H_

#include <ogcsys.h>

struct U8Header
{
	u32 fcc;
	u32 rootNodeOffset;
	u32 headerSize;
	u32 dataOffset;
	u8 zeroes[16];
} ATTRIBUTE_PACKED;

struct U8Entry
{
	struct
	{
		u32 fileType : 8;
		u32 nameOffset : 24;
	};
	u32 fileOffset;
	union
	{
		u32 fileLength;
		u32 numEntries;
	};
} ATTRIBUTE_PACKED;

#ifdef __cplusplus
extern "C" {
#endif

const u8 *u8_get_file_by_index(const u8 *archive, u32 index, u32 *size);
const u8 *u8_get_file(const u8 *archive, const char *filename, u32 *size);

#ifdef __cplusplus
}
#endif

#endif