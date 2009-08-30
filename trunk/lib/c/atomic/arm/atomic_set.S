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




/*
 *	atomic_set.S
 *
 *	atomicly set bits in a memory location
 *
 *	void		atomic_set(volatile unsigned *loc, unsigned bits)
 *	unsigned	atomic_set_value(volatile unsigned *loc, unsigned bits)
 *
 * 	WARNING: the cmpxchg emulation performs memory access with user
 *			 privileges. This means that this code cannot be used for
 *			 non-USR code. procnto uses functions in ker/arm/atomic_*
 */

#include <asmoff.def>

#ifdef	__PIC__
#define	FUNC_ENTER	stmdb	sp!, {r4,sl,lr}
#define	FUNC_RETURN	ldmia	sp!, {r4,sl,pc}
#else
#define	FUNC_ENTER	stmdb	sp!, {r4,lr}
#define	FUNC_RETURN	ldmia	sp!, {r4,pc}
#endif

	.text
	.align 2

atomic_set:
atomic_set_value:
	FUNC_ENTER

#ifdef	__PIC__
	ldr		sl, .Lgot
	ldr		ip, .Lgot+4
.L1:
	add		sl, pc, sl
	ldr		ip, [sl, ip]
#else
	ldr		ip, =__cpu_flags
#endif
	ldr		ip, [ip]
	tst		ip, #ARM_CPU_FLAG_V6
	beq		0f

	/*
	 * ARMv6 version - use ldrex/strex
	 */
	mov		ip, r0
1:	.word	0xe19c0f9f			// ldrex	r0, [ip]
	orr		r2, r0, r1
	.word	0xe18c3f92			// strex	r3, r2, [ip]
	teq		r3, #1
	beq		1b					// strex failed - retry

	FUNC_RETURN

	/*
	 * non-ARMv6 version - use special SWI to emulate cmpxchg operation
	 */
0:	mov		r3, r0
	mov		r4, r1

1:	mov		r0, r3
	ldr		r1, [r0]			// r1 = current value of *loc
	orr		r2, r1, r4			// r2 = new value
	mov		ip, #0xff000000
	swi		0					// cmpxchg(loc, old, new)
	bne		1b					// Z set if successful, r0 contains *loc

	FUNC_RETURN

#ifdef	__PIC__
.Lgot:	.word	_GLOBAL_OFFSET_TABLE_ - (.L1+8)
		.word	__cpu_flags(GOT)
#endif

	.globl	atomic_set
	.type	atomic_set,function
	.size	atomic_set,.-atomic_set

	.globl	atomic_set_value
	.type	atomic_set_value,function
	.size	atomic_set_value,.-atomic_set_value
