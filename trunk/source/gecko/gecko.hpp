
#ifndef _GECKO_HPP_
#define _GECKO_HPP_

#include <gccore.h>

void Gecko_Init(void);
void LogToSD_SetBuffer(bool buf);

#ifdef __cplusplus
extern "C" {
#endif

void gprintf(const char *format, ...);
void ghexdump(void *d, int len);

#ifdef __cplusplus
}
#endif

#endif
