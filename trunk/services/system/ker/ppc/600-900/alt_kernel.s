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

#
# Macros to save and restore the Altivec context. 
#
.macro VMX_RESTORE,reg1
	.if REG_VSCR
		addi	&reg1,&reg1,REG_VSCR
	.endif
	lvx	%v0,0,&reg1
	mtvscr	%v0
	addi	&reg1,&reg1,REG_VMX-REG_VSCR
	lvx	%v0,0,&reg1
	addi	&reg1,&reg1,16
	lvx	%v1,0,&reg1
	addi	&reg1,&reg1,16
	lvx	%v2,0,&reg1
	addi	&reg1,&reg1,16
	lvx	%v3,0,&reg1
	addi	&reg1,&reg1,16
	lvx	%v4,0,&reg1
	addi	&reg1,&reg1,16
	lvx	%v5,0,&reg1
	addi	&reg1,&reg1,16
	lvx	%v6,0,&reg1
	addi	&reg1,&reg1,16
	lvx	%v7,0,&reg1
	addi	&reg1,&reg1,16
	lvx	%v8,0,&reg1
	addi	&reg1,&reg1,16
	lvx	%v9,0,&reg1
	addi	&reg1,&reg1,16
	lvx	%v10,0,&reg1
	addi	&reg1,&reg1,16
	lvx	%v11,0,&reg1
	addi	&reg1,&reg1,16
	lvx	%v12,0,&reg1
	addi	&reg1,&reg1,16
	lvx	%v13,0,&reg1
	addi	&reg1,&reg1,16
	lvx	%v14,0,&reg1
	addi	&reg1,&reg1,16
	lvx	%v15,0,&reg1
	addi	&reg1,&reg1,16
	lvx	%v16,0,&reg1
	addi	&reg1,&reg1,16
	lvx	%v17,0,&reg1
	addi	&reg1,&reg1,16
	lvx	%v18,0,&reg1
	addi	&reg1,&reg1,16
	lvx	%v19,0,&reg1
	addi	&reg1,&reg1,16
	lvx	%v20,0,&reg1
	addi	&reg1,&reg1,16
	lvx	%v21,0,&reg1
	addi	&reg1,&reg1,16
	lvx	%v22,0,&reg1
	addi	&reg1,&reg1,16
	lvx	%v23,0,&reg1
	addi	&reg1,&reg1,16
	lvx	%v24,0,&reg1
	addi	&reg1,&reg1,16
	lvx	%v25,0,&reg1
	addi	&reg1,&reg1,16
	lvx	%v26,0,&reg1
	addi	&reg1,&reg1,16
	lvx	%v27,0,&reg1
	addi	&reg1,&reg1,16
	lvx	%v28,0,&reg1
	addi	&reg1,&reg1,16
	lvx	%v29,0,&reg1
	addi	&reg1,&reg1,16
	lvx	%v30,0,&reg1
	addi	&reg1,&reg1,16
	lvx	%v31,0,&reg1
	addi	&reg1,&reg1,-(31*16+REG_VMX)
.endm

#
# Save VMX context. Base of save area is in reg1
#
.macro VMX_SAVE,reg1
	.if REG_VMX
		addi	&reg1,&reg1,REG_VMX
	.endif
	stvx	%v0,0,&reg1
	addi	&reg1,&reg1,16
	stvx	%v1,0,&reg1
	addi	&reg1,&reg1,16
	stvx	%v2,0,&reg1
	addi	&reg1,&reg1,16
	stvx	%v3,0,&reg1
	addi	&reg1,&reg1,16
	stvx	%v4,0,&reg1
	addi	&reg1,&reg1,16
	stvx	%v5,0,&reg1
	addi	&reg1,&reg1,16
	stvx	%v6,0,&reg1
	addi	&reg1,&reg1,16
	stvx	%v7,0,&reg1
	addi	&reg1,&reg1,16
	stvx	%v8,0,&reg1
	addi	&reg1,&reg1,16
	stvx	%v9,0,&reg1
	addi	&reg1,&reg1,16
	stvx	%v10,0,&reg1
	addi	&reg1,&reg1,16
	stvx	%v11,0,&reg1
	addi	&reg1,&reg1,16
	stvx	%v12,0,&reg1
	addi	&reg1,&reg1,16
	stvx	%v13,0,&reg1
	addi	&reg1,&reg1,16
	stvx	%v14,0,&reg1
	addi	&reg1,&reg1,16
	stvx	%v15,0,&reg1
	addi	&reg1,&reg1,16
	stvx	%v16,0,&reg1
	addi	&reg1,&reg1,16
	stvx	%v17,0,&reg1
	addi	&reg1,&reg1,16
	stvx	%v18,0,&reg1
	addi	&reg1,&reg1,16
	stvx	%v19,0,&reg1
	addi	&reg1,&reg1,16
	stvx	%v20,0,&reg1
	addi	&reg1,&reg1,16
	stvx	%v21,0,&reg1
	addi	&reg1,&reg1,16
	stvx	%v22,0,&reg1
	addi	&reg1,&reg1,16
	stvx	%v23,0,&reg1
	addi	&reg1,&reg1,16
	stvx	%v24,0,&reg1
	addi	&reg1,&reg1,16
	stvx	%v25,0,&reg1
	addi	&reg1,&reg1,16
	stvx	%v26,0,&reg1
	addi	&reg1,&reg1,16
	stvx	%v27,0,&reg1
	addi	&reg1,&reg1,16
	stvx	%v28,0,&reg1
	addi	&reg1,&reg1,16
	stvx	%v29,0,&reg1
	addi	&reg1,&reg1,16
	stvx	%v30,0,&reg1
	addi	&reg1,&reg1,16
	stvx	%v31,0,&reg1
	addi	&reg1,&reg1,REG_VSCR-(31*16+REG_VMX)
	mfvscr	%v0
	stvx	%v0,0,&reg1
	.if REG_VSCR
		addi	&reg1,&reg1,-REG_VSCR
	.endif
