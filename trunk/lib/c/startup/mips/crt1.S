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




#include <mips/asm.h>

#ifndef NO_INIT_ARRAY
#define INIT_ARRAY
#endif
#ifndef NO_CLEAR_ERRNO
#define CLEAR_ERRNO
#endif

	.text

	.extern	_gp
	.extern	_init_libc
	.extern main
	.extern exit
	.extern atexit
	.extern _init
	.extern _fini
#ifdef INIT_ARRAY
	.extern _preinit_array
	.extern _init_array
	.extern _fini_array
#endif
#ifdef CLEAR_ERRNO
	.extern	errno
#endif
	
	.set	noreorder

//	We don't really need to store the %ra -- or restore the %sp, but we
//	do so to make this look API compliant. GDB expects it.

#define STACK_SIZE	32

	.globl	_start
_start:	
	addiu	sp,-STACK_SIZE //Make room for parms
	sw		ra,(STACK_SIZE-4)(sp)
	la		t0,STACK_SIZE(sp) //Save pointer to argc, argv, envv, auxv list
	la		gp,_gp
	lw		a0,(t0)		// get argc
	addiu	a1,t0,4		// point to argv
	sll		t1,a0,2		// skip over argv list
	addu	a2,t1,a1	// ...
	addiu	a2,4		// and point at envv
	move	a3,a2		// find auxv
1:	
	lw		t1,(a3)
	bne		t1,zero, 1b
	 addiu	a3,4

	move	s0,a0			//Save argc, argv, envv for later
	move	s1,a1
	move	s2,a2
	la		t9,_init_libc	//Might be too far away for direct calls
	jal		t9
	 sw		v0,16(sp)	// Pass atexit function ptr to _init_libc
	
#ifdef INIT_ARRAY
	lui             t3,%hi(__preinit_array_start)
	addiu		a0,t3,%lo(__preinit_array_start)
	lui             t3,%hi(__preinit_array_end)
	addiu		a1,t3,%lo(__preinit_array_end)
	la		t9,_preinit_array
	jal		t9
	 nop
#endif
	 
	la		t9,atexit
	la		a0,_fini
	jal 	t9
	nop
	 
#ifdef INIT_ARRAY
	lui             t3,%hi(__fini_array_start)
	addiu		a0,t3,%lo(__fini_array_start)
	lui             t3,%hi(__fini_array_end)
	addiu		a1,t3,%lo(__fini_array_end)
	la		t9,_fini_array
	jal		t9
	nop
#endif

	la		t9,_init
	jal		t9
	nop
	 
#ifdef INIT_ARRAY
	lui             t3,%hi(__init_array_start)
	addiu		a0,t3,%lo(__init_array_start)
	lui             t3,%hi(__init_array_end)
	addiu		a1,t3,%lo(__init_array_end)
	la		t9,_init_array
	jal		t9
	nop
#endif

#ifdef CLEAR_ERRNO
	sw		zero,errno
#endif
	
	move	a0,s0
	move	a1,s1
	la		t9,main
	jal		t9

	 move	a2,s2
	 
	move	a0,v0
	la		t9,exit
	j		t9
	 move	ra,zero

	// not reached -- but the API requires this.
	lw		ra,(STACK_SIZE-4)(sp)
	j		ra
	 addiu	sp,STACK_SIZE //Restore stack
	
	.size _start,.-_start
	.type _start,@function
