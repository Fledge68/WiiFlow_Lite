
#include "mem2.hpp"
#include "mem2alloc.hpp"
#include "gecko.h"

#include <malloc.h>
#include <string.h>
#include <ogc/system.h>

#ifndef APPLOADER_START		/* Also defined in disc.h */
#define APPLOADER_START (void *)0x81200000
#endif
#ifndef APPLOADER_END		/* Also defined in disc.h */
#define APPLOADER_END (void *)0x81700000
#endif

#define MAX_MEM1_ARENA_LO	((void *) (((u32)APPLOADER_START)-size))
#define MEM2_PRIORITY_SIZE	0x1000

// Forbid the use of MEM2 through malloc
u32 MALLOC_MEM2 = 0;

static CMEM2Alloc g_mem2gp;
static CMEM2Alloc g_mem1Lgp;
static CMEM2Alloc g_mem1Ugp;

extern int __init_start;
extern int _end;

extern "C"
{
	void MEM2_init(unsigned int mem2Size)
	{
		if(&_end + 0x100 > APPLOADER_START) gprintf("ZOMG MOVE THE ENTRYPOINT DOWN!\n");

		g_mem2gp.init(mem2Size);
		g_mem2gp.clear();

		/* If these are used, they must be cleared before running the apploader */

		/* Below executable */
		g_mem1Lgp.init((void *)0x80004000, &__init_start - 0x100);
		g_mem1Lgp.clear();

		/* Above Executable */
		g_mem1Ugp.init(APPLOADER_START, APPLOADER_END);
		g_mem1Ugp.clear();
		
		/* Protect space reserved for apploader */
		SYS_SetArena1Hi(APPLOADER_START);
	}

	void MEM2_cleanup(void)
	{
		g_mem2gp.cleanup();
	}

	void MEM2_clear(void)
	{
		g_mem2gp.clear();
	}

	void MEM1_cleanup(void)
	{
		g_mem1Lgp.cleanup();
		g_mem1Ugp.cleanup();
	}

	void MEM1_clear(void)
	{
		g_mem1Lgp.clear();
		g_mem1Ugp.clear();
	}

	void *MEM2_alloc(unsigned int s)
	{
		return g_mem2gp.allocate(s);
	}

	void *MEM1_alloc(unsigned int s)
	{
		if(g_mem1Lgp.FreeSize() >= s)
			return g_mem1Lgp.allocate(s);
		if(g_mem1Ugp.FreeSize() >= s)
			return g_mem1Ugp.allocate(s);

		return NULL;
	}

	void MEM2_free(void *p)
	{
		g_mem2gp.release(p);
	}

	void MEM1_free(void *p)
	{
		if((u32)p < (u32)&__init_start - 0x100 && (u32)p >= 0x80004000)
			g_mem1Lgp.release(p);
		else if((u32)p > (u32)APPLOADER_START && (u32)p < (u32)APPLOADER_END)
			g_mem1Ugp.release(p);
	}

	void *MEM2_realloc(void *p, unsigned int s)
	{
		return g_mem2gp.reallocate(p, s);
	}

	void *MEM1_realloc(void *p, unsigned int s)
	{
		if((u32)p < (u32)&__init_start - 0x100 && (u32)p >= 0x80004000)
			return g_mem1Lgp.reallocate(p, s);
		else if((u32)p > (u32)APPLOADER_START && (u32)p < (u32)APPLOADER_END)
			return g_mem1Ugp.reallocate(p, s);
		
		return NULL;
	}

	unsigned int MEM2_usableSize(void *p)
	{
		return CMEM2Alloc::usableSize(p);
	}

	unsigned int MEM2_freesize()
	{
		return g_mem2gp.FreeSize();
	}

	unsigned int MEM1_freesize()
	{
		return g_mem1Lgp.FreeSize() + g_mem1Ugp.FreeSize();
	}

	extern __typeof(malloc) __real_malloc;
	extern __typeof(calloc) __real_calloc;
	extern __typeof(realloc) __real_realloc;
	extern __typeof(memalign) __real_memalign;
	extern __typeof(free) __real_free;
	extern __typeof(malloc_usable_size) __real_malloc_usable_size;

	void *__wrap_malloc(size_t size)
	{
		void *p;
		if ((SYS_GetArena1Lo() >= MAX_MEM1_ARENA_LO) || size >= MEM2_PRIORITY_SIZE)
		{
			p = MEM2_alloc(size);
			return p != 0 ? p : __real_malloc(size);
		}
		p = __real_malloc(size);
		return p != 0 ? p : MEM2_alloc(size);
	}

	void *__wrap_calloc(size_t n, size_t size)
	{
		void *p;
		if ((SYS_GetArena1Lo() >= MAX_MEM1_ARENA_LO) || (n * size) >= MEM2_PRIORITY_SIZE)
		{
			p = MEM2_alloc(n * size);
			if (p != 0)
			{
				memset(p, 0, n * size);
				return p;
			}
			return __real_calloc(n, size);
		}

		p = __real_calloc(n, size);
		if (p != 0) return p;

		p = MEM2_alloc(n * size);
		if (p != 0) memset(p, 0, n * size);
		return p;
	}

	void *__wrap_memalign(size_t a, size_t size)
	{
		void *p;
		if ((SYS_GetArena1Lo() >= MAX_MEM1_ARENA_LO) || size >= MEM2_PRIORITY_SIZE)
		{
			if (a <= 32 && 32 % a == 0)
			{
				p = MEM2_alloc(size);
				if (p != 0) return p;
			}
			return __real_memalign(a, size);
		}
		p = __real_memalign(a, size);
		return p != 0 ? p : MEM2_alloc(size);
	}

	void __wrap_free(void *p)
	{
		if(!p)
			return;
		if(((u32)p < (u32)&__init_start - 0x100 && (u32)p >= (u32)0x80004000) || ((u32)p > (u32)APPLOADER_START && (u32)p < (u32)APPLOADER_END))
			MEM1_free(p);
		else if((u32)p & 0x10000000)
			MEM2_free(p);
		else		
			__real_free(p);
	}

	void *__wrap_realloc(void *p, size_t size)
	{
		void *n;
		// ptr from mem2
		if (((u32)p & 0x10000000) != 0 || (p == 0 && size > MEM2_PRIORITY_SIZE))
		{
			n = MEM2_realloc(p, size);
			if (n != 0) {
				return n;
			}
			n = __real_malloc(size);
			if (n == 0) {
				return 0;
			}
			if (p != 0)
			{
				memcpy(n, p, MEM2_usableSize(p) < size ? MEM2_usableSize(p) : size);
				MEM2_free(p);
			}
			return n;
		}
		// ptr from malloc
		n = __real_realloc(p, size);
		if (n != 0) {
			return n;
		}
		n = MEM2_alloc(size);
		if (n == 0) {
			return 0;
		}
		if (p != 0)
		{
			memcpy(n, p, __real_malloc_usable_size(p) < size ? __real_malloc_usable_size(p) : size);
			__real_free(p);
		}
		return n;
	}

	size_t __wrap_malloc_usable_size(void *p)
	{
		return ((u32)p & 0x10000000) != 0 ? MEM2_usableSize(p) : __real_malloc_usable_size(p);
	}

} ///extern "C"