/****************************************************************************
 * Snes9x Nintendo Wii/Gamecube Port
 *
 * Tantric 2010
 *
 * mem2.cpp
 *
 * MEM2 memory allocator
 ***************************************************************************/

#ifdef HW_RVL

#include <ogc/machine/asm.h>
#include <ogc/lwp_heap.h>
#include <ogc/system.h>
#include <ogc/machine/processor.h>

static heap_cntrl mem2_heap;

u32 InitMem2Manager () 
{
	int size = (12*1024*1024);
	u32 level;
	_CPU_ISR_Disable(level);
	size &= ~0x1f; // round down, because otherwise we may exceed the area
	void *mem2_heap_ptr = (void *)((u32)SYS_GetArena2Hi()-size);
	SYS_SetArena2Hi(mem2_heap_ptr);
	_CPU_ISR_Restore(level);
	size = __lwp_heap_init(&mem2_heap, mem2_heap_ptr, size, 32);
	return size;
}

void* mem2_malloc(u32 size)
{
	return __lwp_heap_allocate(&mem2_heap, size);
}

bool mem2_free(void *ptr)
{
	return __lwp_heap_free(&mem2_heap, ptr);
}

#endif
