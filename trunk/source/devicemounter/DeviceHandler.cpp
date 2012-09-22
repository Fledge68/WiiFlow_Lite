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
#include <sdcard/wiisd_io.h>
#include "DeviceHandler.hpp"
#include "fat.h"
#include "sdhc.h"
#include "usbthread.h"
#include "usbstorage.h"
#include "usbstorage_libogc.h"
#include "loader/cios.h"
#include "loader/sys.h"
#include "loader/wbfs.h"

DeviceHandler DeviceHandle;

void DeviceHandler::Init()
{
	sd = NULL;
	gca = NULL;
	gcb = NULL;
	usb0 = NULL;
	usb1 = NULL;
	OGC_Device = NULL;
	DolphinSD = false;
}

void DeviceHandler::MountAll()
{
	if(Sys_DolphinMode())
	{
		DolphinSD = fatMountSimple("sd", &__io_wiisd);
		return;
	}
	MountSD();
	MountAllUSB();
}

void DeviceHandler::UnMountAll()
{
	if(Sys_DolphinMode())
	{
		fatUnmount("sd");
		DolphinSD = false;
		return;
	}
	/* Kill possible USB thread */
	KillUSBKeepAliveThread();

	for(u32 i = SD; i < MAXDEVICES; i++)
		UnMount(i);

	if(sd)
		delete sd;
	if(usb0)
		delete usb0;
	if(usb1)
		delete usb1;

	sd = NULL;
	usb0 = NULL;
	usb1 = NULL;

	USBStorage2_Deinit();
	USB_Deinitialize();
	SDHC_Close();
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
		return Sys_DolphinMode() ? DolphinSD : SD_Inserted() && sd->IsMounted(0);

	else if(dev >= USB1 && dev <= USB8)
	{
		int portPart = PartitionToPortPartition(dev-USB1);
		PartitionHandle *usb = GetUSBHandleFromPartition(dev-USB1);
		if(usb)
			return usb->IsMounted(portPart);
	}

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
	/* Set for USB */
	if(CurrentIOS.Type == IOS_TYPE_NORMAL_IOS)
		usb_libogc_mode = 1;
	else
		usb_libogc_mode = 0;
	/* Set for SD */
	if(CurrentIOS.Type == IOS_TYPE_D2X)
		sdhc_mode_sd = 0;
	else
		sdhc_mode_sd = 1;
}

bool DeviceHandler::MountSD()
{
	if(!sd)
		sd = new PartitionHandle(&__io_sdhc);
	if(sd && sd->GetPartitionCount() < 1)
	{
		delete sd;
		sd = NULL;
		return false;
	}
	//! Mount only one SD Partition
	return sd->Mount(0, DeviceName[SD], true);
}

bool DeviceHandler::MountUSB(int pos)
{
	if(!usb0 && !usb1)
		return false;

	if(pos >= GetUSBPartitionCount())
		return false;

	int portPart = PartitionToPortPartition(pos);

	if(PartitionToUSBPort(pos) == 0 && usb0)
		return usb0->Mount(portPart, DeviceName[USB1+pos]);
	else if(usb1)
		return usb1->Mount(portPart, DeviceName[USB1+pos]);

	return false;
}

bool DeviceHandler::MountAllUSB()
{
	/* Kill possible USB thread */
	KillUSBKeepAliveThread();
	/* Wait for our slowass HDD */
	WaitForDevice(GetUSB0Interface());
	/* Get Partitions and Mount them */
	if(!usb0)
		usb0 = new PartitionHandle(GetUSB0Interface());
	if(usb0 && usb0->GetPartitionCount() < 1)
	{
		delete usb0;
		usb0 = NULL;
		return false;
	}
	bool result = false;
	int partCount = GetUSBPartitionCount();
	for(int i = 0; i < partCount; i++)
	{
		if(MountUSB(i))
			result = true;
	}
	if(result && usb_libogc_mode)
		CreateUSBKeepAliveThread();
	return result;
}

