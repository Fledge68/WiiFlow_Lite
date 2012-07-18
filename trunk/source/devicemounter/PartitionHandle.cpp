 /****************************************************************************
 * Copyright (C) 2010
 * by Dimok
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
 * for WiiXplorer 2010
 ***************************************************************************/
#include <gccore.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "cios.h"
#include "PartitionHandle.h"
#include "utils.h"
#include "ntfs.h"
#include "fat.h"
#include "ext2.h"
#include "wbfs.h"
#include <sdcard/gcsd.h>
#include <sdcard/wiisd_io.h>

#ifdef DOLPHIN
const DISC_INTERFACE __io_sdhc = __io_wiisd;
#else
extern const DISC_INTERFACE __io_sdhc;
#endif

#define PARTITION_TYPE_DOS33_EXTENDED		0x05 /* DOS 3.3+ extended partition */
#define PARTITION_TYPE_WIN95_EXTENDED		0x0F /* Windows 95 extended partition */

//! libfat stuff
extern "C"
{
	sec_t FindFirstValidPartition(const DISC_INTERFACE* disc);
}

#define CACHE 32
#define SECTORS 64

static inline const char * PartFromType(int type)
{
	switch (type)
	{
		case 0x00: return "Unused";
		case 0x01: return "FAT12";
		case 0x04: return "FAT16";
		case 0x05: return "Extended";
		case 0x06: return "FAT16";
		case 0x07: return "NTFS";
		case 0x0b: return "FAT32";
		case 0x0c: return "FAT32";
		case 0x0e: return "FAT16";
		case 0x0f: return "Extended";
		case 0x82: return "LxSWP";
		case 0x83: return "LINUX";
		case 0x8e: return "LxLVM";
		case 0xa8: return "OSX";
		case 0xab: return "OSXBT";
		case 0xaf: return "OSXHF";
		case 0xbf: return "WBFS";
		case 0xe8: return "LUKS";
		default: return "Unknown";
	}
}

PartitionHandle::PartitionHandle(const DISC_INTERFACE *discio)
	: interface(discio)
{
	// Sanity check
	if (!interface)
		return;

	// Start the device and check that it is inserted
	if (!interface->startup())
		return;

	if (!interface->isInserted())
		return;

	FindPartitions();
}

PartitionHandle::~PartitionHandle()
{
	 UnMountAll();

	//shutdown device
	if(!neek2o())
		interface->shutdown();
}

bool PartitionHandle::IsMounted(int pos)
{
	if(pos < 0 || pos >= (int) MountNameList.size())
		return false;

	if(MountNameList[pos].size() == 0)
		return false;

	return true;
}

bool PartitionHandle::Mount(int pos, const char * name, bool forceFAT)
{
	if(!valid(pos))
		return false;

	if(!name)
		return false;

	UnMount(pos);

	if(pos >= (int) MountNameList.size())
		MountNameList.resize(pos+1);

	MountNameList[pos] = name;
	SetWbfsHandle(pos, NULL);

	//! Some stupid partition manager think they don't need to edit the freaken MBR.
	//! So we need to check the first 64 sectors and see if some partition is there.
	//! libfat does that by default so let's use it.
	//! We do that only on sd not on usb.
	if(forceFAT && (!GetFSName(pos) || strcmp(GetFSName(pos), "Unknown") == 0))
	{
		if (fatMount(MountNameList[pos].c_str(), interface, 0, CACHE, SECTORS))
		{
			sec_t FAT_startSector = FindFirstValidPartition(interface);
			AddPartition("FAT", FAT_startSector, 0xdeadbeaf, true, 0x0c, 0);
			return true;
		}
	}

	if(strncmp(GetFSName(pos), "FAT", 3) == 0 || strcmp(GetFSName(pos), "GUID-Entry") == 0)
	{
		if (fatMount(MountNameList[pos].c_str(), interface, GetLBAStart(pos), CACHE, SECTORS))
		{
			if(strcmp(GetFSName(pos), "GUID-Entry") == 0)
				PartitionList[pos].FSName = "FAT";
			return true;
		}
	}

	if(strncmp(GetFSName(pos), "NTFS", 4) == 0 || strcmp(GetFSName(pos), "GUID-Entry") == 0)
	{
		if(ntfsMount(MountNameList[pos].c_str(), interface, GetLBAStart(pos), CACHE, SECTORS, NTFS_SHOW_HIDDEN_FILES | NTFS_RECOVER))
		{
			PartitionList[pos].FSName = "NTFS";
			return true;
		}
	}

	if(strncmp(GetFSName(pos), "LINUX", 5) == 0 || strcmp(GetFSName(pos), "GUID-Entry") == 0)
	{
		if(ext2Mount(MountNameList[pos].c_str(), interface, GetLBAStart(pos), CACHE, SECTORS, EXT2_FLAG_DEFAULT))
		{
			PartitionList[pos].FSName = "LINUX";
			return true;
		}
	}
	else if(strncmp(GetFSName(pos), "WBFS", 4) == 0)
	{
		if(interface == &__io_usbstorage2_port0 || interface == &__io_usbstorage2_port1)
			SetWbfsHandle(pos, wbfs_open_partition(__WBFS_ReadUSB, __WBFS_WriteUSB, NULL, USBStorage2_GetSectorSize(), GetSecCount(pos), GetLBAStart(pos), 0));
		else if((neek2o() && interface == &__io_wiisd) || (!neek2o() && interface == &__io_sdhc))
			SetWbfsHandle(pos, wbfs_open_partition(__WBFS_ReadSDHC, __WBFS_WriteSDHC, NULL, 512, GetSecCount(pos), GetLBAStart(pos), 0));
		if(GetWbfsHandle(pos))
			return true;
	}

	MountNameList[pos].clear();

	return false;
}

