//Enable the line below to always write SD log
//#define sd_write_log

#define filebuffer 1024

#include <gccore.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <sys/iosupport.h>

#include "wifi_gecko.h"

/* init-globals */
bool geckoinit = false;
bool textVideoInit = false;

bool bufferMessages = true;
bool WriteToSD = false;

#include <stdarg.h>

static ssize_t __out_write(struct _reent *r __attribute__((unused)), int fd __attribute__((unused)), const char *ptr, size_t len)
{
	if(geckoinit && ptr)
	{
		u32 level;
		level = IRQ_Disable();
		usb_sendbuffer(1, ptr, len);
		IRQ_Restore(level);
	}

	return len;
}

static const devoptab_t gecko_out = {
	"stdout",       // device name
	0,                      // size of file structure
	NULL,           // device open
	NULL,           // device close
	__out_write,// device write
	NULL,           // device read
	NULL,           // device seek
	NULL,           // device fstat
	NULL,           // device stat
	NULL,           // device link
	NULL,           // device unlink
	NULL,           // device chdir
	NULL,           // device rename
	NULL,           // device mkdir
	0,                      // dirStateSize
	NULL,           // device diropen_r
	NULL,           // device dirreset_r
	NULL,           // device dirnext_r
	NULL,           // device dirclose_r
	NULL,           // device statvfs_r
	NULL,           // device ftruncate_r
	NULL,           // device fsync_r
	NULL,           // device deviceData
	NULL,           // device chmod_r
	NULL,           // device fchmod_r
};

static void USBGeckoOutput()
{
	devoptab_list[STD_OUT] = &gecko_out;
	devoptab_list[STD_ERR] = &gecko_out;
}


char *tmpfilebuffer = NULL;
void WriteToFile(char* tmp)
{
	if(bufferMessages)
	{
		if((strlen(tmpfilebuffer) + strlen(tmp)) < filebuffer)
			strcat(tmpfilebuffer, tmp);
	}
	else
	{
		if(tmpfilebuffer != NULL)
		{
			SAFE_FREE(tmpfilebuffer);
			tmpfilebuffer = NULL;
		}
		return;
	}

	if(WriteToSD)
	{
		FILE *outfile = fopen("sd:/wiiflow.log", "a");
		if(outfile)
		{
			fwrite(tmpfilebuffer, 1, strlen(tmpfilebuffer), outfile);
			memset(tmpfilebuffer, 0, sizeof(tmpfilebuffer));
			fclose(outfile);
		}
	}
}

//using the gprintf from crediar because it is smaller than mine
void gprintf( const char *format, ... )
{
	char * tmp = NULL;
	va_list va;
	va_start(va, format);
	if((vasprintf(&tmp, format, va) >= 0) && tmp)
	{
		WriteToFile(tmp);
		WifiGecko_Send(tmp, strlen(tmp));
		if (geckoinit)
			__out_write(NULL, 0, tmp, strlen(tmp));
	}
	va_end(va);

	SAFE_FREE(tmp);
} 

char ascii(char s) {
  if(s < 0x20) return '.';
  if(s > 0x7E) return '.';
  return s;
}

void ghexdump(void *d, int len) {
  u8 *data;
  int i, off;
  data = (u8*)d;

  gprintf("\n       0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F  0123456789ABCDEF");
  gprintf("\n====  ===============================================  ================\n");

  for (off=0; off<len; off += 16)
  {
    gprintf("%04x  ",off);
    for(i=0; i<16; i++)
      if((i+off)>=len) gprintf("   ");
      else gprintf("%02x ",data[off+i]);

    gprintf(" ");
    for(i=0; i<16; i++)
      if((i+off)>=len) gprintf(" ");
      else gprintf("%c",ascii(data[off+i]));
    gprintf("\n");
  }
}

bool InitGecko()
{
	if (geckoinit) return geckoinit;

	USBGeckoOutput();

	tmpfilebuffer = (char*)calloc(filebuffer + 1, sizeof(char));

	#ifdef sd_write_log
		WriteToSD = true;
	#endif

	u32 geckoattached = usb_isgeckoalive(EXI_CHANNEL_1);
	if (geckoattached)
	{
		usb_flush(EXI_CHANNEL_1);
		return true;
	}
	else return false;
}
