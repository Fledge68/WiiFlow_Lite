/****************************************************************************
 * Copyright (C) 2010 by Dimok
 *           (C) 2012 by FIX94
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ***************************************************************************/
#include <malloc.h>
#include <unistd.h>
#include <string.h>
#include <ogc/mutex.h>
#include <ogc/system.h>
#include <sdcard/gcsd.h>
#include "DeviceHandler.hpp"
#include "fat.h"
#include "usbthread.h"
#include "sdhc.h"
#include "wiisd_libogc.h"
#include "usbstorage.h"
#include "usbstorage_libogc.h"
#include "loader/cios.h"
#include "loader/sys.h"
#include "loader/wbfs.h"

DeviceHandler DeviceHandle;

void DeviceHandler::Init()
{
	sd.Init();
	usb.Init();
	OGC_Device.Init();
}

void DeviceHandler::MountAll()
{
	MountSD();
	if(!Sys_DolphinMode())
		MountAllUSB();
}

void DeviceHandler::UnMountAll()
{
	/* Kill possible USB thread */
	KillUSBKeepAliveThread();

	for(u32 i = SD; i < MAXDEVICES; i++)
		UnMount(i);
	USBStorage2_Deinit();
	USB_Deinitialize();
	SDHC_Close();

	sd.Cleanup();
	usb.Cleanup();
}

bool DeviceHandler::Mount(int dev)
{
	if(dev == SD)
		return MountSD();

	else if(dev >= USB1 && dev <= USB8)
		return MountUSB(dev-USB1);

	return false;
}

bool DeviceHandler::IsInserted(int dev)
{
	if(dev == SD)
		return SD_Inserted() && sd.IsMounted(0);
	else if(dev >= USB1 && dev <= USB8)
		return usb.IsMounted(dev-USB1);

	return false;
}

void DeviceHandler::UnMount(int dev)
{
	if(dev == SD)
		UnMountSD();
	else if(dev >= USB1 && dev <= USB8)
		UnMountUSB(dev-USB1);
}

void DeviceHandler::SetModes()
{
	sdhc_mode_sd = 1;
	usb_libogc_mode = 1;
	if(CustomIOS(CurrentIOS.Type))
	{	/* For USB you can use every cIOS */
		usb_libogc_mode = 0;
		/* But not for SD */
		if(CurrentIOS.Type != IOS_TYPE_NEEK2O)
			sdhc_mode_sd = 0;
	}
}

bool DeviceHandler::MountSD()
{
	if(!sd.IsInserted() || !sd.IsMounted(0))
	{
		if(CurrentIOS.Type == IOS_TYPE_HERMES)
		{	/* Slowass Hermes SDHC Module */
			for(int i = 0; i < 50; i++)
			{
				if(SDHC_Init())
					break;
				usleep(1000);
			}
		}
		sd.SetDevice(&__io_sdhc);
		//! Mount only one SD Partition
		return sd.Mount(0, DeviceName[SD], true); /* Force FAT */
	}
	return true;
}

bool DeviceHandler::MountUSB(int pos)
{
	if(pos >= GetUSBPartitionCount())
		return false;
	return usb.Mount(pos, DeviceName[USB1+pos]);
}

bool DeviceHandler::MountAllUSB()
{
	/* Kill possible USB thread */
	KillUSBKeepAliveThread();
	/* Wait for our slowass HDD */
	WaitForDevice(GetUSBInterface());
	/* Get Partitions and Mount them */
	if(!usb.IsInserted() || !usb.IsMounted(0))
		usb.SetDevice(GetUSBInterface());
	bool result = false;
	int partCount = GetUSBPartitionCount();
	for(int i = 0; i < partCount; i++)
	{
		if(MountUSB(i))
			result = true;
	}
	if(!result)
		result = usb.Mount(0, DeviceName[USB1], true); /* Force FAT */
	if(result && usb_libogc_mode)
		CreateUSBKeepAliveThread();
	return result;
}

void DeviceHandler::UnMountUSB(int pos)
{
	if(pos >= GetUSBPartitionCount())
		return;
	return usb.UnMount(pos);
}

void DeviceHandler::UnMountAllUSB()
{
	int partCount = GetUSBPartitionCount();

	for(int i = 0; i < partCount; i++)
		UnMountUSB(i);
}

