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
 *	unsigned	_smp_xchg(volatile unsigned *__dst, unsigned __src)
 */

#include <asmoff.def>

	.text
	.align 2
	.globl	_smp_xchg

#ifdef	__PIC__
#define	FUNC_ENTER	stmdb	sp!, {sl,lr}
#define	FUNC_RETURN	ldmia	sp!, {sl,pc}
#else
#define	FUNC_ENTER
#define	FUNC_RETURN	mov		pc, lr
#endif

	.text
	.align 2
	.globl	_smp_xchg

_smp_xchg:
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
	 * ARMv6 version - requires data memory barrier
	 */
	mcr		p15, 0, r0, c7, c10, 5	// data memory barrier
	mov		r2, r0
	swp		r0, r1, [r2]
	mcr		p15, 0, r0, c7, c10, 5	// data memory barrier

	FUNC_RETURN

	/*
	 * non-ARMv6 version - use swp instruction
	 */
0:	mov		r2, r0
	swp		r0, r1, [r2]

	FUNC_RETURN

#ifdef	__PIC__
.Lgot:	.word	_GLOBAL_OFFSET_TABLE_ - (.L1+8)
		.word	__cpu_flags(GOT)
#endif

	.type	_smp_xchg,function
	.size	_smp_xchg,.-_smp_xchg
