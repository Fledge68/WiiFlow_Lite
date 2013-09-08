/****************************************************************************
 * Copyright (C) 2013 FIX94
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
#include <string.h>
#include <sys/errno.h>
#include "FTP_Dir.hpp"
#include "ftp.h"
#include "net.h"
#include "gecko/gecko.hpp"
#include "devicemounter/DeviceHandler.hpp"
#include "fileOps/fileOps.h"
#include "gui/fmt.h"
#include "loader/wbfs.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

char main_path[MAXPATHLEN];
char real_path[MAXPATHLEN];
u8 cur_part = 0;

char dbg_messages[6][128];

void ftp_init(void)
{
	memset(main_path, 0, MAXPATHLEN);
	main_path[0] = '/';
	memset(real_path, 0, MAXPATHLEN);
	cur_part = 0;
	for(u8 i = 0; i < 6; ++i)
		memset(dbg_messages[i], 0, 128);
}

const char *ftp_getpath(void)
{
	return main_path;
}

static bool check_device(void)
{
	u8 i;
	cur_part = 0;
	for(i = 0; i < MAXDEVICES; ++i)
	{
		if(strncmp(DeviceName[i], main_path+1, strlen(DeviceName[i])) == 0)
		{
			cur_part = i;
			return true;
		}
	}
	return false;
}

static bool change_real_dir(void)
{
	if(main_path[1] == '\0')
		return true;
	if(check_device() == true)
	{
		strncpy(real_path, fmt("%s:/%s", DeviceName[cur_part], strchr((main_path+1), '/')+1), MAXPATHLEN);
		if(!fsop_FolderExist(real_path))
		{
			errno = ENOTDIR;
			return false;
		}
		return true;
	}
	errno = ENODEV;
	return false;
}

DIR *ftp_diropen()
{
	DIR *cur_dir = NULL;
	if(main_path[1] != '\0')
		cur_dir = opendir(real_path);
	if(cur_dir == NULL)
		errno = EIO;
	return cur_dir;
}

int ftp_dirnext(DIR *dir, char *name)
{
	int ret = -1;
	if(main_path[1] == '\0')
	{
		while(cur_part < MAXDEVICES)
		{
			if(DeviceHandle.IsInserted(cur_part) && DeviceHandle.GetFSType(cur_part) != PART_FS_WBFS)
			{
				strncpy(name, DeviceName[cur_part], 8);
				cur_part++;
				ret = 0;
				break;
			}
			cur_part++;
		}
	}
	else if(dir != NULL)
	{
		while(1)
		{
			struct dirent *pent = readdir(dir);
			if(pent == NULL)
				break;
			else if(pent->d_name[0] == '.')
				continue;
			strncpy(name, pent->d_name, MAXPATHLEN);
			ret = 0;
			break;
		}
	}
	return ret;
}

void ftp_dirclose(DIR *dir)
{
	if(dir != NULL)
		closedir(dir);
	dir = NULL;
}

int ftp_changedir(char *path)
{
	int ret = -1;
	/* main changing */
	if(strcmp(path, "..") == 0)
	{
		/* not in root */
		if(strlen(main_path) > 1)
		{
			/* remove new / */
			if(strchr(main_path, '/') != NULL)
			{
				*(strrchr(main_path, '/')) = '\0';
				/* remove the path before the new / */
				if(strchr(main_path, '/') != NULL)
					*(strrchr(main_path, '/')+1) = '\0';
			}
		}
	}
	else /* enter a new path, do some checks */
	{
		if(path[0] == '/') /* full path */
			strcpy(main_path, path);
		else
			strcat(main_path, path);
	}
	char *last = (main_path+strlen(main_path)-1);
	if(*last != '/')
	{
		*(last+1) = '/';
		*(last+2) = '\0';
	}
	if(change_real_dir() == true)
		ret = 0;
	return ret;
}

int ftp_makedir(char *path)
{
	int ret = -1;
	if(strchr(path, '/') != NULL)
	{
		char *real_path = strrchr(path, '/') + 1;
		if(real_path != '\0')
		{
			*strrchr(path, '/') = '\0';
			ftp_changedir(path);
			path = real_path;
		}
	}
	if(main_path[1] != '\0')
	{
		char *new_dir = fmt("%s%s", real_path, path);
		fsop_MakeFolder(new_dir);
		if(fsop_FolderExist(new_dir) == true)
			ret = 0;
	}
	if(ret < 0)
		errno = EACCES;
	return ret;
}

