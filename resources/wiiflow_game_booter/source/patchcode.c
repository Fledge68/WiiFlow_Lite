/*
 *  Copyright (C) 2008 Nuke (wiinuke@gmail.com)
 *
 *  this file is part of GeckoOS for USB Gecko
 *  http://www.usbgecko.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gccore.h>
#include <sys/unistd.h>

#include "apploader.h"
#include "patchcode.h"
#include "memory.h"
#include "gecko.h"
#include "../../../source/defines.h"

u32 hooktype;
u8 configbytes[2];

extern void patchhook(u32 address, u32 len);
extern void patchhook2(u32 address, u32 len);
extern void patchhook3(u32 address, u32 len);

extern void multidolpatchone(u32 address, u32 len);
extern void multidolpatchtwo(u32 address, u32 len);

extern void regionfreejap(u32 address, u32 len);
extern void regionfreeusa(u32 address, u32 len);
extern void regionfreepal(u32 address, u32 len);

extern void removehealthcheck(u32 address, u32 len);

extern void copyflagcheck1(u32 address, u32 len);
extern void copyflagcheck2(u32 address, u32 len);
extern void copyflagcheck3(u32 address, u32 len);
extern void copyflagcheck4(u32 address, u32 len);
extern void copyflagcheck5(u32 address, u32 len);

extern void patchupdatecheck(u32 address, u32 len);

extern void movedvdhooks(u32 address, u32 len);

extern void multidolhook(u32 address);
extern void langvipatch(u32 address, u32 len, u8 langbyte);
extern void vipatch(u32 address, u32 len);

//static const u32 multidolpatch1[2] = {0x3C03FFB4,0x28004F43};
//static const u32 multidolpatch2[2] = {0x3F608000, 0x807B0018};

//static const u32 healthcheckhook[2] = {0x41810010,0x881D007D};
//static const u32 updatecheckhook[3] = {0x80650050,0x80850054,0xA0A50058};

//static const u32 recoveryhooks[3] = {0xA00100AC,0x5400073E,0x2C00000F};

//static const u32 nocopyflag1[3] = {0x540007FF, 0x4182001C, 0x80630068};
//static const u32 nocopyflag2[3] = {0x540007FF, 0x41820024, 0x387E12E2};
// this one is for the GH3 and VC saves
//static const u32 nocopyflag3[5] = {0x2C030000, 0x40820010, 0x88010020, 0x28000002, 0x41820234};
//static const u32 nocopyflag3[5] = {0x2C030000, 0x41820200,0x48000058,0x38610100};
// this removes the display warning for no copy VC and GH3 saves
//static const u32 nocopyflag4[4] = {0x80010008, 0x2C000000, 0x4182000C, 0x3BE00001};
//static const u32 nocopyflag5[3] = {0x801D0024,0x540007FF,0x41820024};

//static const u32 movedvdpatch[3] = {0x2C040000, 0x41820120, 0x3C608109};
//static const u32 regionfreehooks[5] = {0x7C600774, 0x2C000001, 0x41820030,0x40800010,0x2C000000};
//static const u32 cIOScode[16] = {0x7f06c378, 0x7f25cb78, 0x387e02c0, 0x4cc63182};
//static const u32 cIOSblock[16] = {0x2C1800F9, 0x40820008, 0x3B000024};
//static const u32 fwritepatch[8] = {0x9421FFD0,0x7C0802A6,0x90010034,0xBF210014,0x7C9B2378,0x7CDC3378,0x7C7A1B78,0x7CB92B78};  // bushing fwrite
static const u32 vipatchcode[3] = {0x4182000C,0x4180001C,0x48000018};

const u32 viwiihooks[4] = {0x7CE33B78,0x38870034,0x38A70038,0x38C7004C};
const u32 kpadhooks[4] = {0x9A3F005E,0x38AE0080,0x389FFFFC,0x7E0903A6};
const u32 kpadoldhooks[6] = {0x801D0060, 0x901E0060, 0x801D0064, 0x901E0064, 0x801D0068, 0x901E0068};
const u32 joypadhooks[4] = {0x3AB50001, 0x3A73000C, 0x2C150004, 0x3B18000C};
const u32 gxdrawhooks[4] = {0x3CA0CC01, 0x38000061, 0x3C804500, 0x98058000};
const u32 gxflushhooks[4] = {0x90010014, 0x800305FC, 0x2C000000, 0x41820008};
const u32 ossleepthreadhooks[4] = {0x90A402E0, 0x806502E4, 0x908502E4, 0x2C030000};
const u32 axnextframehooks[4] = {0x3800000E, 0x7FE3FB78, 0xB0050000, 0x38800080};
const u32 wpadbuttonsdownhooks[4] = {0x7D6B4A14, 0x816B0010, 0x7D635B78, 0x4E800020};
const u32 wpadbuttonsdown2hooks[4] = {0x7D6B4A14, 0x800B0010, 0x7C030378, 0x4E800020};

const u32 multidolhooks[4] = {0x7C0004AC, 0x4C00012C, 0x7FE903A6, 0x4E800420};
const u32 multidolchanhooks[4] = {0x4200FFF4, 0x48000004, 0x38800000, 0x4E800020};

const u32 langpatch[3] = {0x7C600775, 0x40820010, 0x38000000};

//static const u32 oldpatch002[3] = {0x2C000000, 0x40820214, 0x3C608000};
//static const u32 newpatch002[3] = {0x2C000000, 0x48000214, 0x3C608000};

unsigned char sig_fwrite[32] =
{
    0x94, 0x21, 0xFF, 0xD0,
	0x7C, 0x08, 0x02, 0xA6,
	0x90, 0x01, 0x00, 0x34,
	0xBF, 0x21, 0x00, 0x14, 
    0x7C, 0x9B, 0x23, 0x78,
	0x7C, 0xDC, 0x33, 0x78,
	0x7C, 0x7A, 0x1B, 0x78,
	0x7C, 0xB9, 0x2B, 0x78, 
} ;

unsigned char patch_fwrite[144] =
{
    0x7C, 0x85, 0x21, 0xD7, 0x40, 0x81, 0x00, 0x84, 0x3C, 0xE0, 0xCD, 0x00, 0x3D, 0x40, 0xCD, 0x00, 
    0x3D, 0x60, 0xCD, 0x00, 0x60, 0xE7, 0x68, 0x14, 0x61, 0x4A, 0x68, 0x24, 0x61, 0x6B, 0x68, 0x20, 
    0x38, 0xC0, 0x00, 0x00, 0x7C, 0x06, 0x18, 0xAE, 0x54, 0x00, 0xA0, 0x16, 0x64, 0x08, 0xB0, 0x00, 
    0x38, 0x00, 0x00, 0xD0, 0x90, 0x07, 0x00, 0x00, 0x7C, 0x00, 0x06, 0xAC, 0x91, 0x0A, 0x00, 0x00, 
    0x7C, 0x00, 0x06, 0xAC, 0x38, 0x00, 0x00, 0x19, 0x90, 0x0B, 0x00, 0x00, 0x7C, 0x00, 0x06, 0xAC, 
    0x80, 0x0B, 0x00, 0x00, 0x7C, 0x00, 0x04, 0xAC, 0x70, 0x09, 0x00, 0x01, 0x40, 0x82, 0xFF, 0xF4, 
    0x80, 0x0A, 0x00, 0x00, 0x7C, 0x00, 0x04, 0xAC, 0x39, 0x20, 0x00, 0x00, 0x91, 0x27, 0x00, 0x00, 
    0x7C, 0x00, 0x06, 0xAC, 0x74, 0x09, 0x04, 0x00, 0x41, 0x82, 0xFF, 0xB8, 0x38, 0xC6, 0x00, 0x01, 
    0x7F, 0x86, 0x20, 0x00, 0x40, 0x9E, 0xFF, 0xA0, 0x7C, 0xA3, 0x2B, 0x78, 0x4E, 0x80, 0x00, 0x20, 
};

unsigned char sig_setting[40] =
{
	0x2f, 0x74, 0x69, 0x74,
	0x6c, 0x65, 0x2f, 0x30,
	0x30, 0x30, 0x30, 0x30,
	0x30, 0x30, 0x31, 0x2f,
	0x30, 0x30, 0x30, 0x30,
	0x30, 0x30, 0x30, 0x32,
	0x2f, 0x64, 0x61, 0x74,
	0x61, 0x2f, 0x73, 0x65,
	0x74, 0x74, 0x69, 0x6e,
	0x67, 0x2e, 0x74, 0x78,
};

unsigned char sig_SYSCONF[20] =
{
	0x2f, 0x73, 0x68, 0x61,
	0x72, 0x65, 0x64, 0x32,
	0x2f, 0x73, 0x79, 0x73,
	0x2f, 0x53, 0x59, 0x53,
	0x43, 0x4f, 0x4e, 0x46,
};

//unsigned char patch_SYSCONF[20] =
//{
//	0x2f, 0x74, 0x6d, 0x70,
//	0x2f, 0x73, 0x79, 0x73,
//	0x2f, 0x53, 0x59, 0x53,
//	0x43, 0x4f, 0x4e, 0x46,
//	0x00, 0x00, 0x00, 0x00,
//};

unsigned char patch_SYSCONF[20] =
{
	0x2f, 0x73, 0x79, 0x73,
	0x2f, 0x77, 0x69, 0x69,
	0x66, 0x6c, 0x6f, 0x77,
	0x2e, 0x72, 0x65, 0x67,
	0x00, 0x00, 0x00, 0x00,
};

unsigned char patch_setting[44] = 
{
	0x2f, 0x74, 0x6d, 0x70,
	0x2f, 0x73, 0x79, 0x73,
	0x2f, 0x73, 0x65, 0x74,
	0x74, 0x69, 0x6e, 0x67,
	0x2e, 0x74, 0x78, 0x74,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
};

bool dogamehooks(void *addr, u32 len, bool channel)
{
	/*
	0 No Hook
	1 VBI
	2 KPAD read
	3 Joypad Hook
	4 GXDraw Hook
	5 GXFlush Hook
	6 OSSleepThread Hook
	7 AXNextFrame Hook
	*/

	void *addr_start = addr;
	void *addr_end = addr+len;
	bool hookpatched = false;

	while(addr_start < addr_end)
	{
		switch(hooktype)
		{
			case 0x00:
				hookpatched = true;
				break;

			case 0x01:
				if(memcmp(addr_start, viwiihooks, sizeof(viwiihooks))==0)
				{
					patchhook((u32)addr_start, len);
					hookpatched = true;
				}
				break;

			case 0x02:
				if(memcmp(addr_start, kpadhooks, sizeof(kpadhooks))==0)
				{
					patchhook((u32)addr_start, len);
					hookpatched = true;
				}

				if(memcmp(addr_start, kpadoldhooks, sizeof(kpadoldhooks))==0)
				{
					patchhook((u32)addr_start, len);
					hookpatched = true;
				}
				break;

			case 0x03:
				if(memcmp(addr_start, joypadhooks, sizeof(joypadhooks))==0)
				{
					patchhook((u32)addr_start, len);
					hookpatched = true;
				}
				break;

			case 0x04:
				if(memcmp(addr_start, gxdrawhooks, sizeof(gxdrawhooks))==0)
				{
					patchhook((u32)addr_start, len);
					hookpatched = true;
				}
				break;

			case 0x05:
				if(memcmp(addr_start, gxflushhooks, sizeof(gxflushhooks))==0)
				{
					patchhook((u32)addr_start, len);
					hookpatched = true;
				}
				break;

			case 0x06:
				if(memcmp(addr_start, ossleepthreadhooks, sizeof(ossleepthreadhooks))==0)
				{
					patchhook((u32)addr_start, len);
					hookpatched = true;
				}
				break;

			case 0x07:
				if(memcmp(addr_start, axnextframehooks, sizeof(axnextframehooks))==0)
				{
					patchhook((u32)addr_start, len);
					hookpatched = true;
				}
				break;

			case 0x08:
				/* if(memcmp(addr_start, customhook, customhooksize)==0)
				{
					patchhook((u32)addr_start, len);
					hookpatched = true;
				} */
				break;
		}
		if (hooktype != 0)
		{
			if(channel && memcmp(addr_start, multidolchanhooks, sizeof(multidolchanhooks))==0)
			{
				*(((u32*)addr_start)+1) = 0x7FE802A6;
				DCFlushRange(((u32*)addr_start)+1, 4);

				multidolhook((u32)addr_start+sizeof(multidolchanhooks)-4);
				hookpatched = true;
			}
			else if(!channel && memcmp(addr_start, multidolhooks, sizeof(multidolhooks))==0)
			{
				multidolhook((u32)addr_start+sizeof(multidolhooks)-4);
				hookpatched = true;
			}			
		}
		addr_start += 4;
	}
	return hookpatched;
}

