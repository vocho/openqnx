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
# This file contains all the entry points to the kernel.
# There are serveral sections
#    Kernel call entry
#    Special kernel calls
#    Hardware interrupts
#    Exceptions
#
# While kernel calls can only originate from user space, interrupts
# and exceptions can also occur while in kernel space.
#
	.global __exc_unexpected
	.global __exc_debug
	.global __exc
	.global __exc_trap
	.global __exc_alignment
	.global __exc_access
	.global __exc_emulation
	.global __hardcrash
	.global _kdebug_opt
	.global __exc_fpu
	.global cpu_force_fpu_save

	.extern ker_stack
	.extern actives
	.extern actives_prp
	.extern aspaces_prp
	.extern actives_fpu
	.extern inkernel
	.extern cpupageptr
	.extern	memmgr
	.extern resched
	.extern kererr
	.extern fpusave_alloc
	.extern alt_context_alloc
	.extern usr_fault
	.extern intrevent_drain
	.extern specialret
	.extern _SDA_BASE_
	.extern _SDA2_BASE_
	.extern ppc_ienable_bits
	.extern intrespsave
	.extern kdebug_callout
	.extern _syspage_ptr
	.extern xfer_handlers
	.extern debug_attach_brkpts
	.extern debug_detach_brkpts
	.extern	instr_emulation
	.extern	fpu_emulation
	.extern fpu_emulation_prep
	.extern fpuemul
	.extern __halt
	.extern cpu_save_perfregs
	.extern cpu_restore_perfregs
	.extern disabled_perfregs
        
	.include "asmoff.def"
	.include "ppc/util.ah"
	.include "../context.ah"

#
# Deal with Errata #77 on the 405
#
.ifdef VARIANT_400
	.macro KSTWCX405, rs, ra, rb
		STWCX405 &rs, &ra, &rb
	.endm
	.macro SYNC405
		sync
	.endm
.else
	.macro KSTWCX405, rs, ra, rb
		stwcx.	&rs, &ra, &rb
	.endm
	.macro SYNC405
	.endm
.endif

.section ".sdata","aw"

.ifdef VARIANT_smp
#
# Data definition for inkernel and cpunum bits
#
	.align 2
	.global	cpunum
	.type	 cpunum,@object
	.size	 cpunum,1

	.global	inkernel
	.type	 inkernel,@object
	.size	 inkernel,4
cpunum:
inkernel:
	.byte	0
	.byte	0
	.byte	0
	.byte	0
.endif

.section .text_kernel, "ax"

#
# General notes about register usage in kernel.s
#
# Registers set after a full enterkernel sequence:
#	r1		stack
#	r2		short data #2
#	r13		short data
#	r29		old inkernel value
#	r30		register save area
#	r31		current actives pointer
#
#	Convenience variables (used often)
#	r23		address of inkernel variable
#	r24		ppc_ienable_bits
#   r25     (in ker_exit, on SMP) CPUID number times 4
#
#	SMP notes:
#		On entry to exception handlers, we don't set inkernel
#		immediately. Rather, we only set it once it is known
#		that we want to enter the kernel.
#
#
	

routine_start ker_start, 1
	
.ifdef VARIANT_smp
	# Set inkernel bit of 604/750
	mfmsr	%r6
	SET_CPUINKERNEL	%r6
	mtmsr	%r6
	
	GETCPU	%r3,0
	stb		%r3,cpunum@sdarel(%r13)
	slwi	%r3,%r3,2
.endif

	LW_SMP	%r1,ker_stack,%r3

.ifdef VARIANT_800
	# NOTE: This register should have been properly initialized in the
	# MemPageInit() routine in memmgr/800/mem_page.c. Setting it here
	# again seems a bit strange. The value that it's initing seems
	# awfully weird as well.
	# 	bstecher

mfspr	%r3,PPC800_SPR_MD_CTR
rlwinm	%r3,%r3,0,3,1
mtspr	PPC800_SPR_MD_CTR,%r3	
isync
.endif

	b	__ker_exit

routine_end ker_start

#
# General entry code for an exception. Gets copied into the exception
# handler area, branches to appropriate routine after context save is complete
# This sets INKERNEL_NOW. Exception handlers that set other bits should
# either override this or have their own code snippet
#
EXC_COPY_CODE_START __common_exc_entry
#
# KLUDGE: This NOP instruction gets overwritten when someone attaches
# an interrupt to something we've got an exception handler for (think
# machine check). We use it as a trigger to chain from the interrupt
# handling code to the exception handling if the interrupt id callout
# returns -1. We should really rework the kernel entry sequences to
# make this cleaner. Something for another day...
#
	nop 
	ENTERKERNEL EK_EXC, SRR0, SRR1
EXC_COPY_CODE_END

#
# The code that gets copied at the system call exception location. 
# It then calls the common kernel entry code (below), before branching
# to __ker_entry.
#
EXC_COPY_CODE_START __ker_entry_exc
	ENTERKCALL SRR0, SRR1
EXC_COPY_CODE_END

#
# Common enterkernel code sequence. This code gets copied into 
# low memory, and is called from the exception handler. It does the
# "2nd half" of the context save. Some extra CPU specific register
# saving may be copied to the end. The mmu on code is done last.
#
EXC_COPY_CODE_START enterkernel
	ENTERKERNEL_COMMON
EXC_COPY_CODE_END


#
# Common exit kernel code sequence. This code gets copied to
# low memory and intermixed with CPU specific stuff (mmu off, extra
# reg restorations, etc). Changes to this sequence have to be reflected
# in gen_exit() in init_cpu.c
#
EXC_COPY_CODE_START exitkernel_1
# extra restoring gets done here

.ifdef VARIANT_instr
	# NYI:  This code should be changed to the "copy code" style so that it
	# can be removed from CPU variants which don't support performance regs. 
	# Restore performance registers. 

	# Compare cpu.pcr field to disabled_perfregs address to see if
	# performance registers have been turned on. 

	# If nested, no need to restore regs.
	addi	%r0,%r31,REG_OFF
	cmplw	%r0,%r30
	bne+ 	2f

	lwz		%r3, CPUDATA(%r31)
	lis		%r24, disabled_perfregs@ha
	la		%r24, disabled_perfregs@l(%r24)
	cmplw	%r24, %r3
	beq+	2f
	
	# Pointer to perfregs in R3 
	lis		%r24, cpu_restore_perfregs@ha
	la		%r24, cpu_restore_perfregs@l(%r24)
	mtctr	%r24
	bctrl
2:
.endif # Instrumented restore performance registers

	# If nested, no need to disable MMU
	addi	%r0,%r31,REG_OFF
	cmplw	%r0,%r30
1:	bne-	1b				# patched when installed
EXC_COPY_CODE_END
# mmu off code gets put here
EXC_COPY_CODE_START exitkernel_2
.ifndef VARIANT_smp
	lwz		%r0,TFLAGS(%r31)
	# Check if returning from a kcall, in which case we don't need
	# to restore a bunch of registers
	bittst	%r0,%r0,_NTO_TF_KCALL_ACTIVE
1:	bne+	1b				# patched when installed
.endif
EXC_COPY_CODE_END
EXC_COPY_CODE_START exitkernel_3
# branch from exitkernel_1	comes here
	lwz		%r3,REG_XER(%r30)
	lwz		%r4,REG_CTR(%r30)
	mtxer	%r3
	mtctr	%r4
	lwz		%r4,REG_GPR+4*PPCINT(%r30)
	lwz		%r5,REG_GPR+5*PPCINT(%r30)
	lwz		%r6,REG_GPR+6*PPCINT(%r30)
	lwz		%r7,REG_GPR+7*PPCINT(%r30)
	lwz		%r8,REG_GPR+8*PPCINT(%r30)
	lwz		%r9,REG_GPR+9*PPCINT(%r30)
	lwz		%r10,REG_GPR+10*PPCINT(%r30)
	lwz		%r11,REG_GPR+11*PPCINT(%r30)
	lwz		%r12,REG_GPR+12*PPCINT(%r30)
EXC_COPY_CODE_END
# branch from exitkernel_2 comes here
EXC_COPY_CODE_START exitkernel_4


	lwz		%r13,REG_GPR+13*PPCINT(%r30)
	lwz		%r14,REG_GPR+14*PPCINT(%r30)
	lwz		%r15,REG_GPR+15*PPCINT(%r30)
	lwz		%r16,REG_GPR+16*PPCINT(%r30)
	lwz		%r17,REG_GPR+17*PPCINT(%r30)
	lwz		%r18,REG_GPR+18*PPCINT(%r30)
	lwz		%r19,REG_GPR+19*PPCINT(%r30)
	lwz		%r20,REG_GPR+20*PPCINT(%r30)
	lwz		%r21,REG_GPR+21*PPCINT(%r30)
	lwz		%r22,REG_GPR+22*PPCINT(%r30)
	lwz		%r23,REG_GPR+23*PPCINT(%r30)
	lwz		%r24,REG_GPR+24*PPCINT(%r30)
	lwz		%r25,REG_GPR+25*PPCINT(%r30)
	lwz		%r26,REG_GPR+26*PPCINT(%r30)
	lwz		%r27,REG_GPR+27*PPCINT(%r30)
	lwz		%r28,REG_GPR+28*PPCINT(%r30)
