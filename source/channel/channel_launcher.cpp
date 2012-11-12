
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>

#include "channel_launcher.h"
#include "booter/external_booter.hpp"
#include "gecko/gecko.h"
#include "loader/disc.h"
#include "loader/fs.h"
#include "loader/fst.h"
#include "loader/utils.h"
#include "memory/memory.h"
#include "unzip/lz77.h"
#include "types.h"

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
	u8 *tmdBuffer = ISFS_GetFile(filepath, &tmdSize, -1);
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
	strcpy(filepath, "/sys/cert.sys");
	u32 certSize;
	u8 *certBuffer = ISFS_GetFile(filepath, &certSize, -1);
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
