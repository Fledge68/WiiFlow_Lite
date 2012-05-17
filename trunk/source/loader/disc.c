#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogcsys.h>
#include <unistd.h>
#include <ogc/lwp_threads.h>
#include <ogc/lwp_watchdog.h>
#include "wiiuse/wpad.h"
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
#include "usbstorage.h"
#include "wip.h"
#include "memory.h"
#include "gecko.h"

#define ALIGNED(x) __attribute__((aligned(x)))

/* Constants */
#define PTABLE_OFFSET	0x40000
#define WII_MAGIC	0x5D1C9EA3
#define GC_MAGIC	0xC2339F3D

//appentrypoint 
u32 appentrypoint;
	
/* Disc pointers */
static u32 *buffer = (u32 *)0x93000000;
static u8  *diskid = (u8  *)0x80000000;

GXRModeObj *disc_vmode = NULL;
GXRModeObj *vmode = NULL;
u32 vmode_reg = 0;
u8 vidmode_selected = 0;

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
	*(u32*)0x800000E4	= 0x80431A80;
	*Dev_Debugger		= 0x81800000; // Dev Debugger Monitor Address
	*Simulated_Mem		= 0x01800000; // Simulated Memory Size
	*(vu32*)0xCD00643C	= 0x00000000; // 32Mhz on Bus

	/* Copy disc ID */
	memcpy((void *) Online_Check, (void *) Disc_ID, 4);
}

GXRModeObj * __Disc_SelectVMode(u8 videoselected, u64 chantitle)
{
    vmode = VIDEO_GetPreferredMode(0);

	/* Get video mode configuration */
	bool progressive = (CONF_GetProgressiveScan() > 0) && VIDEO_HaveComponentCable();

	/* Select video mode register */
	switch (CONF_GetVideo())
	{
		case CONF_VIDEO_PAL:
			if (CONF_GetEuRGB60() > 0)
			{
				vmode_reg = VI_EURGB60;
				vmode = progressive ? &TVNtsc480Prog : &TVEurgb60Hz480IntDf;
			}
			else
				vmode_reg = VI_PAL;
			break;

		case CONF_VIDEO_MPAL:
			vmode_reg = VI_MPAL;
			break;

		case CONF_VIDEO_NTSC:
			vmode_reg = VI_NTSC;
			break;
	}

	char Region;
	if(chantitle != 0)
		Region = ((u32)(chantitle) & 0xFFFFFFFF) % 256;
	else Region = diskid[3];

    switch (videoselected)
	{
		case 0: // DEFAULT (DISC/GAME)
			/* Select video mode */
			switch (Region)
			{
				case 'W':
					break; // Don't overwrite wiiware video modes.
				// PAL
				case 'D':
				case 'F':
				case 'P':
				case 'X':
				case 'Y':
					if (CONF_GetVideo() != CONF_VIDEO_PAL)
					{
						vmode_reg = VI_PAL;
						vmode = progressive ? &TVNtsc480Prog : &TVNtsc480IntDf;
					}
					break;
				// NTSC
				case 'E':
				case 'J':
				default:
					if (CONF_GetVideo() != CONF_VIDEO_NTSC)
					{
						vmode_reg = VI_NTSC;
						vmode = progressive ? &TVEurgb60Hz480Prog : &TVEurgb60Hz480IntDf;
					}
					break;
			}
			break;
		case 1: // PAL50
			vmode =  &TVPal528IntDf;
			vmode_reg = vmode->viTVMode >> 2;
			break;
		case 2: // PAL60
			vmode = progressive ? &TVEurgb60Hz480Prog : &TVEurgb60Hz480IntDf;
			vmode_reg = progressive ? vmode->viTVMode >> 2 : vmode->viTVMode >> 2;
			break;
		case 3: // NTSC
			vmode = progressive ? &TVNtsc480Prog : &TVNtsc480IntDf;
			vmode_reg = vmode->viTVMode >> 2;
			break;
		case 4: // AUTO PATCH TO SYSTEM
		case 5: // SYSTEM
			break;
		case 6: // PROGRESSIVE 480P(NTSC + PATCH ALL)
			vmode = &TVNtsc480Prog;
			vmode_reg = vmode->viTVMode >> 2;
			break;
		default:
			break;
	}
	disc_vmode = vmode;

	return vmode;
}

void __Disc_SetVMode(void)
{
	/* Set video mode register */
	*(vu32 *)0x800000CC = vmode_reg;
	DCFlushRange((void *)(0x800000CC), 4);

	/* Set video mode */
	if (disc_vmode != 0)
		VIDEO_Configure(disc_vmode);
}

void __Disc_SetTime(void)
{
	/* Set proper time */
	settime(secs_to_ticks(time(NULL) - 946684800));
}

s32 Disc_FindPartition(u64 *outbuf)
{
	u64 offset = 0;
	u32 cnt;

	u32 *TMP_Buffer = (u32*)MEM1_alloc(0x20);
	if(!TMP_Buffer)
		return -1;

	/* Read partition info */
	s32 ret = WDVD_UnencryptedRead(TMP_Buffer, 0x20, PTABLE_OFFSET);
	if(ret < 0)
	{
		MEM1_free(TMP_Buffer);
		return ret;
	}

	/* Get data */
	u32 nb_partitions = TMP_Buffer[0];
	u64 table_offset  = TMP_Buffer[1] << 2;
	
	if(nb_partitions > 8)
	{
		MEM1_free(TMP_Buffer);
		return -1;
	}

	memset(TMP_Buffer, 0, sizeof(TMP_Buffer));

	/* Read partition table */
	ret = WDVD_UnencryptedRead(TMP_Buffer, 0x20, table_offset);
	if (ret < 0)
	{
		MEM1_free(TMP_Buffer);
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
	MEM1_free(TMP_Buffer);

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

s32 Disc_Open(void)
{
	/* Reset drive */
	s32 ret = WDVD_Reset();
	if (ret < 0) return ret;

	memset(diskid, 0, 32);

	/* Read disc ID */
	ret = WDVD_ReadDiskId(diskid);

	/* Directly set Audio Streaming for GC*/
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

s32 Disc_SetUSB(const u8 *id)
{
	if (id) return set_frag_list((u8 *) id);

	return WDVD_SetUSBMode(wbfsDev, (u8 *) id, -1);
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
		{
			magic = 0xc2339f3d;
		}
		else
		{
			magic = header->magic;
		}
	}

	if (ret < 0) return ret;
		
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
	__Disc_SetVMode();

	/* Shutdown IOS subsystems */
	u32 level = IRQ_Disable();
	__IOS_ShutdownSubsystems();
	__exception_closeall();

	/* Originally from tueidj - taken from NeoGamma (thx) */
	*(vu32*)0xCC003024 = 1;

	if (hooktype != 0)
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

void RunApploader(u64 offset, u8 vidMode, bool vipatch, bool countryString, u8 patchVidMode, int aspectRatio)
{
	WDVD_OpenPartition(offset);

	/* Setup low memory */;
	__Disc_SetLowMem();

	/* Select an appropriate video mode */
	__Disc_SelectVMode(vidMode, 0);

	/* Run apploader */
	Apploader_Run(&p_entry, vidMode, vmode, vipatch, countryString, patchVidMode, aspectRatio);

	appentrypoint = (u32)p_entry;
}
