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

	bool SD_Inserted() { if(sd) return sd->IsInserted(); return false; }
	bool USB0_Inserted() { if(usb0) return usb0->IsInserted(); return false; }
	bool USB1_Inserted() { if(usb1) return usb1->IsInserted(); return false; }
	bool UsablePartitionMounted();
	bool PartitionUsableForNandEmu(int Partition);
	void WaitForDevice(const DISC_INTERFACE *Handle);

	void UnMountSD() { if(sd) delete sd; sd = NULL; }
	void UnMountUSB(int pos);
	void UnMountAllUSB();

	PartitionHandle * GetSDHandle() const { return sd; }
	PartitionHandle * GetUSB0Handle() const { return usb0; }
	PartitionHandle * GetUSB1Handle() const { return usb1; }

	PartitionHandle * GetUSBHandleFromPartition(int part);
	const DISC_INTERFACE *GetUSB0Interface() { return &__io_usbstorage2_port0; }
	const DISC_INTERFACE *GetUSB1Interface() { return &__io_usbstorage2_port1; }

	int PathToDriveType(const char *path);
	const char * GetFSName(int dev);
	int GetFSType(int dev);
	u16 GetUSBPartitionCount();
	const char *PathToFSName(const char *path) { return GetFSName(PathToDriveType(path)); }
	wbfs_t *GetWbfsHandle(int dev);
	s32 OpenWBFS(int dev);
	int PartitionToUSBPort(int part);
	int PartitionToPortPartition(int part);

	/* Special Devolution Stuff */
	bool MountDevolution(int CurrentPartition);
	void UnMountDevolution(int CurrentPartition);
private:
	bool MountUSB(int part);

	PartitionHandle *sd;
	PartitionHandle *gca;
	PartitionHandle *gcb;
	PartitionHandle *usb0;
	PartitionHandle *usb1;
	/* Special Devolution Stuff */
	PartitionHandle *OGC_Device;
	/* Dolphin Stuff */
	bool DolphinSD;
};

extern DeviceHandler DeviceHandle;

#endif
