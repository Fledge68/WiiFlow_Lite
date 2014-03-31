/*
	TinyLoad - a simple region free (original) game launcher in 4k

# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

/* This code comes from HBC's stub which was based on the Twilight Hack code */
// Copyright 2008-2009  Segher Boessenkool  <segher@kernel.crashing.org>
// Copyright 2008-2009  Andre Heider  <dhewg@wiibrew.org>
// Copyright 2008-2009  Hector Martin  <marcan@marcansoft.com>

#include "ios.h"
#include "cache.h"
#include "utils.h"

#define virt_to_phys(x) ((u32*)(((u32)(x))&0x3FFFFFFF))
#define phys_to_virt(x) ((u32*)(((u32)(x))|0x80000000))

// Low-level IPC access.

static inline u32 iread32(u32 addr)
{
	u32 x;

	asm volatile("lwz %0,0(%1) ; sync ; isync" : "=r"(x) : "b"(0xc0000000 | addr));

	return x;
}

static inline void iwrite32(u32 addr, u32 x)
{
	asm volatile("stw %0,0(%1) ; eieio" : : "r"(x), "b"(0xc0000000 | addr));
}

static u32 _ipc_read(u32 reg) __attribute__((noinline));
static void _ipc_write(u32 reg, u32 value) __attribute__((noinline));
static void ipc_bell(u32 w) __attribute__((noinline));

// inline the 4*, don't inline the 0x0d0 stuff. yes, this saves a few bytes.
static u32 _ipc_read(u32 reg)
{
	return iread32(0x0d000000 + reg);
}

static void _ipc_write(u32 reg, u32 value)
{
	iwrite32(0x0d000000 + reg, value);
}

static inline u32 ipc_read(u32 reg)
{
	return _ipc_read(4*reg);
}

static inline void ipc_write(u32 reg, u32 value)
{
	_ipc_write(4*reg, value);
}

static void ipc_bell(u32 w)
{
	ipc_write(1, w);
}

static void ios_delay(void) __attribute__((noinline));

static void ios_delay(void)
{
	usleep(500);
}

static void ipc_wait_ack(void)
{
	while(!(ipc_read(1) & 0x2))
		;
	ios_delay();
}

static void ipc_wait_reply(void)
{
	while(!(ipc_read(1) & 0x4))
		;
	ios_delay();
}

// Mid-level IPC access.

struct ipc {
	u32 cmd;
	int result;
	int fd;
	u32 arg[5];

	u32 user[8];
};

static struct ipc ipc ALIGNED(64);

static void ipc_send_request(void)
{
	sync_after_write(&ipc, 0x40);

	ipc_write(0, (u32)virt_to_phys(&ipc));
	ipc_bell(1);

	ipc_wait_ack();

	ipc_bell(2);
}

static void ipc_recv_reply(void)
{
	for (;;)
	{
		u32 reply;

		ipc_wait_reply();

		reply = ipc_read(2);
		ipc_bell(4);

		ipc_bell(8);

		if (((u32*)reply) == virt_to_phys(&ipc))
			break;
	}

	sync_before_read(&ipc, 0x40);
}


// High-level IPC access.

int ios_close(int fd)
{
	ipc.cmd = 2;
	ipc.fd = fd;

	ipc_send_request();
	ipc_recv_reply();

	return ipc.result;
}

void ios_cleanup()
{
	int fd;
	for (fd = 0; fd != 31; fd++)
	{
		ios_close(fd);
	}
}
