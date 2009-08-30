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



#ifndef NO_INIT_ARRAY
#define INIT_ARRAY
#endif
#ifndef NO_CLEAR_ERRNO
#define CLEAR_ERRNO
#endif

	.text

	.extern	_SDA_BASE_
	.extern	_SDA2_BASE_
	.extern main
	.extern _init_libc
#ifdef INIT_ARRAY
	.extern _preinit_array
	.extern _init_array
	.extern _fini_array
#endif
	.extern _init
	.extern _fini
#ifdef CLEAR_ERRNO
	.extern	errno
#endif

	.globl	_start
_start:	
// @@@ tmp until we figure this one out...
#undef __PIC__
#ifndef __PIC__
	lis		%r13,_SDA_BASE_@ha	
	la		%r13,_SDA_BASE_@l(%r13)
	lis		%r2,_SDA2_BASE_@ha	
	la		%r2,_SDA2_BASE_@l(%r2)
	stwu	%r1,-48(%r1)
	stw		%r3,16(%r1)
	stw		%r4,20(%r1)
	stw		%r5,24(%r1)
	stw		%r6,28(%r1)
	stw		%r7,32(%r1)
	stw		%r8,36(%r1)
	lis		%r9,_init_libc@ha
	la		%r9,_init_libc@l(%r9)
	mtlr		%r9
	blrl

#ifdef INIT_ARRAY
	lis		%r3,__preinit_array_start@ha
	la		%r3,__preinit_array_start@l(%r3)	
	lis		%r4,__preinit_array_end@ha
	la		%r4,__preinit_array_end@l(%r4)
	lis		%r9,_preinit_array@ha
	la		%r9,_preinit_array@l(%r9)
	mtlr	%r9
	blrl
#endif /* INIT_ARRAY */
	lis		%r3,_init@ha
	la		%r3,_init@l(%r3)
	mtlr	%r3
	blrl
#ifdef INIT_ARRAY
	lis		%r3,__init_array_start@ha
	la		%r3,__init_array_start@l(%r3)	
	lis		%r4,__init_array_end@ha
	la		%r4,__init_array_end@l(%r4)
	lis             %r9,_init_array@ha
	la              %r9,_init_array@l(%r9)
	mtlr    %r9
	blrl
#endif /* INIT_ARRAY */
	lis		%r3,_fini@ha
	la		%r3,_fini@l(%r3)
	lis		%r9,atexit@ha
	la		%r9,atexit@l(%r9)
	mtlr		%r9
	blrl
#ifdef INIT_ARRAY
	lis		%r3,__fini_array_start@ha
	la		%r3,__fini_array_start@l(%r3)	
	lis		%r4,__fini_array_end@ha
	la		%r4,__fini_array_end@l(%r4)
	lis             %r9,_fini_array@ha
	la              %r9,_fini_array@l(%r9)
	mtlr    %r9
	blrl
#endif /* INIT_ARRAY */
	lwz		%r3,16(%r1)
	lwz		%r4,20(%r1)
	lwz		%r5,24(%r1)
	lwz		%r6,28(%r1)
	addi	%r1,%r1,48

	li		%r0,0
	addi	%r1,%r1,-16
	stw		%r0,0(%r1)		// Zero the stack backlink for ABI conformity
	stw		%r0,4(%r1)		// Zero the LR save area for ABI conformity
	stwu	%r1,-16(%r1)	// Make a fake stack frame to link to the zeros
	stw		%r0,4(%r1)		// Zero the LR save area for ABI conformity
   	
#ifdef CLEAR_ERRNO
   	li		%r10,0
	lis		%r9,errno@ha	
	stw		%r10,errno@l(%r9)
#endif
	
	lis		%r9,exit@ha
	la		%r9,exit@l(%r9)
	mtlr	%r9
	lis		%r9,main@ha
	la		%r9,main@l(%r9)
	mtctr   %r9
	bctr
#else
	bl	1f
	bl _GLOBAL_OFFSET_TABLE_@local-4
1:
	mflr %r12
	lwz %r11,0(%r12)
	rlwinm %r11,%r11,0,6,29
	add %r11,%r11,%r12
	dcbf 0,%r11
	sync
	isync
	icbi 0,%r11
	isync
	sync
	bl		_GLOBAL_OFFSET_TABLE_@local-4
	mflr		%r11
	lwz		%r13,_SDA_BASE_@got(%r11)   // Load short data ptr.
	lwz		%r2,_SDA2_BASE_@got(%r11)
	stwu	%r1,-48(%r1)
	stw		%r3,16(%r1)
	stw		%r4,20(%r1)
	stw		%r5,24(%r1)
	stw		%r6,28(%r1)
	stw		%r7,32(%r1)
	stw		%r8,36(%r1)
	bl		_init_libc@plt
	lwz		%r3,_init@got(%r11)
	mtlr	%r3
	blrl
	# As above, we really want a better mechanism...
	lwz		%r3,_fini@got(%r11)
	bl		atexit@plt
	lwz		%r3,16(%r1)
	lwz		%r4,20(%r1)
	lwz		%r5,24(%r1)
	lwz		%r6,28(%r1)
	lwz		%r9,exit@got(%r11)
	mtlr	%r9
	addi	%r1,%r1,48

	li		%r0,0
	addi	%r1,%r1,-16
	stw		%r0,0(%r1)		// Zero the stack backlink for ABI conformity
	stw		%r0,4(%r1)		// Zero the LR save area for ABI conformity
	stwu	%r1,-16(%r1)	// Make a fake stack frame to link to the zeros
	stw		%r0,4(%r1)		// Zero the LR save area for ABI conformity

	b		main@plt
#endif
	
	.size _start,.-_start
	.type _start,@function
