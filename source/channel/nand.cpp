/***************************************************************************
 * Copyright (C) 2011
 * by Miigotu
 * Rewritten code from Mighty Channels and Triiforce
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
 * Nand/Emulation Handling Class
 *
 * for wiiflow 2011
 ***************************************************************************/
 
#include <stdio.h>
#include <ogcsys.h>
#include <malloc.h>
#include <string.h>
#include <cstdlib>

#include "nand.hpp"
#include "utils.h"
#include "gecko.h"
#include "mem2.hpp"

static NandDevice NandDeviceList[] = {
	{ "Disable",						0,	0x00,	0x00 },
	{ "SD/SDHC Card",					1,	0xF0,	0xF1 },
	{ "USB 2.0 Mass Storage Device",	2,	0xF2,	0xF3 },
};

Nand * Nand::instance = NULL;

Nand * Nand::Instance()
{
	if(instance == NULL)
		instance = new Nand();
	return instance;
}

void Nand::DestroyInstance()
{
	if(instance) delete instance;
	instance = NULL;
}

void Nand::Init(string path, u32 partition, bool disable)
{
	EmuDevice = disable ? REAL_NAND : partition == 0 ? EMU_SD : EMU_USB;
	Partition = disable ? REAL_NAND : partition > 0 ? partition - 1 : partition;
	Set_NandPath(path);
	Disabled = disable;
}

s32 Nand::Nand_Mount(NandDevice *Device)
{
	s32 fd = IOS_Open("fat", 0);
	if (fd < 0) return fd;

	static ioctlv vector[1] ATTRIBUTE_ALIGN(32);	
	
	vector[0].data = &Partition;
	vector[0].len = sizeof(u32);

	s32 ret = IOS_Ioctlv(fd, Device->Mount, 1, 0, vector);
	IOS_Close(fd);

	return ret;
}

s32 Nand::Nand_Unmount(NandDevice *Device)
{
	s32 fd = IOS_Open("fat", 0);
	if (fd < 0) return fd;

	s32 ret = IOS_Ioctlv(fd, Device->Unmount, 0, 0, NULL);
	IOS_Close(fd);

	return ret;
}

s32 Nand::Nand_Enable(NandDevice *Device)
{
	s32 fd = IOS_Open("/dev/fs", 0);
	if (fd < 0) return fd;

	int NandPathlen = strlen(NandPath) + 1;

	static ioctlv vector[2] ATTRIBUTE_ALIGN(32);

	static u32 mode ATTRIBUTE_ALIGN(32) = Device->Mode | FullMode;

	vector[0].data = &mode;
	vector[0].len = sizeof(u32);
	vector[1].data = NandPath;
	vector[1].len = NandPathlen;

	s32 ret = IOS_Ioctlv(fd, 100, 2, 0, vector);
	IOS_Close(fd);

	return ret;
} 

s32 Nand::Nand_Disable(void)
{
	s32 fd = IOS_Open("/dev/fs", 0);
	if (fd < 0) return fd;

	u32 inbuf ATTRIBUTE_ALIGN(32) = 0;
	s32 ret = IOS_Ioctl(fd, 100, &inbuf, sizeof(inbuf), NULL, 0);
	IOS_Close(fd);

	return ret;
} 

s32 Nand::Enable_Emu()
{
	if(MountedDevice == EmuDevice || Disabled)
		return 0;

	Disable_Emu();

	NandDevice *Device = &NandDeviceList[EmuDevice];

	s32 ret = Nand_Mount(Device);
	if (ret < 0) return ret;

	ret = Nand_Enable(Device);
	if (ret < 0) return ret;

	MountedDevice = EmuDevice;

	return 0;
}	

s32 Nand::Disable_Emu()
{
	if(MountedDevice == 0)
		return 0;

	NandDevice * Device = &NandDeviceList[MountedDevice];

	Nand_Disable();
	Nand_Unmount(Device);

	MountedDevice = 0;

	return 0;
}

void Nand::Set_NandPath(string path)
{
	if(isalnum(*(path.begin()))) path.insert(path.begin(), '/');
	else *(path.begin()) = '/';

	if(isalnum(*(path.end()))) path.push_back('/');
	else *(path.end()) = '/';

	if(path.size() <= 32)
		memcpy(NandPath, path.c_str(), path.size());
}