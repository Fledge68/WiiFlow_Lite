#include <ogcsys.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

#include "ntfs.h"
#include "ntfsfile_frag.h"

#include "wbfs.h"
#include "wbfs_ext.h"
#include "frag.h"
#include "utils.h"
#include "sys.h"
#include "wdvd.h"
#include "ext2_frag.h"
#include "fatfile_frag.h"
#include "devicemounter/usbstorage.h"
#include "gecko/gecko.hpp"
#include "libwbfs/libwbfs.h"

FragList *frag_list = NULL;

void frag_init(FragList *ff, int maxnum)
{
	memset(ff, 0, sizeof(Fragment) * (maxnum+1));
	ff->maxnum = maxnum;
}

void frag_dump(FragList *ff)
{
	u32 i;
	gprintf("frag list: %d %d 0x%x\n", ff->num, ff->size, ff->size);
	for (i = 0; i < ff->num; i++)
	{
		if (i > 10)
		{
			gprintf("...\n");
			break;
		}
		gprintf(" %d : %8x %8x %8x\n", i, ff->frag[i].offset, ff->frag[i].count, ff->frag[i].sector);
	}
}

int frag_append(FragList *ff, u32 offset, u32 sector, u32 count)
{
	int n;
	if (count)
	{
		n = ff->num - 1;
		if (ff->num > 0 && ff->frag[n].offset + ff->frag[n].count == offset && ff->frag[n].sector + ff->frag[n].count == sector)
			ff->frag[n].count += count;
		else
		{
			if (ff->num >= ff->maxnum) return -500; // too many fragments

			n = ff->num;
			ff->frag[n].offset = offset;
			ff->frag[n].sector = sector;
			ff->frag[n].count  = count;
			ff->num++;
		}
	}
	ff->size = offset + count;
	return 0;
}

int _frag_append(void *ff, u32 offset, u32 sector, u32 count)
{
	return frag_append(ff, offset, sector, count);
}

int frag_concat(FragList *ff, FragList *src)
{
	int ret;
	u32 i;
	u32 size = ff->size;

	for (i=0; i<src->num; i++)
	{
		ret = frag_append(ff, size + src->frag[i].offset,
				src->frag[i].sector, src->frag[i].count);
		if (ret) return ret;
	}
	ff->size = size + src->size;

	return 0;
}

// in case a sparse block is requested,
// the returned poffset might not be equal to requested offset
// the difference should be filled with 0
int frag_get(FragList *ff, u32 offset, u32 count, u32 *poffset, u32 *psector, u32 *pcount)
{
	u32 i;
	u32 delta;
	for (i=0; i<ff->num; i++)
	{
		if (ff->frag[i].offset <= offset && ff->frag[i].offset + ff->frag[i].count > offset)
		{
			delta = offset - ff->frag[i].offset;
			*poffset = offset;
			*psector = ff->frag[i].sector + delta;
			*pcount = ff->frag[i].count - delta;
			if (*pcount > count) *pcount = count;
			goto out;
		}
		if (ff->frag[i].offset > offset	&& ff->frag[i].offset < offset + count)
		{
			delta = ff->frag[i].offset - offset;
			*poffset = ff->frag[i].offset;
			*psector = ff->frag[i].sector;
			*pcount = ff->frag[i].count;
			count -= delta;
			if (*pcount > count) *pcount = count;
			goto out;
		}
	}
	// not found
	if (offset + count > ff->size) return -1;  // error: out of range!

	// if inside range, then it must be just sparse, zero filled
	// return empty block at the end of requested
	*poffset = offset + count;
	*psector = 0;
	*pcount = 0;
	out:

	return 0;
}

int frag_remap(FragList *ff, FragList *log, FragList *phy)
{
	u32 i;
	u32 offset;
	u32 sector;
	for (i=0; i<log->num; i++)
	{
		u32 delta = 0;
		u32 count = 0;
		do
		{
			int ret = frag_get(phy,	log->frag[i].sector + delta + count,
						log->frag[i].count - delta - count,	&offset, &sector, &count);
			if (ret) return ret; // error

			delta = offset - log->frag[i].sector;
			ret = frag_append(ff, log->frag[i].offset + delta, sector, count);
			if (ret) return ret; // error

		}
		while (count + delta < log->frag[i].count);
	}
	return 0;
}

