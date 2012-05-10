/***************************************************************************
 * Copyright (C) 2011 by Miigotu
 *           (C) 2012 by OverjoY
 *
 * Rewritten code from Mighty Channels and Triiforce
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * Nand/Emulation Handling Class for Wiiflow
 *
 ***************************************************************************/
 
#include <stdio.h>
#include <ogcsys.h>
#include <malloc.h>
#include <string.h>
#include <cstdlib>
#include <stdarg.h>
#include <dirent.h>

#include "nand.hpp"
#include "utils.h"
#include "mem2.hpp"
#include "wbfs.h"
#include "gecko.h"

u8 *confbuffer ATTRIBUTE_ALIGN(32);
u8 CCode[0x1008];
char SCode[4];
char *txtbuffer ATTRIBUTE_ALIGN(32);

config_header *cfg_hdr;

bool tbdec = false;
bool configloaded = false;

static NandDevice NandDeviceList[] = 
{
	{ "Disable",						0,	0x00,	0x00 },
	{ "SD/SDHC Card",					1,	0xF0,	0xF1 },
	{ "USB 2.0 Mass Storage Device",	2,	0xF2,	0xF3 },
};

Nand * Nand::instance = NULL;

Nand * Nand::Instance()
{
	if(instance == NULL)
		instance = new Nand();
	return instance;
}

void Nand::DestroyInstance()
{
	if(instance) delete instance;
	instance = NULL;
}

void Nand::Init(string path, u32 partition, bool disable)
{
	EmuDevice = disable ? REAL_NAND : partition == 0 ? EMU_SD : EMU_USB;
	Partition = disable ? REAL_NAND : partition > 0 ? partition - 1 : partition;
	Set_NandPath(path);
	Disabled = disable;
}

s32 Nand::Nand_Mount(NandDevice *Device)
{
	s32 fd = IOS_Open("fat", 0);
	if (fd < 0) return fd;

	static ioctlv vector[1] ATTRIBUTE_ALIGN(32);	
	
	vector[0].data = &Partition;
	vector[0].len = sizeof(u32);

	s32 ret = IOS_Ioctlv(fd, Device->Mount, 1, 0, vector);
	IOS_Close(fd);

	return ret;
}

s32 Nand::Nand_Unmount(NandDevice *Device)
{
	s32 fd = IOS_Open("fat", 0);
	if (fd < 0) return fd;

	s32 ret = IOS_Ioctlv(fd, Device->Unmount, 0, 0, NULL);
	IOS_Close(fd);

	return ret;
}

s32 Nand::Nand_Enable(NandDevice *Device)
{
	gprintf("Enabling NAND Emulator\n");
	s32 fd = IOS_Open("/dev/fs", 0);
	if (fd < 0) return fd;

	int NandPathlen = strlen(NandPath) + 1;

	static ioctlv vector[2] ATTRIBUTE_ALIGN(32);

	static u32 mode ATTRIBUTE_ALIGN(32) = Device->Mode | FullMode;

	vector[0].data = &mode;
	vector[0].len = sizeof(u32);
	vector[1].data = NandPath;
	vector[1].len = NandPathlen;

	s32 ret = IOS_Ioctlv(fd, 100, 2, 0, vector);
	IOS_Close(fd);

	return ret;
} 

s32 Nand::Nand_Disable(void)
{
	gprintf("Disabling NAND Emulator\n");
	s32 fd = IOS_Open("/dev/fs", 0);
	if (fd < 0) return fd;

	u32 inbuf ATTRIBUTE_ALIGN(32) = 0;
	s32 ret = IOS_Ioctl(fd, 100, &inbuf, sizeof(inbuf), NULL, 0);
	IOS_Close(fd);

	return ret;
} 

s32 Nand::Enable_Emu()
{
	if(MountedDevice == EmuDevice || Disabled)
		return 0;

	Disable_Emu();

	NandDevice *Device = &NandDeviceList[EmuDevice];

	s32 ret = Nand_Mount(Device);
	if (ret < 0) return ret;

	ret = Nand_Enable(Device);
	if (ret < 0) return ret;

	MountedDevice = EmuDevice;

	return 0;
}	

