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
#ifndef _MEM_MANAGER_HPP_
#define _MEM_MANAGER_HPP_

#include <ogcsys.h>
#include <ogc/mutex.h>

enum mem_states
{
	MEM_FREE = (1<<0),
	ALLOC_USED = (1<<1),
	ALLOC_END = (1<<2),
	MEM_END = (1<<3),
};

class MemManager {
public:
	MemManager();
	void Init(u8 *start, u8 *list, u32 size);
	void ClearMem();
	void *Alloc(u32 size);
	void Free(void *mem);
	u32 MemBlockSize(void *mem);
	u32 FreeSize();
	void *ReAlloc(void *mem, u32 size);
private:
	u8 *startAddr;
	u8 *memList;
	u8 *memListEnd;
	u32 memSize;
};

void MemMutexInit();
void MemMutexDestroy();

#endif