EXC_COPY_CODE_END

#
# 2nd half of EXITKERNEL sequence, restores the regs saved in ENTERKERNEL_EXC
#
EXC_COPY_CODE_START exitkernel_5a
	lwz		%r0,REG_GPR+0*PPCINT(%r30)
EXC_COPY_CODE_END
EXC_COPY_CODE_START exitkernel_5b
	# This code block only gets included when PPC_CPU_STWCX_BUG is on in
	# __cpu_flags, otherwise exitkernel_5a is used. It's used to work around
	# CPU errata 25 on certain Freescale chips that can hang up on the
	# stwcx. instruction at the start of exitkernel_6.
	lwarx	%r0,0,%r30
EXC_COPY_CODE_END

EXC_COPY_CODE_START exitkernel_6
	# This is a dummy stwcx. to clear any reservation before we exit
	# (%r0 needs to be at offset zero of the register save area).
.if REG_GPR+0*PPCINT
	.err
.endif
	stwcx.	%r0,0,%r30
	lwz		%r29,REG_LR(%r30)
	lwz		%r1,REG_MSR(%r30)
.ifdef VARIANT_900
	lwz		%r2,REG_MSR_U(%r30)
	rldicr	%r2,%r2,32,31
	or		%r1,%r1,%r2
.endif	
   	lwz		%r2,REG_IAR(%r30)
	lwz		%r3,REG_CR(%r30)
	mtlr	%r29
	mtspr	SRR1,%r1
	mtspr	SRR0,%r2
	mtcrf	0xff,%r3
	lwz		%r1,REG_GPR+1*PPCINT(%r30)
	lwz		%r2,REG_GPR+2*PPCINT(%r30)
	lwz		%r3,REG_GPR+3*PPCINT(%r30)
	lwz		%r29,REG_GPR+29*PPCINT(%r30)
	lwz		%r31,REG_GPR+31*PPCINT(%r30)
	lwz		%r30,REG_GPR+30*PPCINT(%r30)	# has to be done _last_
	SYNC405 
	RFE
1:	b	1b	# stop instruction pre-fetcher from going any further
EXC_COPY_CODE_END
	
#
# Entry point into the kernel for kernel calls
#

routine_start __ker_entry, 1
	mfmsr	%r4
	or		%r5,%r4,%r24
	
.ifdef VARIANT_smp

	# Try to acquire the kernel. Enable interrupts so we can be preempted
	GETCPU	%r6,24
	loadi	%r7,0x00ffffff
acquire_kernel_beg:
	mtmsr	%r5	 # enable interrupts
1:
	lwarx	%r29,0,%r23
	bittst	%r8,%r29,INKERNEL_NOW
	bne-	1b
	mtmsr	%r4	 # disable interrupts
acquire_kernel_end:
	and		%r8,%r29,%r7
	or		%r8,%r8,%r6
	ori		%r8,%r8,INKERNEL_NOW
	stwcx.	%r8,0,%r23
	bne-	acquire_kernel_beg
	
.else

	lwz		%r29,0(%r23)
	ori		%r8,%r29,INKERNEL_NOW
	stw		%r8,0(%r23)
	
.endif

	mtmsr	%r5	 # enable interrupts
	
	lwz		%r6,TFLAGS(%r31)
	loadi	%r4,(_NTO_TF_KERERR_SET | _NTO_TF_BUFF_MSG)
	slwi	%r3,%r0,2
	andc	%r5,%r6,%r4
	stw		%r0,SYSCALL(%r31)
	
.ifdef VARIANT_instr
.equ	kcall_table,_trace_call_table
.else
.equ	kcall_table,ker_call_table
.endif

	addis	%r3,%r3,kcall_table@ha
.ifndef VARIANT_smp
	# @@@ TF NOTE: Doing this causes us to not properly re-load
	# all the registers, for example when the fp emulator is called. 
	# This optimization can be used if we clear the flag when an 
	# interrupt/exception is serviced.  Until we do that I've commented 
	# this code out so full register sets are always copied.
	#
	# bitset	%r5,%r5,_NTO_TF_KCALL_ACTIVE
.endif
	cmplwi	%r0,__KER_BAD
	stw		%r5,TFLAGS(%r31)
	lwz		%r4,kcall_table@l(%r3)
	bge-	bad_func
	mtctr	%r4
	ori		%r3,%r31,0 		# mr
	la		%r4,REG_OFF+REG_GPR+3*PPCINT(%r31)
	bctrl
	mr.		%r4,%r3
	bge-	set_err

enoerror:
	#
	# Fall through to __ker_exit.
	#
routine_end __ker_entry
	
routine_start __ker_exit, 1 
	GETCPU	%r25,2
	LW_SMP	%r31,actives,%r25

	# Reload inkernel and ppc_ienable_bits; they may get clobbered
	# when we preempt a msg xfer
	la		%r23,inkernel@sdarel(%r13)
	lwz		%r24,ppc_ienable_bits@sdarel(%r13)
	la		%r30,REG_OFF(%r31)

	mfmsr	%r5
	andc	%r16,%r5,%r24
#
# On UP, disable interrupts to set inkernel -- this appears to be way 
# faster than using lwarz/stwcx Interrupts are only disabled
# for 5 opcodes.
# Also save %r16 for later
#
.ifdef VARIANT_smp
	or		%r6,%r5,%r24

	li		%r4,((INKERNEL_NOW+INKERNEL_LOCK+INKERNEL_EXIT)>>8)
	stb		%r4,2+inkernel@sdarel(%r13)
.else
	lwz		%r4,inkernel@sdarel(%r13)
	or		%r6,%r5,%r24
 	ori		%r7,%r4,INKERNEL_NOW+INKERNEL_LOCK+INKERNEL_EXIT
 	stw		%r7,inkernel@sdarel(%r13)
.endif
#
# Make sure interrupts are on
#
	mtmsr	%r6

#
# Check for any pending events.
#
	lwz		%r5,intrevent_pending@sdarel(%r13)
	cmpwi	%r5,0
	bne-	__ker_intrevent
	
#
# Check for a process switch since we may need to remove breakpoints
#
	la		%r26,actives_prp@sdarel(%r13)
	SMP_ADDR %r26,%r25
	lwz		%r27,PROCESS(%r31)			# Note: dbg_check code uses %r27
	lwz		%r28,0(%r26)
	cmpw	%r27,%r28
	beq+	dbgret

	# R26 - &actives_prp, R27 - PROCESS(R31), R28 - actives_prp
	cmpwi	%r28,0
	beq-	dbgret
	lwz		%r22,DEBUGGER(%r28)
	cmpwi	%r22,0
	beq+	dbgret
	la		%r4,aspaces_prp@sdarel(%r13)
	SMP_ADDR %r4,%r25
	lwz		%r5,0(%r4)
	cmpw	%r28,%r5
	beq		1f
	lis		%r8,MEMMGR_ASPACE + memmgr@ha
	lwz		%r8,MEMMGR_ASPACE + memmgr@l(%r8)
	mtctr	%r8
	mr		%r3,%r28
	bctrl
1:
	lwz		%r8,debug_detach_brkpts@sdarel(%r13)
	mtctr	%r8						# remove BP's from old process
	mr		%r3,%r22
	bctrl

dbgret:

#
# Check for a aspace switch since we may need to change aspace mappings
#
	lwz		%r3,ASPACE_PRP(%r31)		# Note: aspace_switch code uses %r3/%r4
	la		%r4,aspaces_prp@sdarel(%r13)
	SMP_ADDR %r4,%r25
	lwz		%r5,0(%r4)					
	cmpw	%r3,%r5
	beq+	aspaceret

	# R3 - ASPACES_PRP(R31), R4 - &aspaces_prp
	# R26 - &actives_prp, R27 - PROCESS(R31), R28 - actives_prp

	cmpwi	%r3,0
	beq-	aspaceret
	lis		%r8,MEMMGR_ASPACE + memmgr@ha
	lwz		%r8,MEMMGR_ASPACE + memmgr@l(%r8)
	mtctr	%r8
	bctrl

aspaceret:

#
# Check for a process switch since we may need to add breakpoints and change PLS
#
	cmpw	%r27,%r28
	beq+	prpret

	# R26 - &actives_prp, R27 - PROCESS(R31), R28 - actives_prp

	stw		%r27,0(%r26)
	# Set process local storage
	LW_SMP	%r3,cpupageptr,%r25
	lwz		%r4,PLS(%r27)
	stw		%r4,CPUPAGE_PLS(%r3)
	lwz		%r3,DEBUGGER(%R27)
	cmpwi	%r3,0
	beq+	prpret
	lwz		%r8,debug_attach_brkpts@sdarel(%r13)
	mtctr	%r8						# set BP's for new process
	bctrl

prpret:

#
# Check for special actions
#
	lwz		%r28,TFLAGS(%r31)
	andi.	%r0,%r28,_NTO_TF_SPECRET_MASK
	bne-	__ker_specialret
