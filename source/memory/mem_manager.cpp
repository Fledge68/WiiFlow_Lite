/****************************************************************************
 * Copyright (C) 2013 FIX94
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include <string.h>
#include <algorithm>
#include "mem_manager.hpp"
#include "gecko/gecko.hpp"
#include "loader/utils.h"

static mutex_t memMutex = 0;
static const u32 MEM_BLOCK_SIZE = 256;

void MemMutexInit()
{
	LWP_MutexInit(&memMutex, 0);
}

void MemMutexDestroy()
{
	LWP_MutexDestroy(memMutex);
	memset(&memMutex, 0, sizeof(mutex_t));
}

MemManager::MemManager()
{
	startAddr = NULL;
	memList = NULL;
	memListEnd = NULL;
	memSize = 0;
}

void MemManager::Init(u8 *start, u8 *list, u32 size)
{
	LWP_MutexLock(memMutex);
	//gprintf("Init %x %i\n", memList, memSize);

	startAddr = start;
	memList = list;
	memSize = ALIGN(MEM_BLOCK_SIZE, size) / MEM_BLOCK_SIZE;
	memListEnd = list + memSize;
	ICInvalidateRange(memList, memSize+1);
	memset(memList, MEM_FREE , memSize);
	memset(memListEnd, MEM_END, 1); //thats the +1
	DCFlushRange(memList, memSize+1);

	LWP_MutexUnlock(memMutex);
}

void MemManager::ClearMem()
{
	LWP_MutexLock(memMutex);
	//gprintf("ClearMem %x %i\n", startAddr, memSize * MEM_BLOCK_SIZE);

	u32 MemFull = memSize * MEM_BLOCK_SIZE;
	ICInvalidateRange(startAddr, MemFull);
	memset(startAddr, 0, MemFull);
	DCFlushRange(startAddr, MemFull);

	LWP_MutexUnlock(memMutex);
}

void *MemManager::Alloc(u32 size)
{
	size = ALIGN(MEM_BLOCK_SIZE, size) / MEM_BLOCK_SIZE;
	if(size > memSize)
		return NULL;

	LWP_MutexLock(memMutex);

	vu8 *tmp_block;
	u32 blocksFree;
	for(vu8 *block = memList; block < memListEnd; block++)
	{
		for( ; *block & (ALLOC_USED | ALLOC_END); block++) ;
		blocksFree = 0;
		for(tmp_block = block; *tmp_block == MEM_FREE; tmp_block++)
		{
			blocksFree++;
			if(blocksFree == size)
			{
				u8 *addr = (u8*)block;
				ICInvalidateRange(addr, blocksFree);
				memset(addr, ALLOC_USED, blocksFree - 1); //start blocks
				memset(addr + blocksFree - 1, ALLOC_END, 1); //end block
				DCFlushRange(addr, blocksFree);
				void *ptr = (void*)(startAddr + ((addr - memList)*MEM_BLOCK_SIZE));
				//gprintf("Alloc %x mem, %i blocks\n", ptr, blocksFree);
				LWP_MutexUnlock(memMutex);
				return ptr;
			}
		}
		block = tmp_block;
	}
	LWP_MutexUnlock(memMutex);
	return NULL;
}

void MemManager::Free(void *mem)
{
	vu8 *blockUsed = memList+(((u8*)mem-startAddr) / MEM_BLOCK_SIZE);
	if(blockUsed < memList || blockUsed >= memListEnd || *blockUsed == 0)
		return;

	LWP_MutexLock(memMutex);

	//gprintf("Free %x mem, %x block\n", mem, blockUsed);

	u32 size = 0;
	vu8 *tmp_block = blockUsed;
	for( ; *tmp_block == ALLOC_USED; tmp_block++)
		size++;
	if(*tmp_block == ALLOC_END)
		size++;

	u8 *addr = (u8*)blockUsed;
	ICInvalidateRange(addr, size);
	memset(addr, MEM_FREE, size);
	DCFlushRange(addr, size);

	LWP_MutexUnlock(memMutex);
}

u32 MemManager::MemBlockSize(void *mem)
{
	vu8 *blockUsed = memList+(((u8*)mem-startAddr) / MEM_BLOCK_SIZE);
	if(blockUsed < memList || blockUsed >= memListEnd || *blockUsed == 0)
		return 0;

	LWP_MutexLock(memMutex);

	u32 size = 0;
	for( ; *blockUsed == ALLOC_USED; blockUsed++)
		size++;
	if(*blockUsed == ALLOC_END)
		size++;
	size *= MEM_BLOCK_SIZE;

	LWP_MutexUnlock(memMutex);

	return size;
}

u32 MemManager::FreeSize()
{
	LWP_MutexLock(memMutex);

	u32 free = 0;
	for(vu8 *block = memList; block < memListEnd; block++)
	{
		for( ; *block == MEM_FREE; block++)
			free++;
	}
	free *= MEM_BLOCK_SIZE;

	LWP_MutexUnlock(memMutex);

	return free;
}

void *MemManager::ReAlloc(void *mem, u32 size)
{
	if(mem == NULL)
		return Alloc(size);

	//gprintf("Realloc %x, %i\n", mem, size);
	void *new_m = Alloc(size);
	if(new_m == NULL)
	{
		Free(mem);
		return NULL;
	}
	const u32 copysize = std::min(MemBlockSize(mem), size);

	LWP_MutexLock(memMutex);
	memcpy(new_m, mem, copysize);
	LWP_MutexUnlock(memMutex);

	Free(mem);

	return new_m;
}