void langpatcher(void *addr, u32 len)
{
	void *addr_start = addr;
	void *addr_end = addr+len;

	while(addr_start < addr_end)
	{
		if(memcmp(addr_start, langpatch, sizeof(langpatch))==0)
			if(configbytes[0] != 0xCD)
				langvipatch((u32)addr_start, len, configbytes[0]);
		addr_start += 4;
	}
}

void vidolpatcher(void *addr, u32 len)
{
	void *addr_start = addr;
	void *addr_end = addr+len;

	while(addr_start < addr_end)
	{
		if(memcmp(addr_start, vipatchcode, sizeof(vipatchcode))==0)
			vipatch((u32)addr_start, len);
		addr_start += 4;
	}
}

void PatchVideoSneek(void *addr, u32 len)
{
	u8 *addr_start = addr;
	u8 *addr_end = addr+len;

	while(addr_start < addr_end)
	{
		if(*(vu32*)(addr_start) == 0x3C608000)
		{
			if(((*(vu32*)(addr_start+4) & 0xFC1FFFFF ) == 0x800300CC) && ((*(vu32*)(addr_start+8) >> 24) == 0x54))
				*(vu32*)(addr_start+4) = 0x5400F0BE | ((*(vu32*)(addr_start+4) & 0x3E00000) >> 5);
		}
		addr_start += 4;
	}
}

