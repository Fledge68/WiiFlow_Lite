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
#include "gecko/gecko.h"

bool checked = false;
bool neek = false;

bool neek2o(void)
{
	if(!checked)
	{
		u32 num = 0;
		ISFS_Initialize();
		neek = !(ISFS_ReadDir("/sneek", NULL, &num));
		gprintf("WiiFlow is in %s mode\n", neek ? "neek2o" : "real nand");
		checked = true;
	}
	return neek;
}

s32 Launch_nk(u64 TitleID, const char *nandpath)
{
	if(neek2o())
	{
		SYS_ResetSystem(SYS_RESTART, 0, 0);		
		return 1;
	}

	FILE *file = NULL;
	long fsize;

	file = fopen( "usb1:/sneek/kernel.bin", "rb" );

	if(!file)
		file = fopen( "sd:/sneek/kernel.bin", "rb" );

	if(file) 
	{
		fseek(file , 0 , SEEK_END);
		fsize = ftell(file);
		rewind(file);
		fread((void *)0x91000000, 1, fsize, file);
		DCFlushRange((void *)0x91000000, fsize);
	}
	else
		return 0;

	fclose(file);
	
	memcfg *MC = (memcfg*)malloc(sizeof(memcfg));
	if(MC == NULL)
		return 0;
	
	memset(MC, 0, sizeof(memcfg));

	MC->magic = 0x666c6f77;
	MC->titleid = TitleID;
	
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
