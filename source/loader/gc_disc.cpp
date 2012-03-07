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
#include <unistd.h>
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
#include "fileOps.h"
#include "defines.h"

using namespace std;

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

s32 GCDump::__DiscWrite(char * path, u32 offset, u32 length, u8 *ReadBuffer, progress_callback_t spinner, void *spinner_data)
{
	gprintf("__DiscWrite(%s, 0x%08x, %x)\n", path, offset, length);
	u32 wrote = 0;
	FILE *f = fopen(path, "wb");

	wrote = __DiscWriteFile(f, offset, length, ReadBuffer, spinner, spinner_data);
	
	SAFE_CLOSE(f);
	return wrote;
}

s32 GCDump::__DiscWriteFile(FILE *f, u32 offset, u32 length, u8 *ReadBuffer, progress_callback_t spinner, void *spinner_data)
{
	u32 toread = 0;
	u32 wrote = 0;

	while(length)
	{
		toread = min(length, gc_readsize);
		s32 ret = __DiscReadRaw(ReadBuffer, offset, (toread+31)&(~31));
		if (ret == 1)
			memset(ReadBuffer, 0, gc_readsize);
		else if (ret > 1)
			return 0;
		fwrite(ReadBuffer, 1, toread, f);
		wrote += toread;
		offset += toread;
		length -= toread;
		gc_done += toread;
		if(spinner)
			spinner(gc_done/1024, DiscSizeCalculated, spinner_data);
	}
	return wrote;
}

bool GCDump::__WaitForDisc(u8 dsc, char *minfo, u32 msg, message_callback_t message, void *message_data)
{
	u8 *ReadBuffer = (u8 *)MEM2_alloc(0x440);
	
	u32 cover = 0;
	bool done = false;
	*(u32*)0xCD0000C0 |= 0x20;
	while(!done)
	{
		message( msg, dsc+1, minfo, message_data);
		while(1)
		{
			WDVD_GetCoverStatus(&cover);
			if(!(cover & 0x2))
				break;
		}

		while(1)
		{
			if(Disc_Wait() < 0)
				continue;

			if(Disc_Open() < 0)
			{
				MEM2_free(ReadBuffer);
				return false;
			}
					
			if(Disc_IsGC() == 0)
			{
				s32 ret = __DiscReadRaw(ReadBuffer, 0, 0x440);
				if(ret > 0)
				{
					MEM2_free(ReadBuffer);
					return false;
				}
						
				ID2 = *(vu32*)(ReadBuffer);	
				Disc2 = *(vu8*)(ReadBuffer+0x06);
							
				if(ID == ID2 && Disc2 == dsc)
				{
					*(u32*)0xCD0000C0 &= ~0x20;
					done = true;
					break;
				}
				else if(ID == ID2 && Disc2 != dsc)
				{
					message( 7, dsc+1, NULL, message_data);
					usleep( 5000000 );
					break;
				}
				else if(ID != ID2)
				{
					message( 8, 0, NULL, message_data);
					usleep( 5000000 );
					break;
				}
			}
			else if(Disc_IsWii() == 0)
			{
				message( 5, 0, NULL, message_data);
				usleep( 5000000 );
				break;
			}
			else
			{
				message( 6, 0, NULL, message_data);
				usleep( 5000000 );
				break;
				
			}
		}
	}
	MEM2_free(ReadBuffer);
	return done;
}

