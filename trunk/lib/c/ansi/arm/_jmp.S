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
 * Routines for saving/restoring context
 *
 * r0-r4, r12, and cpsr can be trashed across function calls, so:
 *	
 *	jmp_buf[0]	= r4
 *	jmp_buf[1]	= r5
 *	jmp_buf[2]	= r6
 *	jmp_buf[3]	= r7
 *	jmp_buf[4]	= r8
 *	jmp_buf[5]	= r9
 *	jmp_buf[6]	= r10
 *	jmp_buf[7]	= r11
 *	jmp_buf[8]	= r13
 *	jmp_buf[9]	= r14 (restored as pc)
 */
 
 	.text
	.align 2

 
	.global _setjmp
	.global _longjmp

	.section ".text"
	.align 2
	
/*
 * int _setjmp (jmp_buf *ptr)
 */
_setjmp:
	stmia	r0,{r4-r11,r13,r14}
	mov		r0, #0
	mov		pc, lr

	.type _setjmp,function
	.size _setjmp,.-_setjmp
 
/*
 * void _longjmp (jmp_buf *ptr, int ret_val)
 */
_longjmp:
	ldmia	r0,{r4-r11,r13,r14}
	movs	r0, r1
	moveq	r0, #1
	mov		pc, lr

	.type _longjmp,function
	.size _longjmp,.-_longjmp
