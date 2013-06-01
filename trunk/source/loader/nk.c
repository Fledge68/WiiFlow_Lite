/***************************************************************************
 * Copyright (C) 2012  OverjoY for Wiiflow
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
 * nk.c
 *
 ***************************************************************************/
#include <stdio.h>
#include <ogcsys.h>
#include <string.h>
#include <malloc.h>

#include "nk.h"
#include "armboot.h"
#include "memory/mem2.hpp"
#include "gecko/gecko.hpp"

bool checked = false;
bool neek = false;
u32 kernelSize = 0;
void *Kernel = NULL;

void check_neek2o(void)
{
	if(checked == true)
		return;
	checked = true;

	s32 ESHandle = IOS_Open("/dev/es", 0);
	neek = (IOS_Ioctlv(ESHandle, 0xA2, 0, 0, NULL) == 0x666c6f77);
	IOS_Close(ESHandle);
	if(!neek)
	{
		s32 FSHandle = IOS_Open("/dev/fs", 0);
		neek = (IOS_Ioctlv(FSHandle, 0x21, 0, 0, NULL) == 0);
		IOS_Close(FSHandle);
	}
	if(!neek)
	{
		u32 num = 0;
		ISFS_Initialize();
		neek = (ISFS_ReadDir("/sneek", NULL, &num) == 0);
		ISFS_Deinitialize();
	}
	gprintf("WiiFlow is in %s mode\n", neek ? "neek2o" : "real nand");
}

bool neek2o(void)
{
	return neek;
}

bool Load_Neek2o_Kernel()
{
	if(neek2o())
		return true;

	FILE *file = NULL;
	file = fopen("usb1:/sneek/kernel.bin", "rb");
	if(!file)
		file = fopen("sd:/sneek/kernel.bin", "rb");
	if(file) 
	{
		fseek(file , 0 , SEEK_END);
		kernelSize = ftell(file);
		rewind(file);
		Kernel = malloc(kernelSize);
		if(!Kernel)
		{
			fclose(file);
			return false;
		}
		fread(Kernel, 1, kernelSize, file);
		fclose(file);
		return true;
	}
	return false;
}

s32 Launch_nk(u64 TitleID, const char *nandpath, u64 ReturnTo)
{
	if(neek2o())
	{
		SYS_ResetSystem(SYS_RESTART, 0, 0);
		return 1;
	}
	memcpy((void*)0x91000000, Kernel, kernelSize);
	DCFlushRange((void*)0x91000000, kernelSize);
	free(Kernel);

	memcfg *MC = (memcfg*)malloc(sizeof(memcfg));
	if(MC == NULL)
		return 0;
	memset(MC, 0, sizeof(memcfg));
	MC->magic = 0x666c6f77;
	if(TitleID)
		MC->titleid = TitleID;
	if(ReturnTo)
	{
		MC->returnto = ReturnTo;
		MC->config |= NCON_EXT_RETURN_TO;
	}

	if(nandpath != NULL)
	{
		strcpy(MC->nandpath, nandpath);
		MC->config |= NCON_EXT_NAND_PATH;
	}
	memcpy((void *)0x81200000, MC, sizeof(memcfg));
	DCFlushRange((void *)(0x81200000), sizeof(memcfg));
	free(MC);

	/*** Thnx giantpune! ***/
	void *mini = MEM1_memalign(32, armboot_size);
	if(!mini)
		return 0;
 
	memcpy(mini, armboot, armboot_size);
	DCFlushRange(mini, armboot_size);
	*(u32*)0xc150f000 = 0x424d454d;
	asm volatile("eieio");
	*(u32*)0xc150f004 = MEM_VIRTUAL_TO_PHYSICAL(mini);
	asm volatile("eieio");
	IOS_ReloadIOS(0xfe);
	MEM1_free(mini);
	return 1;
}
