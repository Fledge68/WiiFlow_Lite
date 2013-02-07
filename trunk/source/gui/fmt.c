
// Simplified use of sprintf
#include "fmt.h"
#include <gccore.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

int currentStr = 0;
static char fmt_buffer[MAX_USES][MAX_MSG_SIZE];
char *fmt(const char *format, ...)
{
	va_list va;
	va_start(va, format);
	currentStr = (currentStr + 1) % MAX_USES;
	vsnprintf(fmt_buffer[currentStr], MAX_MSG_SIZE - 1, format, va);
	fmt_buffer[currentStr][MAX_MSG_SIZE - 1] = '\0';
	va_end(va);
	return fmt_buffer[currentStr];
}

void Asciify(wchar_t *str)
{
	const wchar_t *ptr = str;
	wchar_t *ctr = str;

	while(*ptr != '\0')
	{
		switch(*ptr)
		{
			case 0x14c:
				*ctr = 0x4f;
				break;
		}
		*ctr = *ptr;
		++ptr;
		++ctr;
	}
	*ctr = '\0';
}

void Asciify2(char *str)
{
	u8 i = 0;
	for(i = 0; i < strlen(str); ++i)
	{
		if(str[i] < 0x20 || str[i] > 0x7F)
			str[i] = '_';
		else switch(str[i])
		{
			case '*':
			case '\"':
			case '|':
			case '<':
			case '>':
			case '?':
			case ':':
				str[i] = '_';
				break;
			default:
				break;
		}
	}
}
