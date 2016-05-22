/****************************************************************************
 * Snes9x Nintendo Wii/Gamecube Port
 *
 * Tantric 2010
 *
 * mem2.h
 *
 * MEM2 memory allocator
 ***************************************************************************/

#ifdef HW_RVL

#ifndef _MEM2MANAGER_H_
#define _MEM2MANAGER_H_

u32 InitMem2Manager ();
void* mem2_malloc(u32 size);
bool mem2_free(void *ptr);

#endif

#endif
