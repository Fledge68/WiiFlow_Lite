// Copyright 2008-2009  Segher Boessenkool  <segher@kernel.crashing.org>
// This code is licensed to you under the terms of the GNU GPL, version 2;
// see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

void * _memset(void *b, int c, unsigned int len)
{
	unsigned int i;

	for (i = 0; i < len; i++)
		((unsigned char *)b)[i] = c;

	return b;
}

void * _memcpy(void *dst, const void *src, unsigned int len)
{
	unsigned int i;

	for (i = 0; i < len; i++)
		((unsigned char *)dst)[i] = ((unsigned char *)src)[i];

	return dst;
}
