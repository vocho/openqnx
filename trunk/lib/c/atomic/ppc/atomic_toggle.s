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
# atomic_toggle.s
#	atomicly toggle bit(s) in a memory location
#
.include "ppc/util.ah"
 
 #
 # void atomic_toggle( volatile unsigned *loc, unsigned bits )
 # unsigned atomic_toggle_value( volatile unsigned *loc, unsigned bits )
 #
	.section	".text"
	
atomic_toggle:
atomic_toggle_value:
1:	lwarx	%r0,%r0,%r3
	xor		%r5,%r0,%r4
	STWCX405	%r5,%r0,%r3
	bne-	1b
	mr		%r3,%r0
	blr
	
	.globl	atomic_toggle
	.type	atomic_toggle,@function
	.size	atomic_toggle,.-atomic_toggle

	.globl	atomic_toggle_value
	.type	atomic_toggle_value,@function
	.size	atomic_toggle_value,.-atomic_toggle_value
