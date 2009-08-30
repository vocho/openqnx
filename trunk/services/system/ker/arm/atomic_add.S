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

	.globl	atomic_add
	.globl	atomic_add_value

/*
 * void		atomic_add(volatile unsigned *loc, unsigned val)
 * unsigned	atomic_add_value(volatile unsigned *loc, unsigned val)
 */

atomic_add:
atomic_add_value:
	mrs		ip, cpsr
	orr		r3, ip, #ARM_CPSR_I | ARM_CPSR_F
	msr		cpsr, r3
	mov		r2, r0
	ldr		r0, [r2]
	add		r3, r0, r1
	str		r3, [r2]
	msr		cpsr, ip
	mov		pc, lr


	.type	atomic_add,function
	.size	atomic_add,.-atomic_add

	.type	atomic_add_value,function
	.size	atomic_add_value,.-atomic_add_value
