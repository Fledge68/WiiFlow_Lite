/*////////////////////////////////////////////////////////////////////////////////////////

fsop contains coomprensive set of function for file and folder handling

en exposed s_fsop fsop structure can be used by callback to update operation status

////////////////////////////////////////////////////////////////////////////////////////*/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <ogcsys.h>
#include <ogc/lwp_watchdog.h>
#include <malloc.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/statvfs.h>

#include "fileOps/fileOps.h"
#include "gecko/gecko.h"
#include "loader/utils.h"

#define SET(a, b) a = b; DCFlushRange(&a, sizeof(a));
#define STACKSIZE 8192

static u8 *buff = NULL;
static FILE *fs = NULL, *ft = NULL;
static u32 block = 32768;
static u32 blockIdx = 0;
static u32 blockInfo[2] = {0,0};
static u32 blockReady = 0;
static s32 stopThread;
static u64 folderSize = 0;
u64 FolderProgressBytes;

// return false if the file doesn't exist
bool fsop_GetFileSizeBytes(char *path, size_t *filesize)	// for me stats st_size report always 0 :(
{
	FILE *f;
	size_t size = 0;

	f = fopen(path, "rb");
	if(!f)
	{
		if(filesize)
			*filesize = size;
		return false;
	}

	//Get file size
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	if(filesize)
		*filesize = size;
	fclose(f);

	return true;
}

/*
Recursive fsop_GetFolderBytes
*/
u64 fsop_GetFolderBytes(const char *source)
{
	DIR *pdir;
	struct dirent *pent;
	char newSource[300];
	u64 bytes = 0;

	pdir = opendir(source);

	while((pent = readdir(pdir)) != NULL) 
	{
		// Skip it
		if(strcmp(pent->d_name, ".") == 0 || strcmp(pent->d_name, "..") == 0)
			continue;

		snprintf(newSource, sizeof(newSource), "%s/%s", source, pent->d_name);

		// If it is a folder... recurse...
		if(fsop_DirExist(newSource))
			bytes += fsop_GetFolderBytes(newSource);
		else	// It is a file !
		{
			size_t s;
			fsop_GetFileSizeBytes(newSource, &s);
			bytes += s;
		}
	}
	closedir(pdir);

	return bytes;
}

u32 fsop_GetFolderKb(const char *source)
{
	u32 ret = (u32)round((double)fsop_GetFolderBytes (source) / 1000.0);

	return ret;
}

u32 fsop_GetFreeSpaceKb(char *path) // Return free kb on the device passed
{
	struct statvfs s;

	statvfs(path, &s);

	u32 ret = (u32)round(((double)s.f_bfree / 1000.0) * s.f_bsize);

	return ret ;
}

bool fsop_FileExist(const char *fn)
{
	FILE * f;
	f = fopen(fn, "rb");
	if(f)
	{
		fclose(f);
		return true;
	}
	return false;
}

bool fsop_DirExist(const char *path)
{
	DIR *dir;

	dir = opendir(path);
	if(dir)
	{
		closedir(dir);
		return true;
	}

	return false;
}


void fsop_MakeFolder(char *path)
{
	if(fsop_DirExist(path))
		return;
	gprintf("Folder path to create: %s\n", path);
	mkdir(path, S_IREAD | S_IWRITE);
}

static void *thread_CopyFileReader()
{
	u32 rb;
	stopThread = 0;
	DCFlushRange(&stopThread, sizeof(stopThread));
	do
	{
		SET(rb, fread(&buff[blockIdx*block], 1, block, fs));
		SET(blockInfo[blockIdx], rb);
		SET(blockReady, 1);

		while(blockReady && !stopThread)
			usleep(1);
	}
	while(stopThread == 0);

	stopThread = -1;
	DCFlushRange(&stopThread, sizeof(stopThread));

	return 0;
}

