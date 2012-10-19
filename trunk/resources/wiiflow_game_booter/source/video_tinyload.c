/*
	TinyLoad - a simple region free (original) game launcher in 4k

# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

/* This code comes from HBC's stub which was based on dhewg's geckoloader stub */
// Copyright 2008-2009  Andre Heider  <dhewg@wiibrew.org>
// Copyright 2008-2009  Hector Martin  <marcan@marcansoft.com>
#include <ogc/cache.h>
#include <string.h>
#include "utils.h"

#define COLOR_BLACK        0x00800080
#define COLOR_GRAY         0x80808080
#define COLOR_WHITE        0xFF80FF80
#define COLOR_RED          0x4C544CFF
#define COLOR_GREEN        0x4B554B4A
#define COLOR_BLUE         0x1DFF1D6B
#define COLOR_YELLOW       0xE100E194

#define framebuffer ((u32*)(0x81600000))

u32 vtrdcr;
u32 oldvtrdcr;
u32 vimode;
u32 vto,vte;
u64 oldvtovte;
int progress = 20;

void memset32(u32 *addr, u32 data, u32 count) 
{
	int sc = count;
	void *sa = addr;
	while(count--)
		*addr++ = data;
	DCFlushRange(sa, 4*sc);
}

static void drawbar(int pix)
{
	int i = 16;
	u32* p = framebuffer + 320 * 400;
	while(i--) {
		memset32(p, COLOR_WHITE, pix);
		p += 320;
	}
}

void prog(int p) {
	progress += p;
	drawbar(progress);
}

void setprog(int p)
{
	progress = p;
	drawbar(progress);
}

void prog10(void) {
	prog(10);
}

inline void viw(u32 addr, u32 data) 
{
	*(vu32*)(addr+0xCC002000) = data;
}

void video_init(void)
{
	memset32(framebuffer, COLOR_BLACK, 320*574);
	memset32(framebuffer + 320 * 398, COLOR_WHITE, 320*2);
	memset32(framebuffer + 320 * 416, COLOR_WHITE, 320*2);

	vtrdcr = oldvtrdcr = *(vu32*)(0xCC002000);
	oldvtovte = *(vu64*)0xCC00200c;
	vtrdcr &= 0xFFFFF;
	vtrdcr |= 0x0F000000;
	vimode = (vtrdcr>>8)&3;
	
	vto = 0x30018;
	vte = 0x20019;
	
	if(vtrdcr & 4) { // progressive
		vto = vte = 0x60030;
		vtrdcr += 0x0F000000;
	} else if(vimode == 1) {
		vto = 0x10023;
		vte = 0x24;
		vtrdcr += 0x02F00000;
	}
	viw(0x0, vtrdcr);
	viw(0xc, vto);
	viw(0x10, vte);
	
	viw(0x1c, (((u32)framebuffer) >> 5) | 0x10000000);
	viw(0x24, (((u32)framebuffer) >> 5) | 0x10000000);
	
	prog(0);
}

void video_clear(void)
{
	memset32(framebuffer, COLOR_BLACK, 320*574);
	// this sets VI to black, but I can't fit it in yet...
	viw(0x0, oldvtrdcr);
	*(vu64*)(0xCC00200c) = oldvtovte;
}
