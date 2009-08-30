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

#
# atomic_sub.s
#	atomicly decrement a memory location
#
 
#
# void		atomic_sub(volatile unsigned *loc, unsigned decr)
# unsigned	atomic_sub_value(volatile unsigned *loc, unsigned decr)
#

	.text
	.globl	atomic_sub
	.globl	atomic_sub_value

	.align 2

atomic_sub:
atomic_sub_value:
0:	.word	0x0463		! movli.l	@r4, r0
	sub		r5,r0
	.word	0x0473		! movco.l	r0, @r4
	bf		0b			! retry if movco failed
	rts
	add		r5, r0		! return original value
	
.size atomic_sub,.-atomic_sub
.type atomic_sub,@function
.size atomic_sub_value,.-atomic_sub_value
.type atomic_sub_value,@function
