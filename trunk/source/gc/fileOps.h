#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _FILEOPS
#define _FILEOPS

typedef void (*progress_callback_t)(int status,int total,void *user_data);

typedef struct 
	{
	u64 size, bytes; // for operation that uses more than one file
	u32 startms;
	}
s_fsopmulty;

typedef struct 
	{
	char op[256];	// Calling process can set filename or any other info that fit
	
	u32 size, bytes;

	s_fsopmulty multy;
	
	int flag1;		// user defined flag
	bool breakop;	// allow to stop a long operation
	}
s_fsop;

extern s_fsop fsop;

bool fsop_GetFileSizeBytes (char *path, size_t *filesize);
u64 fsop_GetFolderBytes (char *source);
u32 fsop_GetFolderKb (char *source);
u32 fsop_GetFreeSpaceKb (char *path);
bool fsop_DirExist (char *path);
bool fsop_CopyFile (char *source, char *target, progress_callback_t spinner, void *spinner_data);
bool fsop_CopyFolder (char *source, char *target, progress_callback_t spinner, void *spinner_data);

#endif

#ifdef __cplusplus
}
#endif