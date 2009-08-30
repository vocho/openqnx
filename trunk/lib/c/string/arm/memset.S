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
	
	.globl	memset

memset:
	mov		r3, r0
	cmp		r2, #8
	blt		3f

	/*
	 * Set bytes up to next word boundary
	 */
	ands	ip, r0, #3
	beq		0f
	sub		r2, r2, #4
	add		r2, r2, ip
	sub		ip, ip, #2
	add		pc, pc, ip, lsl #2
	strb	r1, [r3], #1
	strb	r1, [r3], #1
	strb	r1, [r3], #1

	/*
	 * Fill word with value to store and check if we can store quad-words
	 */
0:	orr		r1, r1, r1, lsl #8
	orr		r1, r1, r1, lsl #16	
	cmp		r2, #16
	blt		2f

	/*
	 * Set quad-words
	 */
	stmdb	sp!,{r4-r6}
	mov		r4, r1
	mov		r5, r1
	mov		r6, r1
	mov		ip, r1
0:	stmia	r3!,{r4-r6,ip}
	sub		r2, r2, #16
	cmp		r2, #16
	bge		0b
	ldmia	sp!,{r4-r6}
	b		2f

	/*
	 * Set words
	 */
1:	str		r1, [r3], #4
	sub		r2, r2, #4
2:	cmp		r2, #4
	bge		1b

	/*
	 * Set trailing bytes
	 */
3:	teq		r2, #0
	strneb	r1, [r3], #1
	subnes	r2, r2, #1
	bne		3b
	mov		pc, lr

	.size	memset,.-memset
	.type	memset,function
