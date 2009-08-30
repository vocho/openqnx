/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 * 
 * You must obtain a written license from and pay applicable license fees to QNX 
 * Software Systems before you may reproduce, modify or distribute this software, 
 * or any work that includes all or part of this software.   Free development 
 * licenses are available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *  
 * This file may contain contributions from others.  Please review this entire 
 * file for other proprietary rights or license notices, as well as the QNX 
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
 * for other information.
 * $
 */




	.text

	.extern	_SDA_BASE_
	.extern	_SDA2_BASE_
	.extern _CMain
	.extern _init_libc
	.extern _preinit_array
	.extern _init
	.extern _init_array
	.extern _fini
	.extern _fini_array

	.globl	_start
_start:	
#ifndef __PIC__
	lis		%r13,_SDA_BASE_@ha	
	la		%r13,_SDA_BASE_@l(%r13)
	lis		%r2,_SDA2_BASE_@ha	
	la		%r2,_SDA2_BASE_@l(%r2)
	mr		%r20, %r1
	mr		%r21, %r3
	mr		%r22, %r4
	mr		%r23, %r5
	mr		%r24, %r6
	mr		%r25, %r7
	bl		_init_libc
	lis		%r3,__preinit_array_start@ha
	la		%r3,__preinit_array_start@l(%r3)
	lis		%r4,__preinit_array_end@ha
	la		%r4,__preinit_array_end@l(%r4)
	lis		%r9,_preinit_array@ha
	la		%r9,_preinit_array@l(%r9)
	mtlr	%r9
	blrl
	lis		%r3,_init@ha
	la		%r3,_init@l(%r3)
	mtlr	%r3
	blrl
	lis		%r3,__init_array_start@ha
	la		%r3,__init_array_start@l(%r3)
	lis		%r4,__init_array_end@ha
	la		%r4,__init_array_end@l(%r4)
	lis		%r9,_init_array@ha
	la		%r9,_init_array@l(%r9)
	mtlr	%r9
	blrl
	lis		%r3,_btext@ha
	la		%r3,_btext@l(%r3)
	lis		%r4,_etext@ha
	la		%r4,_etext@l(%r4)
	bl		monstartup
	lis		%r3,_mcleanup@ha
	la		%r3,_mcleanup@l(%r3)
	bl		atexit
	lis		%r3,_fini@ha
	la		%r3,_fini@l(%r3)
	bl		atexit
	lis		%r3,__fini_array_start@ha
	la		%r3,__fini_array_start@l(%r3)
	lis		%r4,__fini_array_end@ha
	la		%r4,__fini_array_end@l(%r4)
	lis		%r9,_fini_array@ha
	la		%r9,_fini_array@l(%r9)
	mtlr	%r9
	blrl
	mr		%r1, %r20
	mr		%r3, %r21
	mr		%r4, %r22
	mr		%r5, %r23
	mr		%r6, %r24
	mr		%r7, %r25
	lis		%r9,main@ha
	la		%r9,main@l(%r9)
	mtlr	%r9
	blrl
	lis		%r9,exit@ha
	la		%r9,exit@l(%r9)
	mtlr	%r9
	blrl
#else
	bl		_GLOBAL_OFFSET_TABLE_@local-4
	mflr		%r11
	lwz		%r13,_SDA_BASE_@got(%r11)   // Load short data ptr.
	lwz		%r2,_SDA2_BASE_@got(%r11)
	mr		%r21, %r3
	mr		%r22, %r4
	mr		%r23, %r5
	mr		%r24, %r6
	mr		%r25, %r7
	bl		_init_libc@plt
	lwz		%r3,_init@got(%r11)
	mtlr	%r3
	blrl
	lwz		%r3,.text@got(%r11)
	lwz		%r4,_etext@got(%r11)
	lwz		%r26,_mcleanup@got(%r11)
	bl		monstartup@plt
	mr		%r3, %r26
	bl		atexit@plt
	lwz		%r3,_fini@got(%r11)
	bl		atexit@plt
	mr		%r3, %r21
	mr		%r4, %r22
	mr		%r5, %r23
	mr		%r6, %r24
	mr		%r7, %r25
	bl		main@plt
	bl		exit@plt
#endif
	
	.size _start,.-_start
	.type _start,@function
