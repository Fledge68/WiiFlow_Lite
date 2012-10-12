/*
	TinyLoad - a simple region free (original) game launcher in 4k

# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

// Copyright 2008-2009  Hector Martin  <marcan@marcansoft.com>

#include "types.h"
#include "ios.h"
#include "utils.h"
#include "debug.h"
#include "di.h"

int di_fd=0;

u32 inbuf[16] ALIGNED(64);
u32 outbuf[16] ALIGNED(64);

extern u32 __bss_start[];
extern u32 __bss_end[];

int di_init(void)
{
	debug_string("Opening /dev/di: ");
	di_fd = ios_open("/dev/di",0);
	debug_uint(di_fd);
	debug_string("\n");
	//memset(inbuf, 0, 0x20 );
	return di_fd;
}

int di_shutdown(void)
{
	return ios_close(di_fd);
}

static int _di_ioctl_std(int num) __attribute__((noinline));
static int _di_ioctl_std(int num)
{
	inbuf[0x00] = num << 24;
	return ios_ioctl( di_fd, num, inbuf, 0x20, outbuf, 0x20);
}

int di_getcoverstatus(void)
{
	//memset(inbuf, 0, 0x20 );
	//memset(outbuf, 0, 0x20 );

	//inbuf[0x00] = 0x88000000;

	int ret = _di_ioctl_std(0x88);
	if( ret != 1 )
		return -1;

	return outbuf[0];
}

int di_identify(void)
{
	//memset(inbuf, 0, 0x20 );
	//memset(outbuf, 0, 0x20 );

	//inbuf[0x00] = 0x12000000;

	return _di_ioctl_std(0x12);
}
#ifndef TINY
int di_waitforcoverclose(void)
{
	//memset(inbuf, 0, 0x20 );
	//memset(outbuf, 0, 0x20 );

	//inbuf[0x00] = 0x79000000;

	int ret = _di_ioctl_std(0x79);
	if( ret < 0 )
		return ret;

	return outbuf[0];
}
#endif
#ifndef TINY
int di_requesterror(void)
{
	//memset(inbuf, 0, 0x20 );
	//memset(outbuf, 0, 0x20 );

	//inbuf[0x00] = 0xE0000000;

	int ret = _di_ioctl_std(0xE0);
	if( ret < 0 )
		return ret;
	return outbuf[0];
}
#endif

int di_read(void *dst, u32 size, u32 offset)
{
	//memset(inbuf, 0, 0x20 );

	inbuf[0x00] = 0x71000000;
	inbuf[0x01] = size;
	inbuf[0x02] = offset;

	return ios_ioctl( di_fd, 0x71, inbuf, 0x20, dst, size);
}

int di_unencryptedread(void *dst, u32 size, u32 offset)
{
	//memset(inbuf, 0, 0x20 );

	inbuf[0x00] = 0x8D000000;
	inbuf[0x01] = size;
	inbuf[0x02] = offset;

	return ios_ioctl( di_fd, 0x8D, inbuf, 0x20, dst, size);
}

int di_reset(void)
{
	//memset(inbuf, 0, 0x20 );

	//inbuf[0x00] = 0x8A000000;
	inbuf[0x01] = 1;
	
	return _di_ioctl_std(0x8A);
}

#ifndef TINY
int di_stopmotor(void)
{
	//memset(inbuf, 0, 0x20 );
	//inbuf[0x00] = 0xE3000000;
	inbuf[0x01] = 0;
	inbuf[0x02] = 0;

	return _di_ioctl_std(0xE3);
}
#endif

int di_openpartition(u32 offset, u8 *tmd)
{
	static struct ioctlv vectors[16] ALIGNED(64);

	//memset(inbuf, 0, 0x20 );
	inbuf[0x00] = 0x8B000000;
	inbuf[0x01] = offset;

	vectors[0].data = inbuf;
	vectors[0].len = 0x20;
	vectors[1].data = NULL;
	vectors[1].len = 0x2a4;
	vectors[2].data = NULL;
	vectors[2].len = 0;

	vectors[3].data = tmd;
	vectors[3].len = 0x49e4;
	vectors[4].data = outbuf;
	vectors[4].len = 0x20;

	return ios_ioctlv(di_fd, 0x8B, 3, 2, vectors);
}
#ifndef TINY
int di_closepartition(void)
{
	//memset(inbuf, 0, 0x20 );
	//inbuf[0x00] = 0x8C000000;
	return _di_ioctl_std(0x8C);
}
#endif
int di_readdiscid(void *dst)
{
	//memset(inbuf, 0, 0x20 );
	//memset(dst, 0, 0x20 );

	inbuf[0x00] = 0x70000000;

	return ios_ioctl( di_fd, 0x70, inbuf, 0x20, dst, 0x20);
}

int WDVD_SetFragList(int device, void *fraglist, int size)
{
	debug_string("WDVD_SetFragList\n");
	debug_uint(device);
	debug_uint((u32)fraglist);
	debug_uint(size);
	/* Set FRAG mode */
	inbuf[0] = 0xF9000000;
	inbuf[1] = device;
	inbuf[2] = (u32)fraglist;
	inbuf[3] = size;

	return ios_ioctl(di_fd, 0xF9, inbuf, 0x20, outbuf, 0x20);
}

int WDVD_SetUSBMode(u32 mode, const u8 *id, s32 partition) 
{
	debug_string("WDVD_SetUSBMode\n");
    memset(inbuf, 0, 0x20);

    /* Set USB mode */
    inbuf[0] = 0xF4000000;
    inbuf[1] = mode;

    /* Copy ID */
    if(id)
	{
        memcpy(&inbuf[2], id, 6);
		if(partition >= 0)
			inbuf[5] = partition;
    }

    return ios_ioctl(di_fd, 0xF4, inbuf, 0x20, outbuf, 0x20);
}
