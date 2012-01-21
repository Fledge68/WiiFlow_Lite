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
	struct stat filestat;
	if (stat(path, &filestat) == 0)
	{
		gprintf("%s Exists!\n", path);
		return;
	}
	gprintf("Creating Game TMD: %s\n", path);

	wbfs_disc_t *disc = WBFS_OpenDisc((u8 *) &hdr->hdr.id, (char *) hdr->path);
	if (!disc) return;

	u8 *titleTMD = NULL;
	u32 tmd_size = wbfs_extract_file(disc, (char *) "TMD", (void **)&titleTMD);
	WBFS_CloseDisc(disc);

	if(!titleTMD) return;

	FILE *file = fopen(path, "wb");
	if(file)
	{
		fwrite(titleTMD, 1, tmd_size, file);
		gprintf("Written Game TMD to: %s\n", path);
		fclose(file);
	}
	else gprintf("Openning %s failed returning %i\n", path, file);


	SAFE_FREE(titleTMD);
}

void CreateSavePath(const char *basepath, struct  dir_discHdr *hdr)
{
	CreateNandPath("%s/import", basepath);
	CreateNandPath("%s/meta", basepath);
	CreateNandPath("%s/shared1", basepath);
	CreateNandPath("%s/shared2", basepath);
	CreateNandPath("%s/sys", basepath);
	CreateNandPath("%s/ticket", basepath);
	CreateNandPath("%s/tmp", basepath);
	CreateNandPath("%s/title", basepath);

	const char *titlePath = "/title/00010000";

	if(	memcmp(hdr->hdr.id, "RGW", 3) == 0)
		titlePath = "/title/00010004";

	char fullpath[ISFS_MAXPATH] ATTRIBUTE_ALIGN(32);
	
	snprintf(fullpath, sizeof(fullpath), "%s%s", basepath, titlePath);
	CreateNandPath(fullpath);

	char nandPath[ISFS_MAXPATH] ATTRIBUTE_ALIGN(32);
	snprintf(nandPath, sizeof(nandPath), "%s/%02x%02x%02x%02x", fullpath, hdr->hdr.id[0], hdr->hdr.id[1], hdr->hdr.id[2], hdr->hdr.id[3]);
	CreateNandPath(nandPath);

	CreateNandPath("%s/data", nandPath);
	CreateNandPath("%s/content", nandPath);

	strcat(nandPath, "/content/title.tmd");
	CreateTitleTMD(nandPath, hdr);
}