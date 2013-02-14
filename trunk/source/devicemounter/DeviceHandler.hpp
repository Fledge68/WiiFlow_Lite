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
#ifndef DEVICE_HANDLER_HPP_
#define DEVICE_HANDLER_HPP_

#include "PartitionHandle.h"
#include "usbstorage.h"
#include "libwbfs/libwbfs.h"

/**
 * libogc device names.
 */
enum
{
	SD = 0,
	USB1,
	USB2,
	USB3,
	USB4,
	USB5,
	USB6,
	USB7,
	USB8,
	MAXDEVICES
};

/**
 * libogc device names.
 */
const char DeviceName[MAXDEVICES][8] =
{
	"sd",
	"usb1",
	"usb2",
	"usb3",
	"usb4",
	"usb5",
	"usb6",
	"usb7",
	"usb8",
};

class DeviceHandler
{
public:
	void Init();
	void SetModes();
	void MountAll();
	void UnMountAll();
	bool Mount(int dev);
	bool IsInserted(int dev);
	void UnMount(int dev);

	//! Individual Mounts/UnMounts...
	bool MountSD();
	bool MountAllUSB();
	bool MountUSBPort1();

	bool SD_Inserted() { return sd.IsInserted(); }
	bool USB_Inserted() { return usb.IsInserted(); }
	bool UsablePartitionMounted();
	bool PartitionUsableForNandEmu(int Partition);
	void WaitForDevice(const DISC_INTERFACE *Handle);

	void UnMountSD() { sd.UnMountAll(); }
	void UnMountUSB(int pos);
	void UnMountAllUSB();

	PartitionHandle *GetUSBHandleFromPartition(int part);
	const DISC_INTERFACE *GetUSBInterface();

	int PathToDriveType(const char *path);
	const char * GetFSName(int dev);
	int GetFSType(int dev);
	u16 GetUSBPartitionCount();
	const char *PathToFSName(const char *path) { return GetFSName(PathToDriveType(path)); }
	wbfs_t *GetWbfsHandle(int dev);
	s32 OpenWBFS(int dev);

	/* Special Devolution Stuff */
	bool MountDevolution();
	void UnMountDevolution();
private:
	bool MountUSB(int part);

	PartitionHandle sd;
	PartitionHandle usb;
	/* Special Devolution Stuff */
	PartitionHandle OGC_Device;
};

extern DeviceHandler DeviceHandle;

#endif
