#include "types.h"
#include "string.h"
#include "cache.h"
#include "ios.h"
#include "usbgecko.h"
void _main(void)
{
	usbgecko_init();
	ios_cleanup(); //hopefully that wont disable any features
	gprintf("Copying External Booter...\n");
	u8 *start = (u8*)0x80A80000;
	_memcpy(start, (u8*)0x90110000, 0xF0000); //960kb safe copying of booter
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
#include "memory.h"
#define SYSCALL_VECTOR	((u8*)0x80000C00)
void __init_syscall()
{
	u8* sc_vector = SYSCALL_VECTOR;
	u32 bytes = (u32)DCFlashInvalidate - (u32)__temp_abe;
	u8* from = (u8*)__temp_abe;
	for ( ; bytes != 0 ; --bytes )
	{
		*sc_vector = *from;
		sc_vector++;
		from++;
	}

	sync_after_write(SYSCALL_VECTOR, 0x100);
	ICInvalidateRange(SYSCALL_VECTOR, 0x100);
}