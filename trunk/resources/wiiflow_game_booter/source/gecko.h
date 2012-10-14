
#ifndef _GECKO_H_
#define _GECKO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <gccore.h>

extern bool bufferMessages;
extern bool WriteToSD;

//use this just like printf();
void gprintf(const char *format, ...);
void ghexdump(void *d, int len);
bool InitGecko();

#ifdef __cplusplus
}
#endif

#endif
