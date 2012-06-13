#include "channel_launcher.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "disc.h"
#include "patchcode.h"
#include "videopatch.h"
#include "fst.h"
#include "lz77.h"
#include "utils.h"
#include "fs.h"
#include "gecko.h"
#include "mem2.hpp"

#define STACK_ALIGN(type, name, cnt, alignment)         \
					u8 _al__##name[((sizeof(type)*(cnt)) + (alignment) + \
					(((sizeof(type)*(cnt))%(alignment)) > 0 ? ((alignment) - \
					((sizeof(type)*(cnt))%(alignment))) : 0))]; \
					type *name = (type*)(((u32)(_al__##name)) + ((alignment) - (( \
					(u32)(_al__##name))&((alignment)-1))))

GXRModeObj * __Disc_SelectVMode(u8 videoselected, u64 chantitle);
void PatchCountryStrings(void *Address, int Size);
void __Disc_SetLowMem(void);
void __Disc_SetVMode(void);
void __Disc_SetTime(void);
void _unstub_start();
u32 entryPoint;

extern void __exception_closeall();

typedef void (*entrypoint) (void);

typedef struct _dolheader
{
	u32 section_pos[18];
	u32 section_start[18];
	u32 section_size[18];
	u32 bss_start;
	u32 bss_size;
	u32 entry_point;
	u32 padding[7];
} __attribute__((packed)) dolheader;

s32 BootChannel(u32 entry, u64 chantitle, u32 ios, u8 vidMode, bool vipatch, bool countryString, u8 patchVidMode, int aspectRatio)
{
	gprintf("Loading Channel...\n");
	entryPoint = entry;

	/* Select an appropriate video mode */
	GXRModeObj * vmode = __Disc_SelectVMode(vidMode, chantitle);

	/* Set time */
	__Disc_SetTime();
	
	/* Set low memory */
	__Disc_SetLowMem();

	if (hooktype != 0)
		ocarina_do_code();

	PatchChannel(vidMode, vmode, vipatch, countryString, patchVidMode, aspectRatio);

	entrypoint appJump = (entrypoint)entryPoint;

	/* Set an appropriate video mode */
	__Disc_SetVMode();

    // IOS Version Check
    *(vu32*)0x80003140 = ((ios << 16)) | 0xFFFF;
    *(vu32*)0x80003188 = ((ios << 16)) | 0xFFFF;
    DCFlushRange((void *)0x80003140, 4);
    DCFlushRange((void *)0x80003188, 4);

    // Game ID Online Check
    memset((void *)0x80000000, 0, 4);
    *(vu32 *)0x80000000 = TITLE_LOWER(chantitle);
	DCFlushRange((void *)0x80000000, 4);

	gprintf("Jumping to entrypoint %08x\n", entryPoint);

	/* Shutdown IOS subsystems */
	u32 level = IRQ_Disable();
	__IOS_ShutdownSubsystems();
	__exception_closeall();

	/* Originally from tueidj - taken from NeoGamma (thx) */
	*(vu32*)0xCC003024 = 1;

	if (entryPoint != 0x3400)
	{
		if(hooktype != 0)
		{
			asm volatile (
				"lis %r3, entryPoint@h\n"
				"ori %r3, %r3, entryPoint@l\n"
				"lwz %r3, 0(%r3)\n"
				"mtlr %r3\n"
				"lis %r3, 0x8000\n"
				"ori %r3, %r3, 0x18A8\n"
				"nop\n"
				"mtctr %r3\n"
				"bctr\n"
			);
		}
		else
			appJump();
	}
 	else if(hooktype != 0)
	{
		asm volatile (
			"lis %r3, returnpoint@h\n"
			"ori %r3, %r3, returnpoint@l\n"
			"mtlr %r3\n"
			"lis %r3, 0x8000\n"
			"ori %r3, %r3, 0x18A8\n"
			"nop\n"
			"mtctr %r3\n"
			"bctr\n"
			"returnpoint:\n"
			"bl DCDisable\n"
			"bl ICDisable\n"
			"li %r3, 0\n"
			"mtsrr1 %r3\n"
			"lis %r4, entryPoint@h\n"
			"ori %r4,%r4,entryPoint@l\n"
			"lwz %r4, 0(%r4)\n"
			"mtsrr0 %r4\n"
			"rfi\n"
		);
	}
	else
		_unstub_start();

	IRQ_Restore(level);

	return 0;
}

void *dolchunkoffset[18];
u32	dolchunksize[18];
u32	dolchunkcount;

u32 LoadChannel(u8 *buffer)
{
	dolchunkcount = 0;
	dolheader *dolfile = (dolheader *)buffer;

	if(dolfile->bss_start)
	{
		if(!(dolfile->bss_start & 0x80000000))
			dolfile->bss_start |= 0x80000000;

		memset((void *)dolfile->bss_start, 0, dolfile->bss_size);
		DCFlushRange((void *)dolfile->bss_start, dolfile->bss_size);
		ICInvalidateRange((void *)dolfile->bss_start, dolfile->bss_size);
	}

	int i;
	for(i = 0; i < 18; i++)
	{
		if(!dolfile->section_size[i]) 
			continue;
		if(dolfile->section_pos[i] < sizeof(dolheader)) 
			continue;
		if(!(dolfile->section_start[i] & 0x80000000)) 
			dolfile->section_start[i] |= 0x80000000;

		dolchunkoffset[dolchunkcount] = (void *)dolfile->section_start[i];
		dolchunksize[dolchunkcount] = dolfile->section_size[i];			

		gprintf("Moving section %u from offset %08x to %08x-%08x...\n", i, dolfile->section_pos[i], dolchunkoffset[dolchunkcount], dolchunkoffset[dolchunkcount]+dolchunksize[dolchunkcount]);
		memmove(dolchunkoffset[dolchunkcount], buffer + dolfile->section_pos[i], dolchunksize[dolchunkcount]);
		DCFlushRange(dolchunkoffset[dolchunkcount], dolchunksize[dolchunkcount]);
		ICInvalidateRange(dolchunkoffset[dolchunkcount], dolchunksize[dolchunkcount]);

		dolchunkcount++;
	}
	return dolfile->entry_point;
}

void PatchChannel(u8 vidMode, GXRModeObj *vmode, bool vipatch, bool countryString, u8 patchVidModes, int aspectRatio)
{
	bool hookpatched = false;

	u32 i;
	for (i=0; i < dolchunkcount; i++)
	{		
		patchVideoModes(dolchunkoffset[i], dolchunksize[i], vidMode, vmode, patchVidModes);
		if (vipatch) vidolpatcher(dolchunkoffset[i], dolchunksize[i]);
		if (configbytes[0] != 0xCD) langpatcher(dolchunkoffset[i], dolchunksize[i]);
		if (countryString) PatchCountryStrings(dolchunkoffset[i], dolchunksize[i]);
		if (aspectRatio != -1) PatchAspectRatio(dolchunkoffset[i], dolchunksize[i], aspectRatio);

		if (hooktype != 0)
			if (dogamehooks(dolchunkoffset[i], dolchunksize[i], true))
				hookpatched = true;
	}
	if (hooktype != 0 && !hookpatched)
	{
		gprintf("Error: Could not patch the hook\n");
		gprintf("Ocarina and debugger won't work\n");
	}
}

bool Identify_GenerateTik(signed_blob **outbuf, u32 *outlen)
{
	signed_blob *buffer = (signed_blob *)MEM2_alloc(STD_SIGNED_TIK_SIZE);
	if (!buffer) return false;
	memset(buffer, 0, STD_SIGNED_TIK_SIZE);

	sig_rsa2048 *signature = (sig_rsa2048 *)buffer;
	signature->type = ES_SIG_RSA2048;

	tik *tik_data  = (tik *)SIGNATURE_PAYLOAD(buffer);
	strcpy(tik_data->issuer, "Root-CA00000001-XS00000003");
	memset(tik_data->cidx_mask, 0xFF, 32);

	*outbuf = buffer;
	*outlen = STD_SIGNED_TIK_SIZE;

	return true;
}

bool Identify(u64 titleid, u32 *ios)
{
	char filepath[ISFS_MAXPATH] ATTRIBUTE_ALIGN(32);

	gprintf("Reading TMD...");
	sprintf(filepath, "/title/%08x/%08x/content/title.tmd", TITLE_UPPER(titleid), TITLE_LOWER(titleid));
	u32 tmdSize;
	u8 *tmdBuffer = ISFS_GetFile((u8 *) &filepath, &tmdSize, -1);
	if (tmdBuffer == NULL || tmdSize == 0)
	{
		gprintf("Failed!\n");
		return false;
	}
	gprintf("Success!\n");

	*ios = (u32)(tmdBuffer[0x18b]);

	u32 tikSize;
	signed_blob *tikBuffer = NULL;

	gprintf("Generating fake ticket...");
	if(!Identify_GenerateTik(&tikBuffer,&tikSize))
	{
		gprintf("Failed!\n");
		return false;
	}
	gprintf("Success!\n");

	gprintf("Reading certs...");
	sprintf(filepath, "/sys/cert.sys");
	u32 certSize;
	u8 *certBuffer = ISFS_GetFile((u8 *) &filepath, &certSize, -1);
	if (certBuffer == NULL || certSize == 0)
	{
		gprintf("Failed!\n");
		MEM2_free(tmdBuffer);
		MEM2_free(tikBuffer);
		return false;
	}
	gprintf("Success!\n");

	gprintf("ES_Identify\n");
	s32 ret = ES_Identify((signed_blob*)certBuffer, certSize, (signed_blob*)tmdBuffer, tmdSize, tikBuffer, tikSize, NULL);
	if (ret < 0)
	{
		switch(ret)
		{
			case ES_EINVAL:
				gprintf("Error! ES_Identify (ret = %d;) Data invalid!\n", ret);
				break;
			case ES_EALIGN:
				gprintf("Error! ES_Identify (ret = %d;) Data not aligned!\n", ret);
				break;
			case ES_ENOTINIT:
				gprintf("Error! ES_Identify (ret = %d;) ES not initialized!\n", ret);
				break;
			case ES_ENOMEM:
				gprintf("Error! ES_Identify (ret = %d;) No memory!\n", ret);
				break;
			default:
				gprintf("Error! ES_Identify (ret = %d)\n", ret);
				break;
		}
	}
	
	MEM2_free(tmdBuffer);
	MEM2_free(tikBuffer);
	MEM2_free(certBuffer);

	return ret < 0 ? false : true;
}

u8 *GetDol(u64 title, u32 bootcontent)
{
	char filepath[ISFS_MAXPATH] ATTRIBUTE_ALIGN(32);
	sprintf(filepath, "/title/%08x/%08x/content/%08x.app", TITLE_UPPER(title), TITLE_LOWER(title), bootcontent);
	
	gprintf("Loading DOL: %s...", filepath);
	u32 contentSize = 0;
	u8 *data = ISFS_GetFile((u8 *) &filepath, &contentSize, -1);
	if (data != NULL && contentSize != 0)
	{	
		gprintf("Done!\n");
	
		if (isLZ77compressed(data))
		{
			u8 *decompressed;
			u32 size = 0;
			if (decompressLZ77content(data, contentSize, &decompressed, &size) < 0)
			{
				gprintf("Decompression failed\n");
				MEM2_free(data);
				return NULL;
			}
			MEM2_free(data);
			data = decompressed;
		}	
		return data;
	}
	gprintf("Failed!\n");
	return NULL;
}

s32 WiiFlow_LaunchTitle(u64 titleID)
{
	u32 numviews;
	STACK_ALIGN(tikview,views,4,32);

	ES_GetNumTicketViews(titleID, &numviews);
	ES_GetTicketViews(titleID, views, numviews);
	return ES_LaunchTitle(titleID, &views[0]);
}
