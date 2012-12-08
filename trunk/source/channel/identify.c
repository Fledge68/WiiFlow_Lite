
/* Code from postloader - thanks to stfour */

#include <gccore.h> 
#include <ogc/machine/processor.h>
#include <string.h>

#include "gecko/gecko.hpp"
#include "memory/memory.h"
#include "identify.h"

static bool apply_patch(char *name, const u8 *old, u32 old_size, const u8 *patch, u32 patch_size, u32 patch_offset)
{
	u8 *ptr = (u8*)0x93400000;
	bool found = false;
	u8 *location = NULL;
	while((u32)ptr < (0x94000000 - patch_size))
	{
		if(memcmp(ptr, old, old_size) == 0)
		{
			found = true;
			location = ptr + patch_offset;
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
	if(found)
		gprintf("apply_patch '%s': found at %08x\n", name, ptr);
	else
		gprintf("apply_patch '%s': not found\n", name);
	return found;
}

const u8 isfs_permissions_old[] = { 0x42, 0x8B, 0xD0, 0x01, 0x25, 0x66 };
const u8 isfs_permissions_patch[] = { 0x42, 0x8B, 0xE0, 0x01, 0x25, 0x66 };
static const u8 setuid_old[] = { 0xD1, 0x2A, 0x1C, 0x39 };
static const u8 setuid_patch[] = { 0x46, 0xC0 };
const u8 es_identify_old[] = { 0x28, 0x03, 0xD1, 0x23 };
const u8 es_identify_patch[] = { 0x00, 0x00 };
const u8 hash_old[] = { 0x20, 0x07, 0x23, 0xA2 };
const u8 hash_patch[] = { 0x00 };
const u8 new_hash_old[] = { 0x20, 0x07, 0x4B, 0x0B };

bool Patch_ISFS_Permission(bool enable)
{
	/* Disable memory protection */
	write16(MEM_PROT, 0);
	/* Do Patches */
	bool ret = false;
	if(enable)
	{
		gprintf("Enabling ISFS Patches...\n");
		ret = apply_patch("isfs_permissions", isfs_permissions_old, sizeof(isfs_permissions_old), isfs_permissions_patch, sizeof(isfs_permissions_patch), 0);
	}
	else /* Just revert it */
	{
		gprintf("Disabling ISFS Patches...\n");
		ret = apply_patch("isfs_permissions", isfs_permissions_patch, sizeof(isfs_permissions_patch), isfs_permissions_old, sizeof(isfs_permissions_old), 0);
	}
	/* Enable memory protection */
	write16(MEM_PROT, 1);
	return ret;
}

void Patch_Channel_Boot(void)
{
	/* Disable memory protection */
	write16(MEM_PROT, 0);
	/* Do Patching */
	apply_patch("es_setuid", setuid_old, sizeof(setuid_old), setuid_patch, sizeof(setuid_patch), 0);
	apply_patch("es_identify", es_identify_old, sizeof(es_identify_old), es_identify_patch, sizeof(es_identify_patch), 2);
	apply_patch("hash_check", hash_old, sizeof(hash_old), hash_patch, sizeof(hash_patch), 1);
	apply_patch("new_hash_check", new_hash_old, sizeof(new_hash_old), hash_patch, sizeof(hash_patch), 1);
	/* Enable memory protection */
	write16(MEM_PROT, 1);
}
