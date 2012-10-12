/*
	TinyLoad - a simple region free (original) game launcher in 4k

# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

OUTPUT_FORMAT("elf32-powerpc")
OUTPUT_ARCH(powerpc:common)

ENTRY(_start)

PHDRS {
	app		PT_LOAD FLAGS(7);
}

SECTIONS {
	. = 0x80F00000;

	.text : { *(.text) *(.text.*) } :app

	.data : { *(.data) *(.data.*) }
	.rodata : { *(.rodata) *(.rodata.*) }
	//.sdata : { *(.sdata) *(.sdata.*) }
	__bss_start = .;
	//.sbss : { *(.sbss) *(.sbss.*) }
	.bss : { *(.bss) *(.bss.*) }
	__bss_end = .;

	. = ALIGN(0x40);
	.stack : {
		. = . + 0x48000;
		_stack_top = .;
	}
}

