
// WBFS FAT by oggzee

#include <stdio.h>
#include <unistd.h>
#include <ogcsys.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/statvfs.h>
#include <ctype.h>

#include "wbfs.h"
#include "wdvd.h"
#include "splits.h"
#include "wbfs_ext.h"
#include "utils.h"
#include "disc.h"

#include "devicemounter/sdhc.h"
#include "devicemounter/usbstorage.h"
#include "fileOps/fileOps.h"
#include "gecko/gecko.hpp"
#include "libwbfs/libwbfs.h"
#include "loader/utils.h"
#include "loader/sys.h"
#include "gui/fmt.h"

#define TITLE_LEN 64

char wbfs_fs_drive[16];
char invalid_path[] = "/\\:|<>?*\"'";

split_info_t split;

struct statvfs wbfs_ext_vfs;

#define STRCOPY(DEST,SRC) strcopy(DEST,SRC,sizeof(DEST)) 
char* strcopy(char *dest, const char *src, int size)
{
	strncpy(dest,src,size);
	dest[size-1] = 0;
	return dest;
}

wbfs_disc_t* WBFS_Ext_OpenDisc(u8 *discid, char *fname)
{
	bool sd = strstr(fname, "sd") != NULL;
	if (strcasecmp(strrchr(fname,'.'), ".iso") == 0)
	{
		// .iso file
		// create a fake wbfs_disc
		int fd = open(fname, O_RDONLY);
		if (fd == -1) return NULL;

		wbfs_disc_t *iso_file = malloc(sizeof(wbfs_disc_t));
		memset(iso_file, 0, sizeof(wbfs_disc_t));

		if (iso_file == NULL) 
			return NULL;

		// mark with a special wbfs_part
		wbfs_iso_file.wbfs_sec_sz = sd ? 512 : USBStorage2_GetSectorSize();
		iso_file->p = &wbfs_iso_file;
		iso_file->header = (void*)fd;
		return iso_file;
	}

	wbfs_t *part = WBFS_Ext_OpenPart(fname);
	if(!part)
		return NULL;

	return wbfs_open_disc(part, discid);
}

void WBFS_Ext_CloseDisc(wbfs_disc_t* disc)
{
	if (!disc) return;
	wbfs_t *part = disc->p;

	// is this really a .iso file?
	if (part == &wbfs_iso_file)
	{
		close((int)disc->header);
		free(disc);
		return;
	}

	wbfs_close_disc(disc);
	WBFS_Ext_ClosePart(part);
}

s32 WBFS_Ext_DiskSpace(f32 *used, f32 *free)
{
	*used = 0;
	*free = 0;

	static s32 wbfs_ext_vfs_have = 0, wbfs_ext_vfs_lba = 0,  wbfs_ext_vfs_dev = 0;

	// statvfs is slow, so cache values
	if (!wbfs_ext_vfs_have || wbfs_ext_vfs_lba != (s32)wbfs_part_lba || wbfs_ext_vfs_dev != wbfsDev )
	{
		if(statvfs(fmt("%s:", wbfs_fs_drive), &wbfs_ext_vfs))
			return 0;

		wbfs_ext_vfs_have = 1;
		wbfs_ext_vfs_lba = wbfs_part_lba;
		wbfs_ext_vfs_dev = wbfsDev;
	}

	/* FS size in GB */
	f32 size = (f32)wbfs_ext_vfs.f_frsize * (f32)wbfs_ext_vfs.f_blocks / GB_SIZE;
	*free = (f32)wbfs_ext_vfs.f_frsize * (f32)wbfs_ext_vfs.f_bfree / GB_SIZE;
	*used = size - *free;

	return 0;
}

wbfs_t* WBFS_Ext_OpenPart(char *fname)
{
	bool sd = strstr(fname, "sd") != NULL;
	if(split_open(&split, fname) < 0)
		return NULL;

	wbfs_set_force_mode(1);
	wbfs_t *part = wbfs_open_partition(split_read_sector, 0, //readonly //split_write_sector,
		&split, sd ? 512 : USBStorage2_GetSectorSize(), split.total_sec, 0, 0);
	wbfs_set_force_mode(0);

	if (!part)
		split_close(&split);

	return part;
}

