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
	.align 2

	.extern	_CMain
	.extern	_preinit
	.extern	_init
	.extern _init_array
	.extern _fini_array
	.globl	_start

	/*
	 * On entry:
	 *	r0 = at_exit pointer (set to 0 by kernel, possibly non-zero from ldd)
	 *  Stack is layed out as follows:
	 *
	 *			:
	 *		 auxv[0]
	 *		 0
	 *		 envp[n]
	 *		 	:
	 *		 envp[0]
	 *		 0
	 *		 argv[argc]
	 *		 	:
	 *		 argv[0]
	 *	sp-> argc
	 */
_start:
	mov		r4, r0				// save at_exit pointer
	ldr		r0, [sp]			// argc
	add		r1, sp, #4			// argv
	add		r2, r1, r0, lsl #2	// skip over argv list
	add		r2, r2, #4			// envp
	mov		r3, r2				// skip over envp list
0:	ldr		ip, [r3], #4
	teq		ip, #0
	bne		0b
	str		r4, [sp, #-4]!
	mov		r4, r0				// save argc, argv, envp
	mov		r5, r1
	mov		r6, r2
	ldr		r0, start_1
	mov		lr, pc
	mov		pc, r0				// _init_libc(argc, argv, envp, auxv, at_exit)
	add		sp, sp, #4

	ldr		r0, =__preinit_array_start
	ldr		r1, =__preinit_array_end
	ldr		r2, start_2
	mov		lr, pc
	mov		pc, r2				// _preinit_array 
	ldr		r0, =_fini
	ldr		r2, start_3
	mov		lr, pc
	mov		pc, r2				// atexit
	ldr             r0, =__fini_array_start
	ldr		r1, =__fini_array_end
	ldr		r2, start_4
	mov		lr, pc
	mov		pc, r2				// _fini_array
	ldr		r2, start_5
	mov		lr, pc
	mov		pc, r2				// _init
	ldr             r0, =__init_array_start
	ldr		r1, =__init_array_end
	ldr		r2, start_6
	mov		lr, pc
	mov		pc, r2				// _init_array
	ldr		r0, =errno
	mov		r1, #0
	str		r1, [r0]
	mov		r0, r4
	mov		r1, r5
	mov		r2, r6
	ldr		r3, start_7
	mov		lr, pc
	mov		pc, r3				// main
	ldr		r3, start_8
	mov		lr, pc
	mov		pc, r3				// exit

	/*
	 * We should never get here
	 */
0:	b	0b

start_1:
	.word _init_libc
start_2:
	.word _preinit_array
start_3:
	.word atexit
start_4:
	.word _fini_array
start_5:
	.word _init
start_6:
	.word _init_array
start_7:	
	.word main
start_8:
	.word exit
	.size _start,.-_start
	.type _start,function