int DeviceHandler::PathToDriveType(const char *path)
{
	if(!path)
		return -1;

	for(int i = SD; i < MAXDEVICES; i++)
	{
		if(strncasecmp(path, DeviceName[i], strlen(DeviceName[i])) == 0)
			return i;
	}

	return -1;
}

const char *DeviceHandler::GetFSName(int dev)
{
	if(dev == SD)
		return sd.GetFSName(0);
	else if(dev >= USB1 && dev <= USB8)
	{
		if(dev-USB1 < usb.GetPartitionCount())
			return usb.GetFSName(dev-USB1);
	}
	return "";
}

int DeviceHandler::GetFSType(int dev)
{
	const char *FSName = GetFSName(dev);
	if(!FSName) return -1;

	if(strncmp(FSName, "WBFS", 4) == 0)
		return PART_FS_WBFS;
	else if(strncmp(FSName, "FAT", 3) == 0)
		return PART_FS_FAT;
	else if(strncmp(FSName, "NTFS", 4) == 0)
		return PART_FS_NTFS;
	else if(strncmp(FSName, "LINUX", 4) == 0)
		return PART_FS_EXT;

	return -1;
}

u16 DeviceHandler::GetUSBPartitionCount()
{
	return usb.GetPartitionCount();
}

wbfs_t * DeviceHandler::GetWbfsHandle(int dev)
{
	if(dev == SD)
		return sd.GetWbfsHandle(0);
	else if(dev >= USB1 && dev <= USB8)
		return usb.GetWbfsHandle(dev-USB1);
	return NULL;
}

s32 DeviceHandler::OpenWBFS(int dev)
{
	u32 part_lba, part_idx = 1;
	u32 part_fs = GetFSType(dev);
	const char *partition = DeviceName[dev];

	if(dev == SD && IsInserted(dev))
		part_lba = sd.GetLBAStart(dev);
	else if(dev >= USB1 && dev <= USB8 && IsInserted(dev))
	{
		part_idx = dev;
		part_lba = usb.GetLBAStart(dev - USB1);
	}
	else
		return -1;

	return WBFS_Init(GetWbfsHandle(dev), part_fs, part_idx, part_lba, partition);
}

void DeviceHandler::WaitForDevice(const DISC_INTERFACE *Handle)
{
	if(Handle == NULL)
		return;
	time_t timeout = time(NULL);
	while(time(NULL) - timeout < 20)
	{
		if(Handle->startup() && Handle->isInserted())
			break;
		usleep(50000);
	}
}

bool DeviceHandler::MountDevolution()
{
	int NewPartition = (currentPartition == SD ? currentPartition : currentPartition - 1);
	const DISC_INTERFACE *handle = (currentPartition == SD) ? &__io_wiisd_ogc : &__io_usbstorage_ogc;
	/* We need to wait for the device to get ready for a remount */
	WaitForDevice(handle);
	/* Only mount the partition we need */
	OGC_Device.SetDevice(handle);
	return OGC_Device.Mount(NewPartition, DeviceName[currentPartition], true);
}

void DeviceHandler::UnMountDevolution()
{
	int NewPartition = (currentPartition == SD ? currentPartition : currentPartition - 1);
	OGC_Device.UnMount(NewPartition);
	OGC_Device.Cleanup();
}

bool DeviceHandler::UsablePartitionMounted()
{
	for(u8 i = SD; i < MAXDEVICES; i++)
	{
		if(IsInserted(i) && !GetWbfsHandle(i)) //Everything besides WBFS for configuration
			return true;
	}
	return false;
}

bool DeviceHandler::PartitionUsableForNandEmu(int Partition)
{
	if(IsInserted(Partition) && GetFSType(Partition) == PART_FS_FAT)
		return true;

	return false;
}

const DISC_INTERFACE *DeviceHandler::GetUSBInterface()
{
	if(((CurrentIOS.Type == IOS_TYPE_HERMES && CurrentIOS.Version > 4) ||
			(CurrentIOS.Type == IOS_TYPE_D2X && CurrentIOS.Version > 8) ||
			(CurrentIOS.Type == IOS_TYPE_NORMAL_IOS && CurrentIOS.Revision == 58))
			&& currentPort == 1)
		return &__io_usbstorage2_port1;
	return &__io_usbstorage2_port0;
}