s32 Nand::Disable_Emu()
{
	if(MountedDevice == 0)
		return 0;

	NandDevice * Device = &NandDeviceList[MountedDevice];

	Nand_Disable();
	Nand_Unmount(Device);

	MountedDevice = 0;

	return 0;
}

void Nand::Set_NandPath(string path)
{
	if(isalnum(*(path.begin()))) path.insert(path.begin(), '/');
	else *(path.begin()) = '/';

	if(isalnum(*(path.end()))) path.push_back('/');
	else *(path.end()) = '/';

	if(path.size() <= 32)
		memcpy(NandPath, path.c_str(), path.size());
}

void Nand::__Dec_Enc_TB(void) 
{	
	u32 key = 0x73B5DBFA;
    int i;
    
	for( i=0; i < 0x100; ++i ) 
	{    
		txtbuffer[i] ^= key&0xFF;
		key = (key<<1) | (key>>31);
	}
	
	tbdec = tbdec ? false : true;	
}

void Nand::__configshifttxt(char *str)
{
	const char *ptr = str;
	char *ctr = str;
	int i;
	int j = strlen(str);
	
	for( i=0; i<j; ++i )
	{		
		if( strncmp( str+(i-3), "PALC", 4 ) == 0 )
			*ctr = 0x0d;
		else if( strncmp( str+(i-2), "LUH", 3 ) == 0 )
			*ctr = 0x0d;
		else if( strncmp( str+(i-2), "LUM", 3 ) == 0 )	
			*ctr = 0x0d;
		else 
			*ctr = str[i];

		ctr++;
	}	
	*ctr = *ptr;
	*ctr = '\0';
}

void Nand::__GetNameList(const char *source, namelist **entries, int *count)
{
	u32 i, j, k, l;
	u32 numentries = 0;	
	char *names;
	char curentry[ISFS_MAXPATH];
	char entrypath[ISFS_MAXPATH];

	s32 ret = ISFS_ReadDir(source, NULL, &numentries);
	names = (char *)MEM2_alloc((ISFS_MAXPATH) * numentries);
	ret = ISFS_ReadDir(source, names, &numentries);	
	*count = numentries;

	if(*entries)
		MEM2_free(*entries);

	*entries = (namelist *)MEM2_alloc(sizeof(namelist)*numentries);	

	for(i = 0, k = 0; i < numentries; i++)
	{
		for(j = 0; names[k] != 0; j++, k++)
			curentry[j] = names[k];
		
		curentry[j] = 0;
		k++;

		strcpy((*entries)[i].name, curentry);

		if(source[strlen(source)-1] == '/')
			snprintf(entrypath, sizeof(entrypath), "%s%s", source, curentry);
		else
			snprintf(entrypath, sizeof(entrypath), "%s/%s", source, curentry);
		
		ret = ISFS_ReadDir(entrypath, NULL, &l);		
		(*entries)[i].type = ret < 0 ? 0 : 1;
	}	
	MEM2_free(names);
}

s32 Nand::__configread(void)
{	
	confbuffer = (u8 *)MEM2_alloc(0x4000);
	txtbuffer = (char *)MEM2_alloc(0x100);
	cfg_hdr = (config_header *)NULL;
	
	FILE *f = fopen(cfgpath, "rb");
	if(f)
	{
		fread(confbuffer, 1, 0x4000, f);
		SAFE_CLOSE(f);
	}
		
	f = fopen(settxtpath, "rb");
	if(f)
	{
		fread(txtbuffer, 1, 0x100, f);
		SAFE_CLOSE(f);
	}
		
	cfg_hdr = (config_header *)confbuffer;
		
	__Dec_Enc_TB();
	
	configloaded = configloaded ? false : true;
	
	if(tbdec && configloaded)
		return 1;		
	
	return 0;
}

