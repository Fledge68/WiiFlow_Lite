
#include <gccore.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include "wip.h"

static WIP_Code * CodeList = NULL;
static u32 CodesCount = 0;
static u32 ProcessedLength = 0;
static u32 Counter = 0;

void do_wip_code(u8 * dst, u32 len)
{
	if(!CodeList)
		return;

	if(Counter < 3)
	{
		Counter++;
		return;
	}

	u32 i = 0;
	s32 n = 0;
	s32 offset = 0;

	for(i = 0; i < CodesCount; i++)
	{
		for(n = 0; n < 4; n++)
		{
			offset = CodeList[i].offset+n-ProcessedLength;

			if(offset < 0 || (u32)offset >= len)
				continue;

			if(dst[offset] == ((u8 *)&CodeList[i].srcaddress)[n])
			{
				dst[offset] = ((u8 *)&CodeList[i].dstaddress)[n];
				//printf("WIP: %08X Address Patched.\n", CodeList[i].offset + n);
			}
			else
			{
				//printf("WIP: %08X Address does not match with WIP entry.\n", CodeList[i].offset+n);
				//printf("Destination: %02X | Should be: %02X.\n", dst[offset], ((u8 *)&CodeList[i].srcaddress)[n]);
			}
		}
	}
	ProcessedLength += len;
	Counter++;
}

//! for internal patches only
//! .wip files override internal patches
//! the codelist has to be freed if the set fails
//! if set was successful the codelist will be freed when it's done
bool set_wip_list(WIP_Code * list, int size)
{
	if(!CodeList && size > 0)
	{
		CodeList = list;
		CodesCount = size;
		return true;
	}

	return false;
}

void wip_reset_counter()
{
	ProcessedLength = 0;
	//alternative dols don't need a skip. only main.dol.
	Counter = 3;
}

void free_wip()
{
	if(CodeList)
		free(CodeList);

	CodesCount = 0;
	ProcessedLength = 0;
}

int load_wip_patches(u8 *dir, u8 *gameid)
{
	char filepath[150];
	char GameID[8];
	memset(GameID, 0, sizeof(GameID));
	memcpy(GameID, gameid, 6);
	snprintf(filepath, sizeof(filepath), "%s/%s.wip", dir, GameID);

	FILE *fp = fopen(filepath, "rb");
	if(!fp)
	{
		memset(GameID, 0, sizeof(GameID));
		memcpy(GameID, gameid, 3);
		snprintf(filepath, sizeof(filepath), "%s/%s.wip", dir, GameID);
		fp = fopen(filepath, "rb");
	}

	if(!fp)
		return -1;

	char line[255];
	//printf("\nLoading WIP code from %s.\n", filepath);

	while(fgets(line, sizeof(line), fp))
	{
		if(line[0] == '#' || strlen(line) < 26)
			continue;

		u32 offset = (u32) strtoul(line, NULL, 16);
		u32 srcaddress = (u32) strtoul(line+9, NULL, 16);
		u32 dstaddress = (u32) strtoul(line+18, NULL, 16);

		if(!CodeList)
			CodeList = malloc(sizeof(WIP_Code));

		WIP_Code *tmp = realloc(CodeList, (CodesCount+1)*sizeof(WIP_Code));
		if(!tmp)
		{
			free(CodeList);
			fclose(fp);
			return -1;
		}

		CodeList = tmp;

		CodeList[CodesCount].offset = offset;
		CodeList[CodesCount].srcaddress = srcaddress;
		CodeList[CodesCount].dstaddress = dstaddress;
		CodesCount++;
	}
	fclose(fp);
	//printf("\n");

	return 0;
}
