
#include <stdio.h>
#include <unistd.h>
#include <ogcsys.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/statvfs.h>
#include <ctype.h>

#include "utils.h"
#include "wbfs.h"
#include "wdvd.h"
#include "splits.h"

#include "wbfs_ext.h"
#include "sys.h"
#include "disc.h"

#include "devicemounter/sdhc.h"
#include "devicemounter/usbstorage.h"
#include "libwbfs/libwbfs.h"
#include "gecko/gecko.h"
#include "memory/mem2.hpp"

/* Constants */
#define MAX_NB_SECTORS	32

/* WBFS device */
s32 wbfsDev = WBFS_MIN_DEVICE;

// partition
int wbfs_part_fs  = PART_FS_WBFS;
u32 wbfs_part_idx = 0;
u32 wbfs_part_lba = 0;
u8 wbfs_mounted = 0;

u8 currentPartition = 1;

/* WBFS HDD */
wbfs_t *hdd = NULL;

/* WBFS callbacks */
static rw_sector_callback_t readCallback  = NULL;
static rw_sector_callback_t writeCallback = NULL;

s32 __WBFS_ReadDVD(void *fp, u32 lba, u32 len, void *iobuf)
{
	s32 ret;

	/* Calculate offset */
	u64 offset = ((u64)lba) << 2;

	/* Calcualte sizes */
	u32 mod  = len % 32;
	u32 size = len - mod;

	/* Read aligned data */
	if (size)
	{
		ret = WDVD_UnencryptedRead(iobuf, size, offset);
		if (ret < 0)
			goto out;
	}

	/* Read non-aligned data */
	if (mod)
	{
		/* Allocate memory */
		fp = memalign(0x20, 0x20);
		if (!fp)
			return -1;

		/* Read data */
		ret = WDVD_UnencryptedRead(fp, 0x20, offset + size);
		if (ret < 0)
			goto out;

		/* Copy data */
		memcpy(iobuf + size, fp, mod);
	}

	/* Success */
	ret = 0;

out:
	/* Free memory */
	free(fp);

	return ret;
}

s32 __WBFS_ReadUSB(void* fp, u32 lba, u32 count, void *iobuf)
{
	u32 cnt = 0;

	/* Do reads */
	while (cnt < count)
	{
		fp = ((u8 *)iobuf) + (cnt * USBStorage2_GetSectorSize());
		u32 sectors = (count - cnt);

		/* Read sectors is too big */
		if (sectors > MAX_NB_SECTORS)
			sectors = MAX_NB_SECTORS;

		/* USB read */
		s32 ret = USBStorage2_ReadSectors(USBStorage2_GetPort(), lba + cnt, sectors, fp);
		if (ret < 0) return ret;

		/* Increment counter */
		cnt += sectors;
	}

	return 0;
}

s32 __WBFS_WriteUSB(void* fp, u32 lba, u32 count, void *iobuf)
{
	u32 cnt = 0;

	/* Do writes */
	while (cnt < count)
	{
		fp = ((u8 *)iobuf) + (cnt * USBStorage2_GetSectorSize());
		u32 sectors = (count - cnt);

		/* Write sectors is too big */
		if (sectors > MAX_NB_SECTORS)
			sectors = MAX_NB_SECTORS;

		/* USB write */
		s32 ret = USBStorage2_WriteSectors(USBStorage2_GetPort(), lba + cnt, sectors, fp);
		if (ret < 0) return ret;

		/* Increment counter */
		cnt += sectors;
	}

	return 0;
}

s32 __WBFS_ReadSDHC(void* fp, u32 lba, u32 count, void *iobuf)
{
	u32 cnt = 0;

	/* Do reads */
	while (cnt < count)
	{
		fp = ((u8 *)iobuf) + (cnt * 512);
		u32 sectors = (count - cnt);

		/* Read sectors is too big */
		if (sectors > MAX_NB_SECTORS)
			sectors = MAX_NB_SECTORS;

		/* SDHC read */
		s32 ret = SDHC_ReadSectors(lba + cnt, sectors, fp);
		if (!ret) return -1;

		/* Increment counter */
		cnt += sectors;
	}

	return 0;
}

s32 __WBFS_WriteSDHC(void* fp, u32 lba, u32 count, void *iobuf)
{
	u32 cnt = 0;
	s32 ret;

	/* Do writes */
	while (cnt < count)
	{
		fp = ((u8 *)iobuf) + (cnt * 512);
		u32 sectors = (count - cnt);

		/* Write sectors is too big */
		if (sectors > MAX_NB_SECTORS)
			sectors = MAX_NB_SECTORS;

		/* SDHC write */
		ret = SDHC_WriteSectors(lba + cnt, sectors, fp);
		if (!ret) return -1;

		/* Increment counter */
		cnt += sectors;
	}

	return 0;
}

bool WBFS_Close()
{
	wbfs_part_fs = 0;
	wbfs_part_lba = 0;
	wbfs_part_idx = 0;
	strcpy(wbfs_fs_drive, "");
	wbfs_mounted = 0;

	return 0;
}

