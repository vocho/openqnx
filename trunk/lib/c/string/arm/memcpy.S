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
	
/*
 * Macros to synthesise a 32-bit word split across a word boundary:
 * - LOBYTES takes bytes from the low-address word
 * - HIBYTES takes bytes from the high-address word
 *
 * When little-endian, LOBYTES makes the least significant bytes in dst, and
 * HIBYTES makes the most significant bytes from src and puts them in dst.
 *
 * When big-endian, the operation is reversed.
 */
.macro	LOBYTES dst, src, shift
.ifdef	VARIANT_le
	mov	&dst, &src, lsr #&shift
.else
	mov	&dst, &src, lsl #&shift
.endif
.endm

.macro	HIBYTES dst, src, shift
.ifdef	VARIANT_le
	orr	&dst, &dst, &src, lsl #&shift
.else
	orr	&dst, &dst, &src, lsr #&shift
.endif
.endm

	.globl	memcpy

/*
 * void *memcpy(void *dst, const void *src, size_t len)
 */
memcpy:
	cmp		r2, #0
	moveq	pc, lr

	stmdb	sp!,{r4-r11,lr}
	mov		r3, r0

	/*
	 * For small copy, just go straight to bytewise copy
	 */
	cmp		r2, #16
	blt		mc_bytes

	/*
	 * Get dst word aligned
	 */
	ands	ip, r3, #3
	beq		0f
	rsb		ip, ip, #4
	cmp		ip, #2
	ldrb	r4, [r1], #1
	ldrgeb	r5, [r1], #1
	ldrgtb	r6, [r1], #1
	strb	r4, [r3], #1
	strgeb	r5, [r3], #1
	strgtb	r6, [r3], #1
	sub		r2, r2, ip

	/*
	 * dst is now word-aligned.
	 * Check alignment of src.
	 */
0:	ands	ip, r1, #3
	bne		mc_unaligned

	/*
	 * Check if we can copy 32 byte chunks
	 */
0:	cmp		r2, #32
	blt		1f
	ldmia	r1!,{r4-r11}
	stmia	r3!,{r4-r11}
	sub		r2, r2, #32
	b		0b

	/*
	 * We now have < 32 bytes left.
	 * Check if we can copy 16 bytes.
	 */
1:	cmp		r2, #16
	blt		2f
	ldmia	r1!,{r4-r7}
	stmia	r3!,{r4-r7}
	sub		r2, r2, #16
	
	/*
	 * We now have < 16 bytes left.
	 * Check if we can copy words.
	 */
2:	tst		r2, #8
	beq		3f
	ldr		r4, [r1], #4
	ldr		r5, [r1], #4
	sub		r2, r2, #8
	str		r4, [r3], #4
	str		r5, [r3], #4
3:	tst		r2, #4
	ldrne	r4, [r1], #4
	subne	r2, r2, #4
	strne	r4, [r3], #4

	/*
	 * We now have < 4 bytes left
	 */
mc_tail:
	cmp		r2, #0
	beq		4f
	cmp		r2, #2
	ldrb	r4, [r1], #1
	ldrgeb	r5, [r1], #1
	ldrgtb	r6, [r1], #1
	strb	r4, [r3], #1
	strgeb	r5, [r3], #1
	strgtb	r6, [r3], #1
	
4:	ldmia	sp!,{r4-r11,pc}

mc_unaligned:
	/*
	 * Word align the src pointer for word loads and work out offset
	 */
	bic		r1, r1, #3
	teq		ip, #1
	beq		mc_1
	teq		ip, #2
	beq		mc_2

/*
 * src is 3 bytes offset from dst
 */
mc_3:
	ldr		r8, [r1], #4
	cmp		r2, #16
	blt		1f

0:	LOBYTES	r4, r8, 24
	ldmia	r1!,{r5-r8}
	HIBYTES	r4, r5, 8

	LOBYTES	r5, r5, 24
	HIBYTES	r5, r6, 8

	LOBYTES	r6, r6, 24
	HIBYTES	r6, r7, 8

	LOBYTES	r7, r7, 24
	HIBYTES	r7, r8, 8

	stmia	r3!,{r4-r7}
	sub		r2, r2, #16
	cmp		r2, #32
	bge		0b
	b		1f

0:	LOBYTES	r4, r8, 24
	ldr		r8, [r1], #4
	HIBYTES	r4, r8, 8
	str		r4, [r3], #4
	sub		r2, r2, #4
1:	cmp		r2, #4
	bge		0b
	sub		r1, r1, #1
	b		mc_tail


/*
 * src is 1 byte offset from dst
 */
mc_1:
	ldr		r8, [r1], #4
	cmp		r2, #16
	blt		1f

0:	LOBYTES	r4, r8, 8
	ldmia	r1!,{r5-r8}
	HIBYTES	r4, r5, 24

	LOBYTES	r5, r5, 8
	HIBYTES	r5, r6, 24

	LOBYTES	r6, r6, 8
	HIBYTES	r6, r7, 24

	LOBYTES	r7, r7, 8
	HIBYTES	r7, r8, 24

	stmia	r3!,{r4-r7}
	sub		r2, r2, #16
	cmp		r2, #32
	bge		0b
	b		1f

0:	LOBYTES	r4, r8, 8
	ldr		r8, [r1], #4
    HIBYTES	r4, r8, 24
    str		r4, [r3], #4
    sub		r2, r2, #4
1:  cmp		r2, #4
    bge		0b
    sub		r1, r1, #3
    b		mc_tail

/*
 * src is 2 bytes offset from dst
 */
mc_2:
	ldr		r8, [r1], #4
	cmp		r2, #16
	blt		1f

0:	LOBYTES	r4, r8, 16
	ldmia	r1!,{r5-r8}
	HIBYTES	r4, r5, 16

	LOBYTES	r5, r5, 16
	HIBYTES	r5, r6, 16

	LOBYTES	r6, r6, 16
	HIBYTES	r6, r7, 16

	LOBYTES	r7, r7, 16
	HIBYTES	r7, r8, 16

	stmia	r3!,{r4-r7}
	sub		r2, r2, #16
	cmp		r2, #32
	bge		0b
	b		1f

0:	LOBYTES	r4, r8, 16
	ldr		r8, [r1], #4
    HIBYTES	r4, r8, 16
    str		r4, [r3], #4
    sub		r2, r2, #4
1:  cmp		r2, #4
    bge		0b
    sub		r1, r1, #2
    b		mc_tail

mc_bytes:
	teq		r2, #0
0:	ldrneb	ip, [r1], #1
	strneb	ip, [r3], #1
	subnes	r2, r2, #1
	bne		0b
	ldmia	sp!,{r4-r11,pc}


	.size	memcpy,.-memcpy
	.type	memcpy,function