//giantpune's magic super patch to return to channels

static u32 ad[ 4 ] = { 0, 0, 0, 0 };//these variables are global on the off chance the different parts needed
static u8 found = 0;			//to find in the dol are found in different sections of the dol
static u8 returnToPatched = 0;

bool PatchReturnTo( void *Address, int Size, u32 id )
{
	if( !id || returnToPatched )
		return 0;
	//gprintf("PatchReturnTo( %p, %08x, %08x )\n", Address, Size, id );

	//new __OSLoadMenu() (SM2.0 and higher)
	u8 SearchPattern[ 12 ] = 	{ 0x38, 0x80, 0x00, 0x02, 0x38, 0x60, 0x00, 0x01, 0x38, 0xa0, 0x00, 0x00 }; //li r4,2
	//li r3,1
	//li r5,0
	//old _OSLoadMenu() (used in launch games)
	u8 SearchPatternB[ 12 ] = 	{ 0x38, 0xC0, 0x00, 0x02, 0x38, 0xA0, 0x00, 0x01, 0x38, 0xE0, 0x00, 0x00 }; //li r6,2
	//li r5,1
	//li r7,0
	//identifier for the safe place
	u8 SearchPattern2[ 12 ] = 	{ 0x4D, 0x65, 0x74, 0x72, 0x6F, 0x77, 0x65, 0x72, 0x6B, 0x73, 0x20, 0x54 }; //"Metrowerks T"

	u8 oldSDK = 0;
	found = 0;

	void *Addr = Address;
	void *Addr_end = Address+Size;

	while (Addr <= Addr_end - 12 )
	{
		//find a safe place or the patch to hang out
		if ( ! ad[ 3 ] && memcmp( Addr, SearchPattern2, 12 ) == 0 )
		{
			ad[ 3 ] = (u32)Addr + 0x30;
		}
		//find __OSLaunchMenu() and remember some addresses in it
		else if ( memcmp( Addr, SearchPattern, 12 )==0 )
		{
			ad[ found++ ] = (u32)Addr;
		}
		else if ( ad[ 0 ] && memcmp( Addr, SearchPattern, 8 )==0 ) //after the first match is found, only search the first 8 bytes for the other 2
		{
			if( !ad[ 1 ] ) ad[ found++ ] = (u32)Addr;
			else if( !ad[ 2 ] ) ad[ found++ ] = (u32)Addr;
			if( found >= 3 )break;
		}
		Addr += 4;
	}
	//check for the older-ass version of the SDK
	if( found < 3 && ad[ 3 ] )
	{
		Addr = Address;
		ad[ 0 ] = 0;
		ad[ 1 ] = 0;
		ad[ 2 ] = 0;
		found = 0;
		oldSDK = 1;

		while (Addr <= Addr_end - 12 )
		{
			//find __OSLaunchMenu() and remember some addresses in it
			if ( memcmp( Addr, SearchPatternB, 12 )==0 )
			{
				ad[ found++ ] = (u32)Addr;
			}
			else if ( ad[ 0 ] && memcmp( Addr, SearchPatternB, 8 ) == 0 ) //after the first match is found, only search the first 8 bytes for the other 2
			{
				if( !ad[ 1 ] ) ad[ found++ ] = (u32)Addr;
				else if( !ad[ 2 ] ) ad[ found++ ] = (u32)Addr;
				if( found >= 3 )break;
			}
			Addr += 4;
		}
	}

	//if the function is found
	if( found == 3 && ad[ 3 ] )
	{
		gprintf("patch __OSLaunchMenu( 0x00010001, 0x%08x )\n", id);
		u32 nop = 0x60000000;

		//the magic that writes the TID to the registers
		u8 jump[ 20 ] = { 0x3C, 0x60, 0x00, 0x01,				//lis r3,1
						  0x60, 0x63, 0x00, 0x01,				//ori r3,r3,1
						  0x3C, 0x80, (u8)( id >> 24 ), (u8)( id >> 16 ),	//lis r4,(u16)(tid >> 16)
						  0x60, 0x84, (u8)( id >> 8 ), (u8)id,			//ori r4,r4,(u16)(tid)
						  0x4E, 0x80, 0x00, 0x20
						};				//blr

		if( oldSDK )
		{
			jump[ 1 ] = 0xA0; //3CA00001					//lis r5,1
			jump[ 5 ] = 0xA5; //60A50001					//ori r5,r5,1
			jump[ 9 ] = 0xC0; //3CC0AF1B					//lis r6,(u16)(tid >> 16)
			jump[ 13 ] = 0xC6;//60C6F516					//ori r6,r6,(u16)(tid)
		}

		void* addr = (u32*)ad[ 3 ];

		//write new stuff to in a unused part of the main.dol
		memcpy( addr, jump, sizeof( jump ) );

		//ES_GetTicketViews()
		u32 newval = ( ad[ 3 ] - ad[ 0 ] );
		newval &= 0x03FFFFFC;
		newval |= 0x48000001;
		addr = (u32*)ad[ 0 ];
		memcpy( addr, &newval, sizeof( u32 ) );					//bl ad[ 3 ]
		memcpy( addr + 4, &nop, sizeof( u32 ) );				//nop
		//gprintf("\t%08x -> %08x\n", addr, newval );

		//ES_GetTicketViews() again
		newval = ( ad[ 3 ] - ad[ 1 ] );
		newval &= 0x03FFFFFC;
		newval |= 0x48000001;
		addr = (u32*)ad[ 1 ];
		memcpy( addr, &newval, sizeof( u32 ) );					//bl ad[ 3 ]
		memcpy( addr + 4, &nop, sizeof( u32 ) );				//nop
		//gprintf("\t%08x -> %08x\n", addr, newval );

		//ES_LaunchTitle()
		newval = ( ad[ 3 ] - ad[ 2 ] );
		newval &= 0x03FFFFFC;
		newval |= 0x48000001;
		addr = (u32*)ad[ 2 ];
		memcpy( addr, &newval, sizeof( u32 ) );					//bl ad[ 3 ]
		memcpy( addr + 4, &nop, sizeof( u32 ) );				//nop
		//gprintf("\t%08x -> %08x\n", addr, newval );

		returnToPatched = 1;
	}

	return returnToPatched;
}

