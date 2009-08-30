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

	.globl	_smp_cmpxchg
	.globl	_mux_smp_cmpxchg

_smp_cmpxchg:
_mux_smp_cmpxchg:
	mrs		ip, cpsr
	orr		r3, ip, #ARM_CPSR_I|ARM_CPSR_F
	msr		cpsr, r3
	ldr		r3, [r0]
	teq		r3, r1
	streq	r2, [r0]
	orreq	ip, ip, #ARM_CPSR_Z
	bicne	ip, ip, #ARM_CPSR_Z
	msr		cpsr, ip
	mov		r0, r3
	mov		pc, lr

	.type	_smp_cmpxchg, function
	.size	_smp_cmpxchg, .-_smp_cmpxchg

	.type	_mux_smp_cmpxchg, function
	.size	_mux_smp_cmpxchg, .-_mux_smp_cmpxchg

	.globl	_smp_xchg

_smp_xchg:
	mov		r2, r0
	swp		r0, r1, [r2]
	mov		pc, lr

	.type	_smp_xchg, function
	.size	_smp_xchg, .-_smp_xchg