void DeviceHandler::UnMountUSB(int pos)
{
	if(pos >= GetUSBPartitionCount())
		return;

	int portPart = PartitionToPortPartition(pos);

	if(PartitionToUSBPort(pos) == 0 && usb0)
		return usb0->UnMount(portPart);
	else if(usb1)
		return usb1->UnMount(portPart);
}

void DeviceHandler::UnMountAllUSB()
{
	int partCount = GetUSBPartitionCount();

	for(int i = 0; i < partCount; i++)
		UnMountUSB(i);

	delete usb0;
	usb0 = NULL;
	delete usb1;
	usb1 = NULL;
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
	if(dev == SD && sd)
		return sd->GetFSName(0);
	else if(dev >= USB1 && dev <= USB8)
	{
		int partCount0 = 0;
		int partCount1 = 0;
		if(usb0)
			partCount0 += usb0->GetPartitionCount();
		if(usb1)
			partCount1 += usb1->GetPartitionCount();

		if(dev-USB1 < partCount0 && usb0)
			return usb0->GetFSName(dev-USB1);
		else if(usb1)
			return usb1->GetFSName(dev-USB1-partCount0);
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
	u16 partCount0 = 0;
	u16 partCount1 = 0;
	if(usb0)
		partCount0 = usb0->GetPartitionCount();
	if(usb1)
		partCount1 = usb1->GetPartitionCount();

	return partCount0+partCount1;
}

wbfs_t * DeviceHandler::GetWbfsHandle(int dev)
{
	if(dev == SD && sd)
		return sd->GetWbfsHandle(0);
	else if(dev >= USB1 && dev <= USB8 && usb0)
		return usb0->GetWbfsHandle(dev-USB1);
	else if(dev >= USB1 && dev <= USB8 && usb1)
		return usb1->GetWbfsHandle(dev-USB1);
	return NULL;
}

s32 DeviceHandler::OpenWBFS(int dev)
{
	u32 part_lba, part_idx = 1;
	u32 part_fs = GetFSType(dev);
	char *partition = (char *)DeviceName[dev];

	if(dev == SD && IsInserted(dev))
		part_lba = sd->GetLBAStart(dev);
	else if(dev >= USB1 && dev <= USB8 && IsInserted(dev))
	{
		part_idx = dev;
		part_lba = usb0->GetLBAStart(dev - USB1);
	}
	else
		return -1;

	return WBFS_Init(GetWbfsHandle(dev), part_fs, part_idx, part_lba, partition, dev);
}

int DeviceHandler::PartitionToUSBPort(int part)
{
	u16 partCount0 = 0;
	if(usb0)
		partCount0 = usb0->GetPartitionCount();

	if(!usb0 || part >= partCount0)
		return 1;
	else
		return 0;
}

int DeviceHandler::PartitionToPortPartition(int part)
{
	u16 partCount0 = 0;
	if(usb0)
		partCount0 = usb0->GetPartitionCount();

	if(!usb0 || part >= partCount0)
		return part-partCount0;
	else
		return part;
}

PartitionHandle *DeviceHandler::GetUSBHandleFromPartition(int part)
{
	if(PartitionToUSBPort(part) == 0)
		return usb0;
	else
		return usb1;
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

bool DeviceHandler::MountDevolution(int CurrentPartition)
{
	int NewPartition = (CurrentPartition == SD ? CurrentPartition : CurrentPartition - 1);
	const DISC_INTERFACE *handle = (CurrentPartition == SD) ? &__io_wiisd : &__io_usbstorage_ogc;
	/* We need to wait for the device to get ready for a remount */
	WaitForDevice(handle);
	/* Only mount the partition we need */
	OGC_Device = new PartitionHandle(handle);
	return OGC_Device->Mount(NewPartition, DeviceName[CurrentPartition], true);
}

void DeviceHandler::UnMountDevolution(int CurrentPartition)
{
	int NewPartition = (CurrentPartition == SD ? CurrentPartition : CurrentPartition - 1);
	OGC_Device->UnMount(NewPartition);
	delete OGC_Device;
}

bool DeviceHandler::UsablePartitionMounted()
{
	if(Sys_DolphinMode())
		return DolphinSD;

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