s32 GCDump::DumpGame(progress_callback_t spinner, message_callback_t message, void *spinner_data)
{	
	static gc_discHdr gcheader ATTRIBUTE_ALIGN(32);

	u8 *ReadBuffer = (u8 *)MEM2_alloc(gc_readsize);

	u32 j;
	gc_done = 0;

	char *FSTNameOff = (char *)NULL;

	char folder[MAX_FAT_PATH];
	bzero(folder, MAX_FAT_PATH);	
	char gamepath[MAX_FAT_PATH];
	bzero(gamepath, MAX_FAT_PATH);	
	char minfo[74];

	for( j=0; j<2; ++j)
	{
		u8 *FSTBuffer;
		u32 wrote = 0;

		s32 ret = Disc_ReadGCHeader(&gcheader);
		Asciify2(gcheader.title);
		
		if(strncmp(gamepartition, "sd", 2) != 0)
			snprintf(folder, sizeof(folder), dmlgamedir, gamepartition); //WTF??
		else
			snprintf(folder, sizeof(folder), DML_DIR, gamepartition);
		if(!fsop_DirExist(folder))
			makedir(folder);
		snprintf(folder, sizeof(folder), "%s:/games/%s [%.06s]%s", gamepartition, gcheader.title, (char *)gcheader.id, j ? "2" : "");
		if(!fsop_DirExist(folder))
			makedir(folder);

		ret = __DiscReadRaw(ReadBuffer, 0, 0x440);
		if(ret > 0)
		{
			MEM2_free(ReadBuffer);
			return 0x31100;
		}

		ID = *(vu32*)(ReadBuffer);
		ApploaderSize = *(vu32*)(ReadBuffer+0x400);
		DOLOffset = *(vu32*)(ReadBuffer+0x420);
		FSTOffset = *(vu32*)(ReadBuffer+0x424);
		FSTSize = *(vu32*)(ReadBuffer+0x428);
		FSTTotal = *(vu32*)(ReadBuffer+0x42c);
		GamePartOffset = *(vu32*)(ReadBuffer+0x434);
		DataSize = *(vu32*)(ReadBuffer+0x438);

		DOLSize = FSTOffset - DOLOffset;
		DiscSize = DataSize + GamePartOffset;

		FSTBuffer = (u8 *)MEM2_alloc((FSTSize+31)&(~31));

		ret = __DiscReadRaw(FSTBuffer, FSTOffset, (FSTSize+31)&(~31));

		if(ret > 0)
		{
			MEM2_free(FSTBuffer);
			MEM2_free(ReadBuffer);
			return 0x31100;
		}

		FSTable = (u8*)FSTBuffer;

		FSTEnt = *(u32*)(FSTable+0x08);

		FSTNameOff = (char*)(FSTable + FSTEnt * 0x0C);
		FST *fst = (FST *)(FSTable);

		snprintf(minfo, sizeof(minfo), "[%.06s] %s", (char *)gcheader.id, gcheader.title);

		if(FSTTotal > FSTSize)
			message(4, j+1, minfo, spinner_data);
		else
			message(3, 0, minfo, spinner_data);

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
			snprintf(folder, sizeof(folder), "%s:/games/%s [%.06s]%s/sys", gamepartition, gcheader.title, (char *)gcheader.id, j ? "2" : "");	
			if(!fsop_DirExist(folder))
				makedir(folder);

			gprintf("Writing %s/boot.bin\n", folder);
			snprintf(gamepath, sizeof(gamepath), "%s/boot.bin", folder);
			gc_done += __DiscWrite(gamepath, 0, 0x440, ReadBuffer, spinner, spinner_data);

			gprintf("Writing %s/bi2.bin\n", folder);
			snprintf(gamepath, sizeof(gamepath), "%s/bi2.bin", folder);
			gc_done += __DiscWrite(gamepath, 0x440, 0x2000, ReadBuffer, spinner, spinner_data);

			gprintf("Writing %s/apploader.img\n", folder);
			snprintf(gamepath, sizeof(gamepath), "%s/apploader.img", folder);
			gc_done += __DiscWrite(gamepath, 0x2440, ApploaderSize, ReadBuffer, spinner, spinner_data);
		}

		snprintf(gamepath, sizeof(gamepath), "%s:/games/%s [%.06s]%s/game.iso", gamepartition, gcheader.title, (char *)gcheader.id, j ? "2" : "");

		gprintf("Writing %s\n", gamepath);
		if(compressed)
		{
			u32 align;
			u32 correction;
			u32 toread;

			FILE *f;
			f = fopen(gamepath, "wb");

			ret = __DiscWriteFile(f, 0, (FSTOffset + FSTSize), ReadBuffer, spinner, spinner_data);
			wrote += (FSTOffset + FSTSize);
			gc_done += wrote;
	
			u32 i;

			for( i=1; i < FSTEnt; ++i )
			{
				if( fst[i].Type )
				{
					continue;
				}
				else
				{
					for(align = 0x8000; align > 2; align/=2)
					{
						if((fst[i].FileOffset & (align-1)) == 0 || force_32k_align)
						{
							correction = 0;
							while(((wrote+correction) & (align-1)) != 0)
								correction++;
							
							wrote += correction;
							while(correction)
							{
								toread=min(correction, gc_readsize);
								memset(ReadBuffer, 0, toread);
								fwrite(ReadBuffer, 1, toread, f);
								correction -= toread;
								gc_done += toread;
							}
							break;
						}
					}
					ret = __DiscWriteFile(f, fst[i].FileOffset, fst[i].FileLength, ReadBuffer, spinner, spinner_data);
					gprintf("Writing: %d/%d: %s from 0x%08x to 0x%08x(%i)\n", i, FSTEnt, FSTNameOff + fst[i].NameOffset, fst[i].FileOffset, wrote, align);
					if( ret >= 0 )
					{
						fst[i].FileOffset = wrote;
						wrote += ret;
					}
					else
					{
						MEM2_free(ReadBuffer);
						MEM2_free(FSTBuffer);
						SAFE_CLOSE(f);
						return gc_error;
					}
				}
			}

			gprintf("Updating FST\n");
			fseek(f, FSTOffset, SEEK_SET);
			fwrite(fst, 1, FSTSize, f);
			SAFE_CLOSE(f);

			gprintf("Done!! Disc old size: %d, disc new size: %d, saved: %d\n", DiscSize, wrote, DiscSize - wrote);
		}
		else
		{
			ret = __DiscWrite(gamepath, 0, DiscSize, ReadBuffer, spinner, spinner_data);
			if( ret < 0 )
			{
				MEM2_free(ReadBuffer);
				MEM2_free(FSTBuffer);
				return gc_error;
			}			
			gprintf("Done!! Disc size: %d\n", DiscSize);
		}
		
		MEM2_free(FSTBuffer);
		
		if(FSTTotal > FSTSize && !j)
			__WaitForDisc(1, minfo, 9, message, spinner_data);
		else
			break;
	}

	MEM2_free(ReadBuffer);

	return gc_skipped;	
}

