
#include <gccore.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include "wip.h"
#include "gecko.h"

static WIP_Code *CodeList = NULL;
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
				gprintf("WIP: %08X Address Patched.\n", CodeList[i].offset + n);
			}
			else
			{
				gprintf("WIP: %08X Address does not match with WIP entry.\n", CodeList[i].offset+n);
				gprintf("Destination: %02X | Should be: %02X.\n", dst[offset], ((u8 *)&CodeList[i].srcaddress)[n]);
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
bool set_wip_list(WIP_Code *list, int size)
{
	if(CodeList == NULL && size > 0)
	{
		WIP_Code *newlist = malloc(size * sizeof(WIP_Code));
		memcpy(newlist, list, size * sizeof(WIP_Code));
		DCFlushRange(newlist, size * sizeof(WIP_Code));
		CodeList = newlist;
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
	if(CodeList != NULL)
		free(CodeList);
	CodesCount = 0;
	ProcessedLength = 0;
}