s32 Nand::__configwrite(void)
{
	if(configloaded)
	{
		__Dec_Enc_TB();	
		
		if(!tbdec)
		{
			FILE *f = fopen(cfgpath, "wb");
			if(f)
			{
				fwrite(confbuffer, 1, 0x4000, f);
				gprintf("SYSCONF written to:\"%s\"\n", cfgpath);
				SAFE_CLOSE(f);
			}
			
			f = fopen(settxtpath, "wb");
			if(f)
			{
				fwrite(txtbuffer, 1, 0x100, f);
				gprintf("setting.txt written to: \"%s\"\n", settxtpath);
				SAFE_CLOSE(f);
			}		
				
			configloaded = configloaded ? false : true;
			
			if(!tbdec && !configloaded)
				return 1;			
		}
	}
	free(confbuffer);
	free(txtbuffer);
	return 0;			
} 

u32 Nand::__configsetbyte(const char *item, u8 val)
{
	u32 i;
	for(i=0; i<cfg_hdr->ncnt; ++i)
	{
		if(memcmp(confbuffer+(cfg_hdr->noff[i] + 1), item, strlen(item)) == 0)
		{
			*(u8*)(confbuffer+cfg_hdr->noff[i] + 1 + strlen(item)) = val;
			break;
		}
	}
	return 0;
}

u32 Nand::__configsetbigarray(const char *item, void *val, u32 size)
{
	u32 i;
	for(i=0; i<cfg_hdr->ncnt; ++i)
	{
		if(memcmp(confbuffer+(cfg_hdr->noff[i] + 1), item, strlen(item)) == 0)
		{
			memcpy(confbuffer+cfg_hdr->noff[i] + 3 + strlen(item), val, size);
			break;
		}
	}
	return 0;
}

u32 Nand::__configsetsetting(const char *item, const char *val)
{		
	char *curitem = strstr(txtbuffer, item);
	char *curstrt, *curend;
	
	if(curitem == NULL)
		return 0;
	
	curstrt = strchr(curitem, '=');
	curend = strchr(curitem, 0x0d);
	
	if(curstrt && curend)
	{
		curstrt += 1;
		u32 len = curend - curstrt;
		if(strlen(val) > len)
		{
			static char buffer[0x100];
			u32 nlen;
			nlen = txtbuffer-(curstrt+strlen(val));
			strcpy( buffer, txtbuffer+nlen );
			strncpy( curstrt, val, strlen(val));
			curstrt += strlen(val); 
			strncpy(curstrt, buffer, strlen(buffer));
		}
		else
		{
			strncpy(curstrt, val, strlen(val));
		}

		__configshifttxt(txtbuffer);

		return 1;
	}
	return 0;
}

bool Nand::__FileExists(const char *path, ...)
{
	FILE *f = fopen(path, "rb");		
	if (f != 0)
	{
		gprintf("File \"%s\" exists\n", path);		
		SAFE_CLOSE(f);
		return true;
	}
	return false;
}

void Nand::__FATify(char *ptr, const char *str)
{
	char ctr;
	while ((ctr = *(str++)) != '\0') 
	{
		const char *esc;
		switch (ctr) 
		{
			case '"':
				esc = "&qt;";
			break;
			case '*':
				esc = "&st;";
			break;
			case ':':
				esc = "&cl;";
			break;
			case '<':
				esc = "&lt;";
			break;
			case '>':
				esc = "&gt;";
			break;
			case '?':
				esc = "&qm;";
			break; 
			case '|':
				esc = "&vb;";
			break;
			default:
				*(ptr++) = ctr;
				continue;
		}
		strcpy(ptr, esc);
		ptr += 4;
	}
	*ptr = '\0';
}