#
# Save/Restore FPU
#
	lwz		%r6,fpuemul@sdarel(%r13)

	li		%r8,0
	cmpwi	%r6,0
	bne		1f

	LW_SMP	%r5,actives_fpu,%r25
	cmpw	cr1,%r5,%r31
	beq+	cr1,1f

	# flip the fpe in the active thread's msr, 
	# to let it cause an fp exc when it want to use fpu regs 
	# so we can do context switch then

	bitset	%r8,%r8,PPC_MSR_FP
1:

.ifdef VARIANT_600
.set PPC_CPU_ALT_SUPPORT,PPC_CPU_ALTIVEC
.set PPC_MSR_ALT_ENABLE,PPC_MSR_VA
.ifdef VARIANT_smp
.set PPC_FLUSHER,vmx_flush
.endif
.endif

.ifdef VARIANT_900
.set PPC_CPU_ALT_SUPPORT,PPC_CPU_ALTIVEC
.set PPC_MSR_ALT_ENABLE,PPC_MSR_VA
.ifdef VARIANT_smp
.set PPC_FLUSHER,vmx_flush
.endif
.endif

.ifdef VARIANT_booke
.set PPC_CPU_ALT_SUPPORT,PPC_CPU_SPE
.set PPC_MSR_ALT_ENABLE,PPC_MSR_SPE
.ifdef VARIANT_smp
.set PPC_FLUSHER,spe_flush
.endif
.endif

#
# Save/Restore altivec/SPE
#
.ifdef PPC_CPU_ALT_SUPPORT
	lwz		%r6,__cpu_flags@sdarel(%r13)
	bittst	%r6,%r6,PPC_CPU_ALT_SUPPORT
	beq		1f

	LW_SMP	%r7,actives_alt,%r25
	lwz		%r6,CPU_ALT_OFF(%r31)
	cmpw	cr1,%r7,%r6
	beq+	cr1,1f

.ifdef PPC_FLUSHER
	cmpwi	%r7,0
	bnel	PPC_FLUSHER
.endif	
	# flip the altivec/SPE in the active thread's msr, 
	# to let it cause an alternate reg exc when it want to use the regs 
	# so we can do context switch then

	bitset	%r8,%r8,PPC_MSR_ALT_ENABLE
1:
.endif

	cmplwi	%r8,0
	beq		1f
	lwz		%r9,REG_OFF+REG_MSR(%r31)
	andc	%r9,%r9,%r8
	stw		%r9,REG_OFF+REG_MSR(%r31)
1:

.ifdef VARIANT_instr
	lwz		%r5,ker_exit_enable_mask@sdarel(%r13)
	cmpwi	%r5,0
	beq		skip_ker_exit
	mr		%r3,%r31
    bl		_trace_ker_exit
	
skip_ker_exit:
.endif

__kerret:
	# disable interrupts -- use %r16 saved above
	# -- this would need to change if we ever get to __kerret directly
	mtmsr	%r16
	lwz		%r5,intrevent_pending@sdarel(%r13)
	la		%r3,ATFLAGS(%r31)
	cmpwi	%r5,0
	bne-	__ker_intrevent

.ifdef VARIANT_smp
1:
	lwarx	%r14,%r0,%r3
	KSTWCX405	%r5,%r0,%r3		# we know R5 is zero from __ker_intrevent branch
	bne-	1b
.else
	lwz		%r14,0(%r3)
	stw		%r5,0(%r3)
.endif
	
	cmpwi	%r14,0
	bne-	__ker_atflags

.ifdef VARIANT_smp
	# Make sure all stores are visible before we release kernel
	sync
	stb		%r5,2+inkernel@sdarel(%r13)
.else
	stw		%r5,inkernel@sdarel(%r13)	# R5 is zero from __ker_intrevent branch
.endif

#
# Set thread local storage
#

__ker_exit2:
	lwz		%r4,TLS(%r31)
	GETCPU	%r25,2
	LW_SMP	%r3,cpupageptr,%r25
	stw		%r4,CPUPAGE_TLS(%r3)

__nmi_lo:
__keriret:
	ba	PPC_KEREXIT_COMMON

routine_end __ker_exit
	
do_fpusave_alloc:
	b	fpusave_alloc
	
do_alt_context_alloc:
	b	alt_context_alloc
	
.ifdef VARIANT_smp
do_user_fault:
	b	usr_fault
.endif

   	#
	# R14 has ATFLAGS(%r31)
	# On SMP, we have the kernel spinlock
	#
__ker_atflags:

.ifdef VARIANT_smp

	bittst	%r0,%r14,_NTO_ATF_FORCED_KERNEL
	beq+	1f

	# restore regs; IRQ's need to be off, as this needs to be atomic
	lwz		%r6,ARGS_ASYNC_IP(%r31)
	lwz		%r7,ARGS_ASYNC_TYPE(%r31)
	stw		%r6,REG_IAR(%r30)
	stw		%r7,REG_GPR+0*PPCINT(%r30)
1:

.endif

	# Turn IRQ's back on
	mfmsr	%r0
	or		%r0,%r0,%r24
	mtmsr	%r0

#
# Careful: the order here is important, as some of the async ops may
# change active.	
#
	bittst	%r0,%r14,_NTO_ATF_FPUSAVE_ALLOC
	bnel	do_fpusave_alloc				# can't reach "fpusave_alloc"
	bittst	%r0,%r14,_NTO_ATF_REGCTX_ALLOC
	bnel	do_alt_context_alloc				# can't reach "alt_alloc"

.ifdef VARIANT_smp
	bittst	%r0,%r14,_NTO_ATF_SMP_EXCEPTION
	beq		1f
	lwz		%r27,ARGS_ASYNC_FAULT_ADDR(%r31)
	lwz		%r5,ARGS_ASYNC_FAULT_TYPE(%r31)
	lwz		%r3,ARGS_ASYNC_CODE(%r31)
	andi.		%r6,%r3,0xff
	cmpwi		%r6,SIGSEGV
	bne		3f

	mr		%r4,%r27
	LW_SMP	%r3,aspaces_prp,%r25
	bl		vmm_fault_shim
	cmpwi	%r3,0
	beq+	1f
3:
	mr		%r4,%r31
	mr		%r5,%r27
	bl		do_user_fault
	
1:
   	lwz		%r11,resched@sdarel(%r13)
	mtctr	%r11
	bittst	%r0,%r14,(_NTO_ATF_SMP_RESCHED|_NTO_ATF_TIMESLICE)
	bnectrl

.else

   	lwz		%r11,resched@sdarel(%r13)
	mtctr	%r11
	bittst	%r0,%r14,_NTO_ATF_TIMESLICE
	bnectrl
	
.endif
	b		__ker_exit

__ker_intrevent:
	mfmsr	%r0
	or		%r0,%r0,%r24
	mtmsr	%r0
	bl		intrevent_drain
	b		__ker_exit
	
__ker_specialret:
	# Reset kernel stack; do it atomically, IRQ's are on
	LW_SMP	%r3,ker_stack,%r25
	ori		%r1,%r3,0
	
.ifdef VARIANT_smp		# Check to see if we forced a NOP kernel call
	mfmsr	%r0
	andc	%r3,%r0,%r24
	mtmsr	%r3
	lwz		%r3,ATFLAGS(%r31)
	loadi	%r4,_NTO_ATF_FORCED_KERNEL
	and.	%r5,%r3,%r4
	beq		1f
	
	# restore regs
	lwz		%r6,ARGS_ASYNC_IP(%r31)
	lwz		%r7,ARGS_ASYNC_TYPE(%r31)
	stw		%r6,REG_IAR(%r30)
	stw		%r7,REG_GPR+0*PPCINT(%r30)

	# Clear flag atomically
	la		%r8,ATFLAGS(%r31)
5:
	lwarx	%r3,0,%r8
	andc	%r3,%r3,%r4
	stwcx.	%r3,0,%r8
	bne-	5b

1:
	mtmsr	%r0
.endif

	mr		%r3,%r31
	bl		specialret
	b		__ker_exit
	
set_err:
	# NOTE: We're setting the error on the initial active thread,
	# not on what's in actives[KERNCPU] after the call.
	# This last branch uses the cr bits from above...
	bne-	1f
.ifdef VARIANT_smp
	li		%r7,((INKERNEL_NOW+INKERNEL_LOCK+INKERNEL_EXIT)>>8)
	stb		%r7,2+inkernel@sdarel(%r13)
.else
 	li		%r7,INKERNEL_NOW+INKERNEL_LOCK+INKERNEL_EXIT
 	stw		%r7,inkernel@sdarel(%r13)
.endif
	stw		%r4,REG_OFF+REG_GPR+0*PPCINT(%r31)
	# @@@we could make it more efficient and not do the lock again at ker_exit
	b		enoerror
	
bad_func:
	loadi	%r4,ENOSYS
1:
	mr		%r3,%r31
	bl		kererr
	b		__ker_exit
	
#
# Overall interrupt handler for PPC systems.
# This code is not used in place. Rather, it is copied elsewhere and
# intermixed with interrupt controller bursts from the startup module.
# Make sure that only position independent code transfers are used here
# (referencing absolute data addresses is OK).
# 

