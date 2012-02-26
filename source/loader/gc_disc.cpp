/***************************************************************************
 * Copyright (C) 2012
 * by OverjoY and FIX94 for Wiiflow
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
 * gc_disc.cpp
 *
 ***************************************************************************/
 
#include <stdio.h>
#include <ogcsys.h>
#include <sys/statvfs.h>

#include "mem2.hpp"
#include "gc_disc.hpp"
#include "DeviceHandler.hpp"
#include "disc.h"
#include "utils.h"
#include "wdvd.h"
#include "text.hpp"
#include "gecko.h"

static u8 *FSTable ALIGNED(32);

s32 GCDump::__DiscReadRaw(void *outbuf, u32 offset, u32 length)
{	
	while(1)
	{
		*(u32*)0xCD0000C0 |= 0x20;
		gc_error = 0;
		s32 ret = WDVD_UnencryptedRead(outbuf, length, offset);
		if( ret != 0 )
		{
			WDVD_LowRequestError(&gc_error);			
			if(gc_error == 0x30200 || gc_error == 0x30201 || gc_error == 0x31100)
			{
				if(gc_retry >= gc_nbrretry)
				{
					if(!skiponerror)
					{
						*(u32*)0xCD0000C0 &= ~0x20;
						return gc_error;
					}
					else
					{
						gc_retry = 0;
						gc_skipped++;
						gprintf("Read error (%x) at offset: 0x%08x. Skipping %d bytes\n", gc_error, offset, length);
						*(u32*)0xCD0000C0 &= ~0x20;
						return 1;
					}
				}
				gc_retry++;
			}
			else
			{
				gprintf("Read error(%x) at offset: 0x%08x.\n", gc_error, offset);
				*(u32*)0xCD0000C0 &= ~0x20;
				return 0;
			}
		}
		else
		{
			*(u32*)0xCD0000C0 &= ~0x20;
			gc_retry = 0;
			return ret;
		}
	}
	return -1;		
}

s32 GCDump::__DiscWrite(char * path, u32 offset, u32 length, progress_callback_t spinner, void *spinner_data)
{
	gprintf("__DiscWrite(%s, 0x%08x, %x)\n", path, offset, length);
	u8 *ReadBuffer = (u8 *)MEM2_alloc(gc_readsize);
	u32 toread = 0;
	u32 wrote = 0;
	FILE *f = fopen(path, "ab");
	
	while(length)
	{
		toread = gc_readsize;
		if(toread > length)
			toread = length;
			
		s32 ret = __DiscReadRaw(ReadBuffer, offset, (toread+31)&(~31));
		if( ret == 1 )
			memset(ReadBuffer, 0, gc_readsize);
		
		else if( ret > 1 )
			return 0;

		fwrite(ReadBuffer, 1, toread, f);		
		
		wrote += toread;
		offset += toread;
		length -= toread;
		if(spinner)
			spinner(wrote, DiscSize, spinner_data);
	}
	
	SAFE_CLOSE(f);
	MEM2_free(ReadBuffer);
	return wrote;
}

s32 GCDump::__DiscWriteAligned(FILE *f, u32 offset, u32 length)
{
	u8 *ReadBuffer = (u8 *)MEM2_alloc(gc_readsize+4);
	u32 toread = 0;
	u32 wrote = 0;

	while(length)
	{
		toread = gc_readsize;
		if (toread > length)
			toread = length;

		s32 ret = __DiscReadRaw(ReadBuffer, offset, (toread+31)&(~31));
		if (ret == 1)
			memset(ReadBuffer, 0, gc_readsize);
		else if (ret > 1)
			return 0;

		fwrite(ReadBuffer, 1, toread, f);
		wrote += toread;
		offset += toread;
		length -= toread;
	}
	MEM2_free(ReadBuffer);
	return wrote;
}