void PatchAspectRatio(void *addr, u32 len, u8 aspect)
{
	if(aspect > 1)
		return;

	static const u32 aspect_searchpattern1[5] = {
		0x9421FFF0, 0x7C0802A6, 0x38800001, 0x90010014, 0x38610008
	};

	static const u32 aspect_searchpattern2[15] = {
		0x2C030000, 0x40820010, 0x38000000, 0x98010008, 0x48000018,
		0x88010008, 0x28000001, 0x4182000C, 0x38000000, 0x98010008,
		0x80010014, 0x88610008, 0x7C0803A6, 0x38210010, 0x4E800020
	};

	u8 *addr_start = (u8 *) addr;
	u8 *addr_end = addr_start + len - sizeof(aspect_searchpattern1) - 4 - sizeof(aspect_searchpattern2);

	while(addr_start < addr_end)
	{
		if(   (memcmp(addr_start, aspect_searchpattern1, sizeof(aspect_searchpattern1)) == 0)
		   && (memcmp(addr_start + 4 + sizeof(aspect_searchpattern1), aspect_searchpattern2, sizeof(aspect_searchpattern2)) == 0))
		{
			*((u32 *)(addr_start+0x44)) = (0x38600000 | aspect);
			break;
		}
		addr_start += 4;
	}
}

void Patch_fwrite(void *Address, int Size)
{
	u8 *addr_start = (u8*)Address;
	u8 *addr_end = (u8*)(Address + Size) - sizeof(patch_fwrite);

	while(addr_start < addr_end)
	{
		if(memcmp(addr_start, sig_fwrite, sizeof(sig_fwrite)) == 0)
			memcpy(addr_start, patch_fwrite, sizeof(patch_fwrite));
		addr_start += 4;
	}
}