EXC_COPY_CODE_START intr_entry
	ENTERKERNEL EK_INTR, SRR0, SRR1
	
   	SPIN_LOCK %r3,%r4,intr_slock, 1
EXC_COPY_CODE_END
	 
routine_start intr_process_queue, 1
#	
#	Virtually speaking, the interrupt queue is processed
#	at this point. Later on, interrupt() should be coded in assembly
#	and moved here for efficiency's sake.
#	 
	mr		%r3,%r15
.ifdef VARIANT_smp
	SPIN_UNLOCK %r4,intr_slock
	mflr	%r20
	
	# call the C function
	bl		interrupt
	
	SPIN_LOCK %r3,%r4,intr_slock
	
	mtlr	%r20
	blr
.else
	# Just call the C function
	b	interrupt
.endif
routine_end	intr_process_queue
	 
routine_start intr_transfer_exc, 1
#
# KLUDGE: The interrupt id callout returned -1 and we've got
# and exception handler for the vector in the table. This routine
# munges the inkernel variable so that we're no longer handling an
# interrupt and (if non-SMP) indicates that we're now in the kernel
# before transfering to the original exception handler.
#
	SPIN_UNLOCK %r6,intr_slock
1:
	lwarx	%r29,%r0,%r23		
	subi	%r6,%r29,1	 # atomically decrement inkernel interrupt count
.ifndef VARIANT_smp	
	bitset	%r6,%r6,INKERNEL_NOW
.endif
	KSTWCX405	%r6,%r0,%r23
	bne-	1b
	blr	
routine_end	intr_transfer_exc

routine_start tlb_flush_all_tlbia,1
	tlbia
	blr
routine_end tlb_flush_all_tlbia

routine_start tlb_flush_all_64,1
	li		%r3,64
	# fall through
routine_end tlb_flush_all_64
tlb_flush_all_XX:
	# have to flush the entries one by one :-(.
	mtctr	%r3
	li		%r3,0
1:
	tlbie	%r3
	addi	%r3,%r3,0x1000
	bdnz	1b
	blr

.ifdef VARIANT_smp
#
# IPI processing code. The generated interrupt code will jump here
# in this case.
#

	.global intr_process_ipi
intr_process_ipi:
	SPIN_UNLOCK %r3,intr_slock
	
	GETCPU	%r25,2
	li		%r5,0
	la		%r3,ipicmds@sdarel(%r13)
	SMP_ADDR %r3,%r25

	LW_SMP	%r26,cpupageptr,%r25			# r26 = cpupageptr[RUNCPU]
	lwz		%r27,CPUPAGE_STATE(%r26)		# r27 = current state value
	li		%r4,1
	stw		%r4,CPUPAGE_STATE(%r26)			# cpupageptr[RUNCPU]->state = 1

1:
	lwarx	%r19,%r0,%r3
	stwcx.	%r5,%r0,%r3
	bne		1b

	andi.	%r7,%r19,IPI_PARKIT
	beq		no_parkit
	
	# Freeze the system, we've got a kernel dump happening
	rlwinm	%r5,%r25,30,2,31
	la		%r6,alives@sdarel(%r13)
	lbzx	%r7,%r5,%r6
	ori		%r7,%r7,0x2
	stbx	%r7,%r5,%r6	# mark the CPU as parked

2:	b		2b	

no_parkit:
	# Remember big-endian ordering...
	andi.	%r7,%r19,IPI_TLB_SAFE
	beq		no_safe_aspace
	
	# Are we nested??
	lwz		%r7,REG_MSR(%r30)
	IS_CPUINKERNEL	%r7, %r10
	bne-	no_safe_aspace

.if 0
	# Can't figure out why this does not work :-(
	# Was procnto active?
	LW_SMP	%r10,actives,%r25
	lwz		%r10,PROCESS(%r10)
	lwz		%r10,PID(%r10)
	cmpwi	%r10,1
	bgt		no_safe_aspace
.endif

	# Need to set safe aspace 
	mflr	%r21			# Save link reg.
	GETCPU	%r3,0
	bl		set_safe_aspace
	mtlr	%r21
	# redisable interrupts
	mfmsr	%r5
	andc	%r5,%r5,%r24
	mtmsr	%r5
	
no_safe_aspace:
	andi.	%r7,%r19,IPI_CLOCK_LOAD
	beq		no_clock_load

	mflr	%r21			# Save link reg.
	bl		clock_load
	mtlr	%r21
	# redisable interrupts
	mfmsr	%r5
	andc	%r5,%r5,%r24
	mtmsr	%r5
	
no_clock_load:
	andi.	%r7,%r19,IPI_TLB_FLUSH
	beq		no_tlb_flush
	
   	lwz		%r3,tlb_flush_all@sdarel(%r13)
	mflr	%r21			# Save link reg.
	mtlr	%r3
	blrl
	mtlr	%r21
	sync
no_tlb_flush:
	li		%r5,0
	
	andi.	%r7,%r19,IPI_RESCHED
	beq		no_resched
	bitset	%r5,%r5,_NTO_ATF_SMP_RESCHED
no_resched:
	andi.	%r7,%r19,IPI_TIMESLICE
	beq		no_timeslice
	bitset	%r5,%r5,_NTO_ATF_TIMESLICE
no_timeslice:
	andi.	%r7,%r19,IPI_CONTEXT_SAVE
	beq		no_fpu_save

	# We need to save the FPU registers
	la		%r10,actives_fpu@sdarel(%r13)
	SMP_ADDR %r10,%r25
	li		%r4,0
	lwz		%r9,0(%r10)
	cmpw	%r9,%r4
	beq-	no_fpu_save
	lwz		%r3,FPUDATA(%r9)
	rlwinm	%r3,%r3,0,0,27
	# OK, we have a save area pointer... Store the registers
	mfmsr	%r7
	bitset 	%r8,%r7,PPC_MSR_FP
	mflr	%r21
	mtmsr	%r8
	isync
	bl		cpu_hwfpu_save
	mtmsr	%r7
	isync
	mtlr	%r21
	stw		%r4,0(%r10)
	# Indicate that this context is no longer in use
	stw		%r3,FPUDATA(%r9)

no_fpu_save:


	mr.		%r5,%r5
	beq-	6f	

	lwz		%r6,REG_MSR(%r30)
	IS_CPUINKERNEL	%r6,%r9
	beq		1f

	# See if we were spining trying to acquire the kernel
	lwz		%r7,REG_IAR(%r30)
	loada	%r6,acquire_kernel_beg
	cmplw	%r7,%r6
	blt		2f
	addi	%r6,%r6,acquire_kernel_end - acquire_kernel_beg
	cmplw	%r7,%r6
	bgt		2f
	bitset	%r5,%r5,_NTO_ATF_WAIT_FOR_KER

1:	
	bitset	%r5,%r5,_NTO_ATF_FORCED_KERNEL

2:
	la		%r9,ATFLAGS(%r31)
	lwarx	%r3,0,%r9
	or		%r4,%r5,%r3
	stwcx.	%r4,0,%r9
	bne-	2b

	bittst	%r4,%r3,_NTO_ATF_FORCED_KERNEL
	bne		6f

	bittst	%r4,%r5,_NTO_ATF_FORCED_KERNEL
	beq		6f

	# Was in user mode. Force entry to kernel
	
	lwz		%r9,REG_IAR+REG_OFF(%r31)

	bittst	%r4,%r5,_NTO_ATF_WAIT_FOR_KER
	beq+	5f
	# We were spining waiting for the kernel. Pretend the IPI happened
	# out in userland.
	subi	%r9,%r9,KER_ENTRY_SIZE
	la		%r30,REG_OFF(%r31)
5:	
	lwz		%r8,REG_GPR+0*PPCINT+REG_OFF(%r31)
	lwz		%r10,kercallptr@sdarel(%r13)

	stw		%r8,ARGS_ASYNC_TYPE(%r31)
	stw		%r9,ARGS_ASYNC_IP(%r31)
	loadi	%r8,__KER_NOP
	stw		%r8,REG_GPR+0*PPCINT+REG_OFF(%r31)
	stw		%r10,REG_IAR+REG_OFF(%r31)
6:	

	mfmsr	%r4
	andc	%r4,%r4,%r24
	mtmsr	%r4
	isync
	
	stw		%r27,CPUPAGE_STATE(%r26)		# restore cpupageptr[RUNCPU]->state

	SPIN_LOCK %r3,%r4,intr_slock
	blr
.endif

routine_start intr_done_chk_fault, 1
#
#	All done dispatching this interrupt. An EOI has been sent to the
#	interrupt controller by the startup/generated code. Check to see
#	if a cpu exception has occurred and handle it if so. If not,
#	figure out how to continue the interrupted context.
#	Interupts are disabled.
#	
	cmpwi	%r3,0							# any exception?
	beq-	intr_done
	
	SPIN_UNLOCK %r5,intr_slock

1:
	lwarx	%r29,%r0,%r23		# atomically decrement inkernel interrupt count
	subi	%r6,%r29,1		
	KSTWCX405	%r6,%r0,%r23
	bne-	1b

	b		__exc				# handle exception