s32 GCDump::DumpGame(progress_callback_t spinner, void *spinner_data)
{	
	static gc_discHdr gcheader ATTRIBUTE_ALIGN(32);

	u8 *ReadBuffer = (u8 *)MEM2_alloc(0x40);
	u8 *FSTBuffer;
	u32 wrote = 0;

	u32 ApploaderSize = 0;
	u32 DOLOffset = 0;
	u32 DOLSize = 0;
	u32 FSTOffset = 0;
	u32 FSTSize = 0;
	u32 FSTEnt = 0;
	u32 GamePartOffset = 0;
	u32 DataSize = 0;	
	char *FSTNameOff = (char *)NULL;

	char folder[MAX_FAT_PATH];
	bzero(folder, MAX_FAT_PATH);	
	char gamepath[MAX_FAT_PATH];
	bzero(gamepath, MAX_FAT_PATH);

	s32 ret = Disc_ReadGCHeader(&gcheader);
	Asciify2(gcheader.title);

	snprintf(folder, sizeof(folder), "%s:/games/%s [%s]", DeviceName[SD], gcheader.title, (char *)gcheader.id);
	makedir((char *)folder);

	if(writeexfiles)
	{
		snprintf(folder, sizeof(folder), "%s:/games/%s [%s]/sys", DeviceName[SD], gcheader.title, (char *)gcheader.id);	
		makedir((char *)folder);
	}

	ret = __DiscReadRaw(ReadBuffer, 0x400, 0x40);
	if(ret > 0)
		return 0x31100;

	ApploaderSize = *(vu32*)(ReadBuffer);
	DOLOffset = *(vu32*)(ReadBuffer+0x20);
	FSTOffset = *(vu32*)(ReadBuffer+0x24);
	FSTSize = *(vu32*)(ReadBuffer+0x28);
	GamePartOffset = *(vu32*)(ReadBuffer+0x34);
	DataSize = *(vu32*)(ReadBuffer+0x38);

	MEM2_free(ReadBuffer);

	DOLSize = FSTOffset - DOLOffset;
	DiscSize =  DataSize + GamePartOffset;

	FSTBuffer = (u8 *)MEM2_alloc((FSTSize+31)&(~31));

	ret = __DiscReadRaw(FSTBuffer, FSTOffset, (FSTSize+31)&(~31));

	if(ret > 0)
		return 0x31100;

	FSTable = (u8*)FSTBuffer;

	FSTEnt = *(u32*)(FSTable+0x08);

	FSTNameOff = (char*)(FSTable + FSTEnt * 0x0C);
	FST *fst = (FST *)(FSTable);

	gprintf("Dumping: %s %s\n", gcheader.title, compressed ? "compressed" : "full");

	gprintf("Apploader size : %d\n", ApploaderSize);
	gprintf("DOL offset     : 0x%08x\n", DOLOffset);
	gprintf("DOL size       : %d\n", DOLSize);
	gprintf("FST offset     : 0x%08x\n", FSTOffset);
	gprintf("FST size       : %d\n", FSTSize);
	gprintf("Num FST entries: %d\n", FSTEnt);
	gprintf("Data Offset    : 0x%08x\n", FSTOffset+FSTSize);
	gprintf("Disc size      : %d\n", DiscSize);

	if(writeexfiles)
	{
		gprintf("Writing %s/boot.bin\n", folder);
		snprintf(gamepath, sizeof(gamepath), "%s/boot.bin", folder);
		__DiscWrite(gamepath, 0, 0x440, spinner, spinner_data);

		gprintf("Writing %s/bi2.bin\n", folder);
		snprintf(gamepath, sizeof(gamepath), "%s/bi2.bin", folder);	
		__DiscWrite(gamepath, 0x440, 0x2000, spinner, spinner_data);
	
		gprintf("Writing %s/apploader.img\n", folder);
		snprintf(gamepath, sizeof(gamepath), "%s/apploader.img", folder);	
		__DiscWrite(gamepath, 0x2440, ApploaderSize, spinner, spinner_data);
	}

	snprintf(gamepath, sizeof(gamepath), "%s:/games/%s [%s]/game.iso", DeviceName[SD], gcheader.title, (char *)gcheader.id);

	gprintf("Writing %s\n", gamepath);
	if(compressed)
	{
		u32 align;
		u32 correction;
		
		FILE *f;
		f = fopen(gamepath, "ab");

		ret = __DiscWriteAligned(f, 0, (FSTOffset + FSTSize));
		wrote += (FSTOffset + FSTSize);
	
		u32 i;

		for( i=1; i < FSTEnt; ++i )
		{
			if( fst[i].Type )
			{
				continue;
			} 
			else 
			{
				correction = 0x00;
				for(align = 0x8000; align > 2; align/=2)
				{
					if((fst[i].FileOffset & (align-1)) == 0 || force_32k_align)
					{
						break;
					}
				}
				
				while(((wrote+correction) & (align-1)) != 0)
					correction++;
				
				if(correction>0x00)
				{
					u32 j;
					for(j=0x00;j<correction;j++)
						fwrite("", 1, 1, f);
					wrote += correction;
				}
				
				ret = __DiscWriteAligned(f, fst[i].FileOffset, fst[i].FileLength);
				gprintf("Writing: %d/%d: %s from 0x%08x to 0x%08x(%i)\n", i, FSTEnt, FSTNameOff + fst[i].NameOffset, fst[i].FileOffset, wrote, align);
				if( ret >= 0 )
				{
					fst[i].FileOffset = wrote;
					wrote += ret;
					spinner(i, FSTEnt, spinner_data);
				}
				else
				{
					spinner(FSTEnt, FSTEnt, spinner_data);
					MEM2_free(FSTBuffer);
					SAFE_CLOSE(f);
					return gc_error;
				}
			}
		}

		SAFE_CLOSE(f);

		gprintf("Updating FST\n");
		f = fopen(gamepath, "r+");
		fseek(f, FSTOffset, SEEK_SET);
		fwrite(fst, 1, FSTSize, f);
		SAFE_CLOSE(f);

		gprintf("Done!! Disc old size: %d, disc new size: %d, saved: %d\n", DiscSize, wrote, DiscSize - wrote);
	}
	else
	{
		ret = __DiscWrite(gamepath, 0, DiscSize, spinner, spinner_data);
		if( ret < 0 )
		{
			MEM2_free(FSTBuffer);
			return gc_error;
		}
		gprintf("Done!! Disc size: %d\n", DiscSize);
	}

	MEM2_free(FSTBuffer);

	return gc_skipped;	
}