.endm

.section .text_kernel, "ax"

#
# VMX assist exception handler. Simply deliver SIGFPE for now.
#
	.global __exc_vmx_assist
__exc_vmx_assist:
	loadi	%r3,SIGFPE  + (FPE_NOFPU*256) + (FLTNOFPU*65536)	
	b 		__exc

#
# Fast VMX context save/restore handling. Installs rightin the exception table
#
# Note that this must be shorter than 0x1000-0xf20 = 0xe0 = 224 bytes.
#

EXC_COPY_CODE_START __exc_fvmx
	mtsprg	0,%r31
	mtsprg	1,%r30
	mtsprg	2,%r29
	mfcr	%r29
	GETCPU	%r30,2
	SMP_ADDR %r31,%r30,actives@ha
	lwz		%r31,actives@l(%r31)
	lwz		%r31,CPU_ALT_OFF(%r31)
	cmpwi	%r31,0
	beq		10f
	SMP_ADDR %r30,%r30,actives_alt@ha
	lwz		%r30,actives_alt@l(%r30)
	cmpw	%r30,%r31
	beq-	1f
	mtcr	%r29
	mfmsr	%r29
	bitset	%r29,%r29,PPC_MSR_VA
	mtmsr	%r29
	isync
	mfcr	%r29
	cmpwi	%r30,0
	bne+	4f
	mtcr	%r29
	mflr	%r29
	bla		__vmx_restore
	b		5f
4:
	mtcr	%r29
	mflr	%r29
	bla		__vmx_saverestore
5:
	mtlr	%r29
	mfcr	%r29
	GETCPU	%r30,2
	SMP_ADDR %r30,%r30,actives_alt@ha
	stw		%r31,actives_alt@l(%r30)
1:
	mtcr	%r29
	mfspr	%r31,SRR1
	bitset	%r31,%r31,PPC_MSR_VA
	mtspr	SRR1,%r31
	mfsprg	%r29,2
	mfsprg	%r30,1
	mfsprg	%r31,0
	RFE
10:
	mtcr	%r29
	mfctr	%r31
	loada	%r29,__exc_vmx_unavailable
	mtctr	%r29
	mfsprg	%r29,2
.ifdef VARIANT_smp
	mfsprg	%r30,1
.endif
	bctr
EXC_COPY_CODE_END

#
# VMX unavailable exception handler. We get here if the thread doesn't 
# have a VMX context allocated yet.
#
	.global __exc_vmx_unavailable
__exc_vmx_unavailable:
	mtctr	%r31
	mfsprg	%r31,0
	ENTERKERNEL EK_EXC, SRR0, SRR1
	bla			PPC_KERENTRY_COMMON

	# check if fpu context is already allocated
	# Later we can take this away
	lwz		%r5,CPU_ALT_OFF(%r31)
	mr.		%r5,%r5
	beq+	3f	
	# We should never get here anymore
	trap
	
3:	
	# We need to do the alloc as the kernel.
	
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
	bitset	%r3,%r3,_NTO_ATF_REGCTX_ALLOC
	stwcx.	%r3,%r0,%r4		
	bne-	2b
	b		__ker_exit
.ifdef VARIANT_smp
5:
	# Someone else is inkernel; do a NOP kernel call
	loadi	%r9,_NTO_ATF_REGCTX_ALLOC
	b		force_kernel
.endif

	.global __vmx_restore

.ifdef PPC_CPUOP_ENABLED
.cpu 7450
.endif

EXC_COPY_CODE_START ctx_save_vmx
	mfspr	%r4,SPR_VRSAVE
	stw		%r4,REG_VRSAVE(%r30)
	dssall
EXC_COPY_CODE_END

#
# Save VMX context. %r30 has base
#
__vmx_saverestore:
	VMX_SAVE %r30
# Fall through to restore...

#
# Restore VMX context. %r31 has the base of save area
#
__vmx_restore:
	VMX_RESTORE %r31
	blr

#
# Force the Altivec registers out of the CPU into the save area
# r3 = thread pointer
#
routine_start alt_force_save, 1
	# enable VMX
	mfmsr	%r0
	bitset 	%r5,%r0,PPC_MSR_VA
	mtmsr	%r5
	isync
	mr		%r7,%r3
	lwz		%r3,CPU_ALT_OFF(%r3)
	VMX_SAVE %r3
	mtmsr	%r0
	isync

.ifdef VVVARIANT_smp
	stw		%r3,FPUDATA(%r7)
	la		%r3,actives_fpu@sdarel(%r13)
	GETCPU	%r4,2
	SMP_ADDR %r3,%r4
	li		%r8,0
	stw		%r8,0(%r3)
.endif
	blr
routine_end alt_force_save


.ifdef VARIANT_smp
#
# This routine is a bit of a hack. We don't track the ownership of the
# Altivec registers properly across CPU changes on SMP like we do the
# FPU registers, so every time we leave the kernel make sure the 
# memory copy of the register contents is up to date.
#
routine_start vmx_flush, 1
     # enable VMX
     mfmsr		%r0
     bitset 	%r5,%r0,PPC_MSR_VA
     mtmsr		%r5
     isync

     VMX_SAVE	%r7

     li			%r5,0
     la			%r3,actives_alt@sdarel(%r13)
     stwx		%r5,%r25,%r3

     blr
routine_end vmx_flush
.endif