s32 Nand::__FlashNandFile(const char *source, const char *dest)
{
	s32 ret;
	FILE *file = fopen(source, "rb");
	if(!file) 
	{
		gprintf("Error opening source: \"%s\"\n", source);
		return 0;
	}
	
	fseek(file, 0, SEEK_END);
	u32 fsize = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	if(fake)
	{
		NandSize += fsize;
		if(showprogress)
			dumper(NandSize, 0x1f400000, 0x1f400000, NandSize, FilesDone, FoldersDone, (char *)"", data);
		
		SAFE_CLOSE(file);
		return 0;
	}
	
	gprintf("Flashing: %s (%uKB) to nand...", dest, (fsize / 0x400)+1);
	
	ISFS_Delete(dest);
	ISFS_CreateFile(dest, 0, 3, 3, 3);
	s32 fd = ISFS_Open(dest, ISFS_OPEN_RW);
	if(fd < 0)
	{
		gprintf(" failed\nError: ISFS_OPEN(%s, %d) %d\n", dest, ISFS_OPEN_RW, fd);
		SAFE_CLOSE(file);
		return fd;
	}

	u8 *buffer = (u8 *)MEM2_alloc(BLOCK);
	u32 toread = fsize;
	while(toread > 0)
	{
		u32 size = BLOCK;
		if(toread < BLOCK)
			size = toread;

		ret = fread(buffer, 1, size, file);
		if(ret <= 0) 
		{
			gprintf(" failed\nError: fread(%p, 1, %d, %s) %d\n", buffer, size, source, ret);
			ISFS_Close(fd);
			SAFE_CLOSE(file);
			MEM2_free(buffer);
			return ret;
		}
		
		ret = ISFS_Write(fd, buffer, size);
		if(ret <= 0) 
		{
			gprintf(" failed\nError: ISFS_Write(%d, %p, %d) %d\n", fd, buffer, size, ret);
			ISFS_Close(fd);
			SAFE_CLOSE(file);
			MEM2_free(buffer);
			return ret;
		}
		toread -= size;
		NandDone += size;
		FileDone += size;
		
		if(showprogress)
		{
			const char *file = strrchr(dest, '/')+1;
			dumper(NandDone, NandSize, fsize, FileDone, FilesDone, FoldersDone, (char *)file, data);
		}
	}	
	gprintf(" done!\n");
	FilesDone++;
	if(showprogress)
	{
		const char *file = strrchr(dest, '/')+1;
		dumper(NandDone, NandSize, fsize, FileDone, FilesDone, FoldersDone, (char *)file, data);
	}
	ISFS_Close(fd);
	MEM2_free(buffer);
	SAFE_CLOSE(file);	
	return 1;
}

