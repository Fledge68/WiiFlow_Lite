/*-------------------------------------------------------------

usbstorage_starlet.c -- USB mass storage support, inside starlet
Copyright (C) 2009 Kwiirk

If this driver is linked before libogc, this will replace the original 
usbstorage driver by svpe from libogc
This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1.	The origin of this software must not be misrepresented; you
must not claim that you wrote the original software. If you use
this software in a product, an acknowledgment in the product
documentation would be appreciated but is not required.

2.	Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3.	This notice may not be removed or altered from any source
distribution.

-------------------------------------------------------------*/

#include <gccore.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include "gecko.h"

/* IOCTL commands */
#define UMS_BASE			(('U'<<24)|('M'<<16)|('S'<<8))
#define USB_IOCTL_UMS_INIT	        (UMS_BASE+0x1)
#define USB_IOCTL_UMS_GET_CAPACITY      (UMS_BASE+0x2)
#define USB_IOCTL_UMS_READ_SECTORS      (UMS_BASE+0x3)
#define USB_IOCTL_UMS_WRITE_SECTORS	(UMS_BASE+0x4)
#define USB_IOCTL_UMS_READ_STRESS	(UMS_BASE+0x5)
#define USB_IOCTL_UMS_SET_VERBOSE	(UMS_BASE+0x6)

#define USB_IOCTL_UMS_WATCHDOG		(UMS_BASE+0x80)

#define UMS_HEAPSIZE			0x8000
#define USB_MEM2_SIZE           0x10000

/* Variables */
static char fs[] ATTRIBUTE_ALIGN(32) = "/dev/usb2";
static char fs2[] ATTRIBUTE_ALIGN(32) = "/dev/usb123";
static char fs3[] ATTRIBUTE_ALIGN(32) = "/dev/usb/ehc";
 
static s32 hid = -1, fd = -1;
u32 sector_size;
static void *usb_buf2;

extern void* SYS_AllocArena2MemLo(u32 size,u32 align);

inline s32 __USBStorage_isMEM2Buffer(const void *buffer)
{
	u32 high_addr = ((u32)buffer) >> 24;

	return (high_addr == 0x90) || (high_addr == 0xD0);
}


u32 USBStorage_GetCapacity(u32 *_sector_size)
{
	if (fd >= 0)
	{
		u32 ret;

		ret = IOS_IoctlvFormat(hid, fd, USB_IOCTL_UMS_GET_CAPACITY, ":i", &sector_size);

		static bool first = true;
		if (first)
		{
			gprintf("\nSECTORS:  %lu\n", ret);
			gprintf("SEC SIZE: %lu\n", sector_size);
			u32 size = ((((ret / 1024U) * sector_size) / 1024U) / 1024U);
			if(size >= 1000U)
			{
				 gprintf("HDD SIZE: %lu.%lu TB [%u]\n", size / 1024U, (size * 100U) % 1024U, sector_size);
			}
			else
			{
				 gprintf("HDD SIZE: %lu GB [%u]\n", size, sector_size);
			}
			first = false;
		}

		if (ret && _sector_size)
			*_sector_size = sector_size;

		return ret;
	}

	return 0;
}

s32 USBStorage_OpenDev()
{
	/* Already open */
	if (fd >= 0) return fd;

	/* Create heap */
	if (hid < 0)
	{
		hid = iosCreateHeap(UMS_HEAPSIZE);
		if (hid < 0) return IPC_ENOMEM;  // = -22
	}

	// allocate buf2
	if (usb_buf2 == NULL) usb_buf2 = SYS_AllocArena2MemLo(USB_MEM2_SIZE, 32);

	/* Open USB device */
	fd = IOS_Open(fs, 0);
	if (fd < 0) fd = IOS_Open(fs2, 0);
	if (fd < 0) fd = IOS_Open(fs3, 0);
	return fd;
}

s32 USBStorage_SetWatchdog(u32 seconds)
{
	if (fd < 0) return fd;

	static ioctlv vector[1] ATTRIBUTE_ALIGN(32);		
	static u32 secs[8] ATTRIBUTE_ALIGN(32);

	secs[0] = seconds;
	vector[0].data = secs;
    vector[0].len = 4;

	return IOS_Ioctlv(fd, USB_IOCTL_UMS_WATCHDOG, 1, 0, vector);
}

