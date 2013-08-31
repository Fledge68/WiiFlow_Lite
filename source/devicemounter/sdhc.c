#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ogcsys.h>
#include "sdhc.h"
#include "wiisd_libogc.h"
#include "memory/mem2.hpp"

/* IOCTL comamnds */
#define IOCTL_SDHC_INIT		0x01
#define IOCTL_SDHC_READ		0x02
#define IOCTL_SDHC_WRITE	0x03
#define IOCTL_SDHC_ISINSERTED	0x04

#define SDHC_HEAPSIZE		0x8000
#define SDHC_MEM2_SIZE		0x10000

int sdhc_mode_sd = 0;
int sdhc_inited = 0;

/* Variables */
static char fs[] ATTRIBUTE_ALIGN(32) = "/dev/sdio/sdhc";

static s32 hid = -1, fd = -1;
static u32 sector_size = SDHC_SECTOR_SIZE;
static void *sdhc_buf2 = NULL;

bool SDHC_Init(void)
{
	s32 ret;

	if (sdhc_inited) return true;

	if (sdhc_mode_sd)
	{
		sdhc_inited = __io_wiisd_ogc.startup();
		return sdhc_inited;
	}

	/* Already open */
	if (fd >= 0) return true;

	/* Create heap */
	if (hid < 0) hid = iosCreateHeap(SDHC_HEAPSIZE);
	if (hid < 0) goto err;

	// allocate buf2
	if(sdhc_buf2 == NULL)
		sdhc_buf2 = MEM2_alloc(SDHC_MEM2_SIZE);
	if(sdhc_buf2 == NULL) goto err;

	/* Open SDHC device */
	fd = IOS_Open(fs, 0);
	if (fd < 0) goto err;

	/* Initialize SDHC */
	ret = IOS_IoctlvFormat(hid, fd, IOCTL_SDHC_INIT, ":");
	if (ret) goto err;

	sdhc_inited = 1;
	return true;

err:
	/* Close SDHC device */
	if (fd >= 0)
	{
		IOS_Close(fd);
		fd = -1;
	}

	return false;
}

bool SDHC_Close(void)
{
	sdhc_inited = 0;
	if(sdhc_mode_sd)
		return __io_wiisd_ogc.shutdown();

	/* Close SDHC device */
	if(fd >= 0) {
		IOS_Close(fd);
		fd = -1;
	}
	if(sdhc_buf2 != NULL)
		MEM2_free(sdhc_buf2);
	sdhc_buf2 = NULL;

	return true;
}

bool SDHC_IsInserted(void)
{
	s32 ret;
	if(sdhc_mode_sd)
		return __io_wiisd_ogc.isInserted();

	/* Check if SD card is inserted */
	ret = IOS_IoctlvFormat(hid, fd, IOCTL_SDHC_ISINSERTED, ":");

	return (!ret) ? true : false;
}

bool SDHC_ReadSectors(u32 sector, u32 count, void *buffer)
{
	if(sdhc_mode_sd)
		return __io_wiisd_ogc.readSectors(sector, count, buffer);

	u32 size;
	s32 ret = -1;

	/* Device not opened */
	if (fd < 0)
		return false;

	/* Buffer not aligned */
	if ((u32)buffer & 0x1F)
	{
		if (!sdhc_buf2)
			return false;

		u32 cnt;
		u32 max_sec = SDHC_MEM2_SIZE / sector_size;
		//dbg_printf("sdhc_read(%u,%u) unaligned(%p)\n", sector, count, buffer);
		while (count)
		{
			if (count > max_sec)
				cnt = max_sec;
			else
				cnt = count;

			size = cnt * sector_size;
			ret = IOS_IoctlvFormat(hid, fd, IOCTL_SDHC_READ,
					"ii:d", sector, cnt, sdhc_buf2, size);

			if (ret)
				return false;

			memcpy(buffer, sdhc_buf2, size);
			count -= cnt;
			sector += cnt;
			buffer += size;
		}
	}
	else
	{
		size = sector_size * count;
		/* Read data */
		ret = IOS_IoctlvFormat(hid, fd, IOCTL_SDHC_READ, "ii:d", sector, count, buffer, size);
	}

	return (!ret) ? true : false;
}

bool SDHC_WriteSectors(u32 sector, u32 count, void *buffer)
{
	if(sdhc_mode_sd)
		return __io_wiisd_ogc.writeSectors(sector, count, buffer);

	u32 size;
	s32 ret = -1;

	/* Device not opened */
	if (fd < 0)
		return false;

	/* Buffer not aligned */
	if ((u32)buffer & 0x1F)
	{
		if (!sdhc_buf2)
			return false;

		u32 cnt;
		u32 max_sec = SDHC_MEM2_SIZE / sector_size;

		while (count)
		{
			if (count > max_sec)
				cnt = max_sec;
			else
				cnt = count;

			size = cnt * sector_size;
			memcpy(sdhc_buf2, buffer, size);
			ret = IOS_IoctlvFormat(hid, fd, IOCTL_SDHC_WRITE, "ii:d", sector, cnt, sdhc_buf2, size);

			if (ret)
				return false;

			count -= cnt;
			sector += cnt;
			buffer += size;
		}
	}
	else
	{
		size = sector_size * count;
		/* Read data */
		ret = IOS_IoctlvFormat(hid, fd, IOCTL_SDHC_WRITE, "ii:d", sector, count, buffer, size);
	}

	return (!ret) ? true : false;
}

bool SDHC_ClearStatus(void)
{
	return true;
}

bool __io_SDHC_Close(void)
{
	return true;
}

const DISC_INTERFACE __io_sdhc = {
	DEVICE_TYPE_WII_SD,
	FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_WII_SD,
	(FN_MEDIUM_STARTUP)&SDHC_Init,
	(FN_MEDIUM_ISINSERTED)&SDHC_IsInserted,
	(FN_MEDIUM_READSECTORS)&SDHC_ReadSectors,
	(FN_MEDIUM_WRITESECTORS)&SDHC_WriteSectors,
	(FN_MEDIUM_CLEARSTATUS)&SDHC_ClearStatus,
	(FN_MEDIUM_SHUTDOWN)&SDHC_Close
	//(FN_MEDIUM_SHUTDOWN)&__io_SDHC_Close
};