void PatchCountryStrings(void *Address, int Size)
{
	u8 SearchPattern[4] = {0x00, 0x00, 0x00, 0x00};
	u8 PatchData[4] = {0x00, 0x00, 0x00, 0x00};
	u8 *Addr = (u8*)Address;
	int wiiregion = CONF_GetRegion();

	switch(wiiregion)
	{
		case CONF_REGION_JP:
			SearchPattern[0] = 0x00;
			SearchPattern[1] = 'J';
			SearchPattern[2] = 'P';
			break;
		case CONF_REGION_EU:
			SearchPattern[0] = 0x02;
			SearchPattern[1] = 'E';
			SearchPattern[2] = 'U';
			break;
		case CONF_REGION_KR:
			SearchPattern[0] = 0x04;
			SearchPattern[1] = 'K';
			SearchPattern[2] = 'R';
			break;
		case CONF_REGION_CN:
			SearchPattern[0] = 0x05;
			SearchPattern[1] = 'C';
			SearchPattern[2] = 'N';
			break;
		case CONF_REGION_US:
		default:
			SearchPattern[0] = 0x01;
			SearchPattern[1] = 'U';
			SearchPattern[2] = 'S';
	}

	const char DiscRegion = ((u8*)Disc_ID)[3];
	switch(DiscRegion)
	{
		case 'J':
			PatchData[1] = 'J';
			PatchData[2] = 'P';
			break;
		case 'D':
		case 'F':
		case 'P':
		case 'X':
		case 'Y':
			PatchData[1] = 'E';
			PatchData[2] = 'U';
			break;

		case 'E':
		default:
			PatchData[1] = 'U';
			PatchData[2] = 'S';
	}
	while (Size >= 4)
	{
		if(Addr[0] == SearchPattern[0] && Addr[1] == SearchPattern[1] && Addr[2] == SearchPattern[2] && Addr[3] == SearchPattern[3])
		{
			//*Addr = PatchData[0];
			Addr += 1;
			*Addr = PatchData[1];
			Addr += 1;
			*Addr = PatchData[2];
			Addr += 1;
			//*Addr = PatchData[3];
			Addr += 1;
			Size -= 4;
		}
		else
		{
			Addr += 4;
			Size -= 4;
		}
	}
}

s32 BlockIOSReload(void)
{
	/* Open ES Module */
	s32 ESHandle = IOS_Open("/dev/es", 0);
	/* IOS Reload Block */
	static ioctlv block_vector[2] ATTRIBUTE_ALIGN(32);
	static u32 mode ATTRIBUTE_ALIGN(32);
	static u32 ios ATTRIBUTE_ALIGN(32);
	mode = 2;
	block_vector[0].data = &mode;
	block_vector[0].len  = sizeof(u32);
	ios = IOS_GetVersion();
	block_vector[1].data = &ios;
	block_vector[1].len  = sizeof(u32);
	s32 ret = IOS_Ioctlv(ESHandle, 0xA0, 2, 0, block_vector);
	/* Close ES Module */
	IOS_Close(ESHandle);
	return ret;
}

void PatchRegion(void *Address, int Size)
{
	u8 *addr_start = (u8*)Address;
	u8 *addr_end = (u8*)(Address + Size);

	while(addr_start < addr_end)
	{
		if(!memcmp(addr_start, sig_setting, sizeof(sig_setting)))
		{
			gprintf("Patching setting region\n");
			memcpy(addr_start, patch_setting, sizeof(patch_setting));
		}
		if(!memcmp(addr_start, sig_SYSCONF, sizeof(sig_SYSCONF)))
		{
			gprintf("Patching SYSCONF region\n");
			memcpy(addr_start, patch_SYSCONF, sizeof(patch_SYSCONF));
		}
		addr_start += 4;
	}
}

/** Patch URLs for private Servers - Thanks to ULGX **/
void PrivateServerPatcher(void *addr, u32 len)
{
	// Patch protocol https -> http
	char *cur = (char *)addr;
	const char *end = cur + len - 8;
	do
	{
		if (memcmp(cur, "https://", 8) == 0 && cur[8] != 0)
		{
			int len = strlen(cur);
			memmove(cur + 4, cur + 5, len - 5);
			cur[len - 1] = 0;
			cur += len;
		}
	}
	while (++cur < end);
	domainpatcher(addr, len, "wiimmfi.de");
}

void domainpatcher(void *addr, u32 len, const char* domain)
{
	if(strlen("nintendowifi.net") < strlen(domain))
		return;

	char *cur = (char *)addr;
	const char *end = cur + len - 16;
	
	do
	{
		if (memcmp(cur, "nintendowifi.net", 16) == 0)
		{
			int len = strlen(cur);
			u8 i;
			memcpy(cur, domain, strlen(domain));
			memmove(cur + strlen(domain), cur + 16, len - 16);
			for(i = 16 - strlen(domain); i > 0 ; i--)
				cur[len - i ] = 0;
			cur += len;
		}
	}
	while (++cur < end);
}

/** 480p Pixel Fix Patch by leseratte
    fix for a Nintendo Revolution SDK bug found by Extrems affecting early Wii console when using 480p video mode.
	https://shmups.system11.org/viewtopic.php?p=1361158#p1361158
	https://github.com/ExtremsCorner/libogc-rice/commit/941d687e271fada68c359bbed98bed1fbb454448
	**/
