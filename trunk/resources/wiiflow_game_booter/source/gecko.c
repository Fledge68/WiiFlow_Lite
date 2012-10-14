#include <gccore.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <sys/iosupport.h>
#include <stdarg.h>
#include "gecko.h"

/* init-globals */
bool geckoinit = false;
static ssize_t __out_write(struct _reent *r __attribute__((unused)), int fd __attribute__((unused)), const char *ptr, size_t len)
{
	if(geckoinit && ptr)
	{
		u32 level;
		level = IRQ_Disable();
		usb_sendbuffer_safe(1, ptr, len);
		IRQ_Restore(level);
	}
	return len;
}

char gprintfBuffer[256];
void gprintf(const char *format, ...)
{
	va_list va;
	va_start(va, format);
	int len = vsnprintf(gprintfBuffer, 255, format, va);
	__out_write(NULL, 0, gprintfBuffer, len);
	va_end(va);
}

bool InitGecko()
{
	if(geckoinit)
		return geckoinit;

	u32 geckoattached = usb_isgeckoalive(EXI_CHANNEL_1);
	if(geckoattached)
	{
		geckoinit = true;
		usb_flush(EXI_CHANNEL_1);
	}
	return geckoinit;
}
