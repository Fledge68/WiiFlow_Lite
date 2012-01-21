#include <stdio.h>
#include <ogcsys.h>
#include <string.h>

#include "gecko.h"

#define MAX_BLOCKSIZE           0x10000

u64 le64(u64 x)
{
 	return 
	((x & 0xFF00000000000000) >> 56) |
	((x & 0x00FF000000000000) >> 40) |
	((x & 0x0000FF0000000000) >> 24) |
	((x & 0x000000FF00000000) >> 8 ) |
	((x & 0x00000000FF000000) << 8 ) |
	((x & 0x0000000000FF0000) << 24) |
	((x & 0x000000000000FF00) << 40) |
	((x & 0x00000000000000FF) << 56);
}

u32 le32(u32 x)
{
	return
	((x & 0x000000FF) << 24) |
	((x & 0x0000FF00) << 8) |
	((x & 0x00FF0000) >> 8) |
	((x & 0xFF000000) >> 24);
}

u16 le16(u16 x)
{
	return
	((x & 0x00FF) << 8) |
	((x & 0xFF00) >> 8);
}

bool str_replace(char *str, const char *olds, const char *news, int size)
{
	char *p = strstr(str, olds);
	if (!p) return false;
	// new len
	int len = strlen(str) - strlen(olds) + strlen(news);
	if (len >= size) return false;
	// move remainder to fit (and nul)
	memmove(p+strlen(news), p+strlen(olds), strlen(p)-strlen(olds)+1);
	// copy new in place
	memcpy(p, news, strlen(news));
	// terminate
	str[len] = 0;
	return true;
}

bool str_replace_all(char *str, const char *olds, const char *news, int size)
{
	int cnt = -1;
	while (str_replace(str, olds, news, size))
		cnt++;

	return (cnt > 0);
}
