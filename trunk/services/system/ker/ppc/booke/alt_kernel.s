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

# These _ev* macros can be removed once we're using a version of the
# assembler that supports SPE instructions

.macro _evmergehi rd,ra,rb
	.long 0b00010000000000000000001000101100 + (&rd<<21) + (&ra<<16) +(&rb<<11)
.endm

.macro _evmergelo rd,ra,rb
	.long 0b00010000000000000000001000101101 + (&rd<<21) + (&ra<<16) +(&rb<<11)
.endm

.macro _evxor rd,ra,rb
	.long 0b00010000000000000000001000010110 + (&rd<<21) + (&ra<<16) +(&rb<<11)
.endm

.macro _evmwumiaa rd,ra,rb
	.long 0b00010000000000000000010101011000 + (&rd<<21) + (&ra<<16) +(&rb<<11)
.endm

.macro _evstdd rs,d,ra
	.long 0b00010000000000000000001100100001 + (&rs<<21) + (&ra<<16) +(&d<<11)
.endm

.macro _evldd rd,d,ra
	.long 0b00010000000000000000001100000001 + (&rd<<21) + (&ra<<16) +(&d<<11)
.endm

.macro _evmra ra,rd
	.long 0b00010000000000000000010011000100 + (&rd<<21) + (&ra<<16)
.endm

.macro SAVE_EVR reg,tmp,base
.if &reg - &tmp
	_evmergehi	&tmp,&tmp,&reg
	stw			&tmp,REG_SPE_GPR_HI+(&reg*4)(&base)
.endif
.endm

.macro REST_EVR reg,tmp,base
.if &reg - &tmp
	lwz			&tmp,REG_SPE_GPR_HI+(&reg*4)(&base)
	_evmergelo	&reg,&tmp,&reg
.endif
.endm

.macro SPE_SAVE base,tmp
	_evmergehi	&tmp,&tmp,&tmp
	stw			&tmp,REG_SPE_GPR_HI+(&tmp*4)(&base)
	SAVE_EVR 0,&tmp,&base
	SAVE_EVR 1,&tmp,&base
	SAVE_EVR 2,&tmp,&base
	SAVE_EVR 3,&tmp,&base
	SAVE_EVR 4,&tmp,&base
	SAVE_EVR 5,&tmp,&base
	SAVE_EVR 6,&tmp,&base
	SAVE_EVR 7,&tmp,&base
	SAVE_EVR 8,&tmp,&base
	SAVE_EVR 9,&tmp,&base
	SAVE_EVR 10,&tmp,&base
	SAVE_EVR 11,&tmp,&base
	SAVE_EVR 12,&tmp,&base
	SAVE_EVR 13,&tmp,&base
	SAVE_EVR 14,&tmp,&base
	SAVE_EVR 15,&tmp,&base
	SAVE_EVR 16,&tmp,&base
	SAVE_EVR 17,&tmp,&base
	SAVE_EVR 18,&tmp,&base
	SAVE_EVR 19,&tmp,&base
	SAVE_EVR 20,&tmp,&base
	SAVE_EVR 21,&tmp,&base
	SAVE_EVR 22,&tmp,&base
	SAVE_EVR 23,&tmp,&base
	SAVE_EVR 24,&tmp,&base
	SAVE_EVR 25,&tmp,&base
	SAVE_EVR 26,&tmp,&base
	SAVE_EVR 27,&tmp,&base
	SAVE_EVR 28,&tmp,&base
	SAVE_EVR 29,&tmp,&base
	SAVE_EVR 30,&tmp,&base
	SAVE_EVR 31,&tmp,&base
	_evxor		&tmp,&tmp,&tmp
	_evmwumiaa	&tmp,&tmp,&tmp
	_evstdd		&tmp,REG_SPE_ACC,&base
.endm

.macro SPE_RESTORE base,tmp
	_evldd		&tmp,REG_SPE_ACC,&base
	_evmra		&tmp,&tmp
	REST_EVR 0,&tmp,&base
	REST_EVR 1,&tmp,&base
	REST_EVR 2,&tmp,&base
	REST_EVR 3,&tmp,&base
	REST_EVR 4,&tmp,&base
	REST_EVR 5,&tmp,&base
	REST_EVR 6,&tmp,&base
	REST_EVR 7,&tmp,&base
	REST_EVR 8,&tmp,&base
	REST_EVR 9,&tmp,&base
	REST_EVR 10,&tmp,&base
	REST_EVR 11,&tmp,&base
	REST_EVR 12,&tmp,&base
	REST_EVR 13,&tmp,&base
	REST_EVR 14,&tmp,&base
	REST_EVR 15,&tmp,&base
	REST_EVR 16,&tmp,&base
	REST_EVR 17,&tmp,&base
	REST_EVR 18,&tmp,&base
	REST_EVR 19,&tmp,&base
	REST_EVR 20,&tmp,&base
	REST_EVR 21,&tmp,&base
	REST_EVR 22,&tmp,&base
	REST_EVR 23,&tmp,&base
	REST_EVR 24,&tmp,&base
	REST_EVR 25,&tmp,&base
	REST_EVR 26,&tmp,&base
	REST_EVR 27,&tmp,&base
	REST_EVR 28,&tmp,&base
	REST_EVR 29,&tmp,&base
	REST_EVR 30,&tmp,&base
	REST_EVR 31,&tmp,&base
	lwz			&tmp,REG_SPE_GPR_HI+(&tmp*4)(&base)
	_evmergelo	&tmp,&tmp,&tmp
