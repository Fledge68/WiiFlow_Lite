#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <dirent.h>
#include <sys/statvfs.h> 

#include "gc_disc.hpp"
#include "DeviceHandler.hpp"
#include "disc.h"
#include "utils.h"
#include "wdvd.h"
#include "text.hpp"

s32 __DiscReadRaw(void *outbuf, u32 offset, u32 length)
{
	return WDVD_UnencryptedRead(outbuf, length, offset);
}

s32 GC_GameDumper(progress_callback_t spinner, void *spinner_data)
{	
	static gc_discHdr gcheader ATTRIBUTE_ALIGN(32);

	FILE *f;
	u8 *ReadBuffer = (u8 *)memalign(32, READSIZE);
	u32 DiscSec = 0;
	u32 ApploaderSize = 0;
	char folder[MAX_FAT_PATH];
	bzero(folder, MAX_FAT_PATH);	
	char gamepath[MAX_FAT_PATH];
	bzero(gamepath, MAX_FAT_PATH);
	
	Disc_ReadGCHeader(&gcheader);
	Asciify2(gcheader.title);

	snprintf(folder, sizeof(folder), "%s:/games/%s [%s]", DeviceName[SD], gcheader.title, (char *)gcheader.id);
	makedir((char *)folder);
	
	snprintf(gamepath, sizeof(gamepath), "%s/game.iso", folder);
	f = fopen(gamepath, "wb");
	while( DiscSec < 0xAE0B )
	{
		__DiscReadRaw(ReadBuffer, DiscSec*READSIZE, READSIZE);
		fwrite(ReadBuffer, 1, READSIZE, f);
		spinner(DiscSec, 0xAE0B, spinner_data);
		DiscSec++;
	}
	fclose(f);	
	
	snprintf(folder, sizeof(folder), "%s:/games/%s [%s]/sys", DeviceName[SD], gcheader.title, (char *)gcheader.id);	
	makedir((char *)folder);
	snprintf(gamepath, sizeof(gamepath), "%s/boot.bin", folder);	
	__DiscReadRaw(ReadBuffer, 0, 0x440);
	
	f = fopen(gamepath, "wb");
	fwrite(ReadBuffer, 1, 0x440, f);
	fclose(f);

	ApploaderSize = *(vu32*)(ReadBuffer+0x400);
	
	snprintf(gamepath, sizeof(gamepath), "%s/bi2.bin", folder);
	__DiscReadRaw(ReadBuffer, 0x440, 0x2000);
	
	f = fopen(gamepath, "wb");
	fwrite(ReadBuffer, 1, 0x2000, f);
	fclose(f);
	
	snprintf(gamepath, sizeof(gamepath), "%s/apploader.img", folder);
	__DiscReadRaw(ReadBuffer, 0x2440, ApploaderSize);
	
	f = fopen(gamepath, "wb");
	fwrite(ReadBuffer, 1, ApploaderSize, f);
	fclose(f);
	
	free(ReadBuffer);	
	
	return 0;	
}