void PatchFix480p() 
{
    u8 prefix[2] = { 0x4b, 0xff }; 

    ///              Patch offset: ----------VVVVVVVV
    u32 Pattern_MKW[8] =     { 0x38000065, 0x9b810019, 0x38810018, 0x386000e0, 0x98010018, 0x38a00002};
    u32 patches_MKW[2] = { 0x38600003, 0x98610019 }; 
    /// Used by: MKWii, Wii Play, Need for Speed Nitro, Wii Sports, ...

    ///              Patch offset: ----------------------------------------------VVVVVVVV
    u32 Pattern_NSMB[8] =   {  0x38000065, 0x9801001c, 0x3881001c, 0x386000e0, 0x9b81001d, 0x38a00002}; 
    u32 patches_NSMB[2] = { 0x38a00003, 0x98a1001d }; 
    /// Used by: New Super Mario Bros, ...
   
    /* 
     * Code block that is being patched (in MKW): 
     * 
     * 4bffe30d: bl WaitMicroTime 
     * 38000065: li r0, 0x65
     * 9b810019: stb r28, 25(r1)        // store the wrong value (1)
     * 38810018: addi r4, r1, 0x18
     * 386000e0: li r3, 0xe0
     * 98010018: stb r0, 24(r1)
     * 38a00002: li r5, 2
     * 4bffe73d: bl __VISendI2CData
     * 
     * r28 is a register that is set to 1 at the beginning of the function. 
     * However, its contents are used elsewhere as well, so we can't just modify this one function. 
     * 
     * The following code first searches for one of the patterns above, then replaces the 
     * "stb r28, 25(r1)" instruction that stores the wrong value on the stack with a branch instead
     * That branch branches to the injected custom code ("li r3, 3; stb r3, 25(r1)") that stores the
     * correct value (3) instead. At the end of the injected code will be another branch that branches
     * back to the instruction after the one that has been replaced (so, to "addi r4, r1, 0x18"). 
     * r3 can safely be used as a temporary register because its contents will be replaced immediately
     * afterwards anyways. 
     * 
     */
   
    void * offset = NULL; 
    void * addr = (void*)0x80000000; 
    u32 len = 0x900000; 

    void * patch_ptr = 0 ; 
    void * a = addr; 

    while ((char*)a < ((char*)addr + len)) {
        if (memcmp(a, &Pattern_MKW, 6 * 4) == 0) {
            // Found pattern?
            if (memcmp(a - 4, &prefix, 2) == 0) {
                if (memcmp(a + 8*4, &prefix, 2) == 0) {
                    offset = a + 4;
                    //hexdump (a, 30); 
                    patch_ptr = &patches_MKW; 
                    break; 
                }
            }
        }
        else if (memcmp(a, &Pattern_NSMB, 6 * 4) == 0) {
            // Found pattern?
            if (memcmp(a - 4, &prefix, 2) == 0) {
                if (memcmp(a + 8*4, &prefix, 2) == 0) {
                    offset = a + 16;
                    //hexdump (a, 30); 
                    patch_ptr = &patches_NSMB; 
                    break; 
                }
            }
        }
        a+= 4; 
    }

   
   
    if (offset == 0) {
        // offset is still 0, we didn't find the pattern, return
        gprintf("Didn't find offset for 480p patch!\n"); 
        return;
    }
   
    // If we are here, we found the offset. Lets grab some space
    // from the heap for our patch
    u32 old_heap_ptr = *(u32*)0x80003110;
    *((u32*)0x80003110) = (old_heap_ptr - 0x20);
    u32 heap_space = old_heap_ptr-0x20;

    gprintf("Found offset for 480p patch - create branch from 0x%x to heap (0x%x)\n", offset, heap_space); 
                    //hexdump (offset, 30); 

    memcpy((void*)heap_space, patch_ptr, 8);
   
    *((u32*)offset) = 0x48000000 + (((u32)(heap_space) - ((u32)(offset))) & 0x3ffffff);
    *((u32*)((u32)heap_space + 8)) = 0x48000000 + (((u32)((u32)offset + 4) - ((u32)(heap_space + 8))) & 0x3ffffff);
    return;
}

