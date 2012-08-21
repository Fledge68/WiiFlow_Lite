
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>

#include "channel_launcher.h"
#include "gecko/gecko.h"
#include "loader/disc.h"
#include "loader/external_booter.hpp"
#include "loader/fs.h"
#include "loader/fst.h"
#include "loader/utils.h"
#include "memory/memory.h"
#include "unzip/lz77.h"
#include "types.h"

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

void *dolchunkoffset[18];
u32	dolchunksize[18];
u32	dolchunkcount;

s32 BootChannel(u32 entry, u64 chantitle, u32 ios, u8 vidMode, bool vipatch, bool countryString, u8 patchVidMode, int aspectRatio)
{
	// IOS Version Check
	*Real_IOSVersion = ((ios << 16)) | 0xFFFF;
	*Game_IOSVersion = ((ios << 16)) | 0xFFFF;
	DCFlushRange((void*)Real_IOSVersion, 4);
	DCFlushRange((void*)Game_IOSVersion, 4);

	// Game ID Online Check
	memset((void*)Disc_ID, 0, 4);
	*Disc_ID = TITLE_LOWER(chantitle);
	DCFlushRange((void*)Disc_ID, 4);

	ExternalBooter_ChannelSetup(dolchunkoffset, dolchunksize, dolchunkcount, entry);
	WiiFlow_ExternalBooter(vidMode, vipatch, countryString, patchVidMode, aspectRatio, 0, TYPE_CHANNEL);
	return 0;
}

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

		gprintf("Moving section %u from offset %08x to %08x-%08x...\n", i, dolfile->section_pos[i], dolchunkoffset[dolchunkcount], (u32)dolchunkoffset[dolchunkcount]+dolchunksize[dolchunkcount]);
		memmove(dolchunkoffset[dolchunkcount], buffer + dolfile->section_pos[i], dolchunksize[dolchunkcount]);
		DCFlushRange(dolchunkoffset[dolchunkcount], dolchunksize[dolchunkcount]);
		ICInvalidateRange(dolchunkoffset[dolchunkcount], dolchunksize[dolchunkcount]);

		dolchunkcount++;
	}
	return dolfile->entry_point;
}

bool Identify_GenerateTik(signed_blob **outbuf, u32 *outlen)
{
	signed_blob *buffer = (signed_blob *)memalign(32, ALIGN32(STD_SIGNED_TIK_SIZE));
	if(!buffer)
		return false;
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

bool Identify(u64 titleid)
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
		free(tmdBuffer);
		free(tikBuffer);
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
	
	free(tmdBuffer);
	free(tikBuffer);
	free(certBuffer);

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
				free(data);
				return NULL;
			}
			free(data);
			data = decompressed;
		}	
		return data;
	}
	gprintf("Failed!\n");
	return NULL;
}
