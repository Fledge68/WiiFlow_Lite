#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogcsys.h>
#include <unistd.h>
#include <ogc/lwp_threads.h>
#include <ogc/lwp_watchdog.h>
#include <ogc/machine/processor.h>

#include "apploader.h"
#include "disc.h"
#include "wdvd.h"
#include "sys.h"

#include "fst.h"
#include "videopatch.h"
#include "wbfs.h"
#include "patchcode.h"
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

GXRModeObj *vmode = NULL;
u32 vmode_reg = 0;

extern void __exception_closeall();

entry_point p_entry;

void __Disc_SetLowMem()
{
	/* Setup low memory */
	*Sys_Magic			= 0x0D15EA5E; // Standard Boot Code
	*Sys_Version		= 0x00000001; // Version
	*Arena_L			= 0x00000000; // Arena Low
	*BI2				= 0x817E5480; // BI2
	*Bus_Speed			= 0x0E7BE2C0; // Console Bus Speed
	*CPU_Speed			= 0x2B73A840; // Console CPU Speed
	*Assembler			= 0x38A00040; // Assembler
	*(vu32*)0x800000E4	= 0x80431A80;
	*Dev_Debugger		= 0x81800000; // Dev Debugger Monitor Address
	*Simulated_Mem		= 0x01800000; // Simulated Memory Size
	*(vu32*)0xCD00643C	= 0x00000000; // 32Mhz on Bus

	/* Fix for Sam & Max (WiiPower) */
	if (!is_ios_type(IOS_TYPE_HERMES, IOS_GetVersion()))
		*GameID_Address	= 0x80000000;

	/* Copy disc ID */
	memcpy((void *)Online_Check, (void *)Disc_ID, 4);
}

GXRModeObj *Disc_SelectVMode(u8 videoselected, u64 chantitle, u32 *rmode_reg)
{
	GXRModeObj *rmode = VIDEO_GetPreferredMode(0);

	/* Get video mode configuration */
	bool progressive = (CONF_GetProgressiveScan() > 0) && VIDEO_HaveComponentCable();

	/* Select video mode register */
	switch (CONF_GetVideo())
	{
		case CONF_VIDEO_PAL:
			if (CONF_GetEuRGB60() > 0)
			{
				*rmode_reg = VI_EURGB60;
				rmode = progressive ? &TVNtsc480Prog : &TVEurgb60Hz480IntDf;
			}
			else
				*rmode_reg = VI_PAL;
			break;

		case CONF_VIDEO_MPAL:
			*rmode_reg = VI_MPAL;
			break;

		case CONF_VIDEO_NTSC:
			*rmode_reg = VI_NTSC;
			break;
	}

	char Region;
	if(chantitle)
		Region = ((u32)(chantitle) & 0xFFFFFFFF) % 256;
	else
		Region = diskid[3];

	switch(videoselected)
	{
		case 0: // DEFAULT (DISC/GAME)
			/* Select video mode */
			switch(Region)
			{
				case 'W':
					break; // Don't overwrite wiiware video modes.
				// PAL
				case 'D':
				case 'F':
				case 'P':
				case 'X':
				case 'Y':
					if(CONF_GetVideo() != CONF_VIDEO_PAL)
					{
						*rmode_reg = VI_PAL;
						rmode = progressive ? &TVNtsc480Prog : &TVNtsc480IntDf;
					}
					break;
				// NTSC
				case 'E':
				case 'J':
				default:
					if(CONF_GetVideo() != CONF_VIDEO_NTSC)
					{
						*rmode_reg = VI_NTSC;
						rmode = progressive ? &TVNtsc480Prog : &TVEurgb60Hz480IntDf;
					}
					break;
			}
			break;
		case 1: // SYSTEM
			break;
		case 2: // PAL50
			rmode =  &TVPal528IntDf;
			*rmode_reg = rmode->viTVMode >> 2;
			break;
		case 3: // PAL60
			rmode = progressive ? &TVNtsc480Prog : &TVEurgb60Hz480IntDf;
			*rmode_reg = progressive ? TVEurgb60Hz480Prog.viTVMode >> 2 : rmode->viTVMode >> 2;
			break;
		case 4: // NTSC
			rmode = progressive ? &TVNtsc480Prog : &TVNtsc480IntDf;
			*rmode_reg = rmode->viTVMode >> 2;
			break;
		case 5: // PROGRESSIVE 480P
			rmode = &TVNtsc480Prog;
			*rmode_reg = Region == 'P' ? TVEurgb60Hz480Prog.viTVMode >> 2 : rmode->viTVMode >> 2;
			break;
		default:
			break;
	}
	return rmode;
}

