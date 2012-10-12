/*
	TinyLoad - a simple region free (original) game launcher in 4k

# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

/* This code comes from the Twilight Hack */
// Copyright 2008-2009  Segher Boessenkool  <segher@kernel.crashing.org>

	.globl _start
_start:

	# Disable interrupts
	mfmsr 3 ; rlwinm 3,3,0,17,15 ; mtmsr 3 ; isync

	# Setup stack.
	lis 1,_stack_top@ha ; addi 1,1,_stack_top@l ; li 0,0 ; stwu 0,-64(1)

#if !defined(TINY) || defined(DEBUG)
	# Clear BSS.
	lis 3,__bss_start@ha ; addi 3,3,__bss_start@l
#if defined(TINY)
	lis 4,0xDEAD
	ori 4,4,0xBEEF
#else
	li 4,0
#endif
	lis 5,__bss_end@ha ; addi 5,5,__bss_end@l ; sub 5,5,3
	bl memset32
#endif

	# Go!
	bl _main

#ifndef TINY
	# If it returns, hang.  Shouldn't happen.
	b .
#endif
