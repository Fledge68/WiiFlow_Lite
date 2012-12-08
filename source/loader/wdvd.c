#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include "gecko/gecko.hpp"

/* Constants */
#define IOCTL_DI_READID					0x70
#define IOCTL_DI_READ					0x71
#define IOCTL_DI_WAITCVRCLOSE			0x79
#define IOCTL_DI_GETCOVER				0x88
#define IOCTL_DI_RESET					0x8A
#define IOCTL_DI_OPENPART				0x8B
#define IOCTL_DI_CLOSEPART				0x8C
#define IOCTL_DI_UNENCREAD				0x8D
#define IOCTL_DI_SEEK					0xAB
#define IOCTL_DI_STOPLASER				0xD2
#define IOCTL_DI_OFFSET					0xD9
#define IOCTL_DI_DISC_BCA				0xDA
#define IOCTL_DI_REQUESTERROR			0xE0
#define IOCTL_DI_STOPMOTOR				0xE3
#define IOCTL_DI_DVDAUDIOBUFFERCFG		0xE4
#define IOCTL_DI_SETWBFSMODE			0xF4

#define IOCTL_DI_SETFRAG				0xF9
#define IOCTL_DI_GETMODE				0xFA
#define IOCTL_DI_HELLO					0xFB

/* Variables */
static u32 inbuf[8]  ATTRIBUTE_ALIGN(32);
static u32 outbuf[8] ATTRIBUTE_ALIGN(32);

static const char di_fs[] ATTRIBUTE_ALIGN(32) = "/dev/di";
static s32 di_fd = -1;

s32 WDVD_Init(void)
{
	/* Open "/dev/di" */
	if (di_fd < 0) {
		di_fd = IOS_Open(di_fs, 0);
		if (di_fd < 0)
			return di_fd;
	}

	return 0;
}

s32 WDVD_Close(void)
{
	/* Close "/dev/di" */
	if (di_fd >= 0) {
		IOS_Close(di_fd);
		di_fd = -1;
	}

	return 0;
}

s32 WDVD_GetHandle(void)
{
	/* Return di handle */
	return di_fd;
}

