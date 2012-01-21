#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogcsys.h>
#include <unistd.h>
#include <ogc/lwp_watchdog.h>
#include <wiiuse/wpad.h>
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

extern void __exception_closeall();

static u8	Tmd_Buffer[0x49e4 + 0x1C] ALIGNED(32);

void __Disc_SetLowMem()
{
	*(vu32 *)0x80000060 = 0x38A00040; // Dev Debugger Hook
	*(vu32 *)0x800000E4 = 0x80431A80;
	*(vu32 *)0x800000EC = 0x81800000; // Dev Debugger Monitor Address
	*(vu32 *)0x800000F0 = 0x01800000; // Simulated Memory Size
	*(vu32 *)0xCD00643C = 0x00000000; // 32Mhz on Bus

	*Sys_Magic	= 0x0d15ea5e;
	*Version	= 1;
	*Arena_L	= 0x00000000;
	*BI2		= 0x817E5480;
	*Bus_Speed	= 0x0E7BE2C0;
	*CPU_Speed	= 0x2B73A840;

	*(vu32 *)0x800030F0 = 0x0000001C; // Dol Args
	*(vu32 *)0x8000318C = 0x00000000; // Launch Code
	*(vu32 *)0x80003190 = 0x00000000; // Return Code

	*(vu32 *)0x80003140 = *(vu32 *)0x80003188; // IOS Version Check
	*(vu32 *)0x80003180 = *(vu32 *)0x80000000; // Game ID Online Check
	*(vu32 *)0x80003184 = 0x80000000;

	/* Flush cache */
	DCFlushRange((void *)0x80000000, 0x3F00);
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
						vmode = progressive ? &TVNtsc480Prog : &TVEurgb60Hz480IntDf;
					}
					break;
			}
			break;
		case 1: // PAL50
			vmode =  &TVPal528IntDf;
			vmode_reg = vmode->viTVMode >> 2;
			break;
		case 2: // PAL60
			vmode = progressive ? &TVNtsc480Prog : &TVEurgb60Hz480IntDf;
			vmode_reg = progressive ? TVEurgb60Hz480Prog.viTVMode >> 2 : vmode->viTVMode >> 2;
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
	// Stop wait message thread
	extern void HideWaitMessage();
	HideWaitMessage();

	/* Set video mode register */
	*(vu32 *)0x800000CC = vmode_reg;

	/* Set video mode */
	if (vmode != 0) VIDEO_Configure(vmode);
		
	/* Setup video  */
 	VIDEO_SetBlack(TRUE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if (vmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();
	else while (VIDEO_GetNextField())
		VIDEO_WaitVSync();
}

void __Disc_SetTime(void)
{
	/* Set proper time */
	settime(secs_to_ticks(time(NULL) - 946684800));
}

s32 __Disc_FindPartition(u64 *outbuf)
{
	u64 offset = 0;
	u32 cnt;

	/* Read partition info */
	s32 ret = WDVD_UnencryptedRead(buffer, 0x20, PTABLE_OFFSET);
	if (ret < 0) return ret;

	/* Get data */
	u32 nb_partitions = buffer[0];
	u64 table_offset  = buffer[1] << 2;
	
	if (nb_partitions > 8) return -1;

	/* Read partition table */
	ret = WDVD_UnencryptedRead(buffer, 0x20, table_offset);
	if (ret < 0) return ret;

	/* Find game partition */
	for (cnt = 0; cnt < nb_partitions; cnt++)
	{
		u32 type = buffer[cnt * 2 + 1];

		/* Game partition */
		if(!type) offset = buffer[cnt * 2] << 2;
	}

	/* No game partition found */
	if (!offset) return -1;

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
	return WDVD_ReadDiskId(diskid);
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
		magic = header->magic;
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

s32 Disc_BootPartition(u64 offset, u8 vidMode, bool vipatch, bool countryString, u8 patchVidMode)
{
	entry_point p_entry;

	s32 ret = WDVD_OpenPartition(offset, 0, 0, 0, Tmd_Buffer);
	if (ret < 0) return ret;

	/* Select an appropriate video mode */
	__Disc_SelectVMode(vidMode, 0);

	/* Setup low memory */;
	__Disc_SetLowMem();

	/* Run apploader */
	ret = Apploader_Run(&p_entry, vidMode, vmode, vipatch, countryString, patchVidMode);
	free_wip();
	if (ret < 0) return ret;

	if (hooktype != 0)
		ocarina_do_code();

	gprintf("\n\nEntry Point is: %0x8\n", p_entry);
//	gprintf("Lowmem:\n\n");
//	ghexdump((void*)0x80000000, 0x3f00);

	/* Set time */
	__Disc_SetTime();

	/* Set an appropriate video mode */
	__Disc_SetVMode();

	u8 temp_data[4];

	// fix for PeppaPig
	memcpy((char *) &temp_data, (void*)0x800000F4,4);

	usleep(100 * 1000);

	/* Shutdown IOS subsystems */
	SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);

	 /* Originally from tueidj - taken from NeoGamme (thx) */
	*(vu32*)0xCC003024 = 1;
	
	// fix for PeppaPig
	memcpy((void*)0x800000F4,(char *) &temp_data, 4);

	appentrypoint = (u32) p_entry;
	
	gprintf("Jumping to entrypoint\n");
	
	if (hooktype != 0)
	{
		__asm__(
			"lis %r3, appentrypoint@h\n"
			"ori %r3, %r3, appentrypoint@l\n"
			"lwz %r3, 0(%r3)\n"
			"mtlr %r3\n"
			"nop\n"
			"lis %r3, 0x8000\n"
			"nop\n"
			"ori %r3, %r3, 0x18A8\n"
			"nop\n"
			"mtctr %r3\n"
			"bctr\n"
		);
	}
	else
	{
		__asm__(
			"lis %r3, appentrypoint@h\n"
			"ori %r3, %r3, appentrypoint@l\n"
			"lwz %r3, 0(%r3)\n"
			"mtlr %r3\n"
			"blr\n"
		);
	}

	return 0;
}

s32 Disc_WiiBoot(u8 vidMode, bool vipatch, bool countryString, u8 patchVidModes)
{
	u64 offset;

	/* Find game partition offset */
	s32 ret = __Disc_FindPartition(&offset);
	if (ret < 0) return ret;

	/* Boot partition */
	return Disc_BootPartition(offset, vidMode, vipatch, countryString, patchVidModes);
}
