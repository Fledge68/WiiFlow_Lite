
#include <gccore.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <sys/iosupport.h>
#include <stdarg.h>

#include "gecko.hpp"
#include "memory/mem2.hpp"
#include "wifi_gecko.hpp"

#define GPRINTF_SIZE	256
#define SDWRITE_SIZE	1024

bool geckoinit = false;
bool sd_inited = false;
bool bufferMessages = true;

char gprintfBuffer[GPRINTF_SIZE];
char sdwritebuffer[SDWRITE_SIZE];

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

static const devoptab_t gecko_out = {
	"stdout",		// device name
	0,				// size of file structure
	NULL,			// device open
	NULL,			// device close
	__out_write,	// device write
	NULL,			// device read
	NULL,			// device seek
	NULL,			// device fstat
	NULL,			// device stat
	NULL,			// device link
	NULL,			// device unlink
	NULL,			// device chdir
	NULL,			// device rename
	NULL,			// device mkdir
	0,				// dirStateSize
	NULL,			// device diropen_r
	NULL,			// device dirreset_r
	NULL,			// device dirnext_r
	NULL,			// device dirclose_r
	NULL,			// device statvfs_r
	NULL,			// device ftruncate_r
	NULL,			// device fsync_r
	NULL,			// device deviceData
	NULL,			// device chmod_r
	NULL,			// device fchmod_r
};

static void USBGeckoOutput()
{
	devoptab_list[STD_OUT] = &gecko_out;
	devoptab_list[STD_ERR] = &gecko_out;
}

static char ascii(char s)
{
	if(s < 0x20)
		return '.';
	if(s > 0x7E)
		return '.';
	return s;
}

static void WriteToFile(const char* tmp, size_t len)
{
	if(bufferMessages == false)
		return;

	if((strlen(sdwritebuffer) + len) < SDWRITE_SIZE)
		strcat(sdwritebuffer, tmp);

	if(sd_inited == false)
		return;

	FILE *outfile = fopen("sd:/wiiflow.log", "a");
	if(outfile)
	{
		fwrite(sdwritebuffer, 1, strlen(sdwritebuffer), outfile);
		memset(sdwritebuffer, 0, SDWRITE_SIZE);
		fclose(outfile);
	}
}

void Gecko_Init(void)
{
	USBGeckoOutput();
	memset(sdwritebuffer, 0, SDWRITE_SIZE);
	memset(gprintfBuffer, 0, GPRINTF_SIZE);

	u32 geckoattached = usb_isgeckoalive(EXI_CHANNEL_1);
	if(geckoattached)
	{
		geckoinit = true;
		usb_flush(EXI_CHANNEL_1);
		const char *initstr = "USB Gecko inited.\n";
		__out_write(NULL, 0, initstr, strlen(initstr));
	}
}

void LogToSD_SetBuffer(bool buf)
{
	bufferMessages = buf;
	sd_inited = true;
}

#ifdef __cplusplus
extern "C"
{
#endif

void gprintf(const char *format, ...)
{
	va_list va;
	va_start(va, format);
	size_t len = vsnprintf(gprintfBuffer, GPRINTF_SIZE - 1, format, va);
	va_end(va);

	__out_write(NULL, 0, gprintfBuffer, len);
	WiFiDebugger.Send(gprintfBuffer, len);
	WriteToFile(gprintfBuffer, len);
}

void ghexdump(void *d, int len)
{
	u8 *data;
	int i, off;
	data = (u8*)d;

	gprintf("\n       0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F  0123456789ABCDEF");
	gprintf("\n====  ===============================================  ================\n");

	for (off = 0; off < len; off += 16)
	{
		gprintf("%04x  ",off);
		for(i = 0; i < 16; i++)
		{
			if((i+off)>=len)
				gprintf("   ");
			else
				gprintf("%02x ",data[off+i]);
		}
		gprintf(" ");
		for(i = 0; i < 16; i++)
		{
			if((i+off)>=len)
				gprintf(" ");
			else
				gprintf("%c",ascii(data[off+i]));
		}
		gprintf("\n");
	}
}

#ifdef __cplusplus
}
#endif