s32 WDVD_Reset(void)
{
	memset(inbuf, 0, sizeof(inbuf));

	/* Reset drive */
	inbuf[0] = IOCTL_DI_RESET << 24;
	inbuf[1] = 1;

	s32 ret = IOS_Ioctl(di_fd, IOCTL_DI_RESET, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
	if (ret < 0) return ret;

	return (ret == 1) ? 0 : -ret;
}

s32 WDVD_ReadDiskId(void *id)
{
	memset(inbuf, 0, sizeof(inbuf));

	/* Read disc ID */
	inbuf[0] = IOCTL_DI_READID << 24;

	s32 ret = IOS_Ioctl(di_fd, IOCTL_DI_READID, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
	if (ret < 0) return ret;

	if (ret == 1)
	{
		memcpy(id, outbuf, sizeof(dvddiskid));
		return 0;
	}

	return -ret;
}

s32 WDVD_Seek(u64 offset)
{
	memset(inbuf, 0, sizeof(inbuf));

	/* Drive seek */
	inbuf[0] = IOCTL_DI_SEEK << 24;
	inbuf[1] = (u32)(offset >> 2);

	s32 ret = IOS_Ioctl(di_fd, IOCTL_DI_SEEK, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
	if (ret < 0) return ret;

	return (ret == 1) ? 0 : -ret;
}

s32 WDVD_Offset(u64 offset) 
{
    //u32 *off = (u32 *)((void *)&offset);
	union { u64 off64; u32 off32[2]; } off;
	off.off64 = offset;

    memset(inbuf, 0, sizeof(inbuf));

    /* Set offset */
    inbuf[0] = IOCTL_DI_OFFSET << 24;
    inbuf[1] = (off.off32[0]) ? 1: 0;
    inbuf[2] = (off.off32[1] >> 2);

    s32 ret = IOS_Ioctl(di_fd, IOCTL_DI_OFFSET, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
    if (ret < 0) return ret;

    return (ret == 1) ? 0 : -ret;
}

s32 WDVD_StopLaser(void)
{
	memset(inbuf, 0, sizeof(inbuf));

	/* Stop laser */
	inbuf[0] = IOCTL_DI_STOPLASER << 24;

	s32 ret = IOS_Ioctl(di_fd, IOCTL_DI_STOPLASER, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
	if (ret < 0) return ret;

	return (ret == 1) ? 0 : -ret;
}

s32 WDVD_StopMotor(void)
{
	memset(inbuf, 0, sizeof(inbuf));

	/* Stop motor */
	inbuf[0] = IOCTL_DI_STOPMOTOR << 24;

	s32 ret = IOS_Ioctl(di_fd, IOCTL_DI_STOPMOTOR, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
	if (ret < 0) return ret;

	return (ret == 1) ? 0 : -ret;
}

s32 WDVD_Eject(void)
{
	memset(inbuf, 0, sizeof(inbuf));

	/* Stop motor */
	inbuf[0] = IOCTL_DI_STOPMOTOR << 24;
	/* Eject DVD */
	inbuf[1] = 1;

	s32 ret = IOS_Ioctl(di_fd, IOCTL_DI_STOPMOTOR, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
	if (ret < 0) return ret;

	return (ret == 1) ? 0 : -ret;
}

s32 WDVD_OpenPartition(u64 offset)
{
	if (di_fd < 0)
		return di_fd;

	static u8 Tmd_Buffer[0x4A00] ATTRIBUTE_ALIGN(32);
	static ioctlv Vectors[5] ATTRIBUTE_ALIGN(32);
	s32 ret;

	memset(inbuf, 0, sizeof inbuf);
	memset(outbuf, 0, sizeof outbuf);

	inbuf[0] = IOCTL_DI_OPENPART << 24;
	inbuf[1] = offset >> 2;

	Vectors[0].data		= inbuf;
	Vectors[0].len		= 0x20;
	Vectors[1].data		= 0;
	Vectors[1].len		= 0;
	Vectors[2].data		= 0;
	Vectors[2].len		= 0;
	Vectors[3].data		= Tmd_Buffer;
	Vectors[3].len		= 0x49e4;
	Vectors[4].data		= outbuf;
	Vectors[4].len		= 0x20;

	ret = IOS_Ioctlv(di_fd, IOCTL_DI_OPENPART, 3, 2, (ioctlv *)Vectors);

	if (ret < 0)
		return ret;

	return (ret == 1) ? 0 : -ret;
}

s32 WDVD_ClosePartition(void)
{
	memset(inbuf, 0, sizeof(inbuf));

	/* Close partition */
	inbuf[0] = IOCTL_DI_CLOSEPART << 24;

	s32 ret = IOS_Ioctl(di_fd, IOCTL_DI_CLOSEPART, inbuf, sizeof(inbuf), NULL, 0);
	if (ret < 0) return ret;

	return (ret == 1) ? 0 : -ret;
}

s32 WDVD_UnencryptedRead(void *buf, u32 len, u64 offset)
{
	memset(inbuf, 0, sizeof(inbuf));

	/* Unencrypted read */
	inbuf[0] = IOCTL_DI_UNENCREAD << 24;
	inbuf[1] = len;
	inbuf[2] = (u32)(offset >> 2);

	s32 ret = IOS_Ioctl(di_fd, IOCTL_DI_UNENCREAD, inbuf, sizeof(inbuf), buf, len);
	if (ret < 0) return ret;

	return (ret == 1) ? 0 : -ret;
}

s32 WDVD_Read(void *buf, u32 len, u64 offset)
{
	memset(inbuf, 0, sizeof(inbuf));

	/* Disc read */
	inbuf[0] = IOCTL_DI_READ << 24;
	inbuf[1] = len;
	inbuf[2] = (u32)(offset >> 2);

	s32 ret = IOS_Ioctl(di_fd, IOCTL_DI_READ, inbuf, sizeof(inbuf), buf, len);
	if (ret < 0) return ret;

	return (ret == 1) ? 0 : -ret;
}

s32 WDVD_LowRequestError(u32 *error)
{
	memset(inbuf, 0, sizeof(inbuf));
	
	inbuf[0] = IOCTL_DI_REQUESTERROR << 24;
	
	s32 ret = IOS_Ioctl(di_fd, IOCTL_DI_REQUESTERROR, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
	if (ret < 0) return ret;
	
	if (ret == 1) 	
		memcpy(error, outbuf, sizeof(u32));

	return (ret == 1) ? 0 : -ret;
}

s32 WDVD_WaitForDisc(void)
{
	memset(inbuf, 0, sizeof(inbuf));

	/* Wait for disc */
	inbuf[0] = IOCTL_DI_WAITCVRCLOSE << 24;

	s32 ret = IOS_Ioctl(di_fd, IOCTL_DI_WAITCVRCLOSE, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
	if (ret < 0) return ret;

	return (ret == 1) ? 0 : -ret;
}

s32 WDVD_GetCoverStatus(u32 *status)
{
	memset(inbuf, 0, sizeof(inbuf));

	/* Get cover status */
	inbuf[0] = IOCTL_DI_GETCOVER << 24;

	s32 ret = IOS_Ioctl(di_fd, IOCTL_DI_GETCOVER, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
	if (ret < 0) return ret;

	if (ret == 1) {
		/* Copy cover status */
		memcpy(status, outbuf, sizeof(u32));

		return 0;
	}

	return -ret;
}

s32 WDVD_SetUSBMode(u32 mode, const u8 *id, s32 partition) 
{
    memset(inbuf, 0, sizeof(inbuf));

    /* Set USB mode */
    inbuf[0] = IOCTL_DI_SETWBFSMODE << 24;
    inbuf[1] = mode;

    /* Copy ID */
    if(id)
	{
        memcpy(&inbuf[2], id, 6);
		if(partition >= 0)
			inbuf[5] = partition;
    }

    s32 ret = IOS_Ioctl(di_fd, IOCTL_DI_SETWBFSMODE, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));

    if(ret < 0)
		return ret;
    return(ret == 1) ? 0 : -ret;
}

s32 WDVD_Read_Disc_BCA(void *buf)
{
	memset(inbuf, 0, sizeof(inbuf));

	/* Disc read */
	inbuf[0] = IOCTL_DI_DISC_BCA << 24;
	//inbuf[1] = 64;

	s32 ret = IOS_Ioctl(di_fd, IOCTL_DI_DISC_BCA, inbuf, sizeof(inbuf), buf, 64);
	if (ret < 0) return ret;

	return (ret == 1) ? 0 : -ret;
}

// frag

s32 WDVD_SetFragList(int device, void *fraglist, int size)
{
	memset(inbuf, 0, sizeof(inbuf));
	memset(outbuf, 0, sizeof(outbuf));

	/* Set FRAG mode */
	inbuf[0] = IOCTL_DI_SETFRAG << 24;
	inbuf[1] = device;
	inbuf[2] = (u32)fraglist;
	inbuf[3] = size;

	DCFlushRange(fraglist, size);
	s32 ret = IOS_Ioctl(di_fd, IOCTL_DI_SETFRAG, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
	if (ret < 0) return ret;

	return (ret == 1) ? 0 : -ret;
}

s32 WDVD_hello(u32 *status)
{
	memset(inbuf, 0, sizeof(inbuf));

	inbuf[0] = IOCTL_DI_HELLO << 24;

	s32 ret = IOS_Ioctl(di_fd, IOCTL_DI_HELLO, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
	if (ret < 0) return ret;

	if (ret == 1)
	{
		if (status) memcpy(status, outbuf, sizeof(u32));
		ghexdump(outbuf, 12);
		return 0;
	}

	return -ret;
}

s32 WDVD_SetStreaming(void)
{
	memset(inbuf, 0, sizeof(inbuf));

	inbuf[0] = IOCTL_DI_DVDAUDIOBUFFERCFG << 24;

	if ((*(u32*)0x80000008)>>24)
	{
		inbuf[1] = 1;
		if(((*(u32*)0x80000008)>>16) & 0xFF)
			inbuf[2] = 10;
		else
			inbuf[2] = 0;
	}
	else
	{	
		inbuf[1] = 0;
		inbuf[2] = 0;
	}

	s32 ret = IOS_Ioctl(di_fd, IOCTL_DI_DVDAUDIOBUFFERCFG, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
	if (ret < 0) return ret;

	return (ret == 1) ? 0 : -ret;
}

s32 WDVD_NEEK_LoadDisc(u32 id, u32 magic)
{
	u32 *vec = (u32*)memalign(32, sizeof(u32) * 2);
	vec[0] = id;
	vec[1] = magic;

	s32 ret = IOS_Ioctl(di_fd, 0x25, vec, sizeof(u32) * 2, NULL, 0);

	gprintf("Load disc: (%08x)(%08x)(%d)(%d)\n", id, magic, ret, di_fd);
	free(vec);

	return ret;
}
