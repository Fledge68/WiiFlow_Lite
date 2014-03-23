// Copyright 2008-2009  Segher Boessenkool  <segher@kernel.crashing.org>
// This code is licensed to you under the terms of the GNU GPL, version 2;
// see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

void sync_before_exec(const void *p, unsigned int len)
{
	unsigned int a, b;

	a = (unsigned int)p & ~0x1f;
	b = ((unsigned int)p + len + 0x1f) & ~0x1f;

	for ( ; a < b; a += 32)
		asm("dcbst 0,%0 ; sync ; icbi 0,%0" : : "b"(a));

	asm("sync ; isync");
}
