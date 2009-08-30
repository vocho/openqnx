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
# atomic_set.s
#	atomicly set bit(s) in a memory location
#
 
#
# void		atomic_set(volatile unsigned *loc, unsigned bits)
# unsigned	atomic_set_value(volatile unsigned *loc, unsigned bits)
#

	.text
	.globl	atomic_set
	.globl	atomic_set_value

	.align 2

atomic_set:
atomic_set_value:
0:	.word	0x0463		! movli.l	@r4, r0
	mov		r0, r1
	or		r5, r0
	.word	0x0473		! movco.l	r0, @r4
	bf		0b			! retry if movco failed
	rts
	mov		r1, r0		! return original value
	
.size atomic_set,.-atomic_set
.type atomic_set,@function
.size atomic_set_value,.-atomic_set_value
.type atomic_set_value,@function