void Disc_SetVMode(GXRModeObj *rmode, u32 rmode_reg)
{
	/* Set video mode register */
	*Video_Mode = rmode_reg;
	DCFlushRange((void*)Video_Mode, 4);

	/* Set video mode */
	if(rmode != 0)
		VIDEO_Configure(rmode);

	/* Setup video */
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();
	else while(VIDEO_GetNextField())
		VIDEO_WaitVSync();

	/* Set black and flush */
	VIDEO_SetBlack(TRUE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();
	else while(VIDEO_GetNextField())
		VIDEO_WaitVSync();
}

void __Disc_SetTime(void)
{
	/* Set proper time */
	settime(secs_to_ticks(time(NULL) - 946684800));
}

s32 Disc_FindPartition(u64 *outbuf)
{
	u8 TMP_Buffer_size = 0x20;
	u64 offset = 0;
	u32 cnt;

	u32 *TMP_Buffer = (u32*)memalign(32, TMP_Buffer_size);
	if(!TMP_Buffer)
		return -1;

	/* Read partition info */
	s32 ret = WDVD_UnencryptedRead(TMP_Buffer, TMP_Buffer_size, PTABLE_OFFSET);
	if(ret < 0)
	{
		free(TMP_Buffer);
		return ret;
	}

	/* Get data */
	u32 nb_partitions = TMP_Buffer[0];
	u64 table_offset  = TMP_Buffer[1] << 2;
	
	if(nb_partitions > 8)
	{
		free(TMP_Buffer);
		return -1;
	}

	memset(TMP_Buffer, 0, TMP_Buffer_size);

	/* Read partition table */
	ret = WDVD_UnencryptedRead(TMP_Buffer, TMP_Buffer_size, table_offset);
	if (ret < 0)
	{
		free(TMP_Buffer);
		return ret;
	}

	/* Find game partition */
	for(cnt = 0; cnt < nb_partitions; cnt++)
	{
		u32 type = TMP_Buffer[cnt * 2 + 1];

		/* Game partition */
		if(!type)
			offset = TMP_Buffer[cnt * 2] << 2;
	}
	free(TMP_Buffer);

	/* No game partition found */
	if (!offset)
		return -1;

	/* Set output buffer */
	*outbuf = offset;

	WDVD_Seek(offset);

	return 0;
}


s32 Disc_Init(void)
{
	/* Init DVD subsystem */
	return WDVD_Init();
}

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
		gprintf("Setting Audio Streaming for GC Games: 0x%08x\n", WDVD_SetStreaming());

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

s32 Disc_SetUSB(const u8 *id, bool frag)
{
	/* ENABLE USB in cIOS */
	if(id)
	{
		if(frag)
			return set_frag_list();
		s32 part = -1;
		if(is_ios_type(IOS_TYPE_HERMES, IOS_GetVersion()))
			part = wbfs_part_idx ? wbfs_part_idx - 1 : 0;
		return WDVD_SetUSBMode(wbfsDev, (u8*)id, part);
	}
	/* DISABLE USB in cIOS */
	return WDVD_SetUSBMode(0, NULL, -1);
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

s32 Disc_BootPartition()
{
	/* Set time */
	__Disc_SetTime();

	/* Set an appropriate video mode */
	Disc_SetVMode(vmode, vmode_reg);

	/* Shutdown IOS subsystems */
	u32 level = IRQ_Disable();
	__IOS_ShutdownSubsystems();
	__exception_closeall();

	/* Originally from tueidj - taken from NeoGamma (thx) */
	*(vu32*)0xCC003024 = 1;

	if(hooktype != 0)
	{
		asm volatile (
			"lis %r3, appentrypoint@h\n"
			"ori %r3, %r3, appentrypoint@l\n"
			"lwz %r3, 0(%r3)\n"
			"mtlr %r3\n"
			"lis %r3, 0x8000\n"
			"ori %r3, %r3, 0x18A8\n"
			"nop\n"
			"mtctr %r3\n"
			"bctr\n"
		);
	}
	else
	{
		asm volatile (
			"lis %r3, appentrypoint@h\n"
			"ori %r3, %r3, appentrypoint@l\n"
			"lwz %r3, 0(%r3)\n"
			"mtlr %r3\n"
			"blr\n"
		);
	}

	IRQ_Restore(level);

	return 0;
}

void Disc_BootWiiGame(u64 offset, u8 vidMode, bool vipatch, bool countryString, u8 patchVidMode, int aspectRatio, u32 returnTo)
{
	/* Open Partition */
	WDVD_OpenPartition(offset);

	/* Setup low memory */
	__Disc_SetLowMem();

	/* Select an appropriate video mode */
	vmode = Disc_SelectVMode(vidMode, 0, &vmode_reg);

	/* Run apploader */
	Apploader_Run(&p_entry, vidMode, vmode, vipatch, countryString, patchVidMode, aspectRatio, returnTo);
	appentrypoint = (u32)p_entry;

	/* Boot Game */
	gprintf("Entry Point: 0x%08x\n", appentrypoint);
	Disc_BootPartition();
}
