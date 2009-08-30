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
# This file contains handlers for PPC Floating Point Processing 
#
	
	.include "asmoff.def"
	.include "ppc/util.ah"
	.include "../context.ah"
	
	.global __exc_fpu_emulation
	.global __exc_fpu_unavail

.section .text_kernel, "ax"

#
# FPU save/restore fast code for the fpu handler. %r30 has the 
# current actives_fpu, while %r31 has the new fpu context to restore
#

EXC_COPY_CODE_START __exc_ffpu
	mtsprg	0,%r31
	mtsprg	1,%r30
	mtsprg	2,%r29
	mfcr	%r29
	GETCPU	%r30,2
	SMP_ADDR %r31,%r30,actives@ha
	lwz		%r31,actives@l(%r31)

	lwz		%r30,FPUDATA(%r31)
	cmpwi	%r30,0
	beq-	10f

.ifdef VARIANT_smp
	# Context busy?
	andi.	%r30,%r30,FPUDATA_BUSY
	bne		10f
.endif
	
	GETCPU	%r30,2
	SMP_ADDR %r30,%r30,actives_fpu@ha
	lwz		%r30,actives_fpu@l(%r30)

	# Already actives_fpu?
	cmpw	%r30,%r31
	beq-	1f

	# OK, we will be switching fpu state
	mtcr	%r29
	GETCPU	%r29,2
	SMP_ADDR %r29,%r29,actives_fpu@ha
	stw		%r31,actives_fpu@l(%r29)

	mfmsr	%r29
	ori		%r29,%r29,PPC_MSR_FP
	mtmsr	%r29
	isync
	mfcr	%r29

	lwz		%r31,FPUDATA(%r31)
	rlwinm	%r31,%r31,0,0,27
	cmpwi	%r30,0
	beq-	4f

	mtcr	%r29
	lwz		%r29,FPUDATA(%r30)
	rlwinm	%r29,%r29,0,0,27
	stw		%r29,FPUDATA(%r30)
	mr		%r30,%r29
	mfcr	%r29
	FPU_SAVE 		%r30

4:	
	FPU_RESTORE 	%r31

.ifdef VARIANT_smp
#
# Set in-use and owner cpu in fpudata
#
	mtcr	%r29
	GETCPU	%r30,0
	slwi	%r31,%r30,2
	SMP_ADDR %r31,%r31,actives@ha
	lwz		%r31,actives@l(%r31)
	lwz		%r29,FPUDATA(%r31)
	or		%r29,%r29,%r30
	ori		%r29,%r29,FPUDATA_BUSY
	stw		%r29,FPUDATA(%r31)
	mfcr	%r29
.endif
1:
	mtcr	%r29
	mfspr	%r31,SRR1
	ori		%r31,%r31,PPC_MSR_FP
	mtspr	SRR1,%r31
	mfsprg	%r29,2
	mfsprg	%r30,1
	mfsprg	%r31,0
	RFE
10:
	mtcr	%r29
	mfsprg	%r29,2
	mfsprg	%r30,1
	mfsprg	%r31,0
	ENTERKERNEL EK_EXC, SRR0, SRR1
EXC_COPY_CODE_END
	

__exc_fpu_unavail:
	# check if fpu context is already allocated
	lwz		%r5,FPUDATA(%r31)
	mr.		%r5,%r5
	beq+	3f	
.ifdef VARIANT_smp
	b	fetch_fpu_context
.endif
	# We should never get here anymore
	trap
	
3:	
	# have a h/w fpu, turn on the flag and return
	
.ifdef VARIANT_smp
2:
 	lwarx	%r4,%r0,%r23
	andi.	%r5,%r4,INKERNEL_NOW+INKERNEL_LOCK
	bne-	5f
 	ori		%r4,%r4,INKERNEL_NOW+INKERNEL_LOCK
	stwcx.	%r4,%r0,%r23
 	bne-	2b

	SETCPU	%r6,%r7		# We have the kernel
.else
2:
 	lwarx	%r4,%r0,%r23		
 	ori		%r4,%r4,INKERNEL_NOW+INKERNEL_LOCK
	stwcx.	%r4,%r0,%r23
 	bne-	2b
.endif

	la		%r4,ATFLAGS(%r31)
2:
	lwarx	%r3,%r0,%r4
	bitset	%r3,%r3,_NTO_ATF_FPUSAVE_ALLOC
	stwcx.	%r3,%r0,%r4		
	bne-	2b
	b		__ker_exit

.ifdef VARIANT_smp
5:
	# Someone else is inkernel; do a NOP kernel call
	loadi	%r9,_NTO_ATF_FPUSAVE_ALLOC
	b		force_kernel
.endif

.ifdef VARIANT_smp
#
# We're on SMP, on someone has the fpu context we need. The low-order
# bits of r5 have the target CPU, we'll just send him an IPI and spin
# until it releases the context.
#
fetch_fpu_context:
	andi.	%r3,%r5,FPUDATA_CPUMASK
	li		%r4,IPI_CONTEXT_SAVE
	bl		send_ipi

	# Up the inkernel count, as we'll be going out through intr_done which decs
1:
	lwarx	%r4,%r0,%r23		# atomically increment inkernel interrupt count
	addi	%r4,%r4,1		
	stwcx.	%r4,%r0,%r23
	bne-	1b

	# Set cpupageptr[RUNCPU]->state so we unwind properly on preemption
	GETCPU	%r25,2
	LW_SMP	%r3,cpupageptr,%r25
	lwz		%r6,CPUPAGE_STATE(%r3)
	li		%r0,1
	stw		%r0,CPUPAGE_STATE(%r3)
	# Reenable interrupts
	mfmsr	%r7
	or		%r8,%r7,%r24
	mtmsr	%r8
	isync
1:
	lwz		%r5,FPUDATA(%r31)
	andi.	%r0,%r5,FPUDATA_BUSY
	bne		1b
	# Context is now free
	mtmsr	%r7						# disable interrupts
	stw		%r6,CPUPAGE_STATE(%r3)	# restore cpupageptr[RUNCPU]->state
	b		intr_done
.endif


__exc_fpu_emulation:
	loadi	%r3,SIGFPE  + (FPE_NOFPU*256) + (FLTNOFPU*65536)	
	b 		__exc_emulation
