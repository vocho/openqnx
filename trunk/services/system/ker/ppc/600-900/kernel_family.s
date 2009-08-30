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
# This file contains all the entry points to the kernel that are specific
# to the 604/700 series.
#
	
	.include "asmoff.def"
	.include "ppc/util.ah"
	.include "../context.ah"

	
	.global __exc_alignment700
	.global __exc_machine_check700
	.global __exc_program700
	.global __exc_trace700
	.global __exc_ibreak700

	.extern fpuemul
	.extern fpusave	
	.extern actives_alt
		
	# temp save area to be used in some exception handlers
	.set	SAVE_AREA,0x2de0	

.section .text_kernel, "ax"

EXC_COPY_CODE_START ctx_save_ear
	mfear	%r4
	stw		%r4,REG_EAR(%r30)
EXC_COPY_CODE_END

EXC_COPY_CODE_START ctx_restore_ear
	lwz		%r20,REG_EAR(%r30)
	mtear	%r20
EXC_COPY_CODE_END
	
__exc_alignment700:
.ifndef VARIANT_smp
1:
	lwarx	%r3,%r0,%r23
	ori		%r3,%r3,INKERNEL_NOW+INKERNEL_EXIT
	stwcx.	%r3,%r0,%r23
	bne-	1b
.endif

	mfdar	%r4
	mfdsisr	%r0
	b		__exc_alignment
	
__exc_machine_check700: 
	#
	# About all we can do is guess
	#
	lwz		%r3,REG_MSR(%r30)
	bittst	%r0,%r3,0x2		# is it recoverable?
	loadi	%r3,SIGBUS + (BUS_OBJERR*256) + (FLTMACHCHK*65536)
	beq-	__hardcrash		# nope
	loadi	%r3,SIGBUS + (BUS_OBJERR*256) + (FLTMACHCHK*65536)
	b 		__exc

	
__exc_program700: 
	lwz		%r5,REG_MSR(%r30)
	bittst	%r0, %r5, 0x00020000 	# TRAPFLG 	
	bne		__exc_trap
	bittst	%r0, %r5, 0x00040000 	# privilege instruction 	
	beq		1f
	loadi	%r3,SIGILL + (ILL_PRVOPC*256) + (FLTPRIV*65536)	
	b		__exc
1:	
	bittst	%r0, %r5, 0x00080000 	# illegal instruction
	beq		1f
	loadi	%r3,SIGILL + (ILL_ILLOPC*256) + (FLTILL*65536)	
	b		__exc
1:	
	bittst	%r0, %r5, 0x00100000 	# FPU exc 	
	bne		__exc_fpu
	
	loadi	%r3,SIGILL + (0*256) + (0*65536)	
	b		__hardcrash

__exc_trace700:
	lwz		%r3,REG_MSR(%r30)
	bitclr	%r3,%r3,(PPC_MSR_SE|PPC_MSR_BE)
	stw		%r3,REG_MSR(%r30)
	loadi	%r3,SIGTRAP + (TRAP_TRACE*256) + (FLTTRACE*65536)	
	b 		__exc_debug
	
__exc_ibreak700:
	mfbar	%r4
	loadi	%r3,SIGCODE_KERNEL + SIGTRAP + (TRAP_BRKPT*256) + (FLTBPT*65536)
	b 		__exc

	.global	__halt

routine_start halt, 1
	lwz 	%r0,__cpu_flags@sdarel(%r13)
	andi. 	%r9,%r0,PPC_CPU_HW_POW
	beq		1f
	mfmsr	%r5
	loadi	%r6,PPC_MSR_POW
	ori 	%r0,%r5,%r6
	sync
	isync
	mtmsr	%r0
	sync
	isync
__halt:	
	b		__halt
1:
	blr
routine_end halt
