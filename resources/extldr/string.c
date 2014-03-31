// Copyright 2008-2009  Segher Boessenkool  <segher@kernel.crashing.org>
// This code is licensed to you under the terms of the GNU GPL, version 2;
// see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
#include "types.h"
void * _memset(void *b, u8 c, u32 len)
{
	u32 i;

	for (i = 0; i < len; i++)
		((u8*)b)[i] = c;

	return b;
}

void * _memcpy(void *dst, const void *src, u32 len)
{
	u32 i;

	for (i = 0; i < len; i++)
		((u8*)dst)[i] = ((u8*)src)[i];

	return dst;
}
