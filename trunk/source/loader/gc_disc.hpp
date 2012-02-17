#ifndef GC_DISC_H_
#define GC_DISC_H_

typedef void (*progress_callback_t)(int status,int total,void *user_data);

s32 GC_GameDumper(progress_callback_t spinner, void *spinner_data);
s32 GC_DiskSpace(u64 *free);

#endif