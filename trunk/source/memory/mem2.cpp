
#include <malloc.h>
#include <string.h>
#include <ogc/system.h>

#include "mem2.hpp"
#include "mem2alloc.hpp"
#include "gecko.h"
#include "utils.h"

// Forbid the use of MEM2 through malloc
u32 MALLOC_MEM2 = 0;

static CMEM2Alloc g_mem2gp;

extern "C"
{

extern __typeof(malloc) __real_malloc;
extern __typeof(calloc) __real_calloc;
extern __typeof(realloc) __real_realloc;
extern __typeof(memalign) __real_memalign;
extern __typeof(free) __real_free;
extern __typeof(malloc_usable_size) __real_malloc_usable_size;

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
	__real_free(p);
}

unsigned int MEM1_freesize()
{
	return SYS_GetArena1Size();
}

void MEM2_init(unsigned int mem2Size)
{
	g_mem2gp.init(mem2Size);
	g_mem2gp.clear();
}

void MEM2_cleanup(void)
{
	g_mem2gp.cleanup();
}

void MEM2_clear(void)
{
	g_mem2gp.clear();
}

void MEM2_free(void *p)
{
	g_mem2gp.release(p);
}

void *MEM2_alloc(unsigned int s)
{
	return g_mem2gp.allocate(s);
}

void *MEM2_memalign(unsigned int a, unsigned int s)
{
	return g_mem2gp.allocate(ALIGN(a, s));
}

void *MEM2_realloc(void *p, unsigned int s)
{
	return g_mem2gp.reallocate(p, s);
}

unsigned int MEM2_usableSize(void *p)
{
	return g_mem2gp.usableSize(p);
}

unsigned int MEM2_freesize()
{
	return g_mem2gp.FreeSize();
}

void *__wrap_malloc(size_t size)
{
	void *p;
	if(SYS_GetArena1Lo() >= MAX_MEM1_ARENA_LO || size >= MEM2_PRIORITY_SIZE)
	{
		p = g_mem2gp.allocate(size);
		if(p != 0) 
			return p; 
		return __real_malloc(size);
	}
	p = __real_malloc(size);
	if(p != 0)
		return p;
	return g_mem2gp.allocate(size);
}

void *__wrap_calloc(size_t n, size_t size)
{
	void *p;
	if(SYS_GetArena1Lo() >= MAX_MEM1_ARENA_LO || (n * size) >= MEM2_PRIORITY_SIZE)
	{
		p = g_mem2gp.allocate(n * size);
		if (p != 0)
		{
			memset(p, 0, n * size);
			return p;
		}
		return __real_calloc(n, size);
	}

	p = __real_calloc(n, size);
	if (p != 0) return p;

	p = g_mem2gp.allocate(n * size);
	if (p != 0)
		memset(p, 0, n * size);
	return p;
}

void *__wrap_memalign(size_t a, size_t size)
{
	void *p;
	if(SYS_GetArena1Lo() >= MAX_MEM1_ARENA_LO || size >= MEM2_PRIORITY_SIZE)
	{
		p = MEM2_memalign(a, size);
		if (p != 0)
			return p;
		return __real_memalign(a, size);
	}
	p = __real_memalign(a, size);
	if(p != 0)
		return p;
	return MEM2_memalign(a, size);
}

void __wrap_free(void *p)
{
	if(!p)
		return;

	if(((u32)p & 0x10000000) != 0)
		g_mem2gp.release(p);
	else
		__real_free(p);
}

void *__wrap_realloc(void *p, size_t size)
{
	void *n;
	// ptr from mem2
	if (((u32)p & 0x10000000) != 0 || (p == 0 && size > MEM2_PRIORITY_SIZE))
	{
		n = g_mem2gp.reallocate(p, size);
		if (n != 0)
			return n;
		n = __real_malloc(size);
		if (n == 0)
			return 0;
		if (p != 0)
		{
			memcpy(n, p, MEM2_usableSize(p) < size ? MEM2_usableSize(p) : size);
			g_mem2gp.release(p);
		}
		return n;
	}
	// ptr from malloc
	n = __real_realloc(p, size);
	if (n != 0)
		return n;
	n = g_mem2gp.allocate(size);
	if (n == 0)
		return 0;
	if (p != 0)
	{
		memcpy(n, p, __real_malloc_usable_size(p) < size ? __real_malloc_usable_size(p) : size);
		__real_free(p);
	}
	return n;
}

size_t __wrap_malloc_usable_size(void *p)
{
	if(((u32)p & 0x10000000) != 0)
		return CMEM2Alloc::usableSize(p);
	return __real_malloc_usable_size(p);
}

} ///extern "C"
