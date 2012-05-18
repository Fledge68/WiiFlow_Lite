#include "smartptr.hpp"

SmartBuf smartMemAlign32(unsigned int size)
{
	return smartAnyAlloc(size);
}

SmartBuf smartMem2Alloc(unsigned int size)
{
	return SmartBuf((unsigned char *)MEM2_alloc(size), SmartBuf::SRCALL_MEM2);
}

SmartBuf smartMem1Alloc(unsigned int size)
{
	return SmartBuf((unsigned char *)MEM1_alloc(size), SmartBuf::SRCALL_MEM1);
}

SmartBuf smartAnyAlloc(unsigned int size)
{
	SmartBuf p(smartMem2Alloc(size));
	return !!p ? p : smartMem1Alloc(size);
}
