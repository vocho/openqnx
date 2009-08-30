#
# $QNXLicenseA:
# Copyright 2007, QNX Software Systems. All Rights Reserved.
# 
# You must obtain a written license from and pay applicable license fees to QNX 
# Software Systems before you may reproduce, modify or distribute this software, 
# or any work that includes all or part of this software.   Free development 
# licenses are available for evaluation and non-commercial purposes.  For more 
# information visit http://licensing.qnx.com or email licensing@qnx.com.
#  
# This file may contain contributions from others.  Please review this entire 
# file for other proprietary rights or license notices, as well as the QNX 
# Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
# for other information.
# $
#

	.globl	xferiov_pos
	.globl	xfer_copy_kern
	.globl	xfer_copy_user

/*
 * unsigned	xferiov_pos(CPU_REGISTERS *)
 *
 * Return current offset into message that has been copied
 *
 * FIXME: this isn't implemented yet. Instead we limit a copy to 4K so
 *		  that a preemption has re-copy at most 4K bytes.
 */
xferiov_pos:
	mov		r0, #0
	mov		pc, lr

#define	MAX_COPY	4096

/*
 * void	xfer_copy_kern(uint32_t *lenp, char *dst, char *src, unsigned len)
 *
 * Copy data from user/kernel thread to kernel thread.
 */
xfer_copy_kern:
.ifdef	VARIANT_v6
xfer_copy_user:
.endif
	stmdb	sp!,{r4-r8,lr}
	mov		r4, r0
	mov		r5, r1
	mov		r6, r2
	mov		r7, r3

	/*
	 * Limit each copy to MAX_COPY bytes
	 */
0:	cmp		r7, #MAX_COPY
	movcc	r8, r7
	movcs	r8, #MAX_COPY
	mov		r0, r5
	mov		r1, r6
	mov		r2, r8
	bl		memcpy

	/*
	 * Update lenp, dst, src and len
	 */
	ldr		ip, [r4]
	add		ip, ip, r8
	str		ip, [r4]
	add		r5, r5, r8
	add		r6, r6, r8
	subs	r7, r7, r8
	bne		0b
	ldmia	sp!,{r4-r8,pc}

.ifndef	VARIANT_v6
/*
 * void	xfer_copy_user(uint32_t *lenp, char *dst, char *src, unsigned len)
 *
 * Copy data from user/kernel thread to user thread.
 * This checks that the user page is writable in user mode.
 */
xfer_copy_user:
	stmdb	sp!,{r4-r9,lr}
	mov		r4, r0
	mov		r5, r1
	mov		r6, r2
	mov		r7, r3
	ldr		r9, [r4]

	/*
	 * Trim first copy so subsequent copies always start on a page boundary.
	 */
	mov		r8, r1, lsl #20
	mov		r8, r8, lsr #20
	rsb		r8, r8, #0x1000
	cmp		r8, r3
	movhi	r8, r3

	/*
	 * Do a write probe to catch an attempt to copy to a read-only page
	 */
	teq		r8, #0
	beq		0f
	ldrb	r0, [r6]		// read first byte from src
	strbt	r0, [r5]		// attempt to store at dst
	mov		r0, r5
	mov		r1, r6
	mov		r2, r8
	bl		memcpy

	/*
	 * Update lenp, dst, src and len
	 */
0:	add		r9, r9, r8
	str		r9, [r4]
	add		r5, r5, r8
	add		r6, r6, r8
	subs	r7, r7, r8
	beq		1f

	/*
	 * Do a write probe to catch an attempt to copy to a read-only page
	 */
	ldrb	r0, [r6]		// read first byte from src
	strbt	r0, [r5]		// attempt to store at dst

	/*
	 * Limit each copy to MAX_COPY bytes
	 */
	cmp		r7, #MAX_COPY
	movcc	r8, r7
	movcs	r8, #MAX_COPY
	mov		r0, r5
	mov		r1, r6
	mov		r2, r8
	adr		lr, 0b
	b		memcpy
1:	ldmia	sp!,{r4-r9,pc}
.endif
