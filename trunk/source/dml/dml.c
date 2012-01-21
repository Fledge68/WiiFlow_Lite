#include <stdint.h>
#include <string.h>
#include <gccore.h>
#include <ogc/es.h>
#include <ogc/video_types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>
#include <math.h>
#include <ogcsys.h>
#include <unistd.h>
#include <sys/statvfs.h>
#include "dml.h"
#define SEP 0xFF

#define BC 0x0000000100000100ULL
#define MIOS 0x0000000100000101ULL

/** Base address for video registers. */
#define MEM_VIDEO_BASE (0xCC002000)
#define IOCTL_DI_DVDLowAudioBufferConfig 0xE4

#define VIDEO_MODE_NTSC 0
#define VIDEO_MODE_PAL 1
#define VIDEO_MODE_PAL60 2
#define VIDEO_MODE_NTSC480P 3
#define VIDEO_MODE_PAL480P 4

syssram* __SYS_LockSram();
u32 __SYS_UnlockSram(u32 write);
u32 __SYS_SyncSram(void);

s32 setstreaming()
	{
	char __di_fs[] ATTRIBUTE_ALIGN(32) = "/dev/di";
	u32 bufferin[0x20] __attribute__((aligned(32)));
	u32 bufferout[0x20] __attribute__((aligned(32)));
	s32 __dvd_fd = -1;
	
	u8 ioctl;
	ioctl = IOCTL_DI_DVDLowAudioBufferConfig;

	__dvd_fd = IOS_Open(__di_fs,0);
	if(__dvd_fd < 0) return __dvd_fd;

	memset(bufferin, 0, 0x20);
	memset(bufferout, 0, 0x20);

	bufferin[0] = (ioctl << 24);

	if ( (*(u32*)0x80000008)>>24 )
		{
		bufferin[1] = 1;
		if( ((*(u32*)0x80000008)>>16) & 0xFF )
			bufferin[2] = 10;
		else 
			bufferin[2] = 0;
		}
	else
		{		
		bufferin[1] = 0;
		bufferin[2] = 0;
		}			
	DCFlushRange(bufferin, 0x20);
	
	int Ret = IOS_Ioctl(__dvd_fd, ioctl, bufferin, 0x20, bufferout, 0x20);
	
	IOS_Close(__dvd_fd);
	
	return ((Ret == 1) ? 0 : -Ret);
	}

void SRAM_PAL()
{
	syssram *sram;
	sram = __SYS_LockSram();
	sram->flags = sram->flags |  (1 << 0);	// Set bit 0 to set the video mode to PAL
	__SYS_UnlockSram(1); // 1 -> write changes
	while(!__SYS_SyncSram());
}

void SRAM_NTSC()
{
	syssram *sram;
	sram = __SYS_LockSram();
	sram->flags = sram->flags & ~(1 << 0);	// Clear bit 0 to set the video mode to NTSC
	__SYS_UnlockSram(1); // 1 -> write changes
	while(!__SYS_SyncSram());
}