s32 GCDump::CheckSpace(u32 *needed, bool comp)
{
	u8 *ReadBuffer = (u8 *)MEM2_alloc(0x40);	
	
	s32 ret = __DiscReadRaw(ReadBuffer, 0x400, 0x40);
	if(ret > 0)
		return 1;
	
	u32 ApploaderSize = *(vu32*)(ReadBuffer);
	u32 FSTOffset = *(vu32*)(ReadBuffer+0x24);
	u32 FSTSize = *(vu32*)(ReadBuffer+0x28);
	u32 GamePartOffset = *(vu32*)(ReadBuffer+0x34);
	u32 DataSize = *(vu32*)(ReadBuffer+0x38);
	
	u32 DiscSize =  DataSize + GamePartOffset;
	
	MEM2_free(ReadBuffer);
	
	u32 size = 0;
	
	if(writeexfiles)
	{
		size += 0xa440;
		size += ApploaderSize;
	}
	
	if(!comp)
	{
		size += DiscSize;		
	}
	else
	{
		u8 *FSTBuffer = (u8 *)MEM2_alloc((FSTSize+31)&(~31));

		ret = __DiscReadRaw(FSTBuffer, FSTOffset, (FSTSize+31)&(~31));
		if(ret > 0)
		{
			MEM2_free(FSTBuffer);
			return 1;
		}
		
		FSTable = (u8*)FSTBuffer;
		u32 FSTEnt = *(u32*)(FSTable+0x08);
		FST *fst = (FST *)(FSTable);
		
		u32 i;
	
		for( i=1; i < FSTEnt; ++i )
		{
			if( fst[i].Type )
			{
				continue;
			} 
			else 
			{	
				size += (fst[i].FileLength+31)&(~31);
			}
		}
		MEM2_free(FSTBuffer);
	}
	*needed = size/0x8000;
	gprintf("Free space needed on SD: %d bytes (%x blocks)\n", size, size/0x8000);
	return 0;
}