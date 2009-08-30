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

/*
 * unsigned cmpxchg(unsigned *dst, unsigned cmp, unsigned src)
 * {
 *     val = *dst
 *     if (val == cmp) {
 *         *dst = src
 *     }
 *     return val
 * }
 */
_smp_cmpxchg:
_mux_smp_cmpxchg:
	mov		ip, r0
	mcr		p15, 0, r0, c7, c10, 5	// data memory barrier
1:	.word	0xe19c0f9f				// ldrex	r0, [ip]
	teq		r0, r1
	.word	0x018c3f92				// strexeq	r3, r2, [ip]
	teqeq	r3, #1
	beq		1b						// strex failed, retry
	mcr		p15, 0, r0, c7, c10, 5	// data memory barrier
	mov		pc, lr

	.type	_smp_cmpxchg, function
	.size	_smp_cmpxchg, .-_smp_cmpxchg

	.type	_mux_smp_cmpxchg, function
	.size	_mux_smp_cmpxchg, .-_mux_smp_cmpxchg

	.globl	_smp_xchg

/*
 * unsigned xchg(unsigned *dst, unsigned new)
 */
_smp_xchg:
	mov		ip, r0
	mcr		p15, 0, r0, c7, c10, 5	// data memory barrier
1:	.word	0xe19c0f9f				// ldrex	r0, [ip]
	.word	0xe18c2f91				// strex	r2, r1, [ip]
	teq		r2, #1
	beq		1b						// strex failed, retry
	mcr		p15, 0, r0, c7, c10, 5	// data memory barrier
	mov		pc, lr

	.type	_smp_xchg, function
	.size	_smp_xchg, .-_smp_xchg