s32 Nand::__DumpNandFile(const char *source, const char *dest)
{
	FileDone = 0;	
	s32 fd = ISFS_Open(source, ISFS_OPEN_READ);
	if (fd < 0) 
	{
		gprintf("Error: IOS_OPEN(%s, %d) %d\n", source, ISFS_OPEN_READ, fd);
		return fd;
	}
	
	fstats *status = (fstats *)MEM2_alloc(sizeof(fstats));
	s32 ret = ISFS_GetFileStats(fd, status);
	if (ret < 0)
	{
		gprintf("Error: ISFS_GetFileStats(%d) %d\n", fd, ret);
		ISFS_Close(fd);
		MEM2_free(status);
		return ret;
	}
	
	if(fake)
	{
		NandSize += status->file_length;
		if(showprogress)
			dumper(NandSize, 0x1f400000, 0x1f400000, NandSize, FilesDone, FoldersDone, (char *)"", data);
		
		ISFS_Close(fd);
		MEM2_free(status);
		return 0;
	}
	
	if(__FileExists(dest))   
		remove(dest);		
	
	FILE *file = fopen(dest, "wb");
	if (!file)
	{
		gprintf("Error opening destination: \"%s\"\n", dest);
		ISFS_Close(fd);
		return 0;
	}
	
	gprintf("Dumping: %s (%ukb)...", source, (status->file_length / 0x400)+1);

	u8 *buffer = (u8 *)MEM2_alloc(BLOCK);
	u32 toread = status->file_length;
	while(toread > 0)
	{
		u32 size = BLOCK;
		if (toread < BLOCK)
			size = toread;

		ret = ISFS_Read(fd, buffer, size);
		if (ret < 0)
		{
			gprintf(" failed\nError: ISFS_Read(%d, %p, %d) %d\n", fd, buffer, size, ret);
			ISFS_Close(fd);
			SAFE_CLOSE(file);
			MEM2_free(status);
			MEM2_free(buffer);
			return ret;
		}
		
		ret = fwrite(buffer, 1, size, file);
		if(ret < 0) 
		{
			gprintf(" failed\nError writing to destination: \"%s\" (%d)\n", dest, ret);
			ISFS_Close(fd);
			SAFE_CLOSE(file);
			MEM2_free(status);
			MEM2_free(buffer);
			return ret;
		}
		toread -= size;
		NandDone += size;
		FileDone += size;
		
		if(showprogress)
		{
			const char *file = strrchr(source, '/')+1;
			dumper(NandDone, NandSize, status->file_length, FileDone, FilesDone, FoldersDone, (char *)file, data);
		}
	}
	FilesDone++;
	if(showprogress)
	{
		const char *file = strrchr(source, '/')+1;
		dumper(NandDone, NandSize, status->file_length, FileDone, FilesDone, FoldersDone, (char *)file, data);
	}
	gprintf(" done!\n");
	SAFE_CLOSE(file);	
	ISFS_Close(fd);	
	MEM2_free(status);
	MEM2_free(buffer);
	
	return 0;
}


s32 Nand::__FlashNandFolder(const char *source, const char *dest)
{	
	char nsource[MAX_FAT_PATH];
	char ndest[ISFS_MAXPATH];
	char tdest[ISFS_MAXPATH];
	
	DIR *dir_iter;
	struct dirent *ent;
	
	dir_iter = opendir(source);
	if (!dir_iter)
		return 1;
		
	while((ent = readdir(dir_iter)) != NULL)
	{
		if(ent->d_name[0] == '.') 
			continue;
			
		if(dest[strlen(dest)-1] == '/')
			snprintf(ndest, sizeof(ndest), "%s%s", dest, ent->d_name);
		else
			snprintf(ndest, sizeof(ndest), "%s/%s", dest, ent->d_name);	
			
		if(source[strlen(source)-1] == '/')
			snprintf(nsource, sizeof(nsource), "%s%s", source, ent->d_name);
		else
			snprintf(nsource, sizeof(nsource), "%s/%s", source, ent->d_name);
		
		if(ent->d_type == DT_DIR)
		{
			//__FATify(tdest, ndest);	
			if(!fake)
			{
				//ISFS_CreateDir(tdest, 0, 3, 3, 3);
				ISFS_CreateDir(ndest, 0, 3, 3, 3);
				FoldersDone++;
			}
			__FlashNandFolder(nsource, ndest);			
		}
		else
		{			
			//__FATify(tdest, ndest);
			//__FlashNandFile(nsource, tdest);
			__FlashNandFile(nsource, ndest);
		}
	}
	return 0;
}

s32 Nand::__DumpNandFolder(const char *source, const char *dest)
{
	namelist *names = NULL;
	int cnt, i;
	char nsource[ISFS_MAXPATH];
	char ndest[MAX_FAT_PATH];
	char tdest[MAX_FAT_PATH];
	
	__GetNameList(source, &names, &cnt);	
	
	for(i = 0; i < cnt; i++) 
	{
		if(source[strlen(source)-1] == '/')
			snprintf(nsource, sizeof(nsource), "%s%s", source, names[i].name);
		else
			snprintf(nsource, sizeof(nsource), "%s/%s", source, names[i].name);		
		
		if(!names[i].type)
		{
			__FATify(tdest, nsource);			
			snprintf(ndest, sizeof(ndest), "%s%s", dest, tdest);
			__DumpNandFile(nsource, ndest);
		}
		else
		{
			if(!fake)
			{
				__FATify(tdest, nsource);						
				CreatePath("%s%s", dest, tdest);
				FoldersDone++;
			}
			
			__DumpNandFolder(nsource, dest);
		}
	}
	SAFE_FREE(names);
	return 0;
}