void PartitionHandle::UnMount(int pos)
{
	if(!interface)
		return;

	if(pos >= (int) MountNameList.size())
		return;

	if(MountNameList[pos].size() == 0)
		return;

	char DeviceSyn[20];
	snprintf(DeviceSyn, sizeof(DeviceSyn), "%s:", MountNameList[pos].c_str());

	wbfs_t* wbfshandle = GetWbfsHandle(pos);
	if(wbfshandle) wbfs_close(wbfshandle);
	SetWbfsHandle(pos, NULL);
	WBFS_Close();

	//closing all open Files write back the cache
	fatUnmount(DeviceSyn);
	//closing all open Files write back the cache
	ntfsUnmount(DeviceSyn, true);
	//closing all open Files write back the cache
	ext2Unmount(DeviceSyn);
	//Remove name from list
	MountNameList[pos].clear();
}

bool PartitionHandle::IsExisting(u64 lba)
{
	for(u32 i = 0; i < PartitionList.size(); ++i)
	{
		if(PartitionList[i].LBA_Start == lba)
			return true;
	}

	return false;
}

int PartitionHandle::FindPartitions()
{
	MASTER_BOOT_RECORD *mbr = (MASTER_BOOT_RECORD *) malloc(MAX_BYTES_PER_SECTOR);
	if(!mbr) return -1;

	// Read the first sector on the device
	if (!interface->readSectors(0, 1, mbr))
	{
		free(mbr);
		return -1;
	}

	// If this is the devices master boot record
	if (mbr->signature != MBR_SIGNATURE)
	{
		free(mbr);
		return -1;
	}

	for (int i = 0; i < 4; i++)
	{
		PARTITION_RECORD * partition = (PARTITION_RECORD *) &mbr->partitions[i];

		if(partition->type == PARTITION_TYPE_GPT)
		{
			int ret = CheckGPT(i);
			if(ret == 0) // if it's a GPT we don't need to go on looking through the mbr anymore
				return ret;
		}

		if(partition->type == PARTITION_TYPE_DOS33_EXTENDED || partition->type == PARTITION_TYPE_WIN95_EXTENDED)
		{
			CheckEBR(i, le32(partition->lba_start));
			continue;
		}

		if(le32(partition->block_count) > 0 && !IsExisting(le32(partition->lba_start)))
		{
			AddPartition(PartFromType(partition->type), le32(partition->lba_start),
									  le32(partition->block_count), (partition->status == PARTITION_BOOTABLE),
									  partition->type, i);
		}
	}

	free(mbr);

	return 0;
}

void PartitionHandle::CheckEBR(u8 PartNum, sec_t ebr_lba)
{
	EXTENDED_BOOT_RECORD *ebr = (EXTENDED_BOOT_RECORD *) malloc(MAX_BYTES_PER_SECTOR);
	if(!ebr) return;
	sec_t next_erb_lba = 0;

	do
	{
		// Read and validate the extended boot record
		if (!interface->readSectors(ebr_lba + next_erb_lba, 1, ebr))
		{
			free(ebr);
			return;
		}

		if (ebr->signature != EBR_SIGNATURE)
		{
			free(ebr);
			return;
		}

		if(le32(ebr->partition.block_count) > 0 && !IsExisting(ebr_lba + next_erb_lba + le32(ebr->partition.lba_start)))
		{
			AddPartition(PartFromType(ebr->partition.type), ebr_lba + next_erb_lba + le32(ebr->partition.lba_start),
									  le32(ebr->partition.block_count), (ebr->partition.status == PARTITION_BOOTABLE),
									  ebr->partition.type, PartNum);
		}
		// Get the start sector of the current partition
		// and the next extended boot record in the chain
		next_erb_lba = le32(ebr->next_ebr.lba_start);
	}
	while(next_erb_lba > 0);

	free(ebr);
}

