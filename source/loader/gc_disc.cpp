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

void GCDump::__AnalizeMultiDisc()
{
	u8 *Buffer = (u8 *)MEM2_alloc(0x10);
	MultiGameCnt = 0;
	while(1)
	{
		__DiscReadRaw(Buffer, 0x40+(MultiGameCnt*4), 4);
		u64 MultiGameOffset = *(vu32*)(Buffer) << 2;
		if(!MultiGameOffset)
			break;		
		
		if(MultiGameDump == MultiGameCnt)
		{
			NextOffset = MultiGameOffset;
		}
		MultiGameCnt++;
	}
	MEM2_free(Buffer);
}

s32 GCDump::__DiscReadRaw(void *outbuf, u64 offset, u32 length)
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
				if(gc_retry >= gc_nbrretry && waitonerror)
				{
					__WaitForDisc(Disc, 10);
					gc_retry = 0;
					waitonerror = false;
					if(FSTTotal > FSTSize)
						message(4, Disc+1, minfo, u_data);
					else
						message(3, 0, minfo, u_data);
				}
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
			else if(gc_error == 0x1023a00)
			{
				__WaitForDisc(Disc, 11);
				if(FSTTotal > FSTSize)
					message(4, Disc+1, minfo, u_data);
				else
					message(3, 0, minfo, u_data);
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

s32 GCDump::__DiscWrite(char * path, u64 offset, u32 length, u8 *ReadBuffer)
{
	gprintf("__DiscWrite(%s, 0x%08x, %x)\n", path, offset, length);
	u32 wrote = 0;
	FILE *f = fopen(path, "wb");

	wrote = __DiscWriteFile(f, offset, length, ReadBuffer);
	
	SAFE_CLOSE(f);
	return wrote;
}

s32 GCDump::__DiscWriteFile(FILE *f, u64 offset, u32 length, u8 *ReadBuffer)
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
			spinner(gc_done/1024, DiscSizeCalculated, u_data);
	}
	return wrote;
}

bool GCDump::__WaitForDisc(u8 dsc, u32 msg)
{
	u8 *ReadBuffer = (u8 *)MEM2_alloc(0x440);
	
	u32 cover = 0;
	bool done = false;	
	while(!done)
	{				
		message( msg, dsc+1, minfo, u_data);		
		while(1)
		{
			*(u32*)0xCD0000C0 |= 0x20;
			usleep( 1000000 );
			*(u32*)0xCD0000C0 &= ~0x20;
			usleep( 1000000 );
			WDVD_GetCoverStatus(&cover);
			if(!(cover & 0x2))
				break;
		}

		while(1)
		{
			*(u32*)0xCD0000C0 |= 0x20;
			usleep( 1000000 );
			*(u32*)0xCD0000C0 &= ~0x20;
			usleep( 1000000 );
			if(Disc_Wait() < 0)
				continue;

			if(Disc_Open() < 0)
			{
				MEM2_free(ReadBuffer);
				return false;
			}
					
			if(Disc_IsGC() == 0)
			{
				s32 ret = __DiscReadRaw(ReadBuffer, NextOffset, 0x440);
				if(ret > 0)
				{
					MEM2_free(ReadBuffer);
					return false;
				}
						
				ID2 = *(vu32*)(ReadBuffer);	
				Disc2 = *(vu8*)(ReadBuffer+0x06);
							
				if(ID == ID2 && Disc2 == dsc)
				{					
					done = true;
					break;
				}
				else if(ID == ID2 && Disc2 != dsc)
				{
					message( 7, Disc2+1, NULL, u_data);
					usleep( 5000000 );
					break;
				}
				else if(ID != ID2)
				{
					message( 8, 0, NULL, u_data);
					usleep( 5000000 );
					break;
				}
			}
			else if(Disc_IsWii() == 0)
			{
				message( 5, 0, NULL, u_data);
				usleep( 5000000 );
				break;
			}
			else
			{
				message( 6, 0, NULL, u_data);
				usleep( 5000000 );
				break;
				
			}
		}
	}
	MEM2_free(ReadBuffer);
	return done;
}

