#include "types.h"
#include "string.h"
#include "cache.h"
#include "ios.h"
#include "usbgecko.h"
u8 *start = (u8*)0x80A80000;
u8 *buffer = (u8*)0x90110000;
void _main(void)
{
	usbgecko_init();
	ios_cleanup(); //hopefully that wont disable any features
	gprintf("Copying External Booter...\n");
	_memcpy(start, buffer, 0xF0000); //960kb safe copying of booter
	sync_after_write(start, 0xF0000);
	gprintf("Done! Jumping to Entrypoint...\n");
	asm volatile (
		"lis %r3, start@h\n"
		"ori %r3, %r3, start@l\n"
		"lwz %r3, 0(%r3)\n"
		"mtlr %r3\n"
		"blr\n"
	);
}