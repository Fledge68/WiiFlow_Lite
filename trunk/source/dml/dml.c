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