void Nand::CreatePath(const char *path, ...)
{
	char *folder = NULL;
	va_list args;
	va_start(args, path);
	if((vasprintf(&folder, path, args) >= 0) && folder)
	{
		if(folder[strlen(folder)-1] == '/')
			folder[strlen(folder)-1] = 0;		
			
		DIR *d;
		d = opendir(folder);

		if(!d)
		{	
			gprintf("Creating folder: \"%s\"\n", folder);
			makedir(folder);
		}
		else
		{
			gprintf("Folder \"%s\" exists\n", folder);
			closedir(d);
		}
	}
	va_end(args);
	SAFE_FREE(folder);
}

void Nand::CreateTitleTMD(const char *path, dir_discHdr *hdr)
{
	wbfs_disc_t *disc = WBFS_OpenDisc((u8 *) &hdr->hdr.id, (char *)hdr->path);
	if(!disc) 
		return;

	u8 *titleTMD = NULL;
	u32 tmd_size = wbfs_extract_file(disc, (char *) "TMD", (void **)&titleTMD);
	WBFS_CloseDisc(disc);

	if(!titleTMD) 
		return;
		
	u32 highTID = *(u32*)(titleTMD+0x18c);
	u32 lowTID = *(u32*)(titleTMD+0x190);
	
	CreatePath("%s/title/%08x/%08x/data", path, highTID, lowTID);
	CreatePath("%s/title/%08x/%08x/content", path, highTID, lowTID);
	
	char nandpath[MAX_FAT_PATH];
	snprintf(nandpath, sizeof(nandpath), "%s/title/%08x/%08x/content/title.tmd", path, highTID, lowTID);

	struct stat filestat;
	if (stat(nandpath, &filestat) == 0)
	{
		SAFE_FREE(titleTMD);
		gprintf("%s Exists!\n", nandpath);
		return;
	}
	gprintf("Creating title TMD: %s\n", nandpath);
	
	FILE *file = fopen(nandpath, "wb");
	if(file)
	{
		fwrite(titleTMD, 1, tmd_size, file);
		gprintf("Title TMD written to: %s\n", nandpath);
		fclose(file);
	}
	else 
		gprintf("Creating title TMD: %s failed (%i)\n", nandpath, file);

	SAFE_FREE(titleTMD);
}

s32 Nand::FlashToNAND(const char *source, const char *dest, dump_callback_t i_dumper, void *i_data)
{	
	ISFS_CreateDir(dest, 0, 3, 3, 3);
	data = i_data;
	dumper = i_dumper;
	fake = false;
	showprogress = true;	
	__FlashNandFolder(source, dest);
	return 0;	
}

s32 Nand::DoNandDump(const char *source, const char *dest, dump_callback_t i_dumper, void *i_data)
{	
	data = i_data;
	dumper = i_dumper;
	fake = false;
	showprogress = true;
	u32 temp = 0;	
	s32 ret = ISFS_ReadDir(source, NULL, &temp);
	if(ret < 0)
	{
		char ndest[MAX_FAT_PATH];
		snprintf(ndest, sizeof(ndest), "%s%s", dest, source);
		CreatePath(dest);
		
		__DumpNandFile(source, ndest);
	}
	else
	{		
		__DumpNandFolder(source, dest);
	}	
	return 0;	
}

s32 Nand::CalcFlashSize(const char *source, dump_callback_t i_dumper, void *i_data)
{	
	data = i_data;
	dumper = i_dumper;
	fake = true;
	showprogress = true;
	__FlashNandFolder(source, "");	
	return NandSize;	
}

