#include <stdio.h>
#include <stdarg.h>
#include "string.h"
#include "sync.h"
typedef void (*entrypoint) (void);
unsigned char *cfg = (unsigned char*)0x80A7FFF0;
unsigned char *start = (unsigned char*)0x80A80000;
unsigned char *buffer = (unsigned char*)0x90100000;
void _main(void)
{
	entrypoint exeEntryPoint = (entrypoint)start;
	memcpy(cfg, buffer, 0x01000000); //1mb safe copying of cfg and booter
	sync_before_exec(cfg, 0x01000000);
	exeEntryPoint();
}