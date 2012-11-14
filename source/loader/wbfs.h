#ifndef _WBFS_H_
#define _WBFS_H_

#include "libwbfs/libwbfs.h"
#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Macros */
#define WBFS_MIN_DEVICE		1
#define WBFS_MAX_DEVICE		2

#define PART_FS_WBFS 0
#define PART_FS_FAT  1
#define PART_FS_NTFS 2
#define PART_FS_EXT  3

extern s32 wbfsDev;
extern int wbfs_part_fs;
extern u32 wbfs_part_idx;
extern u32 wbfs_part_lba;
extern char wbfs_fs_drive[16];

/* Prototypes */
s32 WBFS_Init(wbfs_t *handle, u32 part_fs, u32 part_idx, u32 part_lba, const char *partition);
s32 WBFS_CheckGame(u8 *, char *);
s32 WBFS_AddGame(progress_callback_t spinner, void *spinner_data);
s32 WBFS_RemoveGame(u8 *, char *);
s32 WBFS_GameSize(u8 *, char *, f32 *);
s32 WBFS_DVD_Size(u64 *comp_size, u64 *real_size);
s32 WBFS_DiskSpace(f32 *, f32 *);

wbfs_disc_t* WBFS_OpenDisc(u8 *discid, char *path);
void WBFS_CloseDisc(wbfs_disc_t *disc);
bool WBFS_Close();
bool WBFS_Mounted();

s32 __WBFS_ReadSDHC(void *fp, u32 lba, u32 count, void *iobuf);
s32 __WBFS_WriteSDHC(void *fp, u32 lba, u32 count, void *iobuf);
s32 __WBFS_ReadUSB(void *fp, u32 lba, u32 count, void *iobuf);
s32 __WBFS_WriteUSB(void *fp, u32 lba, u32 count, void *iobuf);
s32 __WBFS_ReadDVD(void *fp, u32 lba, u32 count, void *iobuf);

#ifdef __cplusplus
}
#endif

#endif