static const u8 TYPE_UNUSED[16] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
static const u8 TYPE_BIOS[16] = { 0x48,0x61,0x68,0x21,0x49,0x64,0x6F,0x6E,0x74,0x4E,0x65,0x65,0x64,0x45,0x46,0x49 };
static const u8 TYPE_LINUX_MS_BASIC_DATA[16] = { 0xA2,0xA0,0xD0,0xEB,0xE5,0xB9,0x33,0x44,0x87,0xC0,0x68,0xB6,0xB7,0x26,0x99,0xC7 };

int PartitionHandle::CheckGPT(u8 PartNum)
{
	GPT_HEADER *gpt_header = (GPT_HEADER *) malloc(MAX_BYTES_PER_SECTOR);
	if(!gpt_header) return -1;

	// Read and validate the extended boot record
	if (!interface->readSectors(1, 1, gpt_header))
	{
		free(gpt_header);
		return -1;
	}

	if(strncmp(gpt_header->magic, "EFI PART", 8) != 0)
	{
		free(gpt_header);
		return -1;
	}

	gpt_header->part_table_lba = le64(gpt_header->part_table_lba);
	gpt_header->part_entries = le32(gpt_header->part_entries);
	gpt_header->part_entry_size = le32(gpt_header->part_entry_size);
	gpt_header->part_entry_checksum = le32(gpt_header->part_entry_checksum);

	u8 * sector_buf = new u8[MAX_BYTES_PER_SECTOR];

	u64 next_lba = gpt_header->part_table_lba;

	for(u32 i = 0; i < gpt_header->part_entries; ++i)
	{
		if (!interface->readSectors(next_lba, 1, sector_buf))
			break;

		for(u32 n = 0; n < BYTES_PER_SECTOR/gpt_header->part_entry_size; ++n, ++i)
		{
			GUID_PART_ENTRY * part_entry = (GUID_PART_ENTRY *) (sector_buf+gpt_header->part_entry_size*n);

			if(memcmp(part_entry->part_type_guid, TYPE_UNUSED, 16) == 0)
				continue;

			if(IsExisting(le64(part_entry->part_first_lba)))
				continue;

			bool bootable = (memcmp(part_entry->part_type_guid, TYPE_BIOS, 16) == 0);

			AddPartition("GUID-Entry", le64(part_entry->part_first_lba), le64(part_entry->part_last_lba), bootable, PARTITION_TYPE_GPT, PartNum);
		}

		next_lba++;
	}

	delete [] sector_buf;
	free(gpt_header);

	return 0;
}

void PartitionHandle::AddPartition(const char * name, u64 lba_start, u64 sec_count, bool bootable, u8 part_type, u8 part_num)
{
	char *buffer = (char *) malloc(MAX_BYTES_PER_SECTOR);

	if (!interface->readSectors(lba_start, 1, buffer))
	{
		free(buffer);
		return;
	}

	wbfs_head_t *head = (wbfs_head_t *) buffer;

	if (head->magic == wbfs_htonl(WBFS_MAGIC))
	{
		name = "WBFS";
		part_type = 0xBF;   //Override partition type on WBFS
		//! correct sector size in physical sectors (512 bytes per sector)
		sec_count = (u64) head->n_hd_sec * (u64) (1 << head->hd_sec_sz_s) / (u64) BYTES_PER_SECTOR;

	}
	else if(*((u16 *) (buffer + 0x1FE)) == 0x55AA)
	{
		//! Partition typ can be missleading the correct partition format. Stupid lazy ass Partition Editors.
		if((memcmp(buffer + 0x36, "FAT", 3) == 0 || memcmp(buffer + 0x52, "FAT", 3) == 0) &&
			strncmp(PartFromType(part_type), "FAT", 3) != 0)
		{
			name = "FAT32";
			part_type = 0x0c;
		}
		if (memcmp(buffer + 0x03, "NTFS", 4) == 0)
		{
			name = "NTFS";
			part_type = 0x07;
		}
	}

	PartitionFS PartitionEntrie;
	PartitionEntrie.FSName = name;
	PartitionEntrie.LBA_Start = lba_start;
	PartitionEntrie.SecCount = sec_count;
	PartitionEntrie.Bootable = bootable;
	PartitionEntrie.PartitionType = part_type;
	PartitionEntrie.PartitionNum = part_num;

	PartitionList.push_back(PartitionEntrie);

	free(buffer);
}
