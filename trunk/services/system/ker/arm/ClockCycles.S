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

#include <asmoff.def>

	.globl	ClockCycles

	.text

ClockCycles:
	stmdb	sp!, {r4,lr}

	ldr		r0, =_syspage_ptr
	ldr		r1, =qtimeptr
	ldr		ip, =callout_timer_value
	ldr		r0, [r0]
	ldr		r1, [r1]

	/*
	 * Disable interrupts
	 */
	mrs		r4, cpsr
	orr		r2, r4, #ARM_CPSR_I | ARM_CPSR_F
	msr		cpsr, r2

	mov		lr, pc
	ldr		pc, [ip]

	ldr		r2, =cycles
	ldr		lr, =last_cycles
	ldmia	r2, {r2,r3}

.ifdef VARIANT_le
	adds	r0, r0, r2
	adc		r1, r3, #0
.else
	adds	r1, r0, r3
	adc		r0, r2, #0
.endif

	/*
	 * Adjust by timer_load if timestamp < last_cycles
	 */
	ldmia	lr, {r2,r3}
.ifdef	VARIANT_le
	cmp		r3, r1
	bhi		0f
	bne		1f
	cmp		r2, r0
	bls		1f
0:	ldr		ip, =qtimeptr
	ldr		ip, [ip]
	ldr		ip, [ip, #TIMER_LOAD]
	adds	r0, r0, ip
	adc		r1, r1, #0
.else
	cmp		r2, r0
	bhi		0f
	bne		1f
	cmp		r3, r1
	bls		1f
0:	ldr		ip, =qtimeptr
	ldr		ip, [ip]
	ldr		ip, [ip, #TIMER_LOAD]
	adds	r1, r1, ip
	adc		r0, r0, #0
.endif

	/*
	 * Update last_cycles
	 */
1:	stmia	lr, {r0,r1}

	/*
	 * Restore interrupts and return
	 */
	msr		cpsr, r4
	ldmia	sp!, {r4,pc}
