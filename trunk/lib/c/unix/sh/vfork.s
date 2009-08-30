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


	.text
	.globl vfork
	.type vfork,@function
 
vfork:
.ifdef __PIC__
	mov.l	__got,r1
	mova	__got,r0
	add	r0,r1
	mov.l	vfork_1,r0
	mov.l	@(r0,r1),r1
	xor 	r4,r4
	jmp	@r1
	mov	r15,r5
.else
	mov.l	vfork_1,r0
	xor 	r4,r4
	jmp	@r0
	mov	r15,r5
.endif
	.align 2
vfork_1:
.ifdef __PIC__
	.long	_fork@GOT
__got:
	.long	_GLOBAL_OFFSET_TABLE_
.else
	.long	_fork
.endif

	 .size vfork,.-vfork
