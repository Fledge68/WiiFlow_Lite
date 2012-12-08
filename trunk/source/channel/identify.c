
/* Code from postloader - thanks to stfour */

#include <gccore.h> 
#include <ogc/machine/processor.h>
#include <string.h>

#include "identify.h"

const u8 isfs_permissions_old[] = { 0x42, 0x8B, 0xD0, 0x01, 0x25, 0x66 };
const u8 isfs_permissions_patch[] = { 0x42, 0x8B, 0xE0, 0x01, 0x25, 0x66 };

static bool apply_patch(const u8 *old, u32 old_size, const u8 *patch, u32 patch_size)
{
    u8 *ptr = (u8*)0x93A00000;
    bool found = false;
    u8 *location = NULL;
    while((u32)ptr < 0x93B00000)
	{
        if(!memcmp(ptr, old, old_size))
		{
            found = true;
            location = ptr;
            u8 *start = location;
            u32 i;
            for(i = 0; i < patch_size; i++)
            {
				*location = patch[i];
				location++;
			}
            DCFlushRange((u8 *)(((u32)start) >> 5 << 5), (patch_size >> 5 << 5) + 64);
			break;
        }
        ptr++;
    }
    return found;
}

bool Patch_ISFS_Permission(bool enable)
{
	if(enable)
		return apply_patch(isfs_permissions_old, sizeof(isfs_permissions_old), 
			isfs_permissions_patch, sizeof(isfs_permissions_patch));
	else /* Just revert it */
		return apply_patch(isfs_permissions_patch, sizeof(isfs_permissions_patch), 
			isfs_permissions_old, sizeof(isfs_permissions_old));
}