void WBFS_Ext_ClosePart(wbfs_t* part)
{
	if (!part) return;
	split_info_t *s = (split_info_t*)part->callback_data;
	wbfs_close(part);
	if (s) split_close(s);
}

s32 WBFS_Ext_RemoveGame(u8 *discid, char *gamepath)
{
	DIR *dir_iter;
	struct dirent *ent;
	char file[MAX_FAT_PATH];
	char folder[MAX_FAT_PATH];
	STRCOPY(folder, gamepath);
	char *p = strrchr(folder, '/');
	if(p) *p = 0;

	dir_iter = opendir(folder);
	if (!dir_iter) return 0;
	while((ent = readdir(dir_iter)) != NULL)
	{
		if(strstr(ent->d_name, (char*)discid) != NULL)
		{
			snprintf(file, sizeof(file), "%s/%s", folder, ent->d_name);
			remove(file);
		}
	}
	closedir(dir_iter);
	if(strlen(folder) > 11)
		unlink(folder);
	return 0;
}

s32 WBFS_Ext_AddGame(progress_callback_t spinner, void *spinner_data)
{
	struct discHdr header ATTRIBUTE_ALIGN(32);

	char *illegal = "\"*/:<>?\\|";
	char *cp;
	char *cleantitle;

	char folder[MAX_FAT_PATH];
	memset(folder, 0, MAX_FAT_PATH);

	char gamepath[MAX_FAT_PATH];
	memset(gamepath, 0, MAX_FAT_PATH);

	Disc_ReadHeader(&header);
	asprintf(&cleantitle, header.title);
	for(cp = strpbrk(cleantitle, illegal); cp; cp = strpbrk(cp, illegal))
		*cp = '_';
	strncpy(folder, fmt(wii_games_dir, wbfs_fs_drive), sizeof(folder));
	fsop_MakeFolder(folder);
	strncpy(folder, fmt("%s/%s [%s]", folder, cleantitle, header.id), sizeof(folder));
	fsop_MakeFolder(folder);
	free(cleantitle);

	strncpy(gamepath, fmt("%s/%s.wbfs", folder, header.id), sizeof(gamepath));
	u64 size = (u64)143432*2*0x8000ULL;
	u32 n_sector = size / 512;

	if(split_create(&split, gamepath, OPT_split_size, size, true))
		return -1;

	//force create first file
	u32 scnt = 0;
	if (split_get_file(&split, 0, &scnt, 0) < 0)
	{
		split_close(&split);
		return -1;
	}

	wbfs_t *part = wbfs_open_partition(split_read_sector, split_write_sector, &split, 512, n_sector, 0, 1);
	if (!part)
	{
		split_close(&split);
		return -1;
	}

	extern wbfs_t *hdd;
	wbfs_t *old_hdd = hdd;
	hdd = part; // used by spinner
	s32 ret = wbfs_add_disc(part, __WBFS_ReadDVD, NULL, spinner, spinner_data, REMOVE_UPDATE_PARTITION, 0);
	hdd = old_hdd;

	if(ret == 0) wbfs_trim(part);

	WBFS_Ext_ClosePart(part);
	
	if(ret < 0) WBFS_Ext_RemoveGame(NULL, gamepath);

	return ret < 0 ? ret : 0;
}

s32 WBFS_Ext_DVD_Size(u64 *comp_size, u64 *real_size)
{
	s32 ret;
	u32 comp_sec = 0, last_sec = 0;

	wbfs_t *part = NULL;
	u64 size = (u64)143432*2*0x8000ULL;
	u32 n_sector = size / 512;
	u32 wii_sec_sz; 

	// init a temporary dummy part
	// as a placeholder for wbfs_size_disc
	part = wbfs_open_partition(0, 0, NULL, 512, n_sector, 0, 1);
	if(!part)
		return -1;
	wii_sec_sz = part->wii_sec_sz;

	/* Add game to device */
	ret = wbfs_size_disc(part, __WBFS_ReadDVD, NULL, REMOVE_UPDATE_PARTITION, &comp_sec, &last_sec);
	wbfs_close(part);
	if(ret < 0)
		return ret;

	*comp_size = (u64)wii_sec_sz * comp_sec;
	*real_size = (u64)wii_sec_sz * last_sec;

	return 0;
}
