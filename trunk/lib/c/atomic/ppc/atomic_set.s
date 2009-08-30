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

#
#
# atomic_set.s
#	atomicly set bit(s) in a memory location
#
.include "ppc/util.ah"
 
 #
 # void atomic_set( volatile unsigned *loc, unsigned bits )
 # unsigned atomic_set_value( volatile unsigned *loc, unsigned bits )
 #
	.section 	".text"
atomic_set:
atomic_set_value:
1:	lwarx	%r0,%r0,%r3
	or		%r5,%r0,%r4
	STWCX405	%r5,%r0,%r3
	bne-	1b
	mr		%r3,%r0
	blr
	
	.globl	atomic_set
	.type	atomic_set,@function
	.size	atomic_set,.-atomic_set

	.globl	atomic_set_value
	.type	atomic_set_value,@function
	.size	atomic_set_value,.-atomic_set_value
