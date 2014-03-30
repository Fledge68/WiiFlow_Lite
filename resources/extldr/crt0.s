# Copyright 2008-2009  Segher Boessenkool  <segher@kernel.crashing.org>
# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
.set r0,0;		.set r1,1;		.set r2,2;		.set r3,3;		.set r4,4;
.set r5,5;		.set r6,6;		.set r7,7;		.set r8,8;		.set r9,9;
.set r10,10;	.set r11,11;	.set r12,12;	.set r13,13;	.set r14,14;
.set r15,15;	.set r16,16;	.set r17,17;	.set r18,18;	.set r19,19;
.set r20,20;	.set r21,21;	.set r22,22;	.set r23,23;	.set r24,24;
.set r25,25;	.set r26,26;	.set r27,27;	.set r28,28;	.set r29,29;
.set r30,30;	.set r31,31;

.extern _main
	.globl _start
_start:
	# Disable interrupts
	mfmsr r3
	rlwinm r3,r3,0,17,15
	mtmsr r3
	isync

	# Reset our registers
	bl		__init_registers

	# Clear BSS.
	lis		r3,__bss_start@h
	ori		r3,r3,__bss_start@l
	li		r4,0
	lis		r5,__bss_end@h
	ori		r5,r5,__bss_end@l
	subf	r5,r3,r5
	bl		_memset

	# Go!
	bl		_main

#thanks to megazig for that
__init_registers:
	li      r0,0
	li      r3,0
	li      r4,0
	li      r5,0
	li      r6,0
	li      r7,0
	li      r8,0
	li      r9,0
	li      r10,0
	li      r11,0
	li      r12,0
	li      r14,0
	li      r15,0
	li      r16,0
	li      r17,0
	li      r18,0
	li      r19,0
	li      r20,0
	li      r21,0
	li      r22,0
	li      r23,0
	li      r24,0
	li      r25,0
	li      r26,0
	li      r27,0
	li      r28,0
	li      r29,0
	li      r30,0
	li      r31,0
	lis     r1,__stack_top@h
	ori     r1,r1,__stack_top@l
	addi    r1,r1,-4
	stw     r0,0(r1)
	stwu    r1,-0x38(r1)
	lis     r2,0
	ori     r2,r2,0x8000
	lis     r13,0
	ori     r13,r13,0x8000
	blr
