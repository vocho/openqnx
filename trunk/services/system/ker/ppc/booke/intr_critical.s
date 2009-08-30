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

	.include "asmoff.def"
	.include "ppc/util.ah"
	.include "../context.ah"
	
.text

.ifdef PPC_CPUOP_ENABLED
.cpu booke32
.endif
	 
EXC_COPY_CODE_START intr_entry_critical
#
# For interrupts that use the book E CSRR0/CSRR1 registers to
# save the interrupt state.
#
	mtsprg	1,%r31
	li		%r31,0
	mtmsr	%r31
	mtsprg	2,%r30
	mfcr	%r30
	mfspr	%r31,PPCBKE_SPR_CSRR1
	bittst	%r31,%r31,PPC_MSR_IS
	bne		1f
	#
	# Took a Critical Interrupt in exception handler code.
	# Just keep going.
	# 
	li		%r31,0
	mtspr	PPCBKE_SPR_CSRR1,%r31
	mtcr	%r30
	mfsprg	%r30,2
	mfsprg	%r31,1
	rfci
1:
	mtcr	%r30
	mfsprg	%r30,2
	mfsprg	%r31,1
	ENTERKERNEL EK_INTR, PPCBKE_SPR_CSRR0, PPCBKE_SPR_CSRR1, 1
	
   	SPIN_LOCK %r3,%r4,intr_slock, 1
EXC_COPY_CODE_END
