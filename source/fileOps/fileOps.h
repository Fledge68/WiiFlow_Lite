#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _FILEOPS
#define _FILEOPS

typedef void (*progress_callback_t)(int status,int total,void *user_data);

bool fsop_GetFileSizeBytes(const char *path, size_t *filesize);
u64 fsop_GetFolderBytes(const char *source);
u32 fsop_GetFolderKb(const char *source);
u32 fsop_GetFreeSpaceKb(const char *path);
bool fsop_FileExist(const char *fn);
bool fsop_DirExist(const char *path);
void fsop_MakeFolder(const char *path);
bool fsop_CopyFile(const char *source, const char *target, progress_callback_t spinner, void *spinner_data);
bool fsop_CopyFolder(const char *source, const char *target, progress_callback_t spinner, void *spinner_data);
void fsop_deleteFile(const char *source);
void fsop_deleteFolder(const char *source);

#endif

#ifdef __cplusplus
}
#endif