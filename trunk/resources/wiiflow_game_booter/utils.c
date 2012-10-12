/*
	TinyLoad - a simple region free (original) game launcher in 4k

# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

/* This code comes from HBC's stub which was based on geckoloader and the Twilight Hack code */
/* Some of these routines are from public domain sources */
// Copyright 2008-2009  Segher Boessenkool  <segher@kernel.crashing.org>
// Copyright 2008-2009  Andre Heider  <dhewg@wiibrew.org>
// Copyright 2008-2009  Hector Martin  <marcan@marcansoft.com>

#include "types.h"
#include "utils.h"

#ifndef TINY
void *memset(void *ptr, int c, int size) {
	char* ptr2 = ptr;
	while(size--) *ptr2++ = (char)c;
	return ptr;
}

void *memcpy(void *ptr, const void *src, int size) {
	char* ptr2 = ptr;
	const char* src2 = src;
	while(size--) *ptr2++ = *src2++;
	return ptr;
}

int strlen(const char *ptr) {
	int i=0;
	while(*ptr++) i++;
	return i;
}
#endif

int memcmp(const void *s1, const void *s2, size_t n)
{
     const unsigned char *us1 = (const unsigned char *) s1;
     const unsigned char *us2 = (const unsigned char *) s2;
     while (n-- != 0) {
         if (*us1 != *us2)
#ifdef TINY
             return 1;
#else
             return (*us1 < *us2) ? -1 : +1;
#endif
         us1++;
         us2++;
     }
     return 0;
}


// Timebase frequency is core frequency / 8.  Ignore roundoff, this
// doesn't have to be very accurate.
#define TICKS_PER_USEC (729/8)

static u32 mftb(void)
{
	u32 x;

	asm volatile("mftb %0" : "=r"(x));

	return x;
}

static void __delay(u32 ticks)
{
	u32 start = mftb();

	while (mftb() - start < ticks)
		;
}

void udelay(u32 us)
{
	__delay(TICKS_PER_USEC * us);
}
