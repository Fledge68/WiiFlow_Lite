#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _FILEOPS
#define _FILEOPS

typedef void (*progress_callback_t)(int status,int total,void *user_data);

bool fsop_GetFileSizeBytes (char *path, size_t *filesize);
u64 fsop_GetFolderBytes (char *source);
u32 fsop_GetFolderKb (char *source);
u32 fsop_GetFreeSpaceKb (char *path);
bool fsop_DirExist (char *path);
bool fsop_CopyFile (char *source, char *target, progress_callback_t spinner, void *spinner_data);
bool fsop_CopyFolder (char *source, char *target, progress_callback_t spinner, void *spinner_data);
void fsop_deleteFolder(char *source);

#endif

#ifdef __cplusplus
}
#endif