s32 GCDump::DumpGame()
{	
	static gc_discHdr gcheader ATTRIBUTE_ALIGN(32);

	u8 *ReadBuffer = (u8 *)MEM2_alloc(gc_readsize);

	gc_done = 0;
	gamedone = false;
	multigamedisc = false;
	MultiGameDump = 0;
	MultiGameCnt = 0;
	NextOffset = 0;
	Disc = 0;

	char *FSTNameOff = (char *)NULL;

	char folder[MAX_FAT_PATH];
	bzero(folder, MAX_FAT_PATH);
	char gamepath[MAX_FAT_PATH];
	bzero(gamepath, MAX_FAT_PATH);	

	while(!gamedone)
	{
		u8 *FSTBuffer;
		u32 wrote = 0;

		s32 ret = Disc_ReadGCHeader(&gcheader);
		
		if(memcmp((char *)gcheader.id, "GCOPDV", 6) == 0)
		{
			multigamedisc = true;
			__AnalizeMultiDisc();
			__DiscReadRaw(ReadBuffer, NextOffset, sizeof(gc_discHdr));
			memcpy(gcheader.id, ReadBuffer, 6);
			strcpy(gcheader.title, (char *)ReadBuffer+0x20);			
		}
		
		Asciify2(gcheader.title);

		snprintf(folder, sizeof(folder), sfmt((strncmp(gamepartition, "sd", 2) != 0) ? usb_dml_game_dir : DML_DIR, gamepartition).c_str());
		if(!fsop_DirExist(folder))
		{
			gprintf("Creating directory: %s\n", folder);
			makedir(folder);
		}
		memset(folder, 0, sizeof(folder));
		snprintf(folder, sizeof(folder), "%s/%s [%.06s]%s", sfmt((strncmp(gamepartition, "sd", 2) != 0) ? usb_dml_game_dir : DML_DIR, gamepartition).c_str(), gcheader.title, (char *)gcheader.id, Disc ? "2" : "");
		if(!fsop_DirExist(folder))
		{
			gprintf("Creating directory: %s\n", folder);
			makedir(folder);
		}
		else
		{
			gprintf("Skipping game: %s (Already installed)(%d)\n", gcheader.title, Gamesize[MultiGameDump]);
			gc_done += Gamesize[MultiGameDump];
			MultiGameDump++;
			continue;
		}

		ret = __DiscReadRaw(ReadBuffer, NextOffset, 0x440);
		if(ret > 0)
		{
			MEM2_free(ReadBuffer);
			return 0x31100;
		}

		ID = *(vu32*)(ReadBuffer);
		Disc = *(vu8*)(ReadBuffer+0x06);
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

		ret = __DiscReadRaw(FSTBuffer, FSTOffset+NextOffset, (FSTSize+31)&(~31));

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
			message(4, Disc+1, minfo, u_data);
		else
			message(3, 0, minfo, u_data);

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
			memset(folder, 0, sizeof(folder));
			snprintf(folder, sizeof(folder), "%s/%s [%.06s]%s/sys", sfmt((strncmp(gamepartition, "sd", 2) != 0) ? usb_dml_game_dir : DML_DIR, gamepartition).c_str(), gcheader.title, (char *)gcheader.id, Disc ? "2" : "");
			if(!fsop_DirExist(folder))
			{
				gprintf("Creating directory: %s\n", folder);
				makedir(folder);
			}

			gprintf("Writing %s/boot.bin\n", folder);
			snprintf(gamepath, sizeof(gamepath), "%s/boot.bin", folder);
			gc_done += __DiscWrite(gamepath, NextOffset, 0x440, ReadBuffer);

			gprintf("Writing %s/bi2.bin\n", folder);
			snprintf(gamepath, sizeof(gamepath), "%s/bi2.bin", folder);
			gc_done += __DiscWrite(gamepath, 0x440+NextOffset, 0x2000, ReadBuffer);

			gprintf("Writing %s/apploader.img\n", folder);
			snprintf(gamepath, sizeof(gamepath), "%s/apploader.img", folder);
			gc_done += __DiscWrite(gamepath, 0x2440+NextOffset, ApploaderSize, ReadBuffer);
		}

		snprintf(gamepath, sizeof(gamepath), "%s/%s [%.06s]%s/game.iso", sfmt((strncmp(gamepartition, "sd", 2) != 0) ? usb_dml_game_dir : DML_DIR, gamepartition).c_str(), gcheader.title, (char *)gcheader.id, Disc ? "2" : "");

		gprintf("Writing %s\n", gamepath);
		if(compressed)
		{
			u32 align;
			u32 correction;
			u32 toread;

			FILE *f;
			f = fopen(gamepath, "wb");

			ret = __DiscWriteFile(f, NextOffset, (FSTOffset + FSTSize), ReadBuffer);
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
					ret = __DiscWriteFile(f, fst[i].FileOffset+NextOffset, fst[i].FileLength, ReadBuffer);
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
			ret = __DiscWrite(gamepath, NextOffset, DiscSize, ReadBuffer);
			if( ret < 0 )
			{
				MEM2_free(ReadBuffer);
				MEM2_free(FSTBuffer);
				return gc_error;
			}			
			gprintf("Done!! Disc size: %d\n", DiscSize);
		}
		
		MEM2_free(FSTBuffer);
		
		if(FSTTotal > FSTSize && !multigamedisc)
		{
			if(Disc)
			{
				gamedone = true;
				break;
			}
			else
			{
				__WaitForDisc(1, 9);
				Disc++;
			}
		}
		else if(multigamedisc)
		{
			if(MultiGameDump+1 == MultiGameCnt)
			{
				gamedone = true;
				break;
			}
			else
			{
				MultiGameDump++;
			}
		}
		else
			gamedone = true;
	}

	MEM2_free(ReadBuffer);

	return gc_skipped;	
}

