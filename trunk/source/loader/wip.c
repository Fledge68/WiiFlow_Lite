
#include <gccore.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "wip.h"
#include "gecko/gecko.h"
#include "memory/mem2.hpp"

static WIP_Code *CodeList = NULL;
static u32 CodesCount = 0;

u32 get_wip_count()
{
	return CodesCount;
}

WIP_Code *get_wip_list()
{
	return CodeList;
}

int load_wip_patches(u8 *dir, u8 *gameid)
{
	char filepath[150];
	char GameID[7];
	GameID[6] = '\0';
	memcpy(GameID, gameid, 6);
	snprintf(filepath, sizeof(filepath), "%s/%s.wip", dir, GameID);

	FILE *fp = fopen(filepath, "rb");
	if(!fp)
	{
		memcpy(GameID, gameid, 3);
		GameID[3] = '\0';
		snprintf(filepath, sizeof(filepath), "%s/%s.wip", dir, GameID);
		fp = fopen(filepath, "rb");
	}
	if(!fp)
		return -1;

	char line[255];
	gprintf("\nLoading WIP code from %s.\n", filepath);

	while(fgets(line, sizeof(line), fp))
	{
		if(line[0] == '#' || strlen(line) < 26)
			continue;

		u32 offset = (u32) strtoul(line, NULL, 16);
		u32 srcaddress = (u32) strtoul(line+9, NULL, 16);
		u32 dstaddress = (u32) strtoul(line+18, NULL, 16);

		if(!CodeList)
			CodeList = MEM2_alloc(sizeof(WIP_Code));

		WIP_Code *tmp = MEM2_realloc(CodeList, (CodesCount+1)*sizeof(WIP_Code));
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

	return 0;
}