s32 GCDump::CheckSpace(u32 *needed, bool comp, message_callback_t message, void *message_data)
{
	static gc_discHdr gcheader ATTRIBUTE_ALIGN(32);

	u8 *ReadBuffer = (u8 *)MEM2_alloc(0x440);
	u32 size = 0;
	u32 j;

	char minfo[74];

	for( j=0; j<2; ++j)
	{
		s32 ret = __DiscReadRaw(ReadBuffer, 0, 0x440);
		if(ret > 0)
		{
			MEM2_free(ReadBuffer);
			return 1;
		}

		ID = *(vu32*)(ReadBuffer);
		ID2 = 0;
		Disc = *(vu8*)(ReadBuffer+0x06);
		Disc2 = 0;
		ApploaderSize = *(vu32*)(ReadBuffer+0x400);
		FSTOffset = *(vu32*)(ReadBuffer+0x424);
		FSTSize = *(vu32*)(ReadBuffer+0x428);
		FSTTotal = *(vu32*)(ReadBuffer+0x42c);
		GamePartOffset = *(vu32*)(ReadBuffer+0x434);
		DataSize = *(vu32*)(ReadBuffer+0x438);
		DiscSize =  DataSize + GamePartOffset;

		Disc_ReadGCHeader(&gcheader);
		Asciify2(gcheader.title);

		snprintf(minfo, sizeof(minfo), "[%.06s] %s", (char *)gcheader.id, gcheader.title);

		message( 2, 0, minfo, message_data);	

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

			size += (FSTOffset + FSTSize);

			u32 i;
			u32 correction;
			u32 align;

			for( i=1; i < FSTEnt; ++i )
			{
				if( fst[i].Type )
				{
					continue;
				} 
				else
				{
					for(align = 0x8000; align > 2; align/=2)
					{
						if((fst[i].FileOffset & (align-1)) == 0 || force_32k_align)
						{
							correction = 0x00;
							while(((size+correction) & (align-1)) != 0)
								correction++;
							size += correction;
							break;
						}
					}
					size += fst[i].FileLength;
				}
			}
			MEM2_free(FSTBuffer);
		}

		if(FSTTotal > FSTSize)
		{
			if(Disc == 0 && j == 0)
				__WaitForDisc(1, minfo, 1, message, message_data);
			else if(Disc == 0x01 && j == 0)
				__WaitForDisc(0, minfo, 1, message, message_data);
			
			if(j == 1)
			{
				if(Disc == 0x01)
					__WaitForDisc(0, minfo, 1, message, message_data);				

				break;
			}
		}
		else
			break;		
	}
	MEM2_free(ReadBuffer);
	DiscSizeCalculated = (size/1024);
	*needed = size/0x8000;
	gprintf("Free space needed: %d Mb (%x blocks)\n", (size/1024)/1024, size/0x8000);
	return 0;
}