bool fsop_CopyFile(char *source, char *target, progress_callback_t spinner, void *spinner_data)
{
	gprintf("Creating file: %s\n",target);
	int err = 0;

	u32 size;
	u32 rb, wb;

	fs = fopen(source, "rb");
	if(!fs)
		return false;

	ft = fopen(target, "wt");
	if(!ft)
	{
		fclose(fs);
		return false;
	}

	//Get file size
	fseek(fs, 0, SEEK_END);
	size = ftell(fs);

	if (size == 0)
	{
		fclose(fs);
		fclose(ft);
		return true;
	}

	// Return to beginning....
	fseek(fs, 0, SEEK_SET);

	u8 *threadStack = NULL;
	lwp_t hthread = LWP_THREAD_NULL;

	buff = malloc(block * 2);
	if(buff == NULL)
		return false;

	blockIdx = 0;
	blockReady = 0;
	blockInfo[0] = 0;
	blockInfo[1] = 0;
	u32 bytes = 0;

	threadStack = malloc(STACKSIZE);
	if(threadStack == NULL)
	{
		free(buff);
		return false;
	}

	LWP_CreateThread(&hthread, thread_CopyFileReader, NULL, threadStack, STACKSIZE, 30);

	while(stopThread != 0)
		usleep(5);

	u32 bi;
	do
	{
		while(!blockReady)
			usleep(1); // Let's wait for incoming block from the thread

		bi = blockIdx;

		// let's th thread to read the next buff
		SET(blockIdx, 1 - blockIdx);
		SET(blockReady, 0);

		rb = blockInfo[bi];
		// write current block
		wb = fwrite(&buff[bi*block], 1, rb, ft);

		if(wb != wb || rb == 0)
			err = 1;

		bytes += rb;
		if(spinner)
		{
			FolderProgressBytes += rb;
			spinner(FolderProgressBytes, folderSize, spinner_data);
		}
	}
	while(bytes < size && err == 0);

	stopThread = 1;
	DCFlushRange(&stopThread, sizeof(stopThread));

	while(stopThread != -1)
		usleep(5);

	LWP_JoinThread(hthread, NULL);
	free(threadStack);

	stopThread = 1;
	DCFlushRange(&stopThread, sizeof(stopThread));

	fclose(fs);
	fclose(ft);
	free(buff);

	if(err)
	{
		unlink(target);
		return false;
	}

	return true;
}

/*
Recursive copyfolder
*/
static bool doCopyFolder(char *source, char *target, progress_callback_t spinner, void *spinner_data)
{
	DIR *pdir;
	struct dirent *pent;
	char newSource[300], newTarget[300];
	bool ret = true;

	// If target folder doesn't exist, create it !
	if(!fsop_DirExist(target))
	{
		gprintf("Creating directory: %s\n",target);
		fsop_MakeFolder(target);
	}

	pdir = opendir(source);

	while((pent = readdir(pdir)) != NULL && ret == true) 
	{
		// Skip it
		if(strcmp(pent->d_name, ".") == 0 || strcmp(pent->d_name, "..") == 0)
			continue;

		snprintf(newSource, sizeof(newSource), "%s/%s", source, pent->d_name);
		snprintf(newTarget, sizeof(newTarget), "%s/%s", target, pent->d_name);

		// If it is a folder... recurse...
		if(fsop_DirExist(newSource))
			ret = doCopyFolder(newSource, newTarget, spinner, spinner_data);
		else	// It is a file !
			ret = fsop_CopyFile(newSource, newTarget, spinner, spinner_data);
	}

	closedir(pdir);

	return ret;
}

bool fsop_CopyFolder(char *source, char *target, progress_callback_t spinner, void *spinner_data)
{
	gprintf("DML game USB->SD job started!\n");

	FolderProgressBytes = 0;
	folderSize = fsop_GetFolderBytes(source);
	return doCopyFolder(source, target, spinner, spinner_data);
}

void fsop_deleteFolder(const char *source)
{
	DIR *pdir;
	struct dirent *pent;
	char newSource[300];

	pdir = opendir(source);

	while((pent = readdir(pdir)) != NULL) 
	{
		// Skip it
		if(strcmp(pent->d_name, ".") == 0 || strcmp(pent->d_name, "..") == 0)
			continue;

		snprintf(newSource, sizeof(newSource), "%s/%s", source, pent->d_name);

		// If it is a folder... recurse...
		if(fsop_DirExist(newSource))
			fsop_deleteFolder(newSource);
		else	// It is a file !
			fsop_deleteFile(newSource);
	}
	closedir(pdir);
	gprintf("Deleting directory: %s\n",source);
	unlink(source);
}

void fsop_deleteFile(const char *source)
{
	if(fsop_FileExist(source))
	{
		gprintf("Deleting file: %s\n",source);
		remove(source);
	}
}
