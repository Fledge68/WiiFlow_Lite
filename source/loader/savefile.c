/****************************************************************************
 * Copyright (C) 2011
 * by Dimok
 * heavily modified by Miigotu
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
 ***************************************************************************/

#include <gctypes.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <gccore.h>

#include "wbfs.h"
#include "utils.h"
#include "gecko.h"
#include "savefile.h"

static void CreateNandPath(const char *path, ...)
{
	char *tmp = NULL;
	va_list va;
	va_start(va, path);
	if((vasprintf(&tmp, path, va) >= 0) && tmp)
	{
		gprintf("Creating Nand Path: %s\n", tmp);
		makedir(tmp);
	}
	va_end(va);
	SAFE_FREE(tmp);
}

void CreateTitleTMD(const char *path, struct dir_discHdr *hdr)
{
	wbfs_disc_t *disc = WBFS_OpenDisc((u8 *) &hdr->hdr.id, (char *)hdr->path);
	if(!disc) 
		return;

	u8 *titleTMD = NULL;
	u32 tmd_size = wbfs_extract_file(disc, (char *) "TMD", (void **)&titleTMD);
	WBFS_CloseDisc(disc);

	if(!titleTMD) 
		return;
		
	u32 highTID = *(u32*)(titleTMD+0x18c);
	u32 lowTID = *(u32*)(titleTMD+0x190);
	
	CreateNandPath("%s/title/%08x/%08x/data", path, highTID, lowTID);
	CreateNandPath("%s/title/%08x/%08x/content", path, highTID, lowTID);
	
	char nandpath[ISFS_MAXPATH];
	snprintf(nandpath, sizeof(nandpath), "%s/title/%08x/%08x/content/title.tmd", path, highTID, lowTID);

	struct stat filestat;
	if (stat(nandpath, &filestat) == 0)
	{
		SAFE_FREE(titleTMD);
		gprintf("%s Exists!\n", nandpath);
		return;
	}
	gprintf("Creating Game TMD: %s\n", nandpath);
	
	FILE *file = fopen(nandpath, "wb");
	if(file)
	{
		fwrite(titleTMD, 1, tmd_size, file);
		gprintf("Written Game TMD to: %s\n", nandpath);
		fclose(file);
	}
	else gprintf("Openning %s failed returning %i\n", nandpath, file);

	SAFE_FREE(titleTMD);
}