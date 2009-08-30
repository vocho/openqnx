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

#include <arm/vfp.h>

	.text
	.align 2

	.globl	vfp_v2_save
	.globl	vfp_v2_restore
	.globl	vfp_v3_save
	.globl	vfp_v3_restore

vfp_v2_save:			
	mrc		p10, 7, r2, ARM_VFP_FPEXC, c0, 0		// fmrx r2, fpexc
	orr		r2, r2, #ARM_VFP_FPEXC_EN				// set FPEXC_EN
	mov		r1, r2									// preserve for below
	bic		r2, r2, #ARM_VFP_FPEXC_EX				// clear FPEXC_EX
	mcr		p10, 7, r2, ARM_VFP_FPEXC, c0, 0		// fmxr fpexc, r2

	/*
	 * Save FP registers
	 */
	mov		ip, r0									// register base address
.ifdef	VARIANT_v6
	stc		p11, c0, [ip], #32*4					// fstmiad ip!, {d0-d15}
.else
	stc		p11, c0, [ip], #33*4					// fstmiax ip!, {d0-d15}
.endif

	/*
	 * Save control registers
	 * Only save FPINST/FPINST2 if FPEXC_EX/FP2V are set
	 */
	add		ip, r0, #32*8							// &fpu->un.vfp.fpscr
	mrc		p10, 7, r0, ARM_VFP_FPSCR, c0, 0		// fmrx r0, fpscr
	tst		r1, #ARM_VFP_FPEXC_EX
	mrcne	p10, 7, r2, ARM_VFP_FPINST, c0, 0		// fmrx r2, fpinst
	tstne	r1, #ARM_VFP_FPEXC_FP2V
	mrcne	p10, 7, r3, ARM_VFP_FPINST2, c0, 0		// fmrx r3, fpinst2
	stmia	ip, {r0-r3}
	mov		pc, lr

vfp_v2_restore:
	mrc		p10, 7, r1, ARM_VFP_FPEXC, c0, 0		// fmrx r0, fpexc
	orr		r1, r1, #ARM_VFP_FPEXC_EN				// set FPEXC_EN
	bic		r1, r1, #ARM_VFP_FPEXC_EX				// clear FPEXC_EX
	mcr		p10, 7, r1, ARM_VFP_FPEXC, c0, 0		// fmxr fpexc, r0

	/*
	 * Restore FP registers
	 */
	mov		ip, r0									// register base address
.ifdef	VARIANT_v6
	ldc		p11, c0, [ip], #32*4					// fldmiad ip!, {d0-d15}
.else
	ldc		p11, c0, [ip], #33*4					// fldmiax ip!, {d0-d15}
.endif

	/*
	 * Restore control registers
	 * Only restore FPINST/FPINST2 if FPEXC_EX/FPEXC_FP2V are set
	 */
	add		ip, r0, #32*8							// &fpu->un.vfp.fpscr
	ldmia	ip, {r0-r3}
	tst		r1, #ARM_VFP_FPEXC_EX
	mcrne	p10, 7, r2, ARM_VFP_FPINST, c0, 0		// fmxr fpinst, r2
	tstne	r1, #ARM_VFP_FPEXC_FP2V
	mcrne	p10, 7, r3, ARM_VFP_FPINST2, c0, 0		// fmxr fpinst2, r3
	mcr		p10, 7, r0, ARM_VFP_FPSCR, c0, 0		// fmxr fpscr, r0
	mcr		p10, 7, r1, ARM_VFP_FPEXC, c0, 0		// fmxr fpexc, r1
	mov		pc, lr

/*
 * Save VFPv3 context
 */
vfp_v3_save:
	mrc		p10, 7, r2, ARM_VFP_FPEXC, c0, 0		// fmrx r0, fpexc
	orr		r2, r2, #ARM_VFP_FPEXC_EN				// set FPEXC_EN
	mov		r1, r2									// preserve for below
	bic		r2, r2, #ARM_VFP_FPEXC_EX				// clear FPEXC_EX
	mcr		p10, 7, r2, ARM_VFP_FPEXC, c0, 0		// fmxr fpexc, r0

	/*
	 * Save FP registers
	 * Need to check MVFR0 to see if we have 16 or 32 double registers
	 */
	mov		ip, r0									// register base address
	stc		p11, c0, [ip], #32*4					// fstmiad ip!,{d0-d15}
	mrc		p10, 7, r2, ARM_VFP_MVFR0, c0, 0		// fmrx r2, mvfr0
	and		r2, r2, #0xf
	teq		r2, #2
	stceql	p11, c0, [ip], #32*4					// fstmiad ip!,{d16-d32} 

	/*
	 * Save control registers
	 * Only save FPINST/FPINST2 if FPEXC_EX/FPEXC_FP2V are set
	 */
	add		ip, r0, #32*8							// &fpu->un.vfp.fpscr
	mrc		p10, 7, r0, ARM_VFP_FPSCR, c0, 0		// fmrx r0, fpscr
	tst		r1, #ARM_VFP_FPEXC_EX
	mrcne	p10, 7, r2, ARM_VFP_FPINST, c0, 0		// fmrx r2, fpinst
	tstne	r1, #ARM_VFP_FPEXC_FP2V
	mrcne	p10, 7, r3, ARM_VFP_FPINST2, c0, 0		// fmrx r3, fpinst2
	stmia	ip, {r0-r3}
	mov		pc, lr

vfp_v3_restore:
	mrc		p10, 7, r1, ARM_VFP_FPEXC, c0, 0		// fmrx r0, fpexc
	orr		r1, r1, #ARM_VFP_FPEXC_EN				// set FPEXC_EN
	bic		r1, r1, #ARM_VFP_FPEXC_EX				// clear FPEXC_EX
	mcr		p10, 7, r1, ARM_VFP_FPEXC, c0, 0		// fmxr fpexc, r0

	/*
	 * Restore FP registers
	 * Need to check MVFR0 to see if we have 16 or 32 double registers
	 */
	mov		ip, r0									// register base address
	ldc		p11, c0, [ip], #32*4					// fldmiad ip!, {d0-d15}
	mrc		p10, 7, r2, ARM_VFP_MVFR0, c0, 0		// fmrx r2, mvfr0
	and		r2, r2, #0xf
	teq		r2, #2
	ldceql	p11, c0, [ip], #32*4					// fldmiad ip!,{d16-d32} 

	/*
	 * Restore control registers
	 * Only restore FPINST/FPINST2 if FPEXEC_EN/FP2V are set
	 */
	add		ip, r0, #32*8							// &fpu->un.vfp.fpscr
	ldmia	ip, {r0-r3}
	tst		r1, #ARM_VFP_FPEXC_EX
	mcrne	p10, 7, r2, ARM_VFP_FPINST, c0, 0		// fmxr fpinst, r2
	tstne	r1, #ARM_VFP_FPEXC_FP2V
	mcrne	p10, 7, r3, ARM_VFP_FPINST2, c0, 0		// fmxr fpinst2, r3
	mcr		p10, 7, r0, ARM_VFP_FPSCR, c0, 0		// fmxr fpscr, r0
	mcr		p10, 7, r1, ARM_VFP_FPEXC, c0, 0		// fmxr fpexc, r1
	mov		pc, lr
