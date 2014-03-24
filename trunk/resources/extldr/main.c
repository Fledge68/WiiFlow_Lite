#include <stdio.h>
#include <stdarg.h>
#include "string.h"
#include "sync.h"
#include "usbgecko.h"
typedef void (*entrypoint) (void);
static entrypoint exeEntryPoint = (entrypoint)0x80A80000;
static unsigned char *start = (unsigned char*)0x80A80000;
static unsigned char *buffer = (unsigned char*)0x90110000;
void _main(void)
{
	usbgecko_init();
	gprintf("Copying External Booter...\n");
	memcpy(start, buffer, 0xF0000); //960kb safe copying of booter
	sync_before_exec(start, 0xF0000);
	gprintf("Done! Jumping to Entrypoint...\n");
	exeEntryPoint();
}