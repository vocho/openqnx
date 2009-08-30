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





	.section .init
	.globl 	_init
	.type 	_init,@function
_init:
	mov.l	r14,@-r15
	mov.l	r12,@-r15
	sts.l	pr,@-r15
	mov.l	__got1,r12
	mova	__got1,r0
	add		r0,r12
	bra		1f
	nop
__got1:
	.long	_GLOBAL_OFFSET_TABLE_
1:


	.section .fini
	.globl	_fini
	.type	_fini,@function
_fini:
	mov.l	r14,@-r15
	mov.l	r12,@-r15
	sts.l	pr,@-r15
	mov.l	__got2,r12
	mova	__got2,r0
	add		r0,r12
	bra		1f
	nop
__got2:
	.long	_GLOBAL_OFFSET_TABLE_
	.balignw 32,0x0009
1:
