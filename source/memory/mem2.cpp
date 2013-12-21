
#include <malloc.h>
#include <string.h>
#include <ogc/system.h>
#include <ogc/machine/processor.h>

#include "mem2.hpp"
#include "mem_manager.hpp"
#include "gecko/gecko.hpp"
#include "loader/utils.h"

static const u32 MEM2_PRIORITY_SIZE = 0x1000;

// Forbid the use of MEM2 through malloc
u32 MALLOC_MEM2 = 0;

u8 *MEM1_lo_start = (u8*)0x80004000;
u8 *MEM1_lo_end = (u8*)0x80620000;
u8 *MEM1_lo_list = (u8*)0x93280000;

u8 *MEM2_start = (u8*)0x90200000;
u8 *MEM2_end = (u8*)0x93200000;
u8 *MEM2_list = (u8*)0x93200000;

MemManager g_mem1lo;
MemManager g_mem2gp;

extern "C"
{

extern __typeof(malloc) __real_malloc;
extern __typeof(calloc) __real_calloc;
extern __typeof(realloc) __real_realloc;
extern __typeof(memalign) __real_memalign;
extern __typeof(free) __real_free;
extern __typeof(malloc_usable_size) __real_malloc_usable_size;

void MEM_init()
{
	MemMutexInit();

	g_mem1lo.Init(MEM1_lo_start, MEM1_lo_list, (u32)(MEM1_lo_end-MEM1_lo_start)); //about 6mb
	g_mem1lo.ClearMem();

	g_mem2gp.Init(MEM2_start, MEM2_list, (u32)(MEM2_end-MEM2_start)); //about 48mb
	g_mem2gp.ClearMem();
}

void *MEM1_lo_alloc(unsigned int s)
{
	return g_mem1lo.Alloc(s);
}

void MEM1_lo_free(void *p)
{
	if(!p)
		return;
	g_mem1lo.Free(p);
}

unsigned int MEM1_lo_freesize()
{
	return g_mem1lo.FreeSize();
}


void *MEM1_alloc(unsigned int s)
{
	return __real_malloc(s);
}

void *MEM1_memalign(unsigned int a, unsigned int s)
{
	return __real_memalign(a, s);
}

void *MEM1_realloc(void *p, unsigned int s)
{
	return __real_realloc(p, s);
}

void MEM1_free(void *p)
{
	if(!p)
		return;
	__real_free(p);
}

unsigned int MEM1_freesize()
{
	return SYS_GetArena1Size();
}


void MEM2_free(void *p)
{
	if(!p)
		return;
	g_mem2gp.Free(p);
}

void *MEM2_alloc(unsigned int s)
{
	return g_mem2gp.Alloc(s);
}

/* Placeholder, will be needed with new memory manager */
void *MEM2_memalign(unsigned int /* alignment */, unsigned int s)
{
	return g_mem2gp.Alloc(s);
}

void *MEM2_realloc(void *p, unsigned int s)
{
	return g_mem2gp.ReAlloc(p, s);
}

unsigned int MEM2_usableSize(void *p)
{
	return g_mem2gp.MemBlockSize(p);
}

unsigned int MEM2_freesize()
{
	return g_mem2gp.FreeSize();
}

void *__wrap_malloc(size_t size)
{
	void *p;
	if(size >= MEM2_PRIORITY_SIZE)
	{
		p = g_mem2gp.Alloc(size);
		if(p != 0) 
			return p;
		return __real_malloc(size);
	}
	p = __real_malloc(size);
	if(p != 0)
		return p;
	return g_mem2gp.Alloc(size);
}

void *__wrap_calloc(size_t n, size_t size)
{
	void *p;
	if((n * size) >= MEM2_PRIORITY_SIZE)
	{
		p = g_mem2gp.Alloc(n * size);
		if (p != 0)
		{
			memset(p, 0, n * size);
			return p;
		}
		return __real_calloc(n, size);
	}

	p = __real_calloc(n, size);
	if (p != 0) return p;

	p = g_mem2gp.Alloc(n * size);
	if (p != 0)
		memset(p, 0, n * size);
	return p;
}

void *__wrap_memalign(size_t a, size_t size)
{
	void *p;
	if(size >= MEM2_PRIORITY_SIZE)
	{
		if(a <= 32 && 32 % a == 0)
		{
			p = g_mem2gp.Alloc(size);
			if (p != 0)
				return p;
		}
		return __real_memalign(a, size);
	}
	p = __real_memalign(a, size);
	if(p != 0 || a > 32 || 32 % a != 0)
		return p;

	return g_mem2gp.Alloc(size);
}

void __wrap_free(void *p)
{
	if(!p)
		return;

	if(((u32)p & 0x10000000) != 0)
	{
		//if(p > MEM2_start)
		g_mem2gp.Free(p);
		//else
		//g_mem2lo_gp.Free(p);
	}
	else
		MEM1_free(p);
}

void *__wrap_realloc(void *p, size_t size)
{
	void *n;
	// ptr from mem2
	if(((u32)p & 0x10000000) != 0 || (p == 0 && size > MEM2_PRIORITY_SIZE))
	{
		n = g_mem2gp.ReAlloc(p, size);
		if(n != 0)
			return n;
		n = __real_malloc(size);
		if(n == 0)
			return 0;
		if(p != 0)
		{
			memcpy(n, p, MEM2_usableSize(p) < size ? MEM2_usableSize(p) : size);
			g_mem2gp.Free(p);
		}
		return n;
	}
	// ptr from malloc
	n = __real_realloc(p, size);
	if(n != 0)
		return n;
	n = g_mem2gp.Alloc(size);
	if(n == 0)
		return 0;
	if(p != 0)
	{
		memcpy(n, p, __real_malloc_usable_size(p) < size ? __real_malloc_usable_size(p) : size);
		__real_free(p);
	}
	return n;
}

size_t __wrap_malloc_usable_size(void *p)
{
	if(((u32)p & 0x10000000) != 0)
		return g_mem2gp.MemBlockSize(p);
	return __real_malloc_usable_size(p);
}

} ///extern "C"
