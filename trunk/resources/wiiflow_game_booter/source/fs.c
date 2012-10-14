
#include <ogcsys.h>
#include <locale.h>
#include <malloc.h>
#include <ogc/isfs.h>
#include <string.h>

#include "fs.h"
#include "utils.h"

static fstats stats ATTRIBUTE_ALIGN(32);

u8 *ISFS_GetFile(u8 *path, u32 *size, s32 length)
{
	*size = 0;
	
	s32 fd = ISFS_Open((const char *)path, ISFS_OPEN_READ);
	u8 *buf = NULL;

	if(fd >= 0)
	{
		memset(&stats, 0, sizeof(fstats));
		if(ISFS_GetFileStats(fd, &stats) >= 0)
		{
			if(length <= 0)
				length = stats.file_length;
			if(length > 0)
				buf = (u8 *)memalign(32, ALIGN32(length));
			if(buf)
			{
				*size = stats.file_length;
				if(ISFS_Read(fd, (char*)buf, length) != length)
				{
					*size = 0;
					free(buf);
				}
			}
		}
		ISFS_Close(fd);
	}

	if(*size > 0)
	{
		DCFlushRange(buf, *size);
		ICInvalidateRange(buf, *size);
	}
	return buf;
}
