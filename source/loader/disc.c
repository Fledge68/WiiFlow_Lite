#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogcsys.h>
#include <unistd.h>
#include <ogc/lwp_threads.h>
#include <ogc/lwp_watchdog.h>
#include <ogc/machine/processor.h>

#include "disc.h"
#include "wdvd.h"
#include "sys.h"

#include "fst.h"
#include "wbfs.h"
#include "frag.h"
#include "wip.h"

#include "utils.h"
#include "cios.h"

#include "devicemounter/usbstorage.h"
#include "gecko/gecko.h"
#include "memory/memory.h"

/* Constants */
#define PTABLE_OFFSET	0x40000

//appentrypoint 
u32 appentrypoint;
	
/* Disc pointers */
static u32 *buffer = (u32 *)0x93000000;
static u8  *diskid = (u8  *)0x80000000;

s32 Disc_Open(bool boot_disc)
{
	/* Reset drive */
	s32 ret = WDVD_Reset();
	if (ret < 0) return ret;

	memset(diskid, 0, 32);

	/* Read disc ID */
	ret = WDVD_ReadDiskId(diskid);

	/* Directly set Audio Streaming for GC*/
	if(boot_disc)
		gprintf("Setting Audio Streaming for GC Games %s\n", WDVD_SetStreaming() == 0 ? "succeed" : "failed");

	return ret;
}

s32 Disc_Wait(void)
{
	u32 cover = 0;
	int icounter = 0;

	/* Wait for disc */
	while (!(cover & 0x2))
	{
		/* Get cover status */
		s32 ret = WDVD_GetCoverStatus(&cover);
		if (ret < 0) return ret;
			
		// 10 tries to make sure it doesn´t "freeze" in Install dialog
		// if no Game Disc is insert
		icounter++;
		sleep(1);
		if(icounter > 10)
			return -1;
	}

	return 0;
}

s32 Disc_ReadHeader(void *outbuf)
{
	/* Read Wii disc header */
	return WDVD_UnencryptedRead(outbuf, sizeof(struct discHdr), 0);
}

s32 Disc_ReadGCHeader(void *outbuf)
{
	/* Read GC disc header */
	return WDVD_UnencryptedRead(outbuf, sizeof(struct gc_discHdr), 0);
}

s32 Disc_Type(bool gc)
{
	s32 ret;
	u32 check;
	u32 magic;
	
	if (!gc)
	{
		check = WII_MAGIC;
		struct discHdr *header = (struct discHdr *)buffer;
		ret = Disc_ReadHeader(header);
		magic = header->magic;
	}
	else
	{
		check = GC_MAGIC;
		struct gc_discHdr *header = (struct gc_discHdr *)buffer;
		ret = Disc_ReadGCHeader(header);
		if(strcmp((char *)header->id, "GCOPDV") == 0)
			magic = GC_MAGIC;
		else
			magic = header->magic;
	}

	if (ret < 0)
		return ret;
		
	/* Check magic word */
	if (magic != check) return -1;

	return 0;
}

s32 Disc_IsWii(void)
{
	return Disc_Type(0);
}

s32 Disc_IsGC(void)
{
	return Disc_Type(1);
}