s32 Nand::CalcDumpSpace(const char *source, dump_callback_t i_dumper, void *i_data)
{	
	data = i_data;
	dumper = i_dumper;
	fake = true;
	showprogress = true;		
	
	u32 temp = 0;	
	
	s32 ret = ISFS_ReadDir(source, NULL, &temp);
	if(ret < 0)		
		__DumpNandFile(source, "");
	else
		__DumpNandFolder(source, "");
	
	return NandSize;	
}

void Nand::ResetCounters(void)
{
	NandSize = 0;
	FilesDone = 0;
	FoldersDone = 0;
	NandDone = 0;
}
 
s32 Nand::CreateConfig(const char *path)
{
	CreatePath(path);
	CreatePath("%s/shared2", path);
	CreatePath("%s/shared2/sys", path);
	CreatePath("%s/title", path);
	CreatePath("%s/title/00000001", path);
	CreatePath("%s/title/00000001/00000002", path);
	CreatePath("%s/title/00000001/00000002/data", path);
	
	fake = false;
	showprogress = false;
	
	bzero(cfgpath, MAX_FAT_PATH+1);	
	bzero(settxtpath, MAX_FAT_PATH+1);
	
	snprintf(cfgpath, sizeof(cfgpath), "%s%s", path, SYSCONFPATH);
	snprintf(settxtpath, sizeof(settxtpath), "%s%s", path, TXTPATH);
	
	__DumpNandFile(SYSCONFPATH, cfgpath);
	__DumpNandFile(TXTPATH, settxtpath);
	return 0;	
}

s32 Nand::Do_Region_Change(string id)
{	
	if(__configread())
	{
		switch(id[3])
		{
			case 'J':
			{
				gprintf("Switching region to NTSC-j \n");
				CCode[0] = 1;
				__configsetbyte( "IPL.LNG", 0 );				
				__configsetbigarray( "SADR.LNG", CCode, 0x1007 );
				__configsetsetting( "AREA", "JPN" );
				__configsetsetting( "MODEL", "RVL-001(JPN)" );
				__configsetsetting( "CODE", "LJM" );
				__configsetsetting( "VIDEO", "NTSC" );
				__configsetsetting( "GAME", "JP" );								
			} break;
			case 'E':
			{
				gprintf("Switching region to NTSC-u \n");
				CCode[0] = 31;
				__configsetbyte( "IPL.LNG", 1 );				
				__configsetbigarray( "IPL.SADR", CCode, 0x1007 );
				__configsetsetting( "AREA", "USA" );
				__configsetsetting( "MODEL", "RVL-001(USA)" );
				__configsetsetting( "CODE", "LU" );
				__configsetsetting( "VIDEO", "NTSC" );
				__configsetsetting( "GAME", "US" );									
			} break;
			case 'D':
			case 'F':
			case 'I':
			case 'M':
			case 'P':
			case 'S':
			case 'U':					
			{
				gprintf("Switching region to PAL \n");
				CCode[0] = 110;
				__configsetbyte( "IPL.LNG", 1 );				
				__configsetbigarray( "IPL.SADR", CCode, 0x1007 );
				__configsetsetting( "AREA", "EUR" );
				__configsetsetting( "MODEL", "RVL-001(EUR)" );
				__configsetsetting( "CODE", "LEH" );
				__configsetsetting( "VIDEO", "PAL" );
				__configsetsetting( "GAME", "EU" );								
			} break;
			case 'K':					
			{
				gprintf("Switching region to NTSC-k \n");
				CCode[0] = 137;
				__configsetbyte( "IPL.LNG", 9 );				
				__configsetbigarray( "IPL.SADR", CCode, 0x1007 );
				__configsetsetting( "AREA", "KOR" );
				__configsetsetting( "MODEL", "RVL-001(KOR)" );
				__configsetsetting( "CODE", "LKM" );
				__configsetsetting( "VIDEO", "NTSC" );
				__configsetsetting( "GAME", "KR" );
			} break;		
		}
	}
	__configwrite();
	return 1;	
}