routine_end	intr_done_chk_fault

routine_start intr_done, 1
#
#	All done dispatching this interrupt. An EOI has been sent to the
#	interrupt controller by the startup/generated code.
#	Figure out how to continue the interrupted context.
#	Interupts are disabled.
#	
	SPIN_UNLOCK	%r5,intr_slock

1:
	lwarx	%r4,%r0,%r23		# atomically decrement inkernel interrupt count
	subi	%r4,%r4,1		
	KSTWCX405	%r4,%r0,%r23
	bne-	1b

#
# We have 6 cases to consider in an smp system. (cases 2, 5, 6 do not apply
# 	to non-SMP systems)
#          This CPU      Another CPU    Action
#          --------      -----------    ------
# case1    from user     in user        Become the kernel
# case2*   from user     in kernel      Check preempt and maybe ipi (SMP)
# case3    from kernel   in user        Check preempt and maybe become kernel
# case4    from intr     in user        Return to user
# case5*   from intr     in kernel      Return to user (SMP)
# case6*   spin kentry   *              Try to become kernel, else return (SMP)
#

.ifdef VARIANT_smp
	# Check INKERNEL bit of saved registers
	lwz		%r3,REG_MSR(%r30)
	IS_CPUINKERNEL	%r3, %r7
	bne-	from_kerorintr	# This CPU was either in kernel or intr

	# Fall through for cases 1 or 2

from_user:
case1:
	# Try to become the kernel
1:
	lwarx	%r4,%r0,%r23
	andi.	%r6,%r4,INKERNEL_NOW+INKERNEL_LOCK
	bne-	case2
	ori		%r4,%r4,INKERNEL_NOW+INKERNEL_LOCK	# lock us down
	KSTWCX405	%r4,%r0,%r23
	bne-	1b
	
	SETCPU	%r5,%r6								# We have the kernel
	b		__ker_exit

case2:
	andi.	%r0,%r4,INKERNEL_LOCK+INKERNEL_SPECRET
	bne		__ker_exit2
	
	# The test below is not strictly valid; the other CPU may
	# be running a thread at higher (or lower) prio than ours, so 
	# sending it an IPI could result in a lot of unnecesary activity..
	lbz		%r0,PRIORITY(%r31)
	lwz		%r3,queued_event_priority@sdarel(%r13)
	cmpw	%r0,%r3
	bge		__ker_exit2	

	# Alright, send IPI to the CPU that's in kernel; 
	lbz		%r3,cpunum@sdarel(%r13)
	li		%r4,IPI_CHECK_INTR
	bl		send_ipi
	b		__ker_exit2		# Exit without touching inkernel
	
.else

	# For non-SMP, branch to __ker_exit if not nested
	cmpwi	%r4,0			# are we nested? 
	beq+	__ker_exit
	
.endif
	
	
from_kerorintr:
.ifdef VARIANT_smp
	# Check cpupageptr[RUNCPU]->state to see if we were in an interrupt
	GETCPU	%r25,2
	LW_SMP	%r26,cpupageptr,%r25
	lwz		%r0,CPUPAGE_STATE(%r26)
	andi.	%r0,%r0,1
	bnea-	PPC_KEREXIT_COMMON	# This CPU was in intr
	andi.	%r0,%r4,INKERNEL_LOCK
	bnea	PPC_KEREXIT_COMMON	# We were in kernel and locked
.else
	# R4 has the current inkernel value; 
	# We can examine the INKERNEL bits only if we were the kernel

	andi.	%r0,%r4,INKERNEL_INTRMASK+INKERNEL_LOCK
	bnea	PPC_KEREXIT_COMMON	# __keriret handles case 4
.endif

case3:
	lwz		%r3,REG_IAR(%r30)
.ifdef VARIANT_smp
	lis		%r5,acquire_kernel_beg@ha
	la		%r5,acquire_kernel_beg@l(%r5)
	cmplw	%r3,%r5
	ble		1f
	addi	%r5,%r5,acquire_kernel_end - acquire_kernel_beg
	cmplw	%r3,%r5
	ble		case6
1:
.endif
	
	# Were we in halt?
	lis		%r5,__halt@ha
	la		%r5,__halt@l(%r5)
	cmpw	%r5,%r3
	bne		1f
	# Step over loop instruction
	addi	%r3,%r3,4
	stw		%r3,REG_IAR(%r30)
1:
	lbz		%r0,PRIORITY(%r31)
	lwz		%r3,queued_event_priority@sdarel(%r13)
	cmpw	%r0,%r3
	bgea	PPC_KEREXIT_COMMON

preempt:
	andi.	%r0,%r4,INKERNEL_EXIT
	bne		1f
	lwz		%r3,REG_OFF+REG_IAR(%r31)
	subi	%r3,%r3,KER_ENTRY_SIZE
	stw		%r3,REG_OFF+REG_IAR(%r31)
1:
	li		%r5,INKERNEL_SPECRET
1:
	lwarx	%r4,%r0,%r23		
	ori		%r4,%r4,INKERNEL_NOW+INKERNEL_LOCK	# lock us down
	andc	%r4,%r4,%r5			# make sure SPECRET bit is off
	KSTWCX405	%r4,%r0,%r23
	bne-	1b
	
	lwz		%r3,xfer_handlers@sdarel(%r13)	# were we doing a message pass?
	cmpwi	%r3,0
	beq-	__ker_exit
	li		%r0,0
	stw		%r0,xfer_handlers@sdarel(%r13)
	lwz		%r0,4(%r3)			# get address of restart code
	cmpwi	%r0,0
	beq-	__ker_exit			# if no handler, abort entire call
	mtctr	%r0
	mr		%r3,%r31
	mr		%r4,%r30
	bctrl						# invoke handler
	b		__ker_exit
	
.ifdef VARIANT_smp
case6:	
	# We were attempting to acquire kernel for a kernel call. Back up
	# IAR saved in actives[] and act like we're coming from user mode.
	
	# ??? X86 code makes sure interrupts are enabled in the saved context.
	# ??? I don't think that's required here.
	la		%r30,REG_OFF(%r31)
	lwz		%r3,REG_IAR(%r30)
	subi	%r3,%r3,KER_ENTRY_SIZE
	stw		%r3,REG_IAR(%r30)
	b		case1
.endif
	
routine_end intr_done
	
	
__exc_trap:
	# @@@ this should retrieve opcode using the paddr
	lwz		%r6,REG_IAR(%r30)
	loadi	%r3,SIGTRAP + (TRAP_BRKPT*256) + (FLTBPT*65536)
	lwz		%r4,0(%r6)			# get faulting instruction
	loadi	%r5,0x7fe00008		# brkpoint
	cmpw	%r4,%r5
	beq+	__exc_debug
	loadi	%r5,0x7d821008		# GDB's brkpoint instruction (twge R2,R2)
	cmpw	%r4,%r5
	beq+	__exc_debug
	loadi	%r5,0x7fe10808		# trppoint
	cmpw	%r4,%r5
	beq		handle_trppoint		
	loadi	%r5,0x7fe21008		# msgpoint
	cmpw	%r4,%r5
	beq		handle_msgpoint
	loadi	%r3,SIGTRAP + (TRAP_TRACE*256) + (FLTBOUNDS*65536)
	b		__exc
	
handle_trppoint:
handle_msgpoint:
	# advance to next instruction
	addi	%r6,%r6,4
	stw		%r6,REG_IAR(%r30)
	bitset	%r3,%r3,SIGCODE_KERNEL
	bl		_kdebug_opt
	# No debugger, keep on going
	ba		PPC_KEREXIT_COMMON

__exc_debug:
	la		%r0,REG_OFF(%r31)
	cmplw	%r0,%r30
	beq		__exc

    # If we're here it means we got a genuine exception in kernel mode.
	# Yikes!  Indicate this in the sigcode (in %r3) and then call
	# kdebug_callout to handle it.  If kdebug_callout were to return 0
	# we'll just skip out and continue execution.
	
	bitset	%r3,%r3,SIGCODE_KERNEL
	bl		_kdebug_opt
	b		__exc
	

# %r3 contains the initial error code
__exc_emulation:
	mr		%r22,%r3

.ifdef VARIANT_smp
	#
	# FIXME: don't allow emulation for kernel/ISR for now
	#
	lwz		%r7,REG_MSR(%r30)
	IS_CPUINKERNEL	%r7,%r4
	bne-	__hardcrash

	#
	# If another cpu has the kernel, retry fault until we can acquire it
	#
1:
 	lwarx	%r4,%r0,%r23		
 	andi.	%r7,%r4,INKERNEL_NOW+INKERNEL_LOCK
 	bnea-	PPC_KEREXIT_COMMON
 	ori		%r4,%r4,INKERNEL_NOW+INKERNEL_EXIT
 	stwcx.	%r4,%r0,%r23
 	bne-	1b
	SETCPU %r4,%r7
.else
	lwz		%r4,0(%r23)
 	ori		%r4,%r4,INKERNEL_EXIT
	stw		%r4,0(%r23)
