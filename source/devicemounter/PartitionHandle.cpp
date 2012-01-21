 /****************************************************************************
 * Copyright (C) 2010
 * by Dimok
 * modified for Debugging, GPT, WBFS, and EXT by Miigotu
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
 * By Dimok for WiiXplorer 2010
 * By Miigotu for WiiFlow 2010
 ***************************************************************************/
#include <gccore.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "PartitionHandle.h"
#include "utils.h"
#include "ntfs.h"
#include "fat.h"
#include "ext2.h"
#include "wbfs.h"
#include <sdcard/gcsd.h>

#define PARTITION_TYPE_DOS33_EXTENDED		0x05 /* DOS 3.3+ extended partition */
#define PARTITION_TYPE_WIN95_EXTENDED		0x0F /* Windows 95 extended partition */
#define PARTITION_TYPE_GPT_TABLE			0xEE /* New Standard */

#define CACHE 8
#define SECTORS 64

extern const DISC_INTERFACE __io_sdhc;

extern u32 sector_size;

static inline const char * PartFromType(int type)
{
	switch (type)
	{
		case 0x00: return "Unused"; //Or WBFS
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
		case 0xe8: return "LUKS";
		case 0xee: return "GPT";
		default: return "Unknown";
	}
}

PartitionHandle::PartitionHandle(const DISC_INTERFACE *discio)
{
	interface = discio;

	// Sanity check
	if(!interface) return;

	// Start the device and check that it is inserted
	if(!interface->startup()) return;
	if(!interface->isInserted()) return;

	FindPartitions();
}

