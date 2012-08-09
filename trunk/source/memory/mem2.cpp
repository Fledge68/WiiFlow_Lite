
#include <malloc.h>
#include <string.h>
#include <ogc/system.h>

#include "mem2.hpp"
#include "mem2alloc.hpp"
#include "gecko/gecko.h"
#include "loader/utils.h"

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

extern __typeof(_malloc_r) __real__malloc_r;
extern __typeof(_calloc_r) __real__calloc_r;
extern __typeof(_realloc_r) __real__realloc_r;
extern __typeof(_memalign_r) __real__memalign_r;
extern __typeof(_free_r) __real__free_r;
extern __typeof(_malloc_usable_size_r) __real__malloc_usable_size_r;

bool first_mem = true;

void DisableMEM1allocR()
{
	first_mem = false;
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

/* Placeholder, will be needed with new memory manager */
void *MEM2_memalign(unsigned int /* alignment */, unsigned int s)
{
	return MEM2_alloc(s);
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

void *__wrap__malloc_r(struct _reent *r, size_t size)
{
	if(first_mem)
		return __real__malloc_r(r, size);

	return MEM2_alloc(size);
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

void *__wrap__calloc_r(struct _reent *r, size_t n, size_t size)
{
	if(first_mem)
		return __real__calloc_r(r, n, size);

	void *p = MEM2_alloc(n*size);
	if(p)
		memset(p, 0, n*size);
	return p;
}

void *__wrap_memalign(size_t a, size_t size)
{
	void *p;
	if(SYS_GetArena1Lo() >= MAX_MEM1_ARENA_LO || size >= MEM2_PRIORITY_SIZE)
	{
		if(a <= 32 && 32 % a == 0)
		{
			p = g_mem2gp.allocate(size);
			if (p != 0)
				return p;
		}
		return __real_memalign(a, size);
	}
	p = __real_memalign(a, size);
	if(p != 0 || a > 32 || 32 % a != 0)
		return p;

	return g_mem2gp.allocate(size);
}

void *__wrap__memalign_r(struct _reent *r, size_t a, size_t size)
{
	if(first_mem)
		return __real__memalign_r(r, a, size);

	return MEM2_alloc(size);
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

void __wrap__free_r(struct _reent *r, void *p)
{
	if(first_mem)
		__real__free_r(r, p);
	else
		MEM2_free(p);
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

void *__wrap__realloc_r(struct _reent *r, void *p, size_t size)
{
	if(first_mem)
		return __real__realloc_r(r, p, size);

	return MEM2_realloc(p, size);
}

size_t __wrap_malloc_usable_size(void *p)
{
	if(((u32)p & 0x10000000) != 0)
		return CMEM2Alloc::usableSize(p);
	return __real_malloc_usable_size(p);
}

size_t __wrap__malloc_usable_size_r(struct _reent *r, void *p)
{
	if(first_mem)
		return __real__malloc_usable_size_r(r, p);

	return CMEM2Alloc::usableSize(p);
}

} ///extern "C"