.endm

.section .text_kernel, "ax"

EXC_COPY_CODE_START ctx_save_e500_extra
	mfspr	%r4,PPCE500_SPR_SPEFSCR
	stw		%r4,REG_SPEFSCR(%r30)
EXC_COPY_CODE_END

EXC_COPY_CODE_START ctx_restore_e500_extra
	lwz		%r20,REG_SPEFSCR(%r30)
	mtspr	PPCE500_SPR_SPEFSCR,%r20
EXC_COPY_CODE_END


#
# SPE floating point data exception handler. Simply deliver SIGFPE for now.
#
	.global __exc_spe_data
__exc_spe_data:
	loadi	%r3,SIGFPE  + (FPE_NOFPU*256) + (FLTFPE*65536)	
	b 		__exc

#
# SPE floating point round handler. Simply deliver SIGFPE for now.
#
	.global __exc_spe_round
__exc_spe_round:
	loadi	%r3,SIGFPE  + (FPE_FLTRES*256) + (FLTFPE*65536)	
	b 		__exc
	

#
# Fast SPE context save/restore handling. Installs right in the exception table
#
EXC_COPY_CODE_START __exc_entry_spe_unavail
	mtsprg	0,%r31
	li		%r31,0
	mtmsr	%r31
	mtsprg	1,%r30
	mtsprg	2,%r29
	mfcr	%r29
	GETCPU	%r30,2
	SMP_ADDR %r31,%r30,actives@ha
	lwz		%r31,actives@l(%r31)
	lwz		%r31,CPU_ALT_OFF(%r31)
	cmpwi	%r31,0
	beq		10f
	mtspr	PPCBKE_SPR_SPRG7,%r28
	mfmsr	%r28
	bitset	%r28,%r28,PPC_MSR_SPE
	mtmsr	%r28
	SMP_ADDR %r30,%r30,actives_alt@ha
	lwz		%r30,actives_alt@l(%r30)
	cmpw	%r30,%r31
	beq-	1f
	cmpwi	%r30,0
	isync
	beq		4f
	SPE_SAVE 30,28
4:
	SPE_RESTORE 31,28
	GETCPU	%r30,2
	SMP_ADDR %r30,%r30,actives_alt@ha
	stw		%r31,actives_alt@l(%r30)
1:
	mfspr	%r31,SRR1
	bitset	%r31,%r31,PPC_MSR_SPE
	mtspr	SRR1,%r31
	mfspr	%r28,PPCBKE_SPR_SPRG7RO
	mtcr	%r29
	mfsprg	%r29,2
	mfsprg	%r30,1
	mfsprg	%r31,0
	rfi
10:
	mtcr	%r29
	mfsprg	%r29,2
	mfsprg	%r30,1
	mfsprg	%r31,0
	ENTERKERNEL EK_EXC, SRR0, SRR1
EXC_COPY_CODE_END

#
# SPE unavailable exception handler. We get here if the thread doesn't 
# have a context allocated yet.
#
	.global __exc_spe_unavail
__exc_spe_unavail:
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


#
# Force the SPE registers out of the CPU into the save area
# r3 = thread pointer
#
routine_start alt_force_save, 1
	# enable SPE
	mfmsr	%r0
	bitset 	%r5,%r0,PPC_MSR_SPE
	mtmsr	%r5
	lwz		%r3,CPU_ALT_OFF(%r3)
	isync
	SPE_SAVE 3,4
	mtmsr	%r0
	isync
	blr
routine_end alt_force_save

.ifdef VARIANT_smp
#
# FIXME: we don't track SPE context for SMP properly.
#        Instead, we flush the SPE context when leaving the kernel if we
#        switched threads (actives[RUNCPU] != actives_alt[RUNCPU]).
#        This ensures the memory copy of the context is up to date so we
#        only need to disable the SPE to cause the new thread to reload its
#        SPE context if necessary.
#
routine_start spe_flush, 1
	#
	# Enable SPE and save context
	#
	mfmsr		%r0
	bitset		%r5,%r0,PPC_MSR_SPE
	mtmsr		%r5
	isync

	SPE_SAVE	7,5

	#
	# Clear actives_alt[RUNCPU]
	#
	li			%r5,0
	la			%r3,actives_alt@sdarel(%r13)
	stwx		%r5,%r25,%r3

	blr
routine_end spe_flush
.endif
