/****************************************************************************
 * Copyright (C) 2010
 * by Dimok
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
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#include <malloc.h>
#include <unistd.h>
#include <string.h>
#include <ogc/mutex.h>
#include <ogc/system.h>
#include <sdcard/gcsd.h>
#include "DeviceHandler.hpp"
#include "sdhc.h"
#include "usbstorage.h"
#include "loader/cios.h"
#include "loader/wbfs.h"

DeviceHandler * DeviceHandler::instance = NULL;

DeviceHandler::~DeviceHandler()
{
	UnMountAll();
}

DeviceHandler * DeviceHandler::Instance()
{
	if(instance == NULL)
		instance = new DeviceHandler();
	return instance;
}

void DeviceHandler::DestroyInstance()
{
	if(instance)
		delete instance;
	instance = NULL;
}

void DeviceHandler::MountAll()
{
	MountSD();
	MountAllUSB();
}

void DeviceHandler::UnMountAll()
{
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
		return SD_Inserted() && sd->IsMounted(0);

	else if(dev >= USB1 && dev <= USB8)
	{
		int portPart = PartitionToPortPartition(dev-USB1);
		PartitionHandle *usb = instance->GetUSBHandleFromPartition(dev-USB1);
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

bool DeviceHandler::MountSD()
{
	if(sd)
	{
		delete sd;
		sd = NULL;
	}
	sd = new PartitionHandle(&__io_sdhc);
	if(sd->GetPartitionCount() < 1)
	{
		delete sd;
		sdhc_mode_sd = 1;
		gprintf("Couldn't find SD Card. Trying __io_wiisd mode\n");
		sd = new PartitionHandle(&__io_sdhc);
	}
	if(sd->GetPartitionCount() < 1)
	{
		delete sd;
		sd = NULL;
		sdhc_mode_sd = 0;
		gprintf("SD Card not found.\n");
		return false;
	}
	gprintf("SD Card found.\n");

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
	if(CurrentIOS.Type == IOS_TYPE_NORMAL_IOS)
		usb_libogc_mode = 1;
	else
		usb_libogc_mode = 0;

	if(!usb0)
		usb0 = new PartitionHandle(GetUSB0Interface());
	//if(!usb1 && (Settings.USBPort == 1 || Settings.USBPort == 2))
		//usb1 = new PartitionHandle(GetUSB1Interface());

	if(usb0 && usb0->GetPartitionCount() < 1)
	{
		delete usb0;
		usb0 = NULL;
	}
	if(usb1 && usb1->GetPartitionCount() < 1)
	{
		delete usb1;
		usb1 = NULL;
	}

	bool result = false;
	int partCount = GetUSBPartitionCount();

	for(int i = 0; i < partCount; i++)
	{
		if(MountUSB(i))
			result = true;
	}

	return result;
}

bool DeviceHandler::MountUSBPort1()
{
	if(!usb1)// && (Settings.USBPort == 1 || Settings.USBPort == 2))
		usb1 = new PartitionHandle(GetUSB1Interface());

	if(usb1 && usb1->GetPartitionCount() < 1)
	{
		delete usb1;
		usb1 = NULL;
		return false;
	}

	bool result = false;
	int partCount = GetUSBPartitionCount();
	int partCount0 = 0;
	if(usb0)
		partCount0 = usb0->GetPartitionCount();

	for(int i = partCount0; i < partCount; i++)
	{
		if(MountUSB(i))
			result = true;
	}

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

int DeviceHandler::PathToDriveType(const char * path)
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

const char * DeviceHandler::GetFSName(int dev)
{
	if(dev == SD && DeviceHandler::instance->sd)
	{
		return DeviceHandler::instance->sd->GetFSName(0);
	}
	else if(dev >= USB1 && dev <= USB8)
	{
		int partCount0 = 0;
		int partCount1 = 0;
		if(DeviceHandler::instance->usb0)
			partCount0 += DeviceHandler::instance->usb0->GetPartitionCount();
		if(DeviceHandler::instance->usb1)
			partCount1 += DeviceHandler::instance->usb1->GetPartitionCount();

		if(dev-USB1 < partCount0 && DeviceHandler::instance->usb0)
			return DeviceHandler::instance->usb0->GetFSName(dev-USB1);
		else if(DeviceHandler::instance->usb1)
			return DeviceHandler::instance->usb1->GetFSName(dev-USB1-partCount0);
	}

	return "";
}

int DeviceHandler::GetFSType(int dev)
{
	if(!instance)
		return -1;

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
	if(!instance)
		return 0;

	u16 partCount0 = 0;
	u16 partCount1 = 0;
	if(instance->usb0)
		partCount0 = instance->usb0->GetPartitionCount();
	if(instance->usb1)
		partCount1 = instance->usb1->GetPartitionCount();

	return partCount0+partCount1;
}

wbfs_t * DeviceHandler::GetWbfsHandle(int dev)
{
    if(dev == SD && DeviceHandler::instance->sd)
        return DeviceHandler::instance->sd->GetWbfsHandle(0);
	else if(dev >= USB1 && dev <= USB8 && DeviceHandler::instance->usb0)
        return DeviceHandler::instance->usb0->GetWbfsHandle(dev-USB1);
	else if(dev >= USB1 && dev <= USB8 && DeviceHandler::instance->usb1)
        return DeviceHandler::instance->usb1->GetWbfsHandle(dev-USB1);

    return NULL;
}

s32 DeviceHandler::Open_WBFS(int dev)
{
	u32 part_lba, part_idx = 1;
	u32 part_fs = GetFSType(dev);
	char *partition = (char *)DeviceName[dev];

	if(dev == SD && IsInserted(dev))
		part_lba = Instance()->sd->GetLBAStart(dev);
	else if(dev >= USB1 && dev <= USB8 && IsInserted(dev))
	{
		part_idx = dev;
		part_lba = Instance()->usb0->GetLBAStart(dev - USB1);
	}
	else
		return -1;

	return WBFS_Init(GetWbfsHandle(dev), part_fs, part_idx, part_lba, partition, dev);
}

int DeviceHandler::PartitionToUSBPort(int part)
{
	if(!DeviceHandler::instance)
		return 0;

	u16 partCount0 = 0;
	if(DeviceHandler::instance->usb0)
		partCount0 = instance->usb0->GetPartitionCount();

	if(!instance->usb0 || part >= partCount0)
		return 1;
	else
		return 0;
}

int DeviceHandler::PartitionToPortPartition(int part)
{
	if(!DeviceHandler::instance)
		return 0;

	u16 partCount0 = 0;
	if(instance->usb0)
		partCount0 = instance->usb0->GetPartitionCount();

	if(!instance->usb0 || part >= partCount0)
		return part-partCount0;
	else
		return part;
}

PartitionHandle *DeviceHandler::GetUSBHandleFromPartition(int part) const
{
	if(PartitionToUSBPort(part) == 0)
		return usb0;
	else
		return usb1;
}