FILE *ftp_fopen(char *path, char *type)
{
	FILE *fp = NULL;
	if(main_path[1] != '\0')
	{
		char *new_file = fmt("%s%s", real_path, path);
		fp = fopen(new_file, type);
		if(fp != NULL)
			gprintf("Opening file %s in type %s\n", new_file, type);
	}
	if(fp == NULL)
		errno = (strcmp(type, "rb") == 0 ? EACCES : EROFS);
	return fp;
}

void ftp_fclose(FILE *fp)
{
	if(fp != NULL)
	{
		fclose(fp);
		gprintf("Closing file\n");
	}
	fp = NULL;
}

int ftp_stat(char *file, struct stat *st)
{
	if(file == NULL || st == NULL)
		return -1;
	st->st_mtime = 0;
	st->st_size = 0;
	st->st_mode = 0;
	if(main_path[1] == '\0')
		st->st_mode |= S_IFDIR;
	else
		stat(fmt("%s%s", real_path, file), st);
	return 0;
}

int ftp_rename(char *path, char *new_name)
{
	int ret = -1;
	char *old_path = fmt("%s%s", real_path, path);
	char *new_path = fmt("%s%s", real_path, new_name);
	if(fsop_FileExist(old_path))
	{
		gprintf("Renaming File %s to %s\n", old_path, new_path);
		ret = rename(old_path, new_path);
	}
	else if(fsop_FolderExist(old_path))
	{
		gprintf("Renaming Folder %s to %s\n", old_path, new_path);
		ret = rename(old_path, new_path);
	}
	if(ret < 0)
		errno = EIO;
	return ret;
}

int ftp_delete(char *path)
{
	int ret = -1;
	char *old_path = fmt("%s%s", real_path, path);
	if(fsop_FileExist(old_path))
	{
		gprintf("%s is a file, deleting it\n", old_path);
		fsop_deleteFile(old_path);
		ret = 0;
	}
	else if(fsop_FolderExist(old_path))
	{
		gprintf("%s is a folder, deleting it\n", old_path);
		fsop_deleteFolder(old_path);
		ret = 0;
	}
	if(ret < 0)
		errno = ENOENT;
	return ret;
}

lwp_t ftpThrdPtr = LWP_THREAD_NULL;
volatile bool ftpThrd_running = false;
bool end_ftp = false;
s32 cur_server_num = -1;

void *ftp_loopThrd(void *nothing)
{
	while(end_ftp == false)
	{
		process_ftp_events(cur_server_num);
		usleep(100);
	}
	ftpThrd_running = false;
	return nothing;
}

bool ftp_startThread(void)
{
	ftp_endTread();
	if(create_server() == false)
		return false;
	cur_server_num = get_server_num();
	if(cur_server_num < 0)
		return false;

	end_ftp = false;
	ftpThrd_running = true;
	LWP_CreateThread(&ftpThrdPtr, ftp_loopThrd, NULL, NULL, 0, 64);
	return true;
}

void ftp_endTread(void)
{
	if(ftpThrdPtr == LWP_THREAD_NULL)
		return;

	if(LWP_ThreadIsSuspended(ftpThrdPtr))
		LWP_ResumeThread(ftpThrdPtr);
	end_ftp = true;
	while(ftpThrd_running)
		usleep(50);
	LWP_JoinThread(ftpThrdPtr, NULL);
	ftpThrdPtr = LWP_THREAD_NULL;

	end_server();
	cur_server_num = -1;
}

bool dbg_msg_change = false;
void ftp_dbg_print(char *dbg_info)
{
	dbg_msg_change = true;
	/* for gecko and stuff */
	gprintf(dbg_info);
	/* for our gui */
	for(u8 i = 5; i > 0; --i)
		memcpy(dbg_messages[i], dbg_messages[i-1], 128);
	memcpy(dbg_messages[0], dbg_info, 127);
	*(dbg_messages[0]+127) = '\0';
}

const char *ftp_get_prints(u8 i)
{
	return dbg_messages[i];
}

bool ftp_dbg_print_update(void)
{
	if(dbg_msg_change)
	{
		dbg_msg_change = false;
		return true;
	}
	return false;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
