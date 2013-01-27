
/* Code from postloader - thanks to stfour */

#include <gccore.h> 
#include <ogc/machine/processor.h>
#include <string.h>
#include <unistd.h>
#include "gecko/gecko.hpp"
#include "memory/memory.h"
#include "identify.h"

static inline bool apply_patch(char *name, const u8 *old, const u8 *patch, u32 size)
{
	u8 i;
	u32 found = 0;
	u8 *ptr = (u8*)IOS_Patch_Start;

	u32 level = IRQ_Disable();
	while((u32)ptr < (u32)IOS_Patch_End)
	{
		if(memcmp(ptr, old, size) == 0)
		{
			for(i = 0; i < size; ++i)
				*(vu8*)(ptr+i) = *(vu8*)(patch+i);
			found++;
		}
		ptr++;
	}
	IRQ_Restore(level);

	gprintf("patched %s %u times.\n", name, found);
	return (found > 0);
}

static const u8 isfs_perm_old[] = { 0x42, 0x8B, 0xD0, 0x01, 0x25, 0x66 };
static const u8 isfs_perm_patch[] = { 0x42, 0x8B, 0xE0, 0x01, 0x25, 0x66 };
static const u8 setuid_old[] = { 0xD1, 0x2A, 0x1C, 0x39 };
static const u8 setuid_patch[] = { 0x46, 0xC0, 0x1C, 0x39 };
static const u8 es_identify_old[] = { 0x28, 0x03, 0xD1, 0x23 };
static const u8 es_identify_patch[] = { 0x28, 0x03, 0x00, 0x00 };
static const u8 hash_old[] = { 0x20, 0x07, 0x23, 0xA2 };
static const u8 hash_patch[] = { 0x20, 0x00, 0x23, 0xA2 };
static const u8 new_hash_old[] = { 0x20, 0x07, 0x4B, 0x0B };
static const u8 new_hash_patch[] = { 0x20, 0x00, 0x4B, 0x0B };

void PatchIOS(void)
{
	__ES_Close();
	write16(MEM_PROT, 0);
	/* Do Patching */
	apply_patch("isfs_permissions", isfs_perm_old, isfs_perm_patch, sizeof(isfs_perm_patch));
	apply_patch("es_setuid", setuid_old, setuid_patch, sizeof(setuid_patch));
	apply_patch("es_identify", es_identify_old, es_identify_patch, sizeof(es_identify_patch));
	apply_patch("hash_check", hash_old, hash_patch, sizeof(hash_patch));
	apply_patch("new_hash_check", new_hash_old, new_hash_patch, sizeof(new_hash_patch));
	/* Reinit */
	write16(MEM_PROT, 1);
	__ES_Init();
}
