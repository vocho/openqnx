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

	.text

	/*
	 * Set up exception stacks used for storage when switching to SVC mode
	 * void	smp_exc_save(void *und_save, void *abt_save, void *irq_save, void *fiq_save)
	 */
	.globl	smp_exc_save

smp_exc_save:
	mrs		ip, cpsr
	bic		ip, ip, #ARM_CPSR_MODE_MASK
	orr		ip, ip, #ARM_CPSR_MODE_UND
	msr		cpsr, ip
	mov		sp, r0

	bic		ip, ip, #ARM_CPSR_MODE_MASK
	orr		ip, ip, #ARM_CPSR_MODE_ABT
	msr		cpsr, ip
	mov		sp, r1

	bic		ip, ip, #ARM_CPSR_MODE_MASK
	orr		ip, ip, #ARM_CPSR_MODE_IRQ
	msr		cpsr, ip
	mov		sp, r2

	bic		ip, ip, #ARM_CPSR_MODE_MASK
	orr		ip, ip, #ARM_CPSR_MODE_FIQ
	msr		cpsr, ip
	mov		sp, r3

	/*
	 * Back to SVC mode and return
	 */
	bic		ip, ip, #ARM_CPSR_MODE_MASK
	orr		ip, ip, #ARM_CPSR_MODE_SVC
	msr		cpsr, ip
	mov		pc, lr

	.type	smp_exc_save, function
	.size	smp_exc_save, . - smp_exc_save
