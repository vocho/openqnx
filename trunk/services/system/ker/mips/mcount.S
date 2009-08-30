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

/*
 * _mcount()
 *	Record calls in a circular buffer
 *
 * On entry, AT has the caller of the current function, and ra
 * points to the function's prolog itself.  Two words of stack have
 * been opened by our caller.  On return, we restore AT to ra, and
 * pop the two words of stack.
 *
 * Within here, we atomically allocate a slot in the ring buffer, and
 * then save function, caller, and the first four arguments.  We can
 * have some reordering of entries when an ISR occurs around the time
 * an entry is created.
 */
#include <mips/asm.h>

#define WORDS 4		/* Bytes in a word */
#define NWORD 7		/* # words per record */
#define BUFLEN 8192	/* Number of trace records to gather */

	/*
	 * The trace buffer
	 */
	.bss
	.align	2
mtbuf:	.space	NWORD*WORDS*BUFLEN
mtbufend:

	/*
	 * Current position pointer
	 */
	.sdata
	.align	2
mtptr:	.word	mtbuf
crashed: .word	0

	/*
	 * _mcount() code
	 */
	.text
	.set	mips3
	.set	noat
	.set	noreorder
	.extern	actives,4
	.globl	_mcount
_mcount:
	lw	t0,crashed	/* Stop collecting after a crash */
	bne	t0,zero,3f
	 la	t1,mtptr	/* Point to next record position */
	la	t3,mtbufend	/*  ...and end of buf */
1:	ll	t0,0(t1)
	addiu	t2,t0,NWORD*WORDS	/* Advance pointer */
	sltu	t4,t2,t3	/* Wrap at end */
	bne	t4,zero,2f
	 nop
	la	t2,mtbuf
2:	sc	t2,0(t1)	/* See if we raced */
	beq	t2,zero,1b
	 nop
	sw	AT,0(t0)	/* Save call registers */
	sw	ra,4(t0)
	sw	a0,8(t0)
	sw	a1,12(t0)
	sw	a2,16(t0)
	sw	a3,20(t0)
	lw	t1,actives
	sw	t1,24(t0)

3:	addiu	sp,sp,8		/* Remove stack space */
	jr	ra		/* Restore caller's RA and return */
	 ori	ra,AT,0 /* can't use "move" - cause probs with MIPS32 */
