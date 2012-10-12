/*
	TinyLoad - a simple region free (original) game launcher in 4k

# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

#ifndef __IOS_H__
#define __IOS_H__

// Copyright 2008-2009  Hector Martin  <marcan@marcansoft.com>

struct ioctlv {
	void *data;
	u32 len;
};

void ipc_init(void);
int ios_open(const char *filename, u32 mode);
int ios_close(int fd);
int ios_ioctl(int fd, u32 n, void *in, u32 in_size, void *out, u32 out_size);
int ios_ioctlv(int fd, u32 n, u32 in_count, u32 out_count, struct ioctlv *vec);
int ios_ioctlvreboot(int fd, u32 n, u32 in_count, u32 out_count, struct ioctlv *vec);
int ios_read(int fd, void *buf, u32 size);
void ios_cleanup(void);

#endif