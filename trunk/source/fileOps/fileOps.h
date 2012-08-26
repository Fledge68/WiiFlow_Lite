#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _FILEOPS
#define _FILEOPS

typedef void (*progress_callback_t)(int status,int total,void *user_data);

bool fsop_GetFileSizeBytes(char *path, size_t *filesize);
u64 fsop_GetFolderBytes(const char *source);
u32 fsop_GetFolderKb(const char *source);
u32 fsop_GetFreeSpaceKb(char *path);
bool fsop_FileExist(const char *fn);
bool fsop_DirExist(const char *path);
void fsop_MakeFolder(char *path);
bool fsop_CopyFile(char *source, char *target, progress_callback_t spinner, void *spinner_data);
bool fsop_CopyFolder(char *source, char *target, progress_callback_t spinner, void *spinner_data);
void fsop_deleteFile(const char *source);
void fsop_deleteFolder(const char *source);

#endif

#ifdef __cplusplus
}
#endif