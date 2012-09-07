// 1 MEM2 allocator, one for general purpose
// Aligned and padded to 32 bytes, as required by many functions

#ifndef __MEM2_HPP
#define __MEM2_HPP

#ifdef __cplusplus
extern "C"
{
#endif

void MEM_init();

void *MEM1_lo_alloc(unsigned int s);
void MEM1_lo_free(void *p);

void *MEM1_alloc(unsigned int s);
void *MEM1_memalign(unsigned int a, unsigned int s);
void *MEM1_realloc(void *p, unsigned int s);
void MEM1_free(void *p);
unsigned int MEM1_freesize();

void MEM2_cleanup(void);
void MEM2_clear(void);
void MEM2_free(void *p);
void *MEM2_alloc(unsigned int s);
void *MEM2_memalign(unsigned int /* alignment */, unsigned int s);
void *MEM2_realloc(void *p, unsigned int s);
unsigned int MEM2_usableSize(void *p);
unsigned int MEM2_freesize();

#ifdef __cplusplus
}
#endif

enum Alloc 
{ 
	ALLOC_MALLOC, 
	ALLOC_MEM2, 
};

#endif // !defined(__MEM2_HPP)