u32 do_new_wiimmfi() {

	// As of November 2018, Wiimmfi requires a special Wiimmfi patcher 
	// update which does a bit more than just patch the server adresses. 
	// This function is being called by apploader.c, right before 
	// jumping to the entry point (only for Mario Kart Wii & Wiimmfi), 
	// and applies all the necessary new patches to the game. 
	// This includes support for the new patcher update plus
	// support for StaticR.rel patching. 
	
	// This function has been implemented by Leseratte. Please don't
	// try to modify it without speaking to the Wiimmfi team because
	// doing so could have unintended side effects. 
	
	// check region: 
	char region = *((char *)(0x80000003)); 
	char * patched; 
	void * patch1_offset, *patch2_offset, *patch3_offset; 
	
	// define some offsets and variables depending on the region:
	switch (region) {
		case 'P': 
			patched = (char*)0x80276054; 
			patch1_offset = (void*)0x800ee3a0;
			patch2_offset = (void*)0x801d4efc; 
			patch3_offset = (void*)0x801A72E0; 
			break; 
		case 'E':
			patched = (char*)0x80271d14; 
			patch1_offset = (void*)0x800ee300;
			patch2_offset = (void*)0x801d4e5c; 
			patch3_offset = (void*)0x801A7240; 
			break; 
		case 'J': 
			patched = (char*)0x802759f4;
			patch1_offset = (void*)0x800ee2c0;
			patch2_offset = (void*)0x801d4e1c; 
			patch3_offset = (void*)0x801A7200; 
			break; 
		case 'K': 
			patched = (char*)0x80263E34;
			patch1_offset = (void*)0x800ee418; 
			patch2_offset = (void*)0x801d5258;
			patch3_offset = (void*)0x801A763c;
			break;
		default: 
			return -1; 
	}
	
	if (*patched != '*') return -2; 	// ISO already patched
	
	// This RAM address is set (no asterisk) by all officially
	// updated patchers, so if it is modified, the image is already 
	// patched with a new patcher and we don't need to patch anything.
	
	// For statistics and easier debugging in case of problems, Wiimmfi
	// wants to know what patcher a game has been patched with, thus, 
	// let the game know the exact USB-Loader version. Max length 42 
	// chars, padded with whitespace, without null terminator
	char * fmt = "%s v%-50s";
	char patcher[100] = {0}; 
	snprintf((char *)&patcher, 99, fmt, APP_NAME, APP_VERSION); 	
	strncpy(patched, (char *)&patcher, 42); 
	
	// Do the plain old patching with the string search
	PrivateServerPatcher((void*)0x80004000, 0x385200); 
	
	// Replace some URLs for Wiimmfi's new update system
	char newURL1[] = "http://ca.nas.wiimmfi.de/ca";
	char newURL2[] = "http://naswii.wiimmfi.de/ac";
	char newURL3P[] = "https://main.nas.wiimmfi.de/pp";
	char newURL3E[] = "https://main.nas.wiimmfi.de/pe";
	char newURL3J[] = "https://main.nas.wiimmfi.de/pj";
	char newURL3K[] = "https://main.nas.wiimmfi.de/pk";
	
	
	// Write the URLs to the proper place and do some other patching.
	switch (region) {
		case 'P': 
			memcpy((void*)0x8027A400, newURL1, sizeof(newURL1));
			memcpy((void*)0x8027A400 + 0x28, newURL2, sizeof(newURL2));
			memcpy((void*)0x8027A400 + 0x4C, newURL3P, sizeof(newURL3P));
			*(u32*)0x802a146c = 0x733a2f2f; 
			*(u32*)0x800ecaac = 0x3bc00000; 
			break; 
		case 'E': 
			memcpy((void*)0x802760C0, newURL1, sizeof(newURL1));
			memcpy((void*)0x802760C0 + 0x28, newURL2, sizeof(newURL2));
			memcpy((void*)0x802760C0 + 0x4C, newURL3E, sizeof(newURL3E));
			*(u32*)0x8029D12C = 0x733a2f2f; 
			*(u32*)0x800ECA0C = 0x3bc00000; 
			break; 
		case 'J':
			memcpy((void*)0x80279DA0, newURL1, sizeof(newURL1));
			memcpy((void*)0x80279DA0 + 0x28, newURL2, sizeof(newURL2));
			memcpy((void*)0x80279DA0 + 0x4C, newURL3J, sizeof(newURL3J));
			*(u32*)0x802A0E0C = 0x733a2f2f; 
			*(u32*)0x800EC9CC = 0x3bc00000; 
			break; 
		case 'K':
			memcpy((void*)0x802682B0, newURL1, sizeof(newURL1));
			memcpy((void*)0x802682B0 + 0x28, newURL2, sizeof(newURL2));
			memcpy((void*)0x802682B0 + 0x4C, newURL3K, sizeof(newURL3K));
			*(u32*)0x8028F474 = 0x733a2f2f; 
			*(u32*)0x800ECB24 = 0x3bc00000; 
			break; 
	}

	// Make some space on heap (0x400) for our custom code. 
	u32 old_heap_ptr = *(u32*)0x80003110; 
	*((u32*)0x80003110) = (old_heap_ptr - 0x400); 
	u32 heap_space = old_heap_ptr-0x400; 
	memset((void*)old_heap_ptr-0x400, 0xed, 0x400); 
	
	// Binary blobs with Wiimmfi patches. Do not modify. 
	// Provided by Leseratte on 2018-12-14.		
	
	int binary[] = { 0x37C849A2, 0x8BC32FA4, 0xC9A34B71, 0x1BCB49A2, 
					 0x2F119304, 0x5F402684, 0x3E4FDA29, 0x50849A21, 
					 0xB88B3452, 0x627FC9C1, 0xDC24D119, 0x5844350F, 
					 0xD893444F, 0x19A588DC, 0x16C91184, 0x0C3E237C, 
					 0x75906CED, 0x6E68A55E, 0x58791842, 0x072237E9, 
					 0xAB24906F, 0x0A8BDF21, 0x4D11BE42, 0x1AAEDDC8, 
					 0x1C42F908, 0x280CF2B2, 0x453A1BA4, 0x9A56C869, 
					 0x786F108E, 0xE8DF05D2, 0x6DB641EB, 0x6DFC84BB, 
					 0x7E980914, 0x0D7FB324, 0x23442185, 0xA7744966, 
					 0x53901359, 0xBF2103CC, 0xC24A4EB7, 0x32049A02, 
					 0xC1683466, 0xCA93689D, 0xD8245106, 0xA84987CF, 
					 0xEC9B47C9, 0x6FA688FE, 0x0A4D11A6, 0x8B653C7B, 
					 0x09D27E30, 0x5B936208, 0x5DD336DE, 0xCD092487, 
					 0xEF2C6D36, 0x1E09DF2D, 0x75B1BE47, 0xE68A7F22, 
					 0xB0E5F90D, 0xEC49F216, 0xAD1DCC24, 0xE2B5C841, 
					 0x066F6F63, 0xF4D90926, 0x299F42CD, 0xA3F125D6, 
					 0x077B093C, 0xB5721268, 0x1BE424D1, 0xEBC30BF0, 
					 0x77867BED, 0x4F0C9BCA, 0x3E195930, 0xDC32DE2C, 
					 0x1865D189, 0x70C67E7A, 0x71FA7329, 0x532233D3, 
					 0x06D2E87B, 0x6CBEBA7F, 0x99F08532, 0x52FA601C, 
					 0x05F4B82C, 0x4B64839C, 0xB5C65009, 0x1B8396E3, 
					 0x0A8B2DAF, 0x0DB85BE6, 0x12F1B71D, 0x186F6E4D, 
					 0x2870DC2E, 0x5960B8E6, 0x8F4D71BD, 0x0614E3C3, 
					 0x05E8C725, 0x365D8E3D, 0x74351CDE, 0xE1AB3930, 
					 0xFEDA721B, 0xE53AE4E9, 0xC3B4C9A6, 0xBAE59346, 
					 0x6D45269D, 0x634E4D1A, 0x2FD99A30, 0x26393449, 
					 0xE49768D1, 0x81E1D1A1, 0xFCE1A34A, 0x7EB44697, 
					 0xEB2F8D2D, 0xCECFE5AF, 0x81BD34B6, 0xB1F1696E, 
					 0x5E6ED2B2, 0xA473A4A0, 0x41664B70, 0xBF40968A, 
					 0x662F2CCB, 0xC5DF5B8C, 0xB632B772, 0x74EB6F39, 
					 0xE017DC71, 0xFDA3B890, 0xE3C9713D, 0xCE53E397, 
					 0xA12BC743, 0x5AD98EA5, 0xBC721C9F, 0x4568395A, 
					 0x925E72B4, 0x2D7DE4D7, 0x6777C9C7, 0xD6619396, 
					 0xA502268A, 0x77884D75, 0xF79E9AF0, 0xE6FC3461, 
					 0xF07468A5, 0xF866D11D, 0xF90CA342, 0xCF9546FF, 
					 0x87A48D81, 0x06881A51, 0x309C34D1, 0x79B669CE, 
					 0xFAADD2D7, 0xC8D7A5D1, 0x89214BE5, 0x1B8396EF, 
					 0x0A8B2DE9, 0x0D985B06, 0x12F1B711, 0x186F6E57, 
					 0x2850DC0E, 0x5960B8EA, 0x8F4D71AC, 0x0614E3E3, 
					 0x05E8C729, 0x365D8E39, 0x74351CFE, 0x518E3943, 
					 0x4A397268, 0x9D58E4B8, 0xD394C9A2, 0x0E069344, 
					 0xB522268B, 0x636E4D77, 0x2FF99A37, 0xF6DC346D, 
					 0xE49268B4, 0x2001D1A0, 0x4929A365, 0x7B764691, 
					 0xFFC68D49, 0x16A81A53, 0x247A34D2, 0xA1D16967, 
					 0x4B6DD2D5, 0xDDF4A5B7, 0x454A4B70, 0x0FAE96E2, 
					 0x0A8A2DC7, 0x0D98A47A, 0x06DCB71D, 0x0CCC6E38, 
					 0x55F25CFB, 0xB08C1E88, 0xDF4259C9, 0x0714E387, 
					 0xB00D47AF, 0x7B722975, 0x48BE349A, 0x29CC393C, 
					 0xEA797228, 0x98986471, 0x3778E1A3, 0xD7626D06, 
					 0x1567268D, 0x668ECD00, 0xD614F5C8, 0x133037CF, 
					 0x92F26CF2, 0x00000000, 0x00000000, 0x00000000, 
					 0x00000000, 0x00000000, 0x00000000, 0x00000000};	
	
	// Prepare patching process ....
	int i = 3; 
	int idx = 0; 
	for (; i < 202; i++) {
		if (i == 67 || i == 82) idx++; 
		binary[i] = binary[i] ^ binary[idx]; 
		binary[idx] = ((binary[idx] << 1) | ((binary[idx] >> (32 - 1)) & ~(-1 << 1))); 
	}
	

	// Binary blob needs some changes for regions other than PAL ...		 
	switch (region) {
		case 'E': 
			binary[29] = binary[67]; 	
			binary[37] = binary[68]; 	
			binary[43] = binary[69];	
			binary[185] = 0x61295C74; 	
			binary[189] = 0x61295D40;	
			binary[198] = 0x61086F5C; 	
			break; 
		case 'J': 
			binary[29] = binary[70]; 
			binary[37] = binary[71]; 
			binary[43] = binary[72];
			binary[185] = 0x612997CC; 	
			binary[189] = 0x61299898;	
			binary[198] = 0x61086F1C; 	
			break; 
		case 'K': 
			binary[6] = binary[73]; 	
			binary[9] = binary[74]; 
			binary[11] = binary[75]; 
			binary[23] = binary[76]; 
			binary[29] = binary[77]; 
			binary[33] = binary[78]; 
			binary[37] = binary[79]; 
			binary[43] = binary[80]; 
			binary[63] = binary[81]; 
			binary[184] = 0x3D208088;	
			binary[185] = 0x61298AA4; 	
			binary[188] = 0x3D208088;	
			binary[189] = 0x61298B58;	
			binary[198] = 0x61087358; 	
			break; 
	}

	// Installing all the patches.
	
	memcpy((void*)heap_space, (void*)binary, 820); 	
	u32 code_offset_1 = heap_space + 12; 	
	u32 code_offset_2 = heap_space + 88; 	
	u32 code_offset_3 = heap_space + 92;	
	u32 code_offset_4 = heap_space + 264; 	
	u32 code_offset_5 = heap_space + 328;	
	
	
	*((u32*)patch1_offset) = 0x48000000 + (((u32)(code_offset_1) - ((u32)(patch1_offset))) & 0x3ffffff); 
	*((u32*)code_offset_2) = 0x48000000 + (((u32)(patch1_offset + 4) - ((u32)(code_offset_2))) & 0x3ffffff); 
	*((u32*)patch2_offset) = 0x48000000 + (((u32)(code_offset_3) - ((u32)(patch2_offset))) & 0x3ffffff); 
	*((u32*)code_offset_4) = 0x48000000 + (((u32)(patch2_offset + 4) - ((u32)(code_offset_4))) & 0x3ffffff); 
	*((u32*)patch3_offset) = 0x48000000 + (((u32)(code_offset_5) - ((u32)(patch3_offset))) & 0x3ffffff); 
	
	// Patches successfully installed
	// returns 0 when all patching is done and game is ready to be booted. 
	return 0; 	
}
