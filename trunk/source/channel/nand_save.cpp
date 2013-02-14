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
#include <malloc.h>
#include "nand_save.hpp"
#include "nand.hpp"
#include "identify.h"
#include "gecko/gecko.hpp"
#include "loader/fs.h"
#include "loader/sys.h"
#include "banner/AnimatedBanner.h"
#include "unzip/U8Archive.h"

extern const u8 save_bin[];
extern const u32 save_bin_size;

NandSave InternalSave;

#define BANNER_PATH		"/title/00010000/57465346/data/banner.bin"
#define IOS_SAVE_PATH	"/title/00010000/57465346/data/ios"
#define PORT_SAVE_PATH	"/title/00010000/57465346/data/port"

NandSave::NandSave()
{
	ret = 0;
	fd = 0;
	memset(&ISFS_Path, 0, ISFS_MAXPATH);
	loaded = false;
}

bool NandSave::CheckSave()
{
	/* 10 million variables */
	u32 u8_bin_size = 0;
	u8 *u8_bin = NULL;
	u32 certSize = 0;
	signed_blob *certBuffer = NULL;
	u32 tmd_bin_size = 0;
	const signed_blob *tmd_bin = NULL;
	u32 tik_bin_size = 0;
	const signed_blob *tik_bin = NULL;
	u32 banner_bin_size = 0;
	const u8 *banner_bin = NULL;
	u32 entries = 0;
	/* May our banner already exist */
	memset(&ISFS_Path, 0, ISFS_MAXPATH);
	strcpy(ISFS_Path, BANNER_PATH);
	fd = ISFS_Open(ISFS_Path, ISFS_OPEN_READ);
	if(fd >= 0)
	{
		ISFS_Close(fd);
		gprintf("Found WiiFlow Save\n");
		goto done;
	}
	/* extract our archive */
	u8_bin = DecompressCopy(save_bin, save_bin_size, &u8_bin_size);
	if(u8_bin == NULL || u8_bin_size == 0)
		goto error;
	/* grab cert.sys */
	memset(&ISFS_Path, 0, ISFS_MAXPATH);
	strcpy(ISFS_Path, "/sys/cert.sys");
	certBuffer = (signed_blob*)ISFS_GetFile(ISFS_Path, &certSize, -1);
	if(certBuffer == NULL || certSize == 0)
		goto error;
	/* Install tik and tmd */
	tik_bin = (const signed_blob*)u8_get_file(u8_bin, "tik.bin", &tik_bin_size);
	if(tik_bin == NULL || tik_bin_size == 0)
		goto error;
	ret = ES_AddTicket(tik_bin, tik_bin_size, certBuffer, certSize, NULL, 0);
	if(ret < 0)
		goto error;
	tmd_bin = (const signed_blob*)u8_get_file(u8_bin, "tmd.bin", &tmd_bin_size);
	if(tmd_bin == NULL || tmd_bin_size == 0)
		goto error;
	ret = ES_AddTitleStart(tmd_bin, tmd_bin_size, certBuffer, certSize, NULL, 0);
	if(ret < 0)
		goto error;
	ret = ES_AddTitleFinish();
	if(ret < 0)
		goto error;
	/* WARNING dirty, delete tik again */
	memset(&ISFS_Path, 0, ISFS_MAXPATH);
	strcpy(ISFS_Path, "/ticket/00010000/57465346.tik");
	ret = ISFS_Delete(ISFS_Path);
	if(ret < 0)
		goto error;
	/* Delete the unused ticket folder */
	memset(&ISFS_Path, 0, ISFS_MAXPATH);
	strcpy(ISFS_Path, "/ticket/00010000");
	ret = ISFS_ReadDir(ISFS_Path, NULL, &entries);
	if(ret < 0)
		goto error;
	if(entries == 0)
	{
		ret = ISFS_Delete(ISFS_Path);
		if(ret < 0)
			goto error;
	}
	banner_bin = u8_get_file(u8_bin, "banner.bin", &banner_bin_size);
	if(banner_bin == NULL || banner_bin_size == 0)
		goto error;
	memset(&ISFS_Path, 0, ISFS_MAXPATH);
	strcpy(ISFS_Path, BANNER_PATH);
	/* Write our banner */
	ISFS_CreateFile(ISFS_Path, 0, 3, 3, 3);
	fd = ISFS_Open(ISFS_Path, ISFS_OPEN_WRITE);
	if(fd < 0)
		goto error;
	ret = ISFS_Write(fd, banner_bin, banner_bin_size);
	ISFS_Close(fd);
	if(ret < 0)
	{
		ISFS_Delete(ISFS_Path);
		goto error;
	}
	free(certBuffer);
	free(u8_bin);
	gprintf("Created WiiFlow Save\n");
done:
	loaded = true;
	return loaded;

error:
	gprintf("Error while creating WiiFlow Save\n");
	loaded = false;
	ES_AddTitleCancel();
	if(certBuffer != NULL)
		free(certBuffer);
	certBuffer = NULL;
	if(u8_bin != NULL)
		free(u8_bin);
	u8_bin = NULL;
	tik_bin = NULL;
	tmd_bin = NULL;
	banner_bin = NULL;
	return loaded;
}

void NandSave::LoadSettings()
{
	if(loaded == false)
		return;

	u32 size = 0;
	memset(&ISFS_Path, 0, ISFS_MAXPATH);
	strcpy(ISFS_Path, IOS_SAVE_PATH);
	ios_settings_t *file = (ios_settings_t*)ISFS_GetFile(ISFS_Path, &size, -1);
	if(file != NULL && size == sizeof(ios_settings_t))
	{
		gprintf("Loading IOS Settings from NAND\n");
		if(file->cios > 0)
			mainIOS = file->cios;
		useMainIOS = file->use_cios;
	}
	if(file != NULL)
		free(file);

	strcpy(ISFS_Path, PORT_SAVE_PATH);
	u8 *port = ISFS_GetFile(ISFS_Path, &size, -1);
	if(port != NULL && size == sizeof(u8))
	{
		gprintf("Using Port Settings from NAND\n");
		currentPort = port[0] & 1;
	}
	if(port != NULL)
		free(port);
}

void NandSave::SaveIOS(u8 ios, bool use_ios)
{
	if(loaded == false)
		return;
	memset(&ios_settings, 0, sizeof(ios_settings_t));
	ios_settings.cios = ios;
	ios_settings.use_cios = use_ios;
	gprintf("Saving IOS Settings to NAND\n");
	WriteFile(IOS_SAVE_PATH, (u8*)&ios_settings, sizeof(ios_settings_t));
}

void NandSave::SavePort(u8 port)
{
	if(loaded == false)
		return;
	gprintf("Saving Port Settings to NAND\n");
	WriteFile(PORT_SAVE_PATH, &port, sizeof(port));
}

void NandSave::WriteFile(const char *file_name, u8 *content, u32 size)
{
	memset(&ISFS_Path, 0, ISFS_MAXPATH);
	if(file_name == NULL || content == NULL || size == 0)
		return;
	strcpy(ISFS_Path, file_name);
	ISFS_CreateFile(ISFS_Path, 0, 3, 3, 3);
	fd = ISFS_Open(ISFS_Path, ISFS_OPEN_WRITE);
	if(fd < 0)
		return;
	ret = ISFS_Write(fd, content, size);
	ISFS_Close(fd);
	if(ret < 0)
		ISFS_Delete(ISFS_Path);
}