bool WBFS_Mounted()
{
	return wbfs_mounted != 0;
}

s32 WBFS_Init(wbfs_t * handle, u32 part_fs, u32 part_idx, u32 part_lba, char *partition, u8 current)
{
	WBFS_Close();

	hdd = handle;
	wbfsDev = strncasecmp(partition, "sd", 2) == 0 ? WBFS_DEVICE_SDHC : WBFS_DEVICE_USB;
	strcpy(wbfs_fs_drive, partition);
	strcat(wbfs_fs_drive, ":");

	wbfs_part_fs  = part_fs;
	wbfs_part_lba = part_lba;
	wbfs_part_idx = part_idx;

	currentPartition = current;

	wbfs_mounted = 1;

	return 0;
}

s32 WBFS_Format(u32 lba, u32 size)
{
	u32 sector_size = (currentPartition == 0) ? 512 : USBStorage2_GetSectorSize();
	u32 wbfs_sector_size = sector_size;
	u32 partition_num_sec = size;

    //! If size is over 500GB in sectors and sector size is 512
    //! set 2048 as hdd sector size
    if(size > 1048576000 && sector_size == 512)
    {
        wbfs_sector_size = 2048;
        partition_num_sec = size/(2048/sector_size);
    }
	
	/* Reset partition */
	wbfs_t *partition = wbfs_open_partition(readCallback, writeCallback, NULL, wbfs_sector_size, partition_num_sec, lba, 1);
	if (!partition) return -1;

	/* Free memory */
	wbfs_close(partition);

	return 0;
}

s32 WBFS_CheckGame(u8 *discid, char *path)
{
	/* Try to open game disc */
	wbfs_disc_t *disc = WBFS_OpenDisc(discid, path);
	if (disc) WBFS_CloseDisc(disc);

	return !!disc;
}

s32 WBFS_AddGame(progress_callback_t spinner, void *spinner_data)
{
	if (wbfs_part_fs) return WBFS_Ext_AddGame(spinner, spinner_data);

	/* No device open */
	if (!hdd) return -1;

	/* Add game to device */
	s32 ret = wbfs_add_disc(hdd, __WBFS_ReadDVD, NULL, spinner, spinner_data, REMOVE_UPDATE_PARTITION, 0);

	return ret < 0 ? ret : 0;
}

s32 WBFS_RemoveGame(u8 *discid, char *path)
{
	if (wbfs_part_fs) return WBFS_Ext_RemoveGame(discid, path);

	/* No device open */
	if (!hdd) return -1;

	/* Remove game from device */
	s32 ret = wbfs_rm_disc(hdd, discid);

	return ret < 0 ? ret : 0;
}

s32 WBFS_GameSize(u8 *discid, char *path, f32 *size)
{
	/* Open disc */
	wbfs_disc_t *disc = WBFS_OpenDisc(discid, path);
	if (!disc) return -2;

	/* Get game size in sectors */
	u32 sectors = wbfs_disc_sector_used(disc, NULL);

	/* Copy value */
	*size = (disc->p->wbfs_sec_sz / GB_SIZE) * sectors;

	/* Close disc */
	WBFS_CloseDisc(disc);

	return 0;
}

s32 WBFS_DVD_Size(u64 *comp_size, u64 *real_size)
{
	if (wbfs_part_fs) return WBFS_Ext_DVD_Size(comp_size, real_size);

	u32 comp_sec = 0, last_sec = 0;

	/* No device open */
	if (!hdd) return -1;

	/* Add game to device */
	s32 ret = wbfs_size_disc(hdd, __WBFS_ReadDVD, NULL, REMOVE_UPDATE_PARTITION, &comp_sec, &last_sec);
	if (ret < 0) return ret;

	*comp_size = ((u64)hdd->wii_sec_sz) * comp_sec;
	*real_size = ((u64)hdd->wii_sec_sz) * (last_sec+1);

	return 0;
}


s32 WBFS_DiskSpace(f32 *used, f32 *free)
{
	if (wbfs_part_fs) return WBFS_Ext_DiskSpace(used, free);

	/* No device open */
	if (!hdd) return -1;

	/* Count used blocks */
	u32 cnt = wbfs_count_usedblocks(hdd);

	/* Sector size in GB */
	f32 ssize = hdd->wbfs_sec_sz / GB_SIZE;

	/* Copy values */
	*free = ssize * cnt;
	*used = ssize * (hdd->n_wbfs_sec - cnt);

	return 0;
}

wbfs_disc_t* WBFS_OpenDisc(u8 *discid, char *path)
{
	if (wbfs_part_fs)
		return WBFS_Ext_OpenDisc(discid, path);

	/* No device open */
	if (!hdd)
		return NULL;

	/* Open disc */
	return wbfs_open_disc(hdd, discid);
}

void WBFS_CloseDisc(wbfs_disc_t *disc)
{
	if (wbfs_part_fs)
		return WBFS_Ext_CloseDisc(disc);

	/* No device open */
	if (!hdd || !disc)
		return;

	/* Close disc */
	wbfs_close_disc(disc);
}