PartitionHandle::~PartitionHandle()
{
	UnMountAll();

	//shutdown device
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

bool PartitionHandle::Mount(int pos, const char * name)
{
	if(!valid(pos)) return false;
	if(!name) return false;

	UnMount(pos);

	if(pos >= (int) MountNameList.size())
		MountNameList.resize(GetPartitionCount());

	MountNameList[pos] = name;
	SetWbfsHandle(pos, NULL);

	if(strncmp(GetFSName(pos), "FAT", 3) == 0)
	{
		if(fatMount(MountNameList[pos].c_str(), interface, GetLBAStart(pos), CACHE, SECTORS))
			return true;
	}
	else if(strncmp(GetFSName(pos), "NTFS", 4) == 0)
	{
		if(ntfsMount(MountNameList[pos].c_str(), interface, GetLBAStart(pos), CACHE, SECTORS, NTFS_SU | NTFS_RECOVER | NTFS_IGNORE_CASE))
			return true;
	}
	else if(strncmp(GetFSName(pos), "LINUX", 5) == 0)
	{
		if(ext2Mount(MountNameList[pos].c_str(), interface, GetLBAStart(pos), CACHE, SECTORS, EXT2_FLAG_DEFAULT))
			return true;
	}
	else if(strncmp(GetFSName(pos), "WBFS", 4) == 0)
	{
		if (interface == &__io_usbstorage)
			SetWbfsHandle(pos, wbfs_open_partition(__WBFS_ReadUSB, __WBFS_WriteUSB, NULL, sector_size, GetSecCount(pos), GetLBAStart(pos), 0));
		else if (interface == &__io_sdhc)
			SetWbfsHandle(pos, wbfs_open_partition(__WBFS_ReadSDHC, __WBFS_WriteSDHC, NULL, sector_size, GetSecCount(pos), GetLBAStart(pos), 0));

		if(GetWbfsHandle(pos)) return true;
	}

	MountNameList[pos].clear();

	return false;
}

void PartitionHandle::UnMount(int pos)
{
	if(!interface) return;

	if(pos >= (int) MountNameList.size()) return;

	if(MountNameList[pos].size() == 0) return;

	char DeviceName[20];
	snprintf(DeviceName, sizeof(DeviceName), "%s:", MountNameList[pos].c_str());

	wbfs_t* wbfshandle = GetWbfsHandle(pos);
	if(wbfshandle) wbfs_close(wbfshandle);
	SetWbfsHandle(pos, NULL);
	WBFS_Close();

	fatUnmount(DeviceName);
	ntfsUnmount(DeviceName, true);
	ext2Unmount(DeviceName);

	//Remove mount name from the list
	MountNameList[pos].clear();
}

int PartitionHandle::FindPartitions()
{
	MASTER_BOOT_RECORD mbr;

	// Read the first sector on the device
	if(!interface->readSectors(0, 1, &mbr)) return 0;

	// Check if it's a RAW disc, without a partition table
	if(CheckRAW((VOLUME_BOOT_RECORD *)&mbr)) return 1;

	// Verify this is the device's master boot record
	if(mbr.signature != MBR_SIGNATURE) return 0;

	for (int i = 0; i < 4; i++)
	{
		PARTITION_RECORD * partition = (PARTITION_RECORD *) &mbr.partitions[i];
		VOLUME_BOOT_RECORD vbr;

		if (le32(partition->lba_start) == 0) continue; // Invalid partition

		if(!interface->readSectors(le32(partition->lba_start), 1, &vbr)) continue;

		// Check if the partition is WBFS
		bool isWBFS = memcmp((u8 *)&vbr, WBFS_SIGNATURE, sizeof(WBFS_SIGNATURE)) == 0;

		if(!isWBFS && i == 0 && partition->type == PARTITION_TYPE_GPT_TABLE)
			return CheckGPT() ? PartitionList.size() : 0;

		if(!isWBFS && vbr.Signature != VBR_SIGNATURE && partition->type != 0x83) continue;

		if(!isWBFS && (partition->type == PARTITION_TYPE_DOS33_EXTENDED || partition->type == PARTITION_TYPE_WIN95_EXTENDED))
		{
			CheckEBR(i, le32(partition->lba_start));
			continue;
		}
		if(isWBFS || le32(partition->block_count) > 0)
		{
			PartitionFS PartitionEntry = {"0",0,0,0,0,0,0,0};
			PartitionEntry.FSName = isWBFS ? "WBFS" : PartFromType(partition->type);
			PartitionEntry.LBA_Start = le32(partition->lba_start);
			PartitionEntry.SecCount = isWBFS ? ((wbfs_head_t *)&vbr)->n_hd_sec : le32(partition->block_count);
			PartitionEntry.Bootable = (partition->status == PARTITION_BOOTABLE);
			PartitionEntry.PartitionType = partition->type;
			PartitionEntry.PartitionNum = i;
			PartitionEntry.EBR_Sector = 0;

			PartitionList.push_back(PartitionEntry);
		}
	}
	return PartitionList.size();
}

void PartitionHandle::CheckEBR(u8 PartNum, sec_t ebr_lba)
{
	EXTENDED_BOOT_RECORD ebr;
	sec_t next_erb_lba = 0;

	do
	{
		// Read and validate the extended boot record
		if(!interface->readSectors(ebr_lba + next_erb_lba, 1, &ebr)) return;

		// Check if the partition is WBFS
		bool isWBFS = memcmp((u8 *)&ebr, WBFS_SIGNATURE, sizeof(WBFS_SIGNATURE)) == 0;

		if(!isWBFS && ebr.signature != EBR_SIGNATURE) return;
		
		if(isWBFS || le32(ebr.partition.block_count) > 0)
		{
			PartitionFS PartitionEntry = {"0",0,0,0,0,0,0,0};
			PartitionEntry.FSName = isWBFS ? "WBFS" : PartFromType(ebr.partition.type);
			PartitionEntry.LBA_Start = ebr_lba + next_erb_lba + le32(ebr.partition.lba_start);
			PartitionEntry.SecCount = isWBFS ? ((wbfs_head_t *)&ebr)->n_hd_sec : le32(ebr.partition.block_count);
			PartitionEntry.Bootable = (ebr.partition.status == PARTITION_BOOTABLE);
			PartitionEntry.PartitionType = ebr.partition.type;
			PartitionEntry.PartitionNum = PartNum;
			PartitionEntry.EBR_Sector = ebr_lba + next_erb_lba;

			PartitionList.push_back(PartitionEntry);
		}
		// Get the start sector of the current partition
		// and the next extended boot record in the chain
		next_erb_lba = le32(ebr.next_ebr.lba_start);
	}
	while(next_erb_lba > 0);
}

bool PartitionHandle::CheckGPT(void)
{
	GPT_PARTITION_TABLE gpt;
	bool success = false; // To return false unless at least 1 partition is verified

	if(!interface->readSectors(1, 33, &gpt)) return false;	// To read all 128 possible partitions

	// Verify this is the Primary GPT entry
	if(strncmp(gpt.magic, GPT_SIGNATURE, 8) != 0) 	return false;
	if(le32(gpt.Entry_Size) != 128)				return false;
	if(le64(gpt.Table_LBA) != 2)					return false;
	if(le64(gpt.Header_LBA) != 1)					return false;
	if(le64(gpt.First_Usable_LBA) != 34)			return false;
	if(gpt.Reserved != 0)							return false;

	VOLUME_BOOT_RECORD * vbr = new VOLUME_BOOT_RECORD;
	for(u8 i = 0; i < le32(gpt.Num_Entries) && PartitionList.size() <= 8; i++)
	{
		GUID_PARTITION_ENTRY * entry = (GUID_PARTITION_ENTRY *) &gpt.partitions[i];

		int Start = le64(entry->First_LBA);
		int End = le64(entry->Last_LBA);
		int Size = End - Start;

		if(!interface->readSectors(Start, 1, vbr)) continue;

		PartitionFS PartitionEntry = {"0",0,0,0,0,0,0,0};
		if(memcmp((u8 *)vbr + BPB_NTFS_ADDR, NTFS_SIGNATURE, sizeof(NTFS_SIGNATURE)) == 0)
		{
			PartitionEntry.FSName = "NTFS";
			PartitionEntry.PartitionType = 0x07;
			PartitionEntry.SecCount = le64(vbr->Number_of_Sectors);
		}
		else if(memcmp((u8 *)vbr + BPB_FAT32_ADDR, FAT_SIGNATURE, sizeof(FAT_SIGNATURE)) == 0)
		{
			PartitionEntry.FSName = "FAT32";
			PartitionEntry.PartitionType = 0x0c;
			PartitionEntry.SecCount = le16(vbr->bpb.FatSectors);
			if (PartitionEntry.SecCount == 0)
				PartitionEntry.SecCount = le32(vbr->bpb.Large_Sectors);
		}
		else if(memcmp((u8 *)vbr + BPB_FAT16_ADDR, FAT_SIGNATURE, sizeof(FAT_SIGNATURE)) == 0)
		{
			PartitionEntry.FSName = "FAT16";
			PartitionEntry.PartitionType = 0x0e;
			
			PartitionEntry.SecCount = le16(vbr->bpb.FatSectors);
			if (PartitionEntry.SecCount == 0)
				PartitionEntry.SecCount = le32(vbr->bpb.Large_Sectors);
		}
		else if(memcmp((u8 *)vbr, WBFS_SIGNATURE, sizeof(WBFS_SIGNATURE)) == 0)
		{
			PartitionEntry.FSName = "WBFS";
			PartitionEntry.SecCount = ((wbfs_head_t *)vbr)->n_hd_sec;
		}
		else
		{
			bzero(&PartitionEntry, sizeof(PartitionFS));
			if(interface->readSectors(Start + 1, 1, vbr))
			{
				if(memcmp((u8 *)vbr + BPB_EXT2_ADDR, EXT_SIGNATURE, sizeof(EXT_SIGNATURE)) == 0)
				{
					PartitionEntry.FSName = "LINUX";
					PartitionEntry.PartitionType = 0x83;
					PartitionEntry.SecCount = Size;
				}
				else continue;
			}
			else continue;
		}

		if(PartitionEntry.SecCount != 0 && PartitionEntry.FSName[0] != '0')
		{
			PartitionEntry.LBA_Start = Start;
			PartitionEntry.PartitionNum = i;

			success = true;
			PartitionList.push_back(PartitionEntry);
		}
	}
	return success;
}

bool PartitionHandle::CheckRAW(VOLUME_BOOT_RECORD * vbr)
{
	PartitionFS PartitionEntry = {"0",0,0,0,0,0,0,0};
	if(memcmp((u8 *)vbr + BPB_NTFS_ADDR, NTFS_SIGNATURE, sizeof(NTFS_SIGNATURE)) == 0)
	{
		PartitionEntry.FSName = "NTFS";
		PartitionEntry.PartitionType = 0x07;
		PartitionEntry.SecCount = le64(vbr->Number_of_Sectors);
	}
	else if(memcmp((u8 *)vbr + BPB_FAT32_ADDR, FAT_SIGNATURE, sizeof(FAT_SIGNATURE)) == 0)
	{
		PartitionEntry.FSName = "FAT32";
		PartitionEntry.PartitionType = 0x0c;
		PartitionEntry.SecCount = le16(vbr->bpb.FatSectors);
		if (PartitionEntry.SecCount == 0)
			PartitionEntry.SecCount = le32(vbr->bpb.Large_Sectors);
	}
	else if(memcmp((u8 *)vbr + BPB_FAT16_ADDR, FAT_SIGNATURE, sizeof(FAT_SIGNATURE)) == 0)
	{
		PartitionEntry.FSName = "FAT16";
		PartitionEntry.PartitionType = 0x0e;
		PartitionEntry.SecCount = le16(vbr->bpb.FatSectors);
		if (PartitionEntry.SecCount == 0)
			PartitionEntry.SecCount = le32(vbr->bpb.Large_Sectors);
	}
	else if(memcmp((u8 *)vbr, WBFS_SIGNATURE, sizeof(WBFS_SIGNATURE)) == 0)
	{
		PartitionEntry.FSName = "WBFS";
		PartitionEntry.SecCount = ((wbfs_head_t *)vbr)->n_hd_sec;
	}

	if(PartitionEntry.FSName[0] != '0')
	{
		PartitionList.push_back(PartitionEntry);
		return true;
	}
	return false;
}