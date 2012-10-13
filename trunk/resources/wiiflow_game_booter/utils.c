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
#include "cache.h"

#ifdef __cplusplus
extern "C"
{
#endif

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

int memcmp(const void *s1, const void *s2, size_t n)
{
     const unsigned char *us1 = (const unsigned char *) s1;
     const unsigned char *us2 = (const unsigned char *) s2;
     while (n-- != 0) {
         if (*us1 != *us2)
             return (*us1 < *us2) ? -1 : +1;
         us1++;
         us2++;
     }
     return 0;
}

void memset32(u32 *addr, u32 data, u32 count) 
{
	int sc = count;
	void *sa = addr;
	while(count--)
		*addr++ = data;
	sync_after_write(sa, 4*sc);
}

#ifdef __cplusplus
}
#endif

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
