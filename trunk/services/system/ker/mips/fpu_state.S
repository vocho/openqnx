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

/*
 * fpu_state.S
 *  Saving and restoring the floating point state. Just used for
 *  kernel debugging in some variants of the kernel right now.
 */

/*
 * Get global defs.  Remove conflicting names from other architectures
 */
#include <mips/asm.h>
/*
 * Save all the FP regsisters out to memory.
 */
FRAME(save_fpu,sp,0,ra)
	.set	noreorder
	
	mfc1	t0,FPU_R1
	nop
	sw		t0,1*4(a0)
	mfc1	t0,FPU_R2
	nop
	sw		t0,2*4(a0)
	mfc1	t0,FPU_R3
	nop
	sw		t0,3*4(a0)
	mfc1	t0,FPU_R4
	nop
	sw		t0,4*4(a0)
	mfc1	t0,FPU_R5
	nop
	sw		t0,5*4(a0)
	mfc1	t0,FPU_R6
	nop
	sw		t0,6*4(a0)
	mfc1	t0,FPU_R7
	nop
	sw		t0,7*4(a0)
	mfc1	t0,FPU_R8
	nop
	sw		t0,8*4(a0)
	mfc1	t0,FPU_R9
	nop
	sw		t0,9*4(a0)
	mfc1	t0,FPU_R10
	nop
	sw		t0,10*4(a0)
	mfc1	t0,FPU_R11
	nop
	sw		t0,11*4(a0)
	mfc1	t0,FPU_R12
	nop
	sw		t0,12*4(a0)
	mfc1	t0,FPU_R13
	nop
	sw		t0,13*4(a0)
	mfc1	t0,FPU_R14
	nop
	sw		t0,14*4(a0)
	mfc1	t0,FPU_R15
	nop
	sw		t0,15*4(a0)
	mfc1	t0,FPU_R16
	nop
	sw		t0,16*4(a0)
	mfc1	t0,FPU_R17
	nop
	sw		t0,17*4(a0)
	mfc1	t0,FPU_R18
	nop
	sw		t0,18*4(a0)
	mfc1	t0,FPU_R19
	nop
	sw		t0,19*4(a0)
	mfc1	t0,FPU_R20
	nop
	sw		t0,20*4(a0)
	mfc1	t0,FPU_R21
	nop
	sw		t0,21*4(a0)
	mfc1	t0,FPU_R22
	nop
	sw		t0,22*4(a0)
	mfc1	t0,FPU_R23
	nop
	sw		t0,23*4(a0)
	mfc1	t0,FPU_R24
	nop
	sw		t0,24*4(a0)
	mfc1	t0,FPU_R25
	nop
	sw		t0,25*4(a0)
	mfc1	t0,FPU_R26
	nop
	sw		t0,26*4(a0)
	mfc1	t0,FPU_R27
	nop
	sw		t0,27*4(a0)
	mfc1	t0,FPU_R28
	nop
	sw		t0,28*4(a0)
	mfc1	t0,FPU_R29
	nop
	sw		t0,29*4(a0)
	mfc1	t0,FPU_R30
	nop
	sw		t0,30*4(a0)
	mfc1	t0,FPU_R31
	nop
	j	ra
	 sw		t0,31*4(a0)
	 
	.set	reorder
ENDFRAME(save_fpu)
