#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _FILEOPS
#define _FILEOPS

#include <dirent.h>
#include <unistd.h>
#include "memory/mem2.hpp"

typedef void (*progress_callback_t)(int status,int total,void *user_data);

bool fsop_GetFileSizeBytes(const char *path, size_t *filesize);
u64 fsop_GetFolderBytes(const char *source);
u32 fsop_GetFolderKb(const char *source);
u32 fsop_GetFreeSpaceKb(const char *path);
bool fsop_CopyFile(const char *source, const char *target, progress_callback_t spinner, void *spinner_data);
bool fsop_CopyFolder(const char *source, const char *target, progress_callback_t spinner, void *spinner_data);
void fsop_deleteFolder(const char *source);

static inline bool fsop_FileExist(const char *fn)
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

static inline u8 *fsop_ReadFile(const char *path, u32 *size)
{
	*(size) = 0;
	if(!fsop_FileExist(path))
		return NULL;
	//gprintf("Reading file: %s\n", path);
	FILE *f = fopen(path, "rb");
	fseek(f, 0, SEEK_END);
	u32 filesize = ftell(f);
	u8 *mem = (u8*)MEM2_alloc(filesize);
	if(mem != NULL)
	{
		rewind(f);
		fread(mem, filesize, 1, f);
		*(size) = filesize;
	}
	fclose(f);

	return mem;
}

static inline bool fsop_WriteFile(const char *path, const void *mem, const u32 size)
{
	if(mem == NULL || size == 0)
		return false;

	FILE *f = fopen(path, "wb");
	if(f == NULL)
		return false;
	//gprintf("Writing file: %s\n", path);
	fwrite(mem, size, 1, f);
	fclose(f);
	return true;
}

static inline void fsop_deleteFile(const char *source)
{
	if(!fsop_FileExist(source))
		return;
	remove(source);
}

static inline bool fsop_FolderExist(const char *path)
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

static inline void fsop_MakeFolder(const char *path)
{
	if(fsop_FolderExist(path))
		return;
	//gprintf("Folder path to create: %s\n", path);
	mkdir(path, S_IREAD | S_IWRITE);
}

#endif

#ifdef __cplusplus
}
#endif