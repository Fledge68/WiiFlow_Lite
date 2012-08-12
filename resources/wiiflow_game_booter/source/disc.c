#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogcsys.h>
#include <unistd.h>
#include <malloc.h>
#include <ogc/lwp_watchdog.h>

#include "memory.h"
#include "cios.h"
#include "types.h"
#include "wdvd.h"

/* Constants */
#define PTABLE_OFFSET	0x40000

static u8 *diskid = (u8*)0x80000000;

void Disc_SetLowMem()
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
	if(CurrentIOS.Type != IOS_TYPE_HERMES)
		*GameID_Address	= 0x80000000;

	/* Copy disc ID */
	memcpy((void *)Online_Check, (void *)Disc_ID, 4);
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

void Disc_SetTime()
{
	/* Set proper time */
	settime(secs_to_ticks(time(NULL) - 946684800));
}

GXRModeObj *Disc_SelectVMode(u8 videoselected, u32 *rmode_reg)
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

	char Region = diskid[3];

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