int get_frag_list(u8 *id, char *path, const u32 hdd_sector_size)
{
	char fname[1024];
	bzero(fname, 1024);
	u32 length = strlen(path);
	strcpy(fname, path);

	int ret, ret_val = -1;
	u32 i, j;

	if(fname[length-1] > '0' && fname[length-1] < '9')
		return 0;
	bool isWBFS = wbfs_part_fs != PART_FS_WBFS && strcasestr(strrchr(fname,'.'), ".wbfs") != 0;

	struct stat st;
	FragList *fs = MEM1_lo_alloc(sizeof(FragList));
	FragList *fa = MEM1_lo_alloc(sizeof(FragList));
	FragList *fw = MEM1_lo_alloc(sizeof(FragList));
	if(fs == NULL || fa == NULL || fw == NULL)
		goto out;

	frag_init(fa, MAX_FRAG);

	for (i = 0; i < 10; i++) // .wbfs1 - .wbfsA, 10 possible splits? lol overkill?
	{
		frag_init(fs, MAX_FRAG);
		if (i > 0)
		{
			fname[length-1] = '0' + i;
			if (stat(fname, &st) == -1) break;
		}
		if (wbfs_part_fs == PART_FS_FAT)
		{
			ret = _FAT_get_fragments(fname, &_frag_append, fs);
			if (ret)
			{
				ret_val = 0;
				goto out;
			}
		}
		else if (wbfs_part_fs == PART_FS_NTFS)
		{
			ret = _NTFS_get_fragments(fname, &_frag_append, fs);
			if (ret)
			{
				ret_val = ret;
				goto out;
			}
		}
		else if (wbfs_part_fs == PART_FS_EXT)
		{
			ret = _EXT2_get_fragments(fname, &_frag_append, fs);
			if (ret)
			{
				ret_val = ret;
				goto out;
			}
		}
		else if (wbfs_part_fs == PART_FS_WBFS)
		{
			// if wbfs file format, remap.
			wbfs_disc_t *d = WBFS_OpenDisc(id, path);
			if (!d)
			{
				ret_val = -4;
				WBFS_CloseDisc(d);
				goto out;
			}
			ret = wbfs_get_fragments(d, &_frag_append, fs, hdd_sector_size);
			WBFS_CloseDisc(d);
			if (ret)
			{
				ret_val = -5;
				goto out;
			}
		}
		if (wbfs_part_fs == PART_FS_NTFS || wbfs_part_fs == PART_FS_EXT)
		{
			gprintf("Shifting all frags by sector: %d\n", wbfs_part_lba);
			// offset to start of partition
			for (j = 0; j < fs->num; j++) 
				fs->frag[j].sector += wbfs_part_lba;
		}
		
		frag_concat(fa, fs);
	}

	frag_list = MEM1_lo_alloc(sizeof(FragList));
	if(frag_list == NULL)
		goto out;

	frag_init(frag_list, MAX_FRAG);
	if (isWBFS) // If this is a .wbfs file format, remap.
	{
		wbfs_disc_t *disc = WBFS_OpenDisc(id, path);
		if (!disc)
		{
			ret_val = -4;
			goto out;
		}
		frag_init(fw, MAX_FRAG);
		ret = wbfs_get_fragments(disc, &_frag_append, fw, hdd_sector_size);
		if (ret) 
		{
			ret_val = -5;
			goto out;
		}
		WBFS_CloseDisc(disc); // Close before jumping on fail.
		if(frag_remap(frag_list, fw, fa))
		{
			ret_val = -6;
			goto out;
		}
	}
	else
		memcpy(frag_list, fa, sizeof(FragList)); // .iso files do not need a remap, just copy
	DCFlushRange(frag_list, sizeof(FragList));
	ret_val = 0;

out:
	if(ret_val && frag_list != NULL)
	{
		MEM1_lo_free(frag_list);
		frag_list = NULL;
	}
	if(fs != NULL)
	{
		MEM1_lo_free(fs);
		fs = NULL;
	}
	if(fa != NULL)
	{
		MEM1_lo_free(fa);
		fa = NULL;
	}
	if(fw != NULL)
	{
		MEM1_lo_free(fw);
		fw = NULL;
	}
	return ret_val;
}
