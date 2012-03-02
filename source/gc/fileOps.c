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

#include <dirent.h>
#include <unistd.h>
#include <sys/statvfs.h>

#include "fileOps.h"
#include "memory/mem2.hpp"
#include "utils.h"

s_fsop fsop;

// return false if the file doesn't exist
bool fsop_GetFileSizeBytes (char *path, size_t *filesize)	// for me stats st_size report always 0 :(
{
	FILE *f;
	size_t size = 0;
	
	f = fopen(path, "rb");
	if (!f)
	{
		if (filesize) *filesize = size;
		return false;
	}

	//Get file size
	fseek( f, 0, SEEK_END);
	size = ftell(f);
	if (filesize) *filesize = size;
	SAFE_CLOSE(f);
	
	
	return true;
}

/*
Recursive fsop_GetFolderBytes
*/
u64 fsop_GetFolderBytes (char *source)
{
	DIR *pdir;
	struct dirent *pent;
	char newSource[300];
	u64 bytes = 0;
	
	pdir=opendir(source);
	
	while ((pent=readdir(pdir)) != NULL) 
	{
		// Skip it
		if (strcmp (pent->d_name, ".") == 0 || strcmp (pent->d_name, "..") == 0)
			continue;
			
		sprintf (newSource, "%s/%s", source, pent->d_name);
		
		// If it is a folder... recurse...
		if (fsop_DirExist (newSource))
		{
			bytes += fsop_GetFolderBytes (newSource);
		}
		else	// It is a file !
		{
			size_t s;
			fsop_GetFileSizeBytes (newSource, &s);
			bytes += s;
		}
	}
	
	closedir(pdir);
	
	//Debug ("fsop_GetFolderBytes (%s) = %llu", source, bytes);
	
	return bytes;
	}

u32 fsop_GetFolderKb (char *source)
{
	u32 ret = (u32) round ((double)fsop_GetFolderBytes (source) / 1000.0);

	return ret;
}

u32 fsop_GetFreeSpaceKb (char *path) // Return free kb on the device passed
{
	struct statvfs s;
	
	statvfs (path, &s);
	
	u32 ret = (u32)round( ((double)s.f_bfree / 1000.0) * s.f_bsize);
	
	return ret ;
}

	
bool fsop_DirExist (char *path)
{
	DIR *dir;
	
	dir=opendir(path);
	if (dir)
	{
		closedir(dir);
		return true;
	}
	
	return false;
}

bool fsop_CopyFile (char *source, char *target, progress_callback_t spinner, void *spinner_data)
{
	int err = 0;
	fsop.breakop = 0;
	
	u8 *buff = NULL;
	u32 size;
	u32 bytes, rb,wb;
	u32 block = 71680; //70KB
	FILE *fs = NULL, *ft = NULL;
	
	fs = fopen(source, "rb");
	if (!fs)
	{
		return false;
	}

	ft = fopen(target, "wt");
	if (!ft)
	{
		SAFE_CLOSE(fs);
		return false;
	}
	
	//Get file size
	fseek (fs, 0, SEEK_END);
	size = ftell(fs);

	fsop.size = size;
	
	if (size == 0)
	{
		SAFE_CLOSE(fs);
		SAFE_CLOSE(ft);
		return true;
	}
		
	// Return to beginning....
	fseek(fs, 0, SEEK_SET);
	
	buff = MEM2_alloc(block);
	if (buff == NULL) 
	{
		SAFE_CLOSE(fs);
		return false;
	}
	
	bytes = 0;
	bool spinnerFlag = false;
	if (strstr (source, "game.iso")) {
		spinner(bytes, size, spinner_data);
		spinnerFlag = true;
	}
	
	do
	{
		rb = fread(buff, 1, block, fs);
		wb = fwrite(buff, 1, rb, ft);
		
		if (wb != wb) err = 1;
		if (rb == 0) err = 1;
		bytes += rb;
		
		if (spinnerFlag) spinner(bytes, size, spinner_data);
		
		fsop.multy.bytes += rb;
		fsop.bytes = bytes;
		
		if (fsop.breakop) break;
	}
	while (bytes < size && err == 0);

	SAFE_CLOSE(fs);
	SAFE_CLOSE(ft);
	MEM2_free(buff);
	
	if (err) unlink (target);

	if (fsop.breakop || err) return false;
	
	return true;
}

/*
Recursive copyfolder
*/
static bool doCopyFolder (char *source, char *target, progress_callback_t spinner, void *spinner_data)
{
	DIR *pdir;
	struct dirent *pent;
	char newSource[300], newTarget[300];
	bool ret = true;
	
	// If target folder doesn't exist, create it !
	if (!fsop_DirExist(target))
	{
		makedir(target);
	}

	pdir=opendir(source);
	
	while ((pent=readdir(pdir)) != NULL && ret == true) 
	{
		// Skip it
		if (strcmp (pent->d_name, ".") == 0 || strcmp (pent->d_name, "..") == 0)
			continue;
			
		sprintf (newSource, "%s/%s", source, pent->d_name);
		sprintf (newTarget, "%s/%s", target, pent->d_name);
		
		// If it is a folder... recurse...
		if (fsop_DirExist(newSource))
		{
			ret = doCopyFolder(newSource, newTarget, spinner, spinner_data);
		}
		else	// It is a file !
		{
			strcpy(fsop.op, pent->d_name);
			ret = fsop_CopyFile(newSource, newTarget, spinner, spinner_data);
		}
	}
	
	closedir(pdir);

	return ret;
}
	
bool fsop_CopyFolder (char *source, char *target, progress_callback_t spinner, void *spinner_data)
{
	fsop.breakop = 0;
	fsop.multy.startms = ticks_to_millisecs(gettime());
	fsop.multy.bytes = 0;
	fsop.multy.size = fsop_GetFolderBytes (source);

	return doCopyFolder (source, target, spinner, spinner_data);
}
