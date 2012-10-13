
#include "utils.h"
#include "debug.h"
#include "wip.h"

WIP_Code *CodeList;
u32 CodesCount;
u32 ProcessedLength;
u32 Counter;

void do_wip_code(u8 * dst, u32 len)
{
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
				debug_string("Address patched:"); debug_uint(CodeList[i].offset+n); debug_string("\n");
				//printf("WIP: %08X Address Patched.\n", CodeList[i].offset + n);
			}
			else
			{
				debug_string("Address NOT patched:"); debug_uint(CodeList[i].offset+n); debug_string("\n");
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
u8 set_wip_list(WIP_Code *list, int size)
{
	CodeList = list;
	CodesCount = size;
	ProcessedLength = 0;
	Counter = 0;
	return 1;
}

void free_wip()
{
	CodesCount = 0;
	ProcessedLength = 0;
}
