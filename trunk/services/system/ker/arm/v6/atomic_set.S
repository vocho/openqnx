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

#include "asmoff.def"

	.text

	.globl	atomic_set
	.globl	atomic_set_value

/*
 * void		atomic_set(volatile unsigned *loc, unsigned val)
 * unsigned	atomic_set_value(volatile unsigned *loc, unsigned val)
 */

atomic_set:
atomic_set_value:
	mov		ip, r0
1:	.word	0xe19c0f9f		// ldrex	r0, [ip]
	orr		r2, r0, r1
	.word	0xe18c3f92		// strex	r3, r2, [ip]
	teq		r3, #1
	beq		1b				// strex failed, retry
	mov		pc, lr

	.type	atomic_set,function
	.size	atomic_set,.-atomic_set

	.type	atomic_set_value,function
	.size	atomic_set_value,.-atomic_set_value
