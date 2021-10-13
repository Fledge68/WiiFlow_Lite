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
#include <ogc/machine/processor.h>

#include "nk.h"
#include "sys.h"
#include "fileOps/fileOps.h"
#include "memory/mem2.hpp"
#include "gecko/gecko.hpp"

#define MEM_REG_BASE 0xd8b4000
#define MEM_PROT (MEM_REG_BASE + 0x20a)

u32 kernelSize = 0;
void *Kernel = NULL;

bool Load_Neek2o_Kernel()
{
	bool ret = true;

	if(IsOnWiiU())
		Kernel = fsop_ReadFile("usb1:/sneek/vwiikernel.bin", &kernelSize);
	else
	{
		Kernel = fsop_ReadFile("usb1:/sneek/kernel.bin", &kernelSize);
		if(Kernel == NULL)
			Kernel = fsop_ReadFile("sd:/sneek/kernel.bin", &kernelSize);
	}
	if(Kernel == NULL)
		ret = false;

	return ret;
}

s32 Launch_nk(u64 TitleID, const char *nandpath, u64 ReturnTo)
{
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

	/** boot mini without BootMii IOS code by Crediar. **/
	
	write32(MEM_PROT, read32(MEM_PROT) & 0x0000FFFF);
	unsigned int i = 0x939F02F0;
	unsigned char ES_ImportBoot2[16] =
		{ 0x68, 0x4B, 0x2B, 0x06, 0xD1, 0x0C, 0x68, 0x8B, 0x2B, 0x00, 0xD1, 0x09, 0x68, 0xC8, 0x68, 0x42 };
	
	if( memcmp( (void*)(i), ES_ImportBoot2, sizeof(ES_ImportBoot2) ) != 0 )
		for( i = 0x939F0000; i < 0x939FE000; i+=4 )
			if( memcmp( (void*)(i), ES_ImportBoot2, sizeof(ES_ImportBoot2) ) == 0 )
				break;
	
	if(i >= 0x939FE000)
	{
		gprintf("ES_ImportBoot2 not patched !! Exiting...\n");
		//SYS_ResetSystem( SYS_RETURNTOMENU, 0, 0 );
		return -1;
	}
	
	DCInvalidateRange( (void*)i, 0x20 );
	
	*(vu32*)(i+0x00)        = 0x48034904;   // LDR R0, 0x10, LDR R1, 0x14
	*(vu32*)(i+0x04)        = 0x477846C0;   // BX PC, NOP
	*(vu32*)(i+0x08)        = 0xE6000870;   // SYSCALL
	*(vu32*)(i+0x0C)        = 0xE12FFF1E;   // BLR
	*(vu32*)(i+0x10)        = 0x11000000;   // kernel offset from 0x80000000. Kernel loaded to (void *)0x91000000
	*(vu32*)(i+0x14)        = 0x0000FF01;   // version
	
	DCFlushRange( (void*)i, 0x20 );
	__IOS_ShutdownSubsystems();
	
	s32 fd = IOS_Open( "/dev/es", 0 );
	
	u8 *buffer = (u8*)memalign( 32, 0x100 );
	memset( buffer, 0, 0x100 );
	
	IOS_IoctlvAsync( fd, 0x1F, 0, 0, (ioctlv*)buffer, NULL, NULL );
	return 0;
}
/*
void NKKeyCreate(u8 *tik)
{	
	u32 *TKeyID = (u32*)MEM1_memalign(32, sizeof(u32));	
	u8 *TitleID = (u8*)MEM1_memalign(32, 0x10);
	u8 *EncTitleKey = (u8*)MEM1_memalign(32, 0x10);
	
	memset(TitleID, 0, 0x10);
	memset(EncTitleKey, 0, 0x10);

	memcpy(TitleID, tik + 0x1DC, 8);
	memcpy(EncTitleKey, tik + 0x1BF, 16);

	static ioctlv v[3] ATTRIBUTE_ALIGN(32);
	
	v[0].data = TitleID;
	v[0].len = 0x10;
	v[1].data = EncTitleKey;
	v[1].len = 0x10;
	v[2].data = TKeyID;
	v[2].len = sizeof(u32);

	s32 ESHandle = IOS_Open("/dev/es", 0);
	IOS_Ioctlv(ESHandle, 0x50, 2, 1, (ioctlv *)v);	
	IOS_Close(ESHandle);

	KeyID = *(u32*)(TKeyID);
	
	MEM1_free(TKeyID);
    MEM1_free(EncTitleKey);
    MEM1_free(TitleID);
}

void NKAESDecryptBlock(u8 *in, u8 *out)
{
	static ioctlv v[5] ATTRIBUTE_ALIGN(32);
	
	v[0].data = &KeyID;
	v[0].len = sizeof(u32);
	v[1].data = in + 0x3d0;
	v[1].len = 0x10;
	v[2].data = in + 0x400;
	v[2].len = 0x7c00;
	v[3].data = 0;
	v[3].len = 0;
	v[4].data = out;
	v[4].len = 0x7c00;

	s32 ESHandle = IOS_Open("/dev/es", 0);
	IOS_Ioctlv(ESHandle, 0x2D, 3, 2, (ioctlv *)v);	
	IOS_Close(ESHandle);
}
*/