
#ifndef _FMT_H_
#define _FMT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

enum {
	MAX_MSG_SIZE	= 1024,
	MAX_USES		= 8,
};

char *fmt(const char *format, ...);
void Asciify(wchar_t *str);
void Asciify2(char *str);

#ifdef __cplusplus
}
#endif

#endif //_FMT_H_
