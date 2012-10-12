/*
	TinyLoad - a simple region free (original) game launcher in 4k

# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

// Copyright 2008-2009  Hector Martin  <marcan@marcansoft.com>

#ifndef __DI_H__
#define __DI_H__

int di_init(void);
int di_getcoverstatus(void);
int di_requesterror(void);
int di_read(void *dst, u32 size, u32 offset);
int di_unencryptedread(void *dst, u32 size, u32 offset);
int di_identify(void);
int di_reset(void);
int di_stopmotor(void);
int di_openpartition(u32 offset, u8 *tmd);
int di_closepartition(void);
int di_readdiscid(void *dst);
int di_shutdown(void);

int WDVD_SetFragList(int device, void *fraglist, int size);
int WDVD_SetUSBMode(u32 mode, const u8 *id, s32 partition);

#endif