s32 USBStorage_Init(void)
{
	s32 ret;
	USBStorage_OpenDev();
	if (fd < 0)	return fd;

	/* Initialize USB storage */
	ret = IOS_IoctlvFormat(hid, fd, USB_IOCTL_UMS_INIT, ":");

	/* Get device capacity */
	ret = USBStorage_GetCapacity(NULL);
	if (!ret) goto err;

	return 0;

err:
	/* Close USB device */
	if (fd >= 0)
	{
		IOS_Close(fd);
		fd = -1;
	}
/* 
	if (hid > 0) {
		iosDestroyHeap(hid);
		hid = -1;
	} */
	//USB_Deinitialize(); //Screwing up inputs even worse if i do this here..grr
	return -1;
}

void USBStorage_Deinit(void)
{
	/* Close USB device */
	if (fd >= 0)
	{
		IOS_Close(fd);
		fd = -1;
	}
/* 	if (hid > 0) {
		iosDestroyHeap(hid);
		hid = -1;
	} */
	USB_Deinitialize();
}

s32 USBStorage_ReadSectors(u32 sector, u32 numSectors, void *buffer)
{
	void *buf = (void *)buffer;
	u32	len = (sector_size * numSectors);
	s32 ret;

	/* Device not opened */
	if (fd < 0)
		return fd;

	/* MEM1 buffer */
	if (!__USBStorage_isMEM2Buffer(buffer))
	{
		/* Allocate memory */
		//buf = iosAlloc(hid, len);
		buf = usb_buf2;
		if (!buf) return IPC_ENOMEM;
	}

	/* Read data */
	ret = IOS_IoctlvFormat(hid, fd, USB_IOCTL_UMS_READ_SECTORS, "ii:d", sector, numSectors, buf, len);

	/* Copy data */
	if (buf != buffer)
	{
		memcpy(buffer, buf, len);
		//iosFree(hid, buf);
	}

	return ret;
}

s32 USBStorage_WriteSectors(u32 sector, u32 numSectors, const void *buffer)
{
	void *buf = (void *)buffer;
	u32 len = (sector_size * numSectors);
	s32 ret;

	/* Device not opened */
	if (fd < 0) return fd;

	/* MEM1 buffer */
	if (!__USBStorage_isMEM2Buffer(buffer))
	{
		/* Allocate memory */
		//buf = iosAlloc(hid, len);
		buf = usb_buf2;
		if (!buf) return IPC_ENOMEM;

		/* Copy data */
		memcpy(buf, buffer, len);
	}

	/* Write data */
	ret = IOS_IoctlvFormat(hid, fd, USB_IOCTL_UMS_WRITE_SECTORS, "ii:d", sector, numSectors, buf, len);

	/* Free memory */
	//if (buf != buffer)
	//	iosFree(hid, buf);

	return ret;
}

// DISC_INTERFACE methods

static bool __io_usb_IsInserted(void)
{
	s32 ret;
	u32 sec_size;
	if (fd < 0) return false;
	ret = USBStorage_GetCapacity(&sec_size);
	if (ret == 0) return false;
	if (sec_size != 512) return false;
	return true;
}

static bool __io_usb_Startup(void)
{
	if (USBStorage_Init() < 0) return false;
	return __io_usb_IsInserted();
}

bool __io_usb_ReadSectors(u32 sector, u32 count, void *buffer)
{
	return USBStorage_ReadSectors(sector, count, buffer) >= 0;
}

bool __io_usb_WriteSectors(u32 sector, u32 count, void *buffer)
{
	return USBStorage_WriteSectors(sector, count, buffer) >= 0;
}

static bool __io_usb_ClearStatus(void)
{
	return true;
}

static bool __io_usb_Shutdown(void)
{
	// do nothing
	return true;
}

DISC_INTERFACE __io_usbstorage = {
	DEVICE_TYPE_WII_USB,
	FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_WII_USB,
	(FN_MEDIUM_STARTUP)      &__io_usb_Startup,
	(FN_MEDIUM_ISINSERTED)   &__io_usb_IsInserted,
	(FN_MEDIUM_READSECTORS)  &__io_usb_ReadSectors,
	(FN_MEDIUM_WRITESECTORS) &__io_usb_WriteSectors,
	(FN_MEDIUM_CLEARSTATUS)  &__io_usb_ClearStatus,
	(FN_MEDIUM_SHUTDOWN)     &__io_usb_Shutdown
};