.endif
 	
	mfmsr	%r14
	or		%r4,%r14,%r24			# interrupts on
	mtmsr	%r4
	la		%r15,REG_OFF(%r31)

.ifdef VARIANT_400
	# Need emulation because they left off the mftb/mttb instructions
	.set NEED_EMULATION,1
.endif
.ifdef VARIANT_booke
	# Need emulation because Moto's E500 core left off lswi/lswx/stswi/stwx
	.set NEED_EMULATION,1
.endif

.ifdef NEED_EMULATION
	# cpu specific instr emulations
	mr		%r3,%r30
	bl		instr_emulation	
	mr.		%r3,%r3
	beq-	3f
.endif
	
	# check tls->fpu_data
	lwz		%r4,TLS(%r31)
	lwz		%r3,TLS_FPUEMU_DATA(%r4)
	addi	%r26,%r4,TLS_FPUEMU_DATA
	mr.		%r3,%r3
	bne-	1f
	
	# use instr_emu do fpu load/store
	mr		%r3,%r30
	bl		fpu_emulation	
	
	cmpwi	%r3,0
	bne-	1f
3:	
.ifdef VARIANT_smp
	#
	# We only perform this processing for user exceptions
	# For SMP we have to return via __ker_exit
	#
	b		__ker_exit
.else
	mtmsr	%r14
	cmpw	%r15,%r30
	bne-	4f				# kernel?
	# active didn't change, get out quickly if we can
	lwz		%r5,intrevent_pending@sdarel(%r13)
	cmpwi	%r5,0
	bne-	__ker_exit
	stw		%r5,inkernel@sdarel(%r13)	# we know r5 is zero
	b		__ker_exit2
4:	
	stw		%r29,inkernel@sdarel(%r13)	# restore original inkernel value
	ba		PPC_KEREXIT_COMMON
.endif

1:  # use outside fpu emulator
	lwz		%r5,PROCESS(%r31)
	lwz		%r4,PLS(%r5)
	lwz		%r4,PLS_MATHEMULATOR(%r4)
	mr.		%r27,%r4
	beq-	2f
	
	# no fpu emulation if from kernel mode
	cmpw	%r15,%r30
	bne-	2f

	# Unlock the kernel so that we properly handle the exception
	# if the stack needs to be faulted in.
1:
 	lwarx	%r4,%r0,%r23		
	bitclr	%r4,%r4,INKERNEL_LOCK
	KSTWCX405	%r4,%r0,%r23
 	bne-	1b
	
	# save the thread context	
	mr		%r3,%r30
	mr		%r4,%r31
	loadi	%r5,SIZEOF_REG
	bl		fpu_emulation_prep
	# return value in R3 is where the copy ended up.
	
	# dive to outside fpu emulator (relock the kernel first)
1:
 	lwarx	%r4,%r0,%r23		
 	ori		%r4,%r4,INKERNEL_LOCK
	KSTWCX405	%r4,%r0,%r23
 	bne-	1b
	
	stw		%r3,REG_GPR+4*5(%r30)		
	subi	%r3,%r3,16				#leave space for lr & backchain
	stw		%r3,REG_GPR+4*1(%r30)		
	stw		%r22,REG_GPR+4*3(%r30)		
	stw		%r26,REG_GPR+4*4(%r30)		
	stw		%r27,REG_IAR(%r30)		
	mr		%r3,%r31
	bl		begin_fp_emulation
	cmpwi	%r3,0
	beq+	__ker_exit
	bitset	%r3,%r22,SIGCODE_SSTEP
	stw		%r3,REG_GPR+4*3(%r30)		
	b		__ker_exit
		
2:	# no outside fpu emulator, return error	
	mr		%r3,%r22
	lwz		%r4,REG_IAR(%r30)
	b 		__exc

__exc_alignment:
	lwz		%r6,TFLAGS(%r31)
	loadi	%r3,SIGBUS + (BUS_ADRALN*256) + (FLTACCESS*65536)
	bittst	%r0,%r6,_NTO_TF_ALIGN_FAULT
	bne+	__exc
	mr		%r28,%r4			# save reference address
.ifdef VARIANT_smp
	lwz		%r15,REG_MSR(%r30)
	IS_CPUINKERNEL %r15,%r7
	beq		1f
	b		__hardcrash
	# On SMP, if we're coming from user-mode, just pop back out until
	# we can become the kernel.
1:
 	lwarx	%r4,%r0,%r23		
	andi.	%r0,%r4,INKERNEL_NOW+INKERNEL_LOCK
	bnea-	PPC_KEREXIT_COMMON
 	ori		%r4,%r4,INKERNEL_NOW+INKERNEL_LOCK+INKERNEL_EXIT
	stwcx.	%r4,%r0,%r23
 	bne-	1b
	SETCPU %r8,%r9
.else
1:
 	lwz		%r4,0(%r23)
 	ori		%r4,%r4,INKERNEL_EXIT
	stw		%r4,0(%r23)
.endif
 	
	mfmsr	%r4
	or		%r4,%r4,%r24			# interrupts on
	mtmsr	%r4
	la		%r15,REG_OFF(%r31)
	
	mr		%r3,%r30
	mr		%r4,%r28
	bl		fix_alignment
	cmpwi	%r3,0
.ifdef VARIANT_smp
	beq+	1f
	mr		%r5,%r28
	mr		%r4,%r31
	bl		usr_fault
1:
	b		__ker_exit				# we don't emulate if from kernel mode
.else
	mr		%r4,%r28	# in case we still have an exception to deal with
	bne-	__exc
	cmpw	%r15,%r30
	beq+	__ker_exit				# going back to user code
	
	mfmsr	%r4
	andc	%r4,%r4,%r24			# interrupts off
	mtmsr	%r4
	
	stw		%r29,inkernel@sdarel(%r13)	# restore original inkernel value
	ba		PPC_KEREXIT_COMMON			# going back to system code
.endif
	
#
# Handle a instruction/data access exception. Check with the
# memory manager to see if it can do something before delivering a signal.
# Be careful with R5, it has stuff that needs to be passed to 
# the memmgr fault handler.
#
# IRQ's still off, ker_slock still locked
# we have multiple cases for SMP here, and have to be careful. We've
# just done an ENTERKERNEL with INKERNEL_NOW set; we need to see
# where we were, and what other CPU's are doing:
#
#		This CPU		Other CPU			Action
#		user			user				become kernel,handle fault
#		user			kernel(+intr)		back out, let it try to re-enter
#		kernel			-					handle fault, check locked bit
#		kernel+irq		-					handle fault
#		irq				-					handle fault
#
# (SMP) inkernel only gets set by the exception handlers if no-one was
# already inkernel. This makes returning easier.
#

__exc_access:
	mr		%r28,%r4	# save faulting address

.ifdef VARIANT_smp
	# Load old MSR bits; stash them in %r17, we need them later.
	lwz		%r17,REG_MSR(%r30)

	# Don't touch inkernel; easier to set it if/when we want to 
	# become the kernel, instead of setting it now and undoing 
	# things later. 

	IS_CPUINKERNEL %r17,%r7
	bne		3f

	# Dummy fault code in case we do an async processing of fault
	loadi	%r3,SIGSEGV
	# User fault. Check if another CPU is in kernel...
 	# r4 has the fault addr, r5 has the fault flags
9:
 	lwarx	%r8,%r0,%r23
	andi.	%r10,%r8,INKERNEL_NOW+INKERNEL_LOCK
	bne-	usr_exc_smp
 	ori		%r8,%r8,INKERNEL_NOW+INKERNEL_LOCK+INKERNEL_EXIT
 	stwcx.	%r8,%r0,%r23
 	bne-	9b
	SETCPU	%r9,%r10
 	# We're now the kernel
3: 	
	# We mark CPU as being in interrupt, so that we unwind properly in case of preemption
	GETCPU	%r25,2
	LW_SMP	%r18,cpupageptr,%r25
	lwz		%r19,CPUPAGE_STATE(%r18)
	li		%r16,1
	stw		%r16,CPUPAGE_STATE(%r18)	# cpupageptr[RUNCPU]->state = 1
.else	
	#	
	# We're now in the kernel, front line
	#
1:
 	lwarx	%r4,%r0,%r23		
 	ori		%r4,%r4,INKERNEL_NOW+INKERNEL_LOCK
 	KSTWCX405	%r4,%r0,%r23
 	bne-	1b
.endif
	
	mfmsr	%r16
	or		%r7,%r16,%r24		# Interrupts on
	mtmsr	%r7

	la		%r15,REG_OFF(%r31)
	
	# 
	# see what the memory manager says (R5 has parm from PPC_MMU_ADDR_ON).
	#
	cmpw	%r15,%r30
	beq		1f
	bitset	%r5,%r5,VM_FAULT_INKERNEL	# we were in the kernel for the fault
	bittst	%r3,%r29,INKERNEL_EXIT
	beq		1f
	bitset	%r5,%r5,VM_FAULT_KEREXIT	# we were in kernel exit sequence
1:
	mr		%r27,%r5	# save flags
	mr		%r4,%r28
	GETCPU	%r7,2
	LW_SMP	%r3,aspaces_prp,%r7
	bl		vmm_fault_shim
	
	mtmsr	%r16						# IRQ's off
.ifdef VARIANT_smp
	stw		%r19,CPUPAGE_STATE(%r18)	# restore cpupageptr[RUNCPU]->state
.endif
	mr		%r4,%r28
	cmpw	%r15,%r30
	beq+	usr_access_exc
	
.ifndef VARIANT_smp
	stw		%r29,0(%r23)	# restore original inkernel value
.endif
	
	# 
	# access exception while in system code
	#
	
.ifdef VARIANT_smp
	andi.	%r0,%r19,0x1				# check old cpupageptr[RUNCPU]->state
	bne		2f
	bittst	%r0,%r29,INKERNEL_LOCK
	beq		1f
2:
.else
	andi.	%r0,%r29,INKERNEL_INTRMASK+INKERNEL_LOCK
	beq		1f
.endif
	# in an interrupt handler, or locked
   
#  	cmpwi	%r3,0
#	beqa	PPC_KEREXIT_COMMON
#	#	
#	# If the exception couldn't be fixed up, the kernel's going down.
#	#
#	b 		__hardcrash

	# This section of code should be re-written when the vmm_fault_shim()
	# gear is removed and kernel.s can easily tell if a fault has caused
	# a PageWait() or been completely fixed up by memmgr.fault().
	cmpwi	%r3,0
	# If the exception couldn't be fixed up, the kernel's going down
	bne		__hardcrash
.if 0
	lbz		%r0,STATE(%r31)
	# If the thread is still STATE_RUNNING, we haven't pagewaited it
	cmpwi	%r0,1
	beqa	PPC_KEREXIT_COMMON
	#
	b		__hardcrash
.else
	# The above section of code doesn't work since we might be in an
	# interrupt handler referencing a special region (e.g. CPUPAGE) that
	# is fixed up, but the actives[RUNCPU]->state might be in a state
    # of flux (we're about to block() it). We'll have to live with 
    # not detecting pagewaiting in a locked kernel/interrupt handler
	# until vmm_fault_shim gets removed.
	ba		PPC_KEREXIT_COMMON
.endif	
	

	#
	# Else we were the kernel
	#
	
1:
	bittst	%r0,%r29,INKERNEL_SPECRET
	beq		1f
	cmpwi	%r3,0
	bne		fixup_specret
	#
	# If INKERNEL_SPECRET is set then it has to be cleared. 
	#
 	li		%r3,INKERNEL_SPECRET
9:
 	lwarx	%r4,%r0,%r23
 	andc	%r4,%r4,%r3
 	KSTWCX405	%r4,%r0,%r23
 	bne-	9b
	
   	lwz		%r3,xfer_handlers@sdarel(%r13)
	cmpwi	%r3,0
	beq		__ker_exit
	li		%r0,0
	stw		%r0,xfer_handlers@sdarel(%r13)
	lwz		%r3,4(%r3)
	cmpwi	%r3,0
	beq		__ker_exit
	mtctr	%r3
	mr		%r3,%r31
	mr		%r4,%r30
	bctrl
	b		__ker_exit
1:
	#	
	# page fault, no specret. If an error was 
	# returned by memmgr.fault, need to return
	# EFAULT for kernel call.
	#
  	cmpwi	%r3,0
	beq 2f
	
	# first need to fixup xfer if it is exist
	lwz		%r15,xfer_handlers@sdarel(%r13)
	cmpwi	%r15,0
	bne		fixup_xfer
	
	#then kcall
	b		fixup_kcall 
	
	#	
	# page fault, no specret, no error. 
	#
2:	
	GETCPU	%r9,2
	LW_SMP	%r3,actives,%r9
	cmpw	%r31,%r3
	bne		1f		
	# active didn't change, maybe we can get out quickly
	lwz		%r5,intrevent_pending@sdarel(%r13)
	cmpwi	%r5,0
	beq+	__ker_exit2		# restore TLS
1:
	#
	# Memory manager put thread into pagewait state, have to restart
	# kernel call (if we weren't in the exit processing stage)
	#
	andi.	%r0,%r29,INKERNEL_EXIT
	bne		1f
	lwz		%r3,REG_OFF+REG_IAR(%r31)
	subi	%r3,%r3,KER_ENTRY_SIZE
	stw		%r3,REG_OFF+REG_IAR(%r31)
1:
	lwz		%r3,xfer_handlers@sdarel(%r13)	# were we doing a message pass?
	cmpwi	%r3,0
	beq-	__ker_exit
	li		%r0,0
	stw		%r0,xfer_handlers@sdarel(%r13)
	lwz		%r0,4(%r3)			# get address of restart code
	cmpwi	%r0,0
	beq-	__ker_exit			# if no handler, abort entire call
	mtctr	%r0
	mr		%r3,%r31
	mr		%r4,%r30
	bctrl						# invoke handler
	b		__ker_exit
	 
usr_access_exc:
	#	
	# access exception in user process 
	#
	# the code above guarantees that we're now the kernel
.ifdef VARIANT_smp
	cmpwi	%r3,0
	beq+	__ker_exit		# exception cleared
	mr		%r5,%r4
	mr		%r4,%r31
	# @@@ enable interrupts
	bl		usr_fault		# deliver fault
	b		__ker_exit
.else
	cmpwi	%r3,0
	beq+	__ker_exit		# exception cleared
.endif
	# fall through to exc

# Common exception handler. At this point we have done an ENTERKERNEL and
# R3 contains the fault code. We classify the fault as user or system
# based upon whether the register save area equals &actives[RUNCPU]->reg. 
# R4 contains the address responsible, if there is one.

__exc:
	la		%r15,REG_OFF(%r31)
	cmpw	%r15,%r30
	bne-	sys_exc

usr_exc:

.ifdef VARIANT_smp
9:
 	lwarx	%r7,%r0,%r23
	andi.	%r8,%r7,INKERNEL_NOW+INKERNEL_LOCK
	bne-	usr_exc_smp
 	ori		%r7,%r7,INKERNEL_NOW+INKERNEL_LOCK
 	stwcx.	%r7,%r0,%r23
	bne-	9b
	SETCPU	%r9,%r10
	mr		%r5,%r4
.else
	mr		%r5,%r4
1:
 	lwarx	%r4,%r0,%r23		
 	ori		%r4,%r4,INKERNEL_NOW+INKERNEL_LOCK
 	KSTWCX405	%r4,%r0,%r23
 	bne-	1b
.endif

	# @@@ Turn interrupts on
	mr		%r4,%r31
	bl		usr_fault
	b		__ker_exit

#
# An exception occurred while executing system code or an interrupt handler.
# We will be using the kernel stack.
#
sys_exc:

.ifdef VARIANT_smp
	lwz		%r15,REG_MSR(%r30)
	# Check cpupageptr[RUNCPU]->state to see if we were in an interrupt
	GETCPU	%r25,2
	LW_SMP	%r26,cpupageptr,%r25
	lwz		%r0,CPUPAGE_STATE(%r26)
	andi.	%r0,%r0,0x1
	bne-	fixup_intr			# Note: this is also set when calling virt. fault
	# We were the kernel, then;
	
.else
	andi.	%r0,%r29,INKERNEL_INTRMASK
	bne-	fixup_intr
.endif

	andi.	%r0,%r29,INKERNEL_NOW
	beq-	__hardcrash
	andi.	%r0,%r29,INKERNEL_LOCK
	bne-	__hardcrash
	andi.	%r0,%r29,INKERNEL_SPECRET
	bne		fixup_specret
	lwz		%r15,xfer_handlers@sdarel(%r13)
	cmpwi	%r15,0
	bne		fixup_xfer
	bitset	%r3,%r3,SIGCODE_KERNEL
	bl		_kdebug_opt

fixup_kcall:
	li		%r0,0
	stw		%r0,TIMEOUT_FLAGS(%r31)		# remove any timeout flags
	li		%r0,ERRNO_EFAULT
	
	# return to thread register save space	
	la		%r30,REG_OFF(%r31)

	stw		%r0,REG_GPR+(0*PPCINT)(%r30)
	lwz		%r0,TFLAGS(%r31)
	bitset	%r4,%r0,_NTO_TF_KERERR_SET
	stw		%r4,TFLAGS(%r31)
	bittst	%r0,%r0,_NTO_TF_KERERR_SET
	bne-	__ker_exit
	lwz		%r4,REG_IAR(%r30)
	addi	%r4,%r4,KERERR_SKIPAHEAD
	stw		%r4,REG_IAR(%r30)
	b		__ker_exit

#
# Knock down the special return flag which is active and return from kcall
#
# r3 may contain error code from other places(eg. mem.fault), save it 
fixup_specret:
	mr 		%r14,%r3
 	li		%r0,INKERNEL_SPECRET
1:
 	lwarx	%r4,%r0,%r23		
 	ori		%r4,%r4,INKERNEL_LOCK
 	andc	%r4,%r4,%r0
 	KSTWCX405	%r4,%r0,%r23
 	bne-	1b
	lwz		%r6,inspecret@sdarel(%r13)
	lwz		%r4,TFLAGS(%r31)
	andc	%r6,%r4,%r6
	stw		%r6,TFLAGS(%r31)
	mr 		%r3,%r14	

	# first need to fixup xfer if it is exist
	lwz		%r15,xfer_handlers@sdarel(%r13)
	cmpwi	%r15,0
	bne		fixup_xfer

	# then kercall	
	b		fixup_kcall