s32 GCDump::CheckSpace(u32 *needed, bool comp)
{
	static gc_discHdr gcheader ATTRIBUTE_ALIGN(32);

	u8 *ReadBuffer = (u8 *)MEM2_alloc(0x440);
	u32 size = 0;
	bool scnddisc = false;
	gamedone = false;
	multigamedisc = false;
	MultiGameDump = 0;
	MultiGameCnt = 0;
	NextOffset = 0;
	Disc = 0;

	while(!gamedone)
	{
		u32 multisize = 0;
		Disc_ReadGCHeader(&gcheader);
		
		if(memcmp((char *)gcheader.id, "GCOPDV", 6) == 0)
		{
			multigamedisc = true;
			__AnalizeMultiDisc();
			__DiscReadRaw(ReadBuffer, NextOffset, sizeof(gc_discHdr));
			memcpy(gcheader.id, ReadBuffer, sizeof(gcheader.id));
			strcpy(gcheader.title, (char *)ReadBuffer+0x20);
		}
		
		Asciify2(gcheader.title);
		
		s32 ret = __DiscReadRaw(ReadBuffer, NextOffset, 0x440);
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

		snprintf(minfo, sizeof(minfo), "[%.06s] %s", (char *)gcheader.id, gcheader.title);

		message( 2, 0, minfo, u_data);	

		if(writeexfiles)
		{
			multisize += 0xa440;
			multisize += ApploaderSize;
		}

		if(!comp)
		{
			multisize += DiscSize;		
		}
		else
		{
			u8 *FSTBuffer = (u8 *)MEM2_alloc((FSTSize+31)&(~31));

			ret = __DiscReadRaw(FSTBuffer, FSTOffset+NextOffset, (FSTSize+31)&(~31));
			if(ret > 0)
			{
				MEM2_free(FSTBuffer);
				return 1;
			}

			FSTable = (u8*)FSTBuffer;
			u32 FSTEnt = *(u32*)(FSTable+0x08);
			FST *fst = (FST *)(FSTable);

			multisize += (FSTOffset + FSTSize);

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
							correction = 0;
							while(((multisize+correction) & (align-1)) != 0)
								correction++;
							multisize += correction;
							break;
						}
					}
					multisize += fst[i].FileLength;
				}
			}
			MEM2_free(FSTBuffer);
		}
		size += multisize;
		Gamesize[MultiGameDump] = multisize;

		if(FSTTotal > FSTSize && !multigamedisc)
		{
			if(Disc == 0 && !scnddisc)
				__WaitForDisc(1, 1);
			else if(Disc == 0x01 && !scnddisc)
				__WaitForDisc(0, 1);

			if(scnddisc)
			{
				if(Disc == 0x01)
					__WaitForDisc(0, 1);

				gamedone = true;
				break;
			}
			scnddisc = true;
		}
		else if(multigamedisc)
		{
			if(MultiGameDump+1 == MultiGameCnt)
			{
				gamedone = true;
				break;
			}
			else
			{
				MultiGameDump++;
			}
		}
		else
			gamedone = true;
	}
	MEM2_free(ReadBuffer);
	DiscSizeCalculated = size/0x400;
	*needed = (size/0x8000) >> 2;
	gprintf("Free space needed: %d Mb (%d blocks)\n", size/0x100000, (size/0x8000) >> 2);
	return 0;
}

u32 GCDump::GetFreeSpace(char *path, u32 Value)
{
	struct statvfs stats;
	memset(&stats, 0, sizeof(stats));
	statvfs(path , &stats);
	
	u64 free = (u64)stats.f_frsize * (u64)stats.f_bfree;
	
	switch(Value)
	{
		case KB:
			return free/0x400;
		case BL:
			return (free/0x8000) >> 2;	
		case MB:
			return free/0x100000;
		case GB:
			return free/0x40000000;		
	}	
	return 0;
}