#
# The fixup code is in nano_xfer 
# %r15 -- (xfer_handlers)
#
fixup_xfer:
	sub		%r0,%r0,%r0
	mr		%r5,%r3
	stw		%r0,xfer_handlers@sdarel(%r13)
	mr		%r3,%r31
	lwz		%r14,0(%r15)
	mr		%r4,%r30
	mtctr	%r14
	bctrl							
	b		fixup_kcall

fixup_intr:
	bitset	%r3,%r3,SIGCODE_INTR
	bl		_kdebug_opt
	cmpwi	%r3,0
	beqa	PPC_KEREXIT_COMMON

	# Just crash if get here. Not worth to recover from intr 
	# as PPC and MIPS have so many regs to save.	
	b	__hardcrash
	
__exc_unexpected:
	lwz		%r3,REG_LR(%r30)
	bl		exc_report_unexpected
	loadi	%r3, 0xff + (0xff*256) + (0xff*65536)	
	#
	# fall into __hardcrash
	#

#
# For diagnostic purposes %r3 = signal | (si_code*256) | (fault_num*65536)
#
__hardcrash:
/*
# infinite loop for debug use
	mr		%r2,%r3
	loadi	%r3, 0x2200
	mfspr	%r4,SRR0
	stwu	%r4,4(%r3)
	mfspr	%r4,SRR1
	stwu	%r4,4(%r3)
	mfspr	%r4,PPC_SPR_DAR 	
	stwu	%r4,4(%r3)
	mfspr	%r4,PPC_SPR_DSISR
	stwu	%r4,4(%r3)
	stwu	%r2,4(%r3)
1:
b 1b
*/
.ifndef VARIANT_smp
	stw		%r29,0(%r23)		# restore original inkernel value
.endif
	bitset	%r3,%r3,SIGCODE_FATAL
	mr		%r4,%r30
	bl		kdebug_callout
	cmpwi	%r3,0
	beqa	PPC_KEREXIT_COMMON
	mr		%r4,%r30
	b		shutdown
	

_kdebug_opt:
.ifndef VARIANT_smp
	stw		%r29,0(%r23)		# restore original inkernel value
.endif
	mflr	%r28
	mr		%r4,%r30
	bl		kdebug_callout
	cmpwi	%r3,0
	beqa	PPC_KEREXIT_COMMON
	mtlr	%r28
	rlwinm	%r3,%r3,0,8,31 
	blr
	
__exc_fpu:
	GETCPU	%r9,2
	LW_SMP	%r5,actives_fpu,%r9
	mr.		%r5,%r5
	beq-	1f

	lwz		%r5,FPUDATA(%r5)
	rlwinm	%r5,%r5,0,0,27
	# turn on fp bit in msr
	mfmsr	%r6
	bitset 	%r0,%r6,PPC_MSR_FP
	mtmsr	%r0
	isync
	
	# get fpscr	
	lwz		%r4,REG_FPSCR+4(%r5)
	stfd	%f0,REG_FPR+0*8(%r5)	# fpscr in [32 - 63] of f0
	mffs	%f0
	stfd	%f0,REG_FPSCR(%r5)
	lwz		%r3,REG_FPSCR+4(%r5)
	stw		%r4,REG_FPSCR+4(%r5)
	lfd		%f0,REG_FPSCR(%r5)
	mtfsf	0xff,%f0
	lfd		%f0,REG_FPR+0*8(%r5)
	
	mtmsr	%r6
	isync

	bittst	%r0,%r3,PPC_FPSCR_ZX				
	beq		2f
	loadi	%r3, SIGFPE + (FPE_FLTDIV*256) + (FLTFPE*65536)	
	b __exc
2:		
	bittst	%r0,%r3,PPC_FPSCR_OX			
	beq		2f
	loadi	%r3, SIGFPE + (FPE_FLTOVF*256) + (FLTFPE*65536)	
	b __exc
2:		
	bittst	%r0,%r3,PPC_FPSCR_UX			
	beq		2f
	loadi	%r3, SIGFPE + (FPE_FLTUND*256) + (FLTFPE*65536)	
	b __exc
2:		
	bittst	%r0,%r3,PPC_FPSCR_VX			
	beq		2f
	loadi	%r3, SIGFPE + (FPE_FLTINV*256) + (FLTFPE*65536)	
	b __exc
2:		
	bittst	%r0,%r3,PPC_FPSCR_XX			
	beq		2f
	loadi	%r3, SIGFPE + (FPE_FLTRES*256) + (FLTFPE*65536)	
	b __exc
2:
	# no error
	#
	# SMP: we have to assume that the FPU fault came from a user
	# process, not from the kernel. Else we should crash.
	#

.ifdef VARIANT_smp
2:
 	lwarx	%r4,%r0,%r23		
	andi.	%r6,%r4,INKERNEL_NOW+INKERNEL_EXIT
	# Someone else is inkernel; just go to __keriret; doesn't hurt, we
	# haven't enabled the interrupts anywhere before getting here.
	bnea-	PPC_KEREXIT_COMMON
 	ori		%r4,%r4,INKERNEL_NOW+INKERNEL_EXIT
	stwcx.	%r4,%r0,%r23
 	bne-	2b
	SETCPU	%r6,%r7		# We have the kernel
.endif

	b __ker_exit		

1:	
	loadi	%r3, SIGFPE + (FPE_NOFPU*256) + (FLTNOFPU*65536)	
	b __exc
	
   
#
# This code is used as the MMU ON/OFF routines in a physical system.
# In this case, we just change the system to be in a recoverable exception 
# state.
#
EXC_COPY_CODE_START phys_mmu_on
	mfmsr	%r3							
	ori		%r3,%r3,PPC_MSR_RI
	mtmsr	%r3
EXC_COPY_CODE_END
	
EXC_COPY_CODE_START phys_mmu_off
EXC_COPY_CODE_END
	
	
#
# Routine to only save the FPU regs;
# %r3 contains the actives fpu thread
#
# SMP fpu note: this is always called when the current cpu is in the kernel,
# and the KERNCPU also has the FPU registers
#
	
routine_start cpu_force_fpu_save, 1

	# enable FP
	mfmsr	%r0
	bitset 	%r0,%r0,PPC_MSR_FP
	mtmsr	%r0
	isync
	mr		%r7,%r3
	lwz		%r3,FPUDATA(%r3)
	rlwinm	%r3,%r3,0,0,27
	mflr	%r5
	bl		cpu_hwfpu_save
    bitclr  %r0,%r0,PPC_MSR_FP
	mtmsr	%r0
	isync

.ifdef VARIANT_smp
	stw		%r3,FPUDATA(%r7)
	la		%r3,actives_fpu@sdarel(%r13)
	GETCPU	%r4,2
	SMP_ADDR %r3,%r4
	li		%r8,0
	stw		%r8,0(%r3)
.endif
	mtlr	%r5
	blr

routine_end cpu_force_fpu_save

routine_start cpu_hwfpu_save, 1
	FPU_SAVE %r3
	blr
routine_end cpu_hwfpu_save


# Various SMP support routines

.ifdef VARIANT_smp

# Exc. code is in %r3, addr is in %r4, flags are in %r5
usr_exc_smp:
	loadi		%r9,_NTO_ATF_SMP_EXCEPTION
	stw			%r3,ARGS_ASYNC_CODE(%r31)
	stw			%r5,ARGS_ASYNC_FAULT_TYPE(%r31)
	stw			%r4,ARGS_ASYNC_FAULT_ADDR(%r31)


#
# force a NOP kernel call; %r9 has the bits to set ATFLAGS to
# Assumes interrupts are off, and kernel spinlock is not locked.
#

	.global force_kernel

force_kernel:

	lwz		%r7,ATFLAGS(%r31)
	bittst	%r0,%r7,_NTO_ATF_FORCED_KERNEL
	bne-	7f
	
	lwz		%r8,REG_OFF+REG_GPR+0*PPCINT(%r31)
	lis		%r10,kercallptr@ha
	lwz		%r11,REG_OFF+REG_IAR(%r31)
	lwz		%r10,kercallptr@l(%r10)
	cmpw	%r10,%r11
	beq		7f
	bitset	%r9,%r9,_NTO_ATF_FORCED_KERNEL
	stw		%r8,ARGS_ASYNC_TYPE(%r31)
	stw		%r11,ARGS_ASYNC_IP(%r31)
	loadi	%r8,__KER_NOP
	stw		%r8,REG_OFF+REG_GPR+0*PPCINT(%r31)
	stw		%r10,REG_OFF+REG_IAR(%r31)
7:
	la		%r12,ATFLAGS(%r31)
	lwarx	%r7,%r0,%r12
	or		%r7,%r7,%r9
	stwcx.	%r7,%r0,%r12
	bne-	7b

already_forced:
	ba		PPC_KEREXIT_COMMON
.endif
