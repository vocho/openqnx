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
 *	kernel.s
 *
 * Main entry points into the kernel
 */

#include <asmoff.def>

	.globl	ker_start
	.globl	__ker_exit
	.globl	intr_entry_start
	.globl	intr_entry_end
	.globl	intr_process_queue
	.globl	intr_done
	.globl	init_traps
	.globl	mmu_set_domain
.ifdef	VARIANT_smp
	.globl	intr_process_ipi
.endif

/*
 * Macro to save register context on exception entry:
 * - pc_adjust specifies adjustment required to exception mode return pc
 * - if entered in SVC mode, store on kernel stack
 * - if entered in USR/SYS mode, save in actives[RUNCPU]->reg (SVC mode sp)
 *
 * On exit:
 * - r4 set to saved register context
 * - Z flag set if entered in SVC mode
 */
.macro	PUSHREG pc_adjust
	/*
	 * Save r0-r3 to pass info across switch to SVC mode
	 */
	stmia	sp, {r0-r3}
	sub		r1, lr, #&pc_adjust
	mrs		r2, spsr
	mov		r3, sp
.ifdef	VARIANT_v6
	.word	0xf1020013			// cps	#0x13
.else
	mrs		r0, cpsr
	bic		r0, r0, #ARM_CPSR_MODE_MASK
	orr		r0, r0, #ARM_CPSR_MODE_SVC
	msr		cpsr, r0
.endif

	/*
	 * If spsr was SVC mode, sp is kernel stack.
	 * Otherwise, sp is top of actives[RUNCPU]->reg.
	 */
	stmdb	sp, {r1,r2}			// save spsr and return pc
	sub		r1, sp, #8
	and		r2, r2, #ARM_CPSR_MODE_MASK
	teq		r2, #ARM_CPSR_MODE_SVC
	stmeqdb	r1, {sp,lr}			// save svc mode banked registers
	stmnedb	r1, {sp,lr}^		// save usr mode banked registers
	sub		sp, sp, #17*4		// base of saved register context
	ldmia	r3, {r0-r3}
	stmia	sp, {r0-r12}
	mov		r4, sp				// remember saved context
.endm

/*
 * Macro to save Xscale CP0 register context.
 * This is not required for ARMv6 processors.
 * - context points to base of saved register context
 *   Assumed to be &actives[RUNCPU]->reg.
 *   CP0 registers are saved in the storage below this (ARM_ALT_REGISTERS).
 * - cpu_flags indicates how we get the __cpu_flags value.
 *
 * Assumes:
 * - Z flag is set if we entered from SVC mode (don't need to save state)
 * - r0, r2, r3 can be trashed.
 */
.macro	XSCALE_CP0_SAVE context cpu_flags
.ifndef	VARIANT_v6
	/*
	 * Check if we need to save Xscale CP0 state
	 * Assumed Z flag is set if entered from SVC mode
	 *
	 * Uses r0, r2, r3
	 */
	beq		0f
	ldr		r0, &cpu_flags
	ldr		r0, [r0]
	tst		r0, #ARM_CPU_FLAG_XSCALE_CP0
	.word	0x1c532000						// mrane	r2,r3,acc0
	stmnedb	&context, {r2,r3}
0:
.endif
.endm

/*
 * Macro to restore Xscale CP0 register context
 * - context points to base of saved register context.
 *   Assumed to be &actives[RUNCPU]->reg.
 *   CP0 registers are loaded from the storage below this (ARM_ALT_REGISTERS).
 */
.macro	XSCALE_CP0_RESTORE context
.ifndef	VARIANT_v6
	/*
	 * Check if we need to restore Xscale CP0 state
	 */
	ldr		r1, =__cpu_flags
	ldr		r1, [r1]
	tst		r1, #ARM_CPU_FLAG_XSCALE_CP0
	ldmnedb	&context, {r2,r3}
	.word	0x1c432000							// marne	acc0, r2, r3
.endif
.endm

/*
 * Macro to save context for und/abt exceptions.
 *
 *	pc_adjust specifies the adjustment to the banked lr register.
 *
 * On exit:
 *	r4 = saved register context
 *	r5 = &inkernel
 *	ip = inkernel
 *	(SMP) r10 = 0 if entered from SVC mode, 1 if entered from USR/SYS mode
 */
.macro	SAVE_CONTEXT pc_adjust
	/*
	 * Save registers:
	 * sets r4 to base of saved register context
	 * sets Z flag if entered from SVC mode
	 */
	PUSHREG &pc_adjust

	/*
	 * Set kernel stack if necessary
	 */
	ldrne	sp, =ker_stack
	GETCPU	r3
	SMPLDR	sp, sp, r3, ne

.ifdef	VARIANT_smp
	/*
	 * Set r10 so we can tell what mode we entered from
	 */
	moveq	r10, #0			// entered from SVC mode
	movne	r10, #1			// entered from USER/SYSTEM mode
.endif

	/*
	 * Save Xscale CP0 registers if necessary
	 * - Z flag was set if entered in SVC mode
	 * - r4 set to base of saved register context
	 * - macro uses r0, r2, r3
	 */
	XSCALE_CP0_SAVE r4, "=__cpu_flags"

	ldr		r5, =inkernel
	ldr		ip, [r5]
.endm


/*
 * Various macros used to abstract SMP operations:
 * - access to arrays indexed by RUNCPU
 * - set inkernel bits
 */
.ifdef	VARIANT_smp
	/*
	 * inkernel for SMP is implemented as follows:
	 * byte 0 - contains interrupt nesting count
	 * byte 1 - contains inkernel flags (INKERNEL_BITS_SHIFT is 8)
	 * byte 2 - currently unused
	 * byte 3 - contains cpu number of cpu currently in the kernel
	 */
	.data

	.globl	cpunum
	.type	cpunum,object
	.size	cpunum,1

	.globl	inkernel
	.type	inkernel,object
	.size	inkernel,4

	/*
	 * FIXME_SMP: assumes little-endian?
	 */
inkernel:
		.byte 0
		.byte 0
		.byte 0
cpunum:	.byte 0	

.macro	GETCPU reg
	mrc		p15, 0, &reg, c0, c0, 5			// get cpuid
	and		&reg, &reg, #0xf
.endm

.macro	SMPADD	dst, loc, cpu
	add		&dst, &loc, &cpu, lsl #2		// dst = &loc[cpu]
.endm

.macro	SMPLDR	dst, src, cpu, cc=
	ldr&cc	&dst, [&src, &cpu, lsl #2]		// dst = src[cpu]
.endm

.macro	SMPSTR	src, dst, cpu
	str		&src, [&dst, &cpu, lsl #2]		// dst[cpu] = src
.endm

.macro	ACQUIRE_INTR_SLOCK	label="=intr_slock"
	ldr		ip, &label
	mov		r1, #1
0:	ldr		r0, [ip]						// first try regular ldr to
	teq		r0, #0							// avoid locking up the bus
	bne		0b
	swp		r0, r1, [ip]
	teq		r0, #0
	bne		0b
	mcr		p15, 0, r0, c7, c10, 5			// data memory barrier
.endm

.macro	RELEASE_INTR_SLOCK
	ldr		ip, =intr_slock
	mcr		p15, 0, r0, c7, c10, 5			// data memory barrier
	mov		r0, #0
	str		r0, [ip]
.endm

.else	/* !SMP */

.macro	GETCPU	cpu
.endm

.macro	SMPADD	dst, loc, cpu
.ifnc &dst, &loc
	mov		&dst, &loc						// dst = &loc[0]
.endif
.endm

.macro	SMPLDR	dst, src, cpu, cc=
	ldr&cc	&dst, [&src]					// dst = src[0]
.endm

.macro	SMPSTR	src, dst, cpu
	str		&src, [&dst]					// dst[0] = src
.endm

.macro	ACQUIRE_INTR_SLOCK	label="=intr_slock"
.endm

.macro	RELEASE_INTR_SLOCK
.endm

.endif	// VARIANT_smp

	.align 2
	.text

__specret_mask:		.word	_NTO_TF_SPECRET_MASK
.ifndef	VARIANT_v6
__enable_domains:	.word	0x55555555
.endif

/*
 * ---------------------------------------------------------------------
 * init_traps initialises the trap vectors and exception mode stacks
 * ---------------------------------------------------------------------
 */
init_traps:

	/*
	 * Set up the trap handlers executed by the exception entry points.
	 * The startup program has set each entry point to "ldr pc, [pc, #0x18]"
	 *
	 * The virtual address of the trap vectors are at:
	 *
	 *	0xffff0000 if X bit (bit 13) in the MMU control register is set
	 *	0x00000000 if X bit (bit 13) in the MMU control register is clear
	 */
	mov		r1, #32
	mrc		p15, 0, r0, c1, c0, 0
	tst		r0, #(1 << 13)
	orrne	r1, r1, #0xff000000
	orrne	r1, r1, #0x00ff0000

	adr		r0, __trap_vectors
	mov		r2, #8
1:	ldr		ip, [r0], #4
	str		ip, [r1], #4
	subs	r2, r2, #1
	bne		1b

	/*
	 * Set up the exception mode "stacks".
	 * These are used as temporary storage for registers that are trashed
	 * when switching to SVC mode from the exception mode.
	 */
	mrs		r0, cpsr

	bic		r0, r0, #ARM_CPSR_MODE_MASK
	orr		r0, r0, #ARM_CPSR_MODE_UND
	msr		cpsr, r0
	ldr		sp, =__und_save

	bic		r0, r0, #ARM_CPSR_MODE_MASK
	orr		r0, r0, #ARM_CPSR_MODE_ABT
	msr		cpsr, r0
	ldr		sp, =__abt_save

	bic		r0, r0, #ARM_CPSR_MODE_MASK
	orr		r0, r0, #ARM_CPSR_MODE_IRQ
	msr		cpsr, r0
	ldr		sp, =__irq_save

	bic		r0, r0, #ARM_CPSR_MODE_MASK
	orr		r0, r0, #ARM_CPSR_MODE_FIQ
	msr		cpsr, r0
	ldr		sp, =__fiq_save

	/*
	 * Back to SVC mode and return
	 */
	bic		r0, r0, #ARM_CPSR_MODE_MASK
	orr		r0, r0, #ARM_CPSR_MODE_SVC
	msr		cpsr, r0
	mov		pc, lr

	/*
	 * Trap vector table copied by init_traps
	 */
__trap_vectors:
	.word	__rsv_entry
	.word	__und_entry
	.word	__ker_entry
	.word	__prf_entry
	.word	__abt_entry
	.word	__rsv_entry
	.word	0				// patched by cpu_interrupt_init
	.word	__fiq_entry

	/*
	 * Save area for scratch register used to transition into SVC mode
	 */
	.data

__und_save:	.word	0
			.word	0
			.word	0
			.word	0

__abt_save:	.word	0
			.word	0
			.word	0
			.word	0

__irq_save:	.word	0
			.word	0
			.word	0
			.word	0

__fiq_save:	.word	0
			.word	0
			.word	0
			.word	0

	.text

/*
 * ---------------------------------------------------------------------
 * set_mmu_domain is used to set the MMU domain register on kernel exit.
 *	r0 contains the "active domain" (0-15)
 *  domain 0 is used for kernel mappings, and is always active
 * ---------------------------------------------------------------------
 */
mmu_set_domain:
	ldr		ip, =mmu_domain
	mov		r0, r0, lsl #1
	mov		r1, #1
	orr		r1, r1, r1, lsl r0
	GETCPU	r3
	SMPSTR	r1, ip, r3				// set mmu_domain[RUNCPU]

	/*
	 * FIXME: Make sure we enable this domain.
	 *        I have seen a fault occur in specialret() where the domain
	 *        register was set to a different domain. I'm not sure how
	 *        this happened, but this fixes that problem and should be
	 *        harmless, as the domain register is reset in __ker_exit,
	 */
	mrc		p15, 0, ip, c3, c0, 0
	orr		ip, ip, r1
	mcr		p15, 0, ip, c3, c0, 0

	mov		pc, lr

/*
 * ---------------------------------------------------------------------
 * ker_start is used only once to start execution of the first thread.
 * ---------------------------------------------------------------------
 */
ker_start:
	ldr		ip, =ker_stack
	GETCPU	r3
	SMPLDR	sp, ip, r3				// sp = ker_stack[RUNCPU]
.ifdef	VARIANT_smp
	/*
	 * Set inkernel so interrupts on other cpus know we have the kernel
	 * We don't need to use ldrex/strex to protect inkernel because:
	 * - idle() ensures only one cpu at a time comes through here.
	 * - idle() does not enable the clock interrupt until all cpus have
	 *   started, so we should not have any interrupts yet.
	 */
	ldr		r1, =inkernel
	mov		r3, r3, lsl #24			// set cpunum
	orr		r3, r3, #INKERNEL_NOW|INKERNEL_LOCK
	str		r3, [r1]
.endif
	b		__ker_exit

/*
 * ---------------------------------------------------------------------
 * Entry point for system calls.
 * r0-r5 contain system call arguments, ip contains the syscall number.
 * ---------------------------------------------------------------------
 */
__ker_entry:
	/*
	 * First check for "special" calls:
	 *
	 *	0xff000000	- smp_cmpxchg emulation
	 *	0xfe000000	- ClockCycles emulation
	 */
	tst		ip, #0xff000000
	bne		__emu_decode

__ker_entry_save:
	/*
	 * Save context - we assume entry from user/system mode.
	 * sp is set to top of actives[RUNCPU]->reg.
	 *
	 * If we entered from SVC mode, sp will be the current stack pointer.
	 * In this case, we'll save the context and then crash, since the only
	 * code running in SVC mode is either the kernel or an ISR.
	 */
	sub		sp, sp, #17*4
	str		lr, [sp, #ARM_REG_PC*4]			// save return pc
	stmia	sp, {r0-r14}^					// save user mode registers
	mrs		r0, spsr
	str		r0, [sp, #ARM_REG_SPSR*4]
	mov		r1, sp							// remember args pointer

	/*
	 * Crash if we came from SVC mode
	 */
	and		r0, r0, #ARM_CPSR_MODE_MASK
	teq		r0, #ARM_CPSR_MODE_SVC
	moveq	r8, #0							// fake signo == 0
	beq		__hardcrash

	/*
	 * Save Xscale CP0 registers if necessary
	 */
	XSCALE_CP0_SAVE r1, "=__cpu_flags"

	/*
	 * Set kernel stack
	 */
	ldr		sp, =ker_stack
	ldr		r5, =inkernel					// r5 = &inkernel
	GETCPU	r3
	SMPLDR	sp, sp, r3						// sp = ker_stack[RUNCPU]

	/*
	 * Set inkernel
	 */
.ifdef	VARIANT_smp
	mov		r0, #INKERNEL_NOW
	bl		usr_acquire_kernel
.else
	ldr		r2, [r5]
	orr		r2, r2, #INKERNEL_NOW
	str		r2, [r5]
.endif

.ifndef	VARIANT_v6
	/*
	 * Enable all MMU domains to allow access to all user address spaces.
	 *
	 * WARNING: assumes proc is using ProcessID mapping to implement user
	 *			address spaces. See proc/mem_virtual.c: virtual_aspace().
	 */
	ldr		r2, __enable_domains
	ldr		r0, =actives
	mcr		p15, 0, r2, c3, c0, 0
.else
	ldr		r0, =actives
.endif

	/*
	 * Get active thread and enable interrupts
	 */
	GETCPU	r3
	SMPLDR	r4, r0, r3				// r4 = actives[RUNCPU]
	mrs		r3, cpsr
	ldr		r2, [r4, #TFLAGS]
	bic		r3, r3, #ARM_CPSR_I | ARM_CPSR_F
	msr		cpsr, r3

	/*
	 * Clear act->flags and set act->syscall
	 */
	bic		r2, r2, #_NTO_TF_KERERR_SET
	bic		r2, r2, #_NTO_TF_BUFF_MSG
	str		r2, [r4, #TFLAGS]
	str		ip, [r4, #SYSCALL]

	/*
	 * Invoke ker_call_table[ip](act, args) and set error code if required
	 */
	cmp		ip, #__KER_BAD
	bhs		__bad_func
.ifdef VARIANT_instr
	ldr		r2, =_trace_call_table
.else
	ldr		r2, =ker_call_table
.endif
	mov		r0, r4
	mov		lr, pc
	ldr		pc, [r2, ip, lsl #2]

	/*
	 * If error == ENOERROR, just fall through to __ker_exit
	 * If error > 0, call kererr to set error return
	 * If error == 0, set return value (r0) to 0
	 */
	cmp		r0, #0
	blt		__ker_exit
	bgt		__set_err
.ifdef	VARIANT_smp
0:	.word	0xe1953f9f				// ldrex	r3, [r5]
	orr		r3, r3, #INKERNEL_NOW|INKERNEL_LOCK|INKERNEL_EXIT
	.word	0xe1853f93				// strex	r3, r3, [r5]
	teq		r3, #0
	bne		0b						// strex failed, retry
.else
	mov		r3, #INKERNEL_NOW|INKERNEL_LOCK|INKERNEL_EXIT
	str		r3, [r5]
.endif
	add		r4, r4, #REG_OFF
	str		r0, [r4, #ARM_REG_R0*4]
	/*
	 * Fall through into __ker_exit
	 */

/*
 * ---------------------------------------------------------------------
 * Perform miscellaneous checks on exit from kernel
 * ---------------------------------------------------------------------
 */
__ker_exit:
	/*
	 * Set inkernel, then make sure interrupts are enabled
	 */
	ldr		r0, =inkernel
	mrs		r1, cpsr
.ifdef	VARIANT_smp
0:	.word	0xe190cf9f					// ldrex	ip, [r0]
	bic		r1, r1, #ARM_CPSR_I | ARM_CPSR_F
	orr		ip, ip, #INKERNEL_NOW | INKERNEL_EXIT | INKERNEL_LOCK
	.word	0xe180cf9c					// strex	ip, ip, [r0]
	teq		ip, #0
	bne		0b							// strex failed, retry
.else
	ldr		ip, [r0]
	bic		r1, r1, #ARM_CPSR_I | ARM_CPSR_F
	orr		ip, ip, #INKERNEL_NOW | INKERNEL_EXIT | INKERNEL_LOCK
	str		ip, [r0]
.endif
	ldr		ip, =intrevent_pending

	/*
	 * Drain pending interrupt events.
	 * This returns back to __ker_exit, in case current thread was preempted.
	 */
	ldr		ip, [ip]
	msr		cpsr, r1					// interrupts now enabled
	teq		ip, #0
	bne		__ker_intrevent

	/*
	 * Load various pointers into callee-saved registers
	 */
	ldr		r4, =actives
	ldr		r5, =actives_prp			// r5 = &actives_prp[0]
	ldr		r6, =aspaces_prp			// r6 = &aspaces_prp[0]
	ldr		r7, =actives_fpu			// r7 = &actives_fpu[0]
	GETCPU	r3
	SMPLDR	r4, r4, r3					// r4 = actives[RUNCPU]
	SMPADD	r5, r5, r3					// r5 = &actives_prp[RUNCPU]
	SMPADD	r6, r6, r3					// r6 = &aspaces_prp[RUNCPU]
	SMPADD	r7, r7, r3					// r7 = &actives_fpu[RUNCPU]

	/*
	 * Detach breakpoints if necessary
	 */
	ldr		r0, [r5]					// actives_prp[RUNCPU]
	ldr		r1, [r4, #PROCESS]
	teq		r0, r1
	blne	__ker_dbg_switch

	/*
	 * Switch address spaces if necessary
	 */
	ldr		r0, [r4, #ASPACE_PRP]
	ldr		ip, [r6]					// aspaces_prp[RUNCPU]
	cmp		r0, ip
	blne	__ker_asp_switch

	/*
	 * Switch process if necessary
	 */
	ldr		r0, [r4, #PROCESS]
	ldr		r1, [r5]					// actives_prp[RUNCPU]
	cmp		r0, r1
	blne	__ker_prp_switch

	/*
	 * Perform special return actions if necessary.
	 * This returns back to __ker_exit, in case current thread was preempted.
	 */
	ldr		ip, [r4, #TFLAGS]
	ldr		r0, __specret_mask
	tst		ip, r0					
	bne		__ker_specialret

	/*
	 * Disable FPU if actives[RUNCPU] != actives_fpu[RUNCPU]
	 */
	ldr		ip, =fpu_disable
	ldr		r1, [r7]					// actives_fpu[RUNCPU]
	ldr		ip, [ip]
	teq		r1, r4
	teqne	ip, #0
	movne	lr, pc
	movne	pc, ip						// fpu_disable()

.ifdef VARIANT_instr
	/*
	 * if ker_exit_enable_mask != 0, call _trace_ker_exit(act)
	 */
	ldr		ip, =ker_exit_enable_mask
	ldr		ip, [ip]
	teq		ip, #0
	movne	r0, r4
	blne	_trace_ker_exit
.endif

	/*
	 * Check if we were in critical region for mutex operation
	 */
	add		r0, r4, #REG_OFF
	ldr		r1, [r0, #ARM_REG_SP*4]
	mov		r0, r4
	tst		r1, #1
	blne	cpu_mutex_adjust

	/*
	 * Disable interrupts
	 */
	mrs		ip, cpsr
	orr		ip, ip, #ARM_CPSR_I | ARM_CPSR_F
	msr		cpsr, ip

	/*
	 * Drain remaining interrupt events.
	 * This returns back to __ker_exit, in case the thread needs preempting.
	 */
	ldr		ip, =intrevent_pending
	ldr		ip, [ip]
	teq		ip, #0
	bne		__ker_intrevent

	/*
	 * Process any pending async_flags.
	 * This returns back to __ker_exit, in case the current thread was preempted.
	 */
	add		ip, r4, #ATFLAGS
	mov		r0, #0
	swp		r0, r0, [ip]
	teq		r0, #0
	bne		__ker_atflags

	/*
	 * Set cpupageptr[RUNCPU]->tls
	 */
	ldr		r0, =cpupageptr
	GETCPU	r3
	ldr		ip, [r4, #TLS]
	SMPLDR	r0, r0, r3					// cpupageptr[RUNCPU]
	str		ip, [r0, #CPUPAGE_TLS]

	/*
	 * Clear inkernel
	 */
	ldr		r1, =inkernel
.ifdef	VARIANT_smp
0:	.word	0xe1912f9f					// ldrex	r2, [r1]
	bic		r2, r2, #0xff00				// clear inkernel bits
	.word	0xe1812f92					// strex	r2, r2, [r1]
	teq		r2, #0
	bne		0b
.else
	mov		r2, #0
	str		r2, [r1]
.endif

.ifdef	VARIANT_smp
	/*
	 * __ker_exit2 is branched to by intr_done to return to user if:
	 * - the kernel is locked by another cpu (case2)
	 * - queued_event_priority requires some rescheduling (case2)
	 * - we were spinning waiting to acquire the kernel (case6 -> case2)
	 *
	 * In this case, we haven't checked intrevent_pending so we send an
	 * IPI to the next cpu to try to get the events processed.
	 *
	 * FIXME_SMP: this code slows down the regular __ker_exit path, and
	 *            should probably only be done on the intr_done path.
	 *            However, it needs careful analysis to ensure we don't
	 *            open up any race conditions.
	 */
__ker_exit2:
	/*
	 * FIXME_SMP: check need_to_run/need_to_run_cpu so we can set
	 *            _NTO_ATF_FORCED_KERNEL/SMP_RESCHED and force thread to
	 *            perform a __KER_NOP call
	 */

	/*
	 * Check if we need to send an IPI to another cpu to get it to process
	 * pending events.
	 */
	ldr		r1, =intrevent_pending
	GETCPU	r0
	ldr		r1, [r1]
	teq		r1, #0
	beq		0f

	/*
	 * Send ipi to (RUNCPU + 1) % num_processors
	 */
	ldr		r1, =num_processors
	add		r0, r0, #1
	ldr		r1, [r1]
	teq		r1, r0
	moveq	r0, #0
	mov		r1, #IPI_CHECK_INTR
	bl		send_ipi
0:
.endif

.ifndef	VARIANT_v6
	/*
	 * Disable access to all but the active MMU domain.
	 *
	 * WARNING:	assumes proc is using ProcessID mapping to implement user
	 *			address spaces. See proc/mem_virtual.c: virtual_aspace().
	 */
	ldr		r1, =mmu_domain
	GETCPU	r3
	SMPLDR	r2, r1, r3					// mmu_domain[RUNCPU]
	mcr		p15, 0, r2, c3, c0, 0
.endif

.ifdef	VARIANT_v6
	/*
	 * Clear exclusive monitor
	 *
	 * FIXME: ARMARM says this only needs doing on context switches.
	 *        We haven't necessarily performed a context switch here
	 *        eg. we could be restoring interrupted user mode context.
	 */
.ifdef	VARIANT_smp
	.word	0xf57ff01f				// clrex
.else
	ldr		r1, =dummy_strex		// store to dummy_strex variable
	.word	0xe1812f93				// strex r2, r3, [r1]
.endif
.endif

	/*
	 * Set SVC mode sp above the end of act->reg
	 */
	add		r0, r4, #REG_OFF
	add		sp, r0, #17*4

	/*
	 * Restore Xscale CP0 registers if necessary
	 */
	XSCALE_CP0_RESTORE r0

	/*
	 * Restore context
	 */
	ldr		ip, [r0, #ARM_REG_SPSR*4]
	ldr		lr, [r0, #ARM_REG_PC*4]
	bic		ip, ip, #ARM_CPSR_I | ARM_CPSR_F	// ensure interrupts are enabled
	msr		spsr_cxsf, ip
	and		ip, ip, #ARM_CPSR_MODE_MASK	
	teq		ip, #ARM_CPSR_MODE_SVC
	beq		__ker_exit_svc
	ldmia	r0, {r0-lr}^
	mov		r0, r0
	movs	pc, lr

__ker_exit_svc:
	/*
	 * Should not be returning to SVC mode here
	 * Crash with a breakpoint trap
	 */
	.word	0xe7ffdefe

/*
 * ---------------------------------------------------------------------
 * Set error return code and return from kernel to active thread.
 * ---------------------------------------------------------------------
 */
__bad_func:
	mov		r0, #ENOSYS
	/*
	 * Fall through to __set_err
	 */
__set_err:
	/*
	 * r0 contains return code from syscall
	 * r4 is original active thread (not necessarily actives[KERNCPU])
	 */
	mov		r1, r0
	mov		r0, r4
	adr		lr, __ker_exit
	b		kererr

/*
 * ---------------------------------------------------------------------
 * Drain pending events.
 * ---------------------------------------------------------------------
 */
__ker_intrevent:
	mrs		ip, cpsr
	bic		ip, ip, #ARM_CPSR_I | ARM_CPSR_F
	msr		cpsr, ip
	adr		lr, __ker_exit
	b		intrevent_drain

/*
 * ---------------------------------------------------------------------
 * Perform special return actions, then go back to __ker_exit in case
 * current thread has been preempted etc.
 * On entry:
 *	r4	= actives[0]
 * ---------------------------------------------------------------------
 */
__ker_specialret:
	/*
	 * Reset stack pointer to avoid recursion
	 */
	ldr		ip, =ker_stack
	GETCPU	r3
	SMPLDR	sp, ip, r3				// sp = ker_stack[RUNCPU]

.ifdef	VARIANT_smp
	/*
	 * If _NTO_ATF_FORCED_KERNEL was set, clear it and restore registers
	 */
	add		ip, r4, #ATFLAGS
0:	.word	0xe19c0f9f				// ldrex	r0, [ip]
	tst		r0, #_NTO_ATF_FORCED_KERNEL
	beq		1f
	bic		r0, r0, #_NTO_ATF_FORCED_KERNEL
	.word	0xe18c1f90				// strex	r1, r0, [ip]
	teq		r1, #0
	bne		0b						// strex failed, retry
	ldr		r1, [r4, #ARGS_ASYNC_TYPE]
	ldr		r2, [r4, #ARGS_ASYNC_IP]
	add		ip, r4, #REG_OFF
	str		r1, [ip, #ARM_REG_IP*4]
	str		r2, [ip, #ARM_REG_PC*4]
1:
.endif

	mov		r0, r4
	adr		lr, __ker_exit
	b		specialret

/*
 * ---------------------------------------------------------------------
 * Switch out breakpoints from outgoing address space.
 * On entry:
 *	r0	= actives_prp[RUNCPU]
 *	r4	= actives[RUNCPU]
 *	r6	= &aspaces_prp[RUNCPU]
 * ---------------------------------------------------------------------
 */
__ker_dbg_switch:									
	/*
	 * Nothing to do if actives_prp[RUNCPU] == 0 || actives_prp[RUNCPU]->debug == 0
	 */
	teq		r0, #0
	moveq	pc, lr
	ldr		r8, [r0, #DEBUGGER]
	teq		r8, #0
	moveq	pc, lr

	/*
	 * Switch address space if actives_prp[RUNCPU] != aspaces_prp[RUNCPU]
	 */
	ldr		ip, [r6]			// aspaces_prp[RUNCPU]
	mov		r9, lr
	cmp		r0, ip
	beq		1f

	/*
	 * Call memmgr.aspace(actives_prp[RUNCPU], &aspaces_prp[RUNCPU])
	 */
	ldr		ip, =memmgr
	mov		r1, r6				// &aspaces_prp[RUNCPU]
	mov		lr, pc
	ldr		pc, [ip, #MEMMGR_ASPACE]
1:
	mov		r0, r8
	ldr		ip, =debug_detach_brkpts
	mov		lr, r9
	ldr		pc, [ip]

/*
 * ---------------------------------------------------------------------
 * Switch address space.
 * On entry:
 *	r0	= actives[RUNCPU]->aspace_prp
 *	r6	= &aspaces_prp[RUNCPU]
 * ---------------------------------------------------------------------
 */
__ker_asp_switch:
	/*
	 * Only switch if act->aspace_prp != 0
	 */
	teq		r0, #0
	moveq	pc, lr

	/*
	 * Call memmgr.aspace(actives[RUNCPU]->aspace_prp, &aspaces_prp[RUNCPU])
	 */
	ldr		ip, =memmgr
	mov		r1, r6					// &aspaces_prp[RUNCPU]
	ldr		pc, [ip, #MEMMGR_ASPACE]

/*
 * ---------------------------------------------------------------------
 * Switch process
 * On entry:
 *	r0	= actives[RUNCPU]->process
 *	r5	= &actives_prp[RUNCPU]
 * ---------------------------------------------------------------------
 */
__ker_prp_switch:
	/*
	 * Set actives_prp[RUNCPU]
	 */
	str		r0, [r5]

	/*
	 * Set cpupageptr[RUNCPU]->pls
	 */
	ldr		r1, =cpupageptr
	GETCPU	r3
	SMPLDR	r1, r1, r3			// cpupageptr[RUNCPU]
	ldr		ip, [r0, #PLS]
	str		ip, [r1, #CPUPAGE_PLS]

	/*
	 * Attach debug breakpoints if necessary
	 */
	ldr		r0, [r0, #DEBUGGER]
	teq		r0, #0
	moveq	pc, lr
	ldr		ip, =debug_attach_brkpts
	ldr		pc, [ip]

/*
 * ---------------------------------------------------------------------
 * Process async_flags.
 * On entry:
 *	r0	= async_flags value
 *	r4  = actives[RUNCPU]
 * ---------------------------------------------------------------------
 */
__ker_atflags:
.ifdef	VARIANT_smp
	tst		r0, #_NTO_ATF_FORCED_KERNEL
	beq		1f
	/*
	 * Restore original ip/pc value before forced kernel call
	 */
	ldr		r1, [r4, #ARGS_ASYNC_TYPE]
	ldr		r2, [r4, #ARGS_ASYNC_IP]
	add		ip, r4, #REG_OFF
	str		r1, [ip, #ARM_REG_IP*4]
	str		r2, [ip, #ARM_REG_PC*4]
1:
.endif
	mrs		ip, cpsr
	bic		ip, ip, #ARM_CPSR_I | ARM_CPSR_F
	msr		cpsr, ip

	/*
	 * Check if we need to allocate coprocessor save area
	 */
	mov		r5, r0
	tst		r5, #_NTO_ATF_FPUSAVE_ALLOC
	blne	fpusave_alloc

	/*
	 * Check if we need to reschedule
	 */
	tst		r5, #_NTO_ATF_TIMESLICE|_NTO_ATF_SMP_RESCHED
	/*
	 *
	 * FIXME_SMP: need to check need_to_run/need_to_run_cpu here before
	 *            going back into __ker_exit:
	 *				if (need_to_run == 0 || need_to_run_cpu != RUNCPU)
	 *					goto __ker_exit
	 */
	beq		__ker_exit

	/*
	 * Call through resched function pointer
	 */
	ldr		r0, =resched
	adr		lr, __ker_exit
	ldr		pc, [r0]

/*
 * ---------------------------------------------------------------------
 * Preempt the currently active thread.
 * On entry:
 *	r4	= actives[RUNCPU]
 *	r5	= &inkernel
 *
 * If we are called from intr_done, interrupts are disabled.
 * ---------------------------------------------------------------------
 */
__preempt:
	/*
	 * FIXME_SMP: x86 has SMP_MSGOPT support here
	 */

	/*
	 * If INKERNEL_EXIT not set, adjust return pc to restart the kernel call.
	 */
.ifdef	VARIANT_smp
	/*
	 * FIXME_SMP: x86 has SMP_MSGOPT support here
	 */
0:	.word	0xe195cf9f				// ldrex	ip, [r5]
	orr		r2, ip, #INKERNEL_NOW|INKERNEL_LOCK
	bic		r2, r2, #INKERNEL_SPECRET
	.word	0xe1852f92				// strex	r2, r2, [r5]
	teq		r2, #0
	bne		0b						// strex failed, retry

	tst		ip, #INKERNEL_EXIT
	ldreq	r0, [r4, #REG_OFF + (ARM_REG_PC*4)]
	subeq	r0, r0, #KER_ENTRY_SIZE
	streq	r0, [r4, #REG_OFF + (ARM_REG_PC*4)]
.else
	ldr		ip, [r5]
	tst		ip, #INKERNEL_EXIT
	ldreq	r0, [r4, #REG_OFF + (ARM_REG_PC*4)]
	subeq	r0, r0, #KER_ENTRY_SIZE
	streq	r0, [r4, #REG_OFF + (ARM_REG_PC*4)]

	orr		ip, ip, #INKERNEL_NOW|INKERNEL_LOCK
	bic		ip, ip, #INKERNEL_SPECRET
	str		ip, [r5]
.endif

	/*
	 * If xfer_handlers is non-NULL, clear it and check the restart handler
	 */
	ldr		r2, =xfer_handlers
	mov		ip, #0
#ifdef	SMP_MSGOPT
	GETCPU	r3
	SMPADD	r2, r2, r3				// &xfer_handlers[RUNCPU]
#endif
	swp		ip, ip, [r2]
	teq		ip, #0
	beq		__ker_exit

	/*
	 * Run the restart handler if it is non-null
	 */
	ldr		ip, [ip, #4]
	teq		ip, #0
	beq		__ker_exit
	mov		r0, r4
	mov		r1, sp
	adr		lr, __ker_exit
	mov		pc, ip

/*
 * ---------------------------------------------------------------------
 * Fix up exception that occurred in specialret() processing.
 * On entry:
 *	r4	= actives[0]
 * ---------------------------------------------------------------------
 */
__fixup_specret:
	/*
	 * Clear the specialret() action being processed from act->flags
	 */
	ldr		ip, [r4, #TFLAGS]
	ldr		r1, =inspecret
	ldr		r1, [r1]
	bic		ip, ip, r1
	str		ip, [r4, #TFLAGS]
	/*
	 * Fall through to __fixup_kcall
	 */

/*
 * ---------------------------------------------------------------------
 * Force a kernel call to return EFAULT.
 * On entry:
 *	r4	= actives[0]
 *	r6	= fault code (0 = read, 1 = write)
 *	r8	= signal code
 * ---------------------------------------------------------------------
 */
__fixup_kcall:
	/*
	 * If xfer_handlers is non-0, call its fault handler
	 */
	ldr		r0, =xfer_handlers
	mov		r1, #0
#ifdef	SMP_MSGOPT
	GETCPU	r3
	SMPADD	r0, r0, r3				// &xfer_handlers[RUNCPU]
#endif
	swp		r1, r1, [r0]
	teq		r1, #0
	blne	__fixup_xfer

	/*
	 * Clear any timeout flags, and set errno return value
	 */
	mov		ip, #0
	str		ip, [r4, #TIMEOUT_FLAGS]
	mov		ip, #ERRNO_EFAULT
	str		ip, [r4, #REG_OFF + (ARM_REG_R0*4)]

	/*
	 * If this is the first kernel error, adjust the return pc to resume
	 * at the libc error handling code in the system call stub.
	 */
	ldr		ip, [r4, #TFLAGS]
	tst		ip, #_NTO_TF_KERERR_SET
	bne		__ker_exit
	orr		ip, ip, #_NTO_TF_KERERR_SET
	str		ip, [r4, #TFLAGS]
	ldr		ip, [r4, #REG_OFF + ARM_REG_PC*4]
	add		ip, ip, #KERERR_SKIPAHEAD
	str		ip, [r4, #REG_OFF + ARM_REG_PC*4]
	b		__ker_exit

__fixup_xfer:
	ldr		ip, [r1]			// xfer_handlers->fault()
	mov		r0, r4
	mov		r1, sp
	mov		r2, r8
	mov		pc, ip

/*
 * ---------------------------------------------------------------------
 * Crash
 * On entry:
 *	r8	= signo code
 *	sp	= exception register context
 * ---------------------------------------------------------------------
 */
__hardcrash:
	orr		r0, r8, #SIGCODE_FATAL
	mov		r1, sp
	bl		kdebug_callout
	movs	r8, r0
	beq		__svc_exit
	mov		r1, sp
	b		shutdown


.ifdef	VARIANT_smp
/*
 * ---------------------------------------------------------------------
 * Code to acquire the kernel for SMP.
 *
 * WARNING: we maintain the r0 value to indicate which bits to set.
 *          case6 of intr_done looks at this register to figure out if
 *          it needs to back up the pc for a system call entry.
 *          If the register changes, the intr_done code must be modified
 *          to look at the correct register containing the bits to set. 
 * ---------------------------------------------------------------------
 */
beg_acquire_kernel_attempt:

/*
 * Acquire kernel from user mode entry.
 * r0 = flags to set
 * r5 = &inkernel
 */
usr_acquire_kernel:
0:	mrs		r2, cpsr
	bic		r2, r2, #ARM_CPSR_I|ARM_CPSR_F
	msr		cpsr, r2

	/*
	 * FIXME_SMP: check need_to_run/need_to_run_cpu here...
	 */

	/*
	 * Wait until INKERNEL_NOW is clear
	 */
1:	.word	0xe1952f9f						// ldrex	r2, [r5]
	tst		r2, #INKERNEL_NOW
	bne		1b

	/*
	 * Disable interrupts and attempt to set inkernel
	 */
	mrs		r3, cpsr
	orr		r3, r3, #ARM_CPSR_I
	msr		cpsr, r3

	GETCPU	r3
	bic		r2, r2, #0xff000000
	orr		r2, r2, r3, lsl #24				// set cpunum
	orr		r2, r2, r0						// set required bits
	.word	0xe1853f92						// strex	r3, r2, [r5]
	teq		r3, #0
	bne		0b
	mov		pc, lr

/*
 * Acquire kernel from system mode entry
 * r0 = flags to set
 * r5 = &inkernel
 */
sys_acquire_kernel:
0:	mrs		r2, cpsr
	bic		r2, r2, #ARM_CPSR_I|ARM_CPSR_F
	msr		cpsr, r2

	GETCPU	r3

	/*
	 * Wait until INKERNEL_NOW is clear unless we have the kernel
	 */
1:	.word	0xe1952f9f						// ldrex	r2, [r5]
	tst		r2, #INKERNEL_NOW
	movne	r1, r2, lsr #24					// do we have the kernel?
	teqne	r1, r3
	bne		1b

	/*
	 * Disable interrupts and attempt to set inkernel
	 */
	mrs		r3, cpsr
	orr		r3, r3, #ARM_CPSR_I
	msr		cpsr, r3

end_acquire_kernel_attempt:

	GETCPU	r3
	bic		r2, r2, #0xff000000
	orr		r2, r2, r3, lsl #24				// set cpunum
	orr		r2, r2, r0						// set required bits
	.word	0xe1853f92						// strex	r3, r2, [r5]
	teq		r3, #0
	bne		0b
	mov		pc, lr

.endif

/*
 * ---------------------------------------------------------------------
 * IRQ Interrupt
 *
 * The code sequence between intr_entry_start and intr_entry_end is used
 * by cpu_interrupt_init to mix with startup provided code bursts on the
 * fly to detect the interrupting source.
 *
 * Once the interrupt source has been isolated, intr_process_queue is
 * invoked to dispatch the interrupt to the appropriate handlers, and on
 * return from interrupt handling, intr_done is invoked to return from
 * the interrupt.
 * ---------------------------------------------------------------------
 */
intr_entry_start:
	/*
	 * Save registers:
	 * sets r4 to base of saved register context
	 * sets Z flag if entered from SVC mode
	 */
	PUSHREG 4

	/*
	 * Set kernel stack if necessary
	 */
	ldrne	sp, LIker_stack
	GETCPU	r3
	SMPLDR	sp, sp, r3, ne

.ifdef	VARIANT_smp
	/*
	 * Set r10 so intr_done can tell what mode we interrupted
	 */
	moveq	r10, #0			// entered from SVC mode
	movne	r10, #1			// entered from USER/SYSTEM mode
.endif

	/*
	 * Save Xscale CP0 registers if necessary
	 * - Z flag was set if we entered from SVC mode
	 */
	XSCALE_CP0_SAVE r4, LI__cpu_flags

	/*
	 * Increment the inkernel interrupt count
	 */
	ldr		r0, LIinkernel
.ifdef	VARIANT_smp
0:	.word	0xe190cf9f		// ldrex	ip, [r0]
	add		ip, ip, #1
	.word	0xe1801f9c		// strex	r1, ip, [r0]
	teq		r1, #0
	bne		0b				// strex failed, retry
.else
	ldr		ip, [r0]
	add		ip, ip, #1
	str		ip, [r0]
.endif

.ifndef	VARIANT_v6
	/*
	 * Enable all MMU domains to allow access to all user address spaces.
	 *
	 * WARNING:	assumes proc is using ProcessID mapping to implement user
	 *			address spaces. See proc/mem_virtual.c: virtual_aspace().
	 */
	ldr		ip, LIenable_domains
	mcr		p15, 0, ip, c3, c0, 0
.endif

	ACQUIRE_INTR_SLOCK LIintr_slock

	/*
	 * Branch to startup/generated interrupt identification code
	 */
	b		intr_entry_end

	/*
	 * We need these labels here since this code is _copied_
	 */
LIinkernel:		.word	inkernel
LIker_stack:	.word	ker_stack
LIactives:		.word	actives
.ifndef	VARIANT_v6
LIenable_domains:	.word	0x55555555
.endif
LI__cpu_flags:	.word	__cpu_flags
.ifdef	VARIANT_smp
LIintr_slock:	.word	intr_slock
.endif

intr_entry_end:

/*
 * ---------------------------------------------------------------------
 * This code fragment is branched to by the interrupt dispatch code generated
 * by cpu_interrupt_init(). This passes the INTRLEVEL in r8.
 *
 * FIXME: we could hand-code an optimised version of interrupt() here.
 * ---------------------------------------------------------------------
 */
intr_process_queue:
.ifdef	VARIANT_smp
	RELEASE_INTR_SLOCK

	mov		r0, r8
	mov		r5, lr
	bl		interrupt

	ACQUIRE_INTR_SLOCK
	mov		pc, r5
.else
	mov		r0, r8
	b		interrupt
.endif

/*
 * ---------------------------------------------------------------------
 * This code fragment is branched to by the interrupt handling code built by
 * cpu_interrupt_init(), to return from interrupt handling.
 * ---------------------------------------------------------------------
 */
intr_done:
.ifdef	VARIANT_smp
	RELEASE_INTR_SLOCK

	/*
	 * Decrement inkernel interrupt count.
	 */
	ldr		r5, =inkernel
0:	.word	0xe195cf9f				// ldrex	ip, [r5]
	sub		ip, ip, #1
	.word	0xe1850f9c				// strex	r0, ip, [r5]
	teq		r0, #0
	bne		0b						// strex failed, retry

	/*
	 * We have 6 cases to consider for SMP:
	 *			This CPU		Another CPU		Action
	 *			--------		-----------		------
	 * case1	from user		in user			acquire kernel
	 * case2	from user		in kernel		check preempt and maybe IPI
	 * case3	from kernel		in user			check preempt and maybe IPI
	 * case4	from intr		in user			return to user
	 * case5	from intr		in kernel		return to user
	 * case6	acquire spin	*				try to acquire kernel, else return
	 */

	/*
	 * r10 is 0 if we entered from SVC mode, 1 if we entered from user mode
	 */
	GETCPU	r3
	teq		r10, #0
	beq		from_kerorintr

case1:
0:	.word	0xe195cf9f				// ldrex	ip, [r5]
	tst		ip, #INKERNEL_NOW|INKERNEL_LOCK
	bne		case2
	orr		ip, ip, #INKERNEL_NOW|INKERNEL_LOCK
	bic		ip, ip, #0xff000000
	orr		ip, ip, r3, lsl #24		// set cpunum
	.word	0xe1850f9c				// strex	r0, ip, [r5]
	teq		r0, #0
	bne		0b						// strex failed, retry
	b		__ker_exit

case2:
	ldr		r4, =actives
	ldr		r2, =queued_event_priority
	SMPLDR	r4, r4, r3				// r4 = actives[RUNCPU]

	/*
	 * Can't preempt if kernel is locked
	 */
	tst		ip, #INKERNEL_LOCK|INKERNEL_SPECRET
	bne		__ker_exit2

	ldr		r2, [r2]				// queued_event_priority
	ldrb	r3, [r4, #PRIORITY]
	cmp		r3, r2
	bhs		__ker_exit2

	/*
	 * queued_event_priority is higher than act->priority
	 * send an ipi to cpu in the kernel to reschedule if necessary
	 */
	mov		r0, ip, lsr #24			// cpunum
	mov		r1, #IPI_CHECK_INTR
	bl		send_ipi
	b		__ker_exit2

from_kerorintr:
	/*
	 * FIXME_SMP: x86 has SMP_MSGOPT support here
	 */

	/*
	 * If nested interrupt or kernel is locked, restore kernel context
	 */
	tst		ip, #INKERNEL_INTRMASK
	tsteq	ip, #INKERNEL_LOCK
	bne		__svc_exit

	/*
	 * Check if we were spinning waiting to acquire the kernel
	 */
	ldr		r1, [sp, #ARM_REG_PC*4]			// get interrupted pc
	adr		r0, beg_acquire_kernel_attempt
	cmp		r0, r1
	bhi		case3
	adr		r0, end_acquire_kernel_attempt
	cmp		r0, r1
	blo		case3

case6:
	/*
	 * We were spinning waiting to acquire the kernel
	 * If we were in system mode, return immediately
	 */
	adr		r0, sys_acquire_kernel
	cmp		r0, r1
	blo		__svc_exit

	/*
	 * Check what inkernel flags we were trying to set.
	 * WARNING: must match register used in acquire kernel spin loop
	 */
	ldr		r2, [sp, #ARM_REG_R0*4]			// inkernel flags we were setting
	teq		r2, #INKERNEL_NOW
	bne		case1							// return to user

	/*
	 * We were spinning in syscall entry - back up pc to restart the call
	 */
	ldr		r2, =actives
	SMPLDR	r2, r2, r3
	add		r2, r2, #REG_OFF
	ldr		r1, [r2, #ARM_REG_PC*4]
	sub		r1, r1, #KER_ENTRY_SIZE
	str		r1, [r2, #ARM_REG_PC*4]
	
	ldr		sp, =ker_stack
	SMPLDR	sp, sp, r3						// reset to ker_stack[RUNCPU]
	b		case1

case3:
	ldr		r4, =actives
	ldr		r2, =queued_event_priority
	SMPLDR	r4, r4, r3						// actives[RUNCPU]
	ldr		r2, [r2]						// queued_event_priority
	ldrb	r3, [r4, #PRIORITY]
	cmp		r3, r2
	blo		__preempt

case4:
case5:
	/*
	 * If there are pending events, send an ipi to another cpu to handle it
	 */
	ldr		r1, =intrevent_pending
	GETCPU	r0
	ldr		r1, [r1]
	teq		r1, #0
	beq		__svc_exit

	/*
	 * Send ipi to (RUNCPU + 1) % num_processors
	 */
	ldr		r1, =num_processors
	add		r0, r0, #1
	ldr		r1, [r1]
	teq		r1, r0
	moveq	r0, #0
	mov		r1, #IPI_CHECK_INTR
	adr		lr, __svc_exit
	b		send_ipi

.else
	/*
	 * Decrement the inkernel interrupt count.
	 * If inkernel is clear, do regular kernel exit processing.
	 */
	ldr		r5, =inkernel
	ldr		ip, [r5]
	subs	ip, ip, #1
	str		ip, [r5]
	beq		__ker_exit

	/*
	 * Return immediately if kernel is locked, or this is a nested interrupt
	 */
	tst		ip, #INKERNEL_INTRMASK
	tsteq	ip, #INKERNEL_LOCK
	bne		__svc_exit

	/*
	 * Preempt the thread if its priority < queued_event_priority.
	 * Otherwise, return immediately.
	 */
	ldr		r4, =actives
	ldr		r2, =queued_event_priority
	GETCPU	r3
	SMPLDR	r4, r4, r3			// actives[RUNCPU]
	ldr		r2, [r2]			// queued_event_priority
	ldrb	r3, [r4, #PRIORITY]
	cmp		r3, r2
	blo		__preempt

	/*
	 * Fall through to __svc_exit
	 */
.endif

__svc_exit:
	/*
	 * Restore kernel mode context
	 */
	mrs		ip, cpsr
	orr		ip, ip, #ARM_CPSR_I | ARM_CPSR_F
	msr		cpsr, ip

	ldr		r0, [sp, #ARM_REG_SPSR*4]
	msr		spsr_cxsf, r0
	ldmia	sp, {r0-r15}^

.ifdef	VARIANT_smp
/*
 * ---------------------------------------------------------------------
 * SMP inter-processor interrupt handler
 *
 * This code fragment is branched to by the interrupt dispatch code
 * when it detects an interrupt vector marked with INTR_CONFIG_FLAG_IPI
 * ---------------------------------------------------------------------
 */

intr_process_ipi:
	RELEASE_INTR_SLOCK

	mov		r6, lr					// save lr for return

	ldr		r0, =cpupageptr
	GETCPU	r7							// r7 = RUNCPU
	SMPADD	r0, r0, r7					// &cpupageptr[RUNCPU]
	ldr		r5, [r0, #CPUPAGE_STATE]	// r5 = cpupageptr[RUNCPU]->state
	mov		r2, #1
	str		r2, [r0, #CPUPAGE_STATE]	// cpupageptr[RUNCPU]->state = 1

	/*
	 * Atomically get ipicmds[RUNCPU] and set it to 0
	 */
	ldr		r0, =ipicmds
	SMPADD	r0, r0, r7				// r0 = &ipicmds[RUNCPU]
	mov		r2, #0
	swp		r9, r2, [r0]

	/*
	 * FIXME_SMP: x86 has SMP_MSGOPT support here
	 */

	tst		r9, #IPI_PARKIT
	beq		1f
	// Freeze the system - we've got a kernel dump happening
	ldr		r0, =alives
	add		r0, r0, r7
	ldrb	r1, [r0]	
	orr		r1, r1, #0x2	
	strb	r1, [r0]	// Mark the CPU as parked
2:	b		2b	

1:	
	tst		r9, #IPI_TLB_SAFE
	beq		1f
	teq		r5, #0
	bne		1f						// don't set_safe_aspace if in interrupt
	mov		r0, r7
	bl		set_safe_aspace
	mrs		ip, cpsr				// aspace switch can re-enable interrupts
	orr		ip, ip, #ARM_CPSR_I
	msr		cpsr, ip

1:	tst		r9, #IPI_CLOCK_LOAD
	beq		1f
	bl		clock_load
	mrs		ip, cpsr				// make sure interrupts are disabled
	orr		ip, ip, #ARM_CPSR_I
	msr		cpsr, ip

	/* 
	 * FIXME_SMP: do we need to handle this?
	 */
1:	tst		r9, #IPI_INTR_MASK
	beq		1f
	mov		r0, #INTR_FLAG_SMP_BROADCAST_MASK
	bl		interrupt_smp_sync

	/* 
	 * FIXME_SMP: do we need to handle this?
	 */
1:	tst		r9, #IPI_INTR_UNMASK
	beq		1f
	mov		r0, #INTR_FLAG_SMP_BROADCAST_UNMASK
	bl		interrupt_smp_sync

1:	tst		r9, #IPI_TLB_FLUSH
	beq		1f
	// FIXME_SMP: is this correct?
	mov		r0, #0
	mcr		p15, 0, r0, c8, c7, 0		// flush I+D TLBs
	mcr		p15, 0, r0, c7, c10, 4		// data synchronisation barrier

1:	tst		r9, #IPI_RESCHED
	mov		r0, #0
	orrne	r0, r0, #_NTO_ATF_SMP_RESCHED

	tst		r9, #IPI_TIMESLICE
	orrne	r0, r0, #_NTO_ATF_TIMESLICE

1:	tst		r9, #IPI_CONTEXT_SAVE
	beq		1f

	/*
	 * call cpu_force_fpu_save(FPUDATA(actives_fpu[RUNCPU]->fpudata))
	 */
	ldr		r1, =actives_fpu
	mov		r11, r0						// save pending async_flags
	SMPLDR	r0, r1, r7					// r0 = actives_fpu[RUNCPU]
	teq		r0, #0
	beq		0f
	bl		cpu_force_fpu_save

	/*
	 * Clear actives_fpu[RUNCPU]
	 */
	ldr		r1, =actives_fpu
	mov		r0, #0
	SMPSTR	r0, r1, r7					// actives_fpu[RUNCPU] = 0
0:	mov		r0, r11						// restore pending async_flags

	/*
	 * Check if we need to set async_flags and force entry into kernel
	 */
1:	teq		r0, #0						// no need if no async flags set
	beq		1f

	/*
	 * Only force kernel entry if we are in user mode
	 */
	teq		r10, #0
	orrne	r0, r0, #_NTO_ATF_FORCED_KERNEL

	ldr		r3, =actives
	SMPLDR	r3, r3, r7
	add		r1, r3, #ATFLAGS			// r1 = &actives[RUNCPU]->async_flags
0:	.word	0xe1912f9f					// ldrex	r2, [r1]
	orr		ip, r2, r0
	.word	0xe181cf9c					// strex	ip, ip, [r1]
	teq		ip, #0
	bne		0b							// strex failed - retry

	/*
	 * Don't force kernel entry if _NTO_ATF_FORCED_KERNEL was already set
	 */
	tst		r0, #_NTO_ATF_FORCED_KERNEL
	beq		1f
	tst		r2, #_NTO_ATF_FORCED_KERNEL
	bne		1f

	add		ip, r3, #REG_OFF
	ldr		r0, [ip, #ARM_REG_IP*4]
	ldr		r1, [ip, #ARM_REG_PC*4]
	str		r0, [r3, #ARGS_ASYNC_TYPE]	// preserve original ip register
	str		r1, [r3, #ARGS_ASYNC_IP]	// preserve original pc register

	ldr		r1, =kercallptr
	mov		r0, #__KER_NOP
	ldr		r1, [r1]
	str		r0, [ip, #ARM_REG_IP*4]		// __KER_NOP call
	str		r1, [ip, #ARM_REG_PC*4]		// execute code at kercallptr

1:	ldr		r0, =cpupageptr
	SMPADD	r0, r0, r7					// &cpupageptr[RUNCPU]
	str		r5, [r0, #CPUPAGE_STATE]	// restore cpupageptr[RUNCPU]->state

	ACQUIRE_INTR_SLOCK
	mov		pc, r6
.endif

/*
 * ---------------------------------------------------------------------
 * Undefined instruction exception
 * ---------------------------------------------------------------------
 */
__und_entry:
	/*
	 * Save context, setting return pc to the faulting instruction
	 *	r4 = saved context
	 *	r5 = &inkernel
	 *	ip = inkernel
	 */
	SAVE_CONTEXT 4

.ifdef	VARIANT_smp
	/*
	 * Acquire the kernel, setting INKERNEL_EXIT to ensure we restart
	 * the faulting instruction if we get preempted
	 */
	teq		r10, #0
	mov		r0, #INKERNEL_NOW|INKERNEL_EXIT
	bleq	sys_acquire_kernel
	blne	usr_acquire_kernel
.else
	mov		r6, ip
.endif
	ldr		r0, =actives
	mov		r1, r4
	GETCPU	r3
	SMPLDR	r0, r0, r3			// actives[RUNCPU]
	bl		arm_undef
.ifdef	VARIANT_smp
	teq		r10, #0
	beq		__svc_exit
	bne		__ker_exit
.else
	teq		r6, #0
	beq		__ker_exit
	bne		__svc_exit
.endif


/*
 * ---------------------------------------------------------------------
 * Prefetch abort exception
 * ---------------------------------------------------------------------
 */
__prf_entry:
	/*
	 * Save context, setting return pc to restart the faulting instruction
	 *	r4 = ptr to saved context
	 *	r5 = &inkernel
	 *	ip = inkernel value
	 */
	SAVE_CONTEXT 4

	mov		r6, #0							// fault status
	ldr		r7, [r4, #ARM_REG_PC*4]			// fault address
	ldr		r8, __prf_code					// sigcode
	mov		r9, ip
	b		__abt_common

__prf_code:	.word	SIGSEGV | (FLTPAGE   << 16) | (SEGV_MAPERR << 8)

	/*
	 * Map FSR bits 0..3 to signal code
	 */
__abt_code:	.word	SIGSEGV | (FLTACCESS << 16) | (SEGV_ACCERR << 8)	// 0
			.word	SIGBUS  | (FLTACCESS << 16) | (BUS_ADRALN  << 8)	// 1
			.word	SIGBUS  | (FLTACCESS << 16) | (BUS_OBJERR  << 8)	// 2
			.word	SIGBUS  | (FLTACCESS << 16) | (BUS_ADRALN  << 8)	// 3
			.word	SIGBUS  | (FLTACCESS << 16) | (BUS_OBJERR  << 8)	// 4
			.word	SIGSEGV | (FLTPAGE   << 16) | (SEGV_MAPERR << 8)	// 5
			.word	SIGBUS  | (FLTACCESS << 16) | (BUS_OBJERR  << 8)	// 6
			.word	SIGSEGV | (FLTPAGE   << 16) | (SEGV_MAPERR << 8)	// 7
			.word	SIGBUS  | (FLTACCESS << 16) | (BUS_OBJERR  << 8)	// 8
			.word	SIGSEGV | (FLTPAGE   << 16) | (SEGV_ACCERR << 8)	// 9
			.word	SIGBUS  | (FLTACCESS << 16) | (BUS_OBJERR  << 8)	// a
			.word	SIGSEGV | (FLTPAGE   << 16) | (SEGV_ACCERR << 8)	// b
			.word	SIGSEGV | (FLTACCESS << 16) | (SEGV_MAPERR << 8)	// c
			.word	SIGSEGV | (FLTPAGE   << 16) | (SEGV_ACCERR << 8)	// d
			.word	SIGSEGV | (FLTACCESS << 16) | (SEGV_MAPERR << 8)	// e
			.word	SIGSEGV | (FLTPAGE   << 16) | (SEGV_ACCERR << 8)	// f

/*
 * ---------------------------------------------------------------------
 * Data abort exception
 * ---------------------------------------------------------------------
 */
__abt_entry:
	/*
	 * Save context, setting return pc to restart the faulting instruction
	 *	r4 = ptr to saved context
	 *	r5 = &inkernel
	 *	ip = inkernel value
	 */
	SAVE_CONTEXT 8
	
	/*
	 *	r6 = fault status
	 *	r7 = fault address
	 */
	mrc		p15, 0, r6, c5, c0, 0			// MMU FSR
	mrc		p15, 0, r7, c6, c0, 0			// MMU FAR

.ifdef	VARIANT_v6
	mov		r9, ip							// save inkernel value

	/*
	 * Clear exclusive monitor - state is unpredictable on data abort
	 */
.ifdef	VARIANT_smp
	.word	0xf57ff01f				// clrex
.else
	ldr		r1, =dummy_strex		// store to dummy_strex variable
	.word	0xe1812f93				// strex r2, r3, [r1]
.endif
.else
	/*
	 * Some processor cores pass modified address in FAR.
	 * Remove the PID mask if necessary.
	 */
	mrc		p15, 0, r1, c13, c0, 0
	and		r2, r7, #0xfe000000
	teq		r1, r2
	biceq	r7, r7, r2

	/*
	 * Perform data abort fixup if necessary
	 */
	ldr		r1, =mmu_abort
	mov		r9, ip
	ldr		ip, [r1]
	mov		r0, r4
	teq		ip, #0
	movne	lr, pc
	movne	pc, ip
.endif

	/*
	 * Get signal code corresponding to fault
	 */
	adr		r0, __abt_code
	and		r6, r6, #0xf
	ldr		r8, [r0, r6, lsl #2]

	/*
	 * Indicate whether fault was a load or store
	 * FIXME: only works for ARM mode code - needs work for Thumb
	 */
	ldr		r0, [r4, #ARM_REG_PC*4]
	ldr		r0, [r0]
	tst		r0, #(1 << 20)
	orreq	r6, r6, #SIGCODE_STORE
	orreq	r8, r8, #SIGCODE_STORE

	/*
	 * Fall through to abt_common
	 */

/*
 * ---------------------------------------------------------------------
 * Common processing for prefetch/data aborts.
 *	r4 = actives[0]
 *	r5 = &inkernel
 *	r6 = fault_code
 *	r7 = fault_addr
 *	r8 = sigcode
 *	r9 = original inkernel
 * ---------------------------------------------------------------------
 */
__abt_common:
	/*
	 * Get the active thread and check whether it was a kernel fault
	 */
	ldr		r4, =actives
	GETCPU	r3
	SMPLDR	r4, r4, r3			// actives[RUNCPU]
.ifdef	VARIANT_smp
	teq		r10, #0
	beq		__abt_sys
.else
	teq		r9, #0
	bne		__abt_sys
.endif

	/*
	 * User page fault.
	 * Lock the kernel and call the memmgr.fault() hook.
	 * Interrupts will be re-enabled before calling the hook.
	 */
.ifdef	VARIANT_smp
	mov		r0, #INKERNEL_NOW|INKERNEL_LOCK
	bl		usr_acquire_kernel
.else
	orr		ip, r9, #INKERNEL_NOW | INKERNEL_LOCK
	str		ip, [r5]
.endif
	bl		__handle_page_fault
	/*
	 * If memmgr can handle the fault, the thread will be blocked in
	 * STATE_WAITPAGE until kerext_page_cont() is called.
	 */
	movs	r8, r0
	beq		__ker_exit

	/*
	 * Check if we need to do alignment fault emulation:
	 * (fsr == 1 || fsr == 3) && (thp->flags & _NTO_TF_ALIGN_FAULT) == 0
	 */
	and		r1, r6, #0xf
	teq		r1, #1
	teqne	r1, #3
	bne		0f
	ldr		r1, [r4, #TFLAGS]
	tst		r1, #_NTO_TF_ALIGN_FAULT
	bne		0f

	/*
	 * Call arm_align_emulate(&thp->reg, fault_addr)
	 */
	add		r0, r4, #REG_OFF
	mov		r1, r7
	bl		arm_align_emulate
	teq		r0, #0
	beq		__ker_exit

0:
	/*
	 * It didn't handle the fault - deliver a signal
	 */
	mov		r1, r4
	mov		r2, r7
	bl		usr_fault
	b		__ker_exit

__abt_sys:
	tst		r9, #INKERNEL_LOCK
	bne		__hardcrash

.ifdef	VARIANT_smp
	/*
	 * Need to check cpupageptr[RUNCPU]->state to see if we are in interrupt
	 * r3 is still set from above.
	 */
	ldr		r0, =cpupageptr
	SMPLDR	r0, r0, r3
	ldr		r0, [r0, #CPUPAGE_STATE]
	teq		r0, #0
.else
	tst		r9, #INKERNEL_INTRMASK
.endif
	bne		__hardcrash

	teq		r4, #0
	beq		__hardcrash

	/*
	 * Lock the kernel and call the memmgr.fault() hook.
	 * Interrupts will be re-enabled before calling the hook.
	 */
.ifdef	VARIANT_smp
	mov		r0, #INKERNEL_NOW|INKERNEL_LOCK
	bl		sys_acquire_kernel
	ldr		r9, [r5]					// reload inkernel
.else
	orr		ip, r9, #INKERNEL_LOCK
	str		ip, [r5]
.endif

	/*
	 * Add additional info to sigcode for the VM fault handling:
	 * - set SIGCODE_KERNEL
	 * - set SIGCODE_KEREXIT if INKERNEL_EXIT is set
	 * - set SIGCODE_INXFER  if xfer_handlers[RUNCPU] is non-NULL
	 */
	ldr		r0, =xfer_handlers
	tst		r9, #INKERNEL_EXIT
	orrne	r8, r8, #SIGCODE_KEREXIT
#ifdef SMP_MSGOPT
	GETCPU	r3
	SMPLDR	r0, r0, r3					// r0 = xfer_handlers[RUNCPU]
#else
	ldr		r0, [r0]					// r0 = xfer_handlers[0]
#endif
	orr		r8, r8, #SIGCODE_KERNEL
	teq		r0, #0
	orrne	r8, r8, #SIGCODE_INXFER

	bl		__handle_page_fault
	mov		r8, r0

	tst		r9, #INKERNEL_SPECRET
	bne		__abt_specret

	/*
	 * If we didn't handle the fault, make the kernel call return EFAULT.
	 */
	teq		r8, #0
	bne		__fixup_kcall

	/*
	 * The fault is being handled and the current thread is blocked in
	 * STATE_WAITPAGE until kerext_page_cont() is called.
	 */
	b		__preempt

__abt_specret:
	/*
	 * Clear INKERNEL_SPECRET
	 */
.ifdef	VARIANT_smp
0:	.word	0xe195cf9f					// ldrex	ip, [r5]
	orr		ip, ip, #INKERNEL_LOCK
	bic		ip, ip, #INKERNEL_SPECRET
	.word	0xe185cf9c					// strex	ip, ip, [r5]
	teq		ip, #0
	bne		0b							// strex failed, retry
.else
	orr		ip, r9, #INKERNEL_LOCK
	bic		ip, ip, #INKERNEL_SPECRET
	str		ip, [r5]
.endif

	/*
	 * If we didn't handle the fault, make the kernel call return EFAULT.
	 */
	teq		r8, #0
	bne		__fixup_specret

	/*
	 * The fault is being handled and the current thread is blocked in
	 * STATE_WAITPAGE until kerext_page_cont() is called.
	 */

	/*
	 * If xfer_handlers is non-NULL, clear it and check the restart handler
	 */
	ldr		r2, =xfer_handlers
	mov		ip, #0
#ifdef	SMP_MSGOPT
	GETCPU	r3
	SMPADD	r2, r2, r3				// &xfer_handlers[RUNCPU]
#endif
	swp		ip, ip, [r2]
	teq		ip, #0
	beq		__ker_exit

	/*
	 * Run the restart handler if it is non-null
	 */
	ldr		ip, [ip, #4]
	teq		ip, #0
	beq		__ker_exit
	mov		r0, r4
	mov		r1, sp
	mov		lr, pc
	mov		pc, ip
	b		__ker_exit

/*
 * ---------------------------------------------------------------------
 * Invoke the memmgr.fault() hook.
 * On entry:
 *	r4	= actives[0]
 *	r6	= fault_code
 *	r7	= fault_addr
 *	r8  = sigcode
 * ---------------------------------------------------------------------
 */
__handle_page_fault:
	/*
	 * Re-enable interrupts before calling the hook.
	 */
	mrs		ip, cpsr
	bic		ip, ip, #ARM_CPSR_I|ARM_CPSR_F
	msr		cpsr, ip

	/*
	 * Set up fault_info on stack
	 */
	ldr		ip, =aspaces_prp
	stmdb	sp!, {lr}				// save return address
	sub		sp, sp, #SIZEOF_FAULT_INFO
	mov		r0, sp					// &fault_info
	GETCPU	r3
	SMPLDR	ip, ip, r3				// aspaces_prp[RUNCPU]
	str		r6, [r0, #FI_CPU_CODE]	// fault_info->cpu.code
	str		r7, [r0, #FI_VADDR]		// fault_info->vaddr
	str		r8, [r0, #FI_SIGCODE]	// fault_info->sigcode
	str		ip, [r0, #FI_PRP]		// fault_info->prp

	/*
	 * Call memmgr.fault(&fault_info)
	 */
	ldr		ip, =memmgr
	mov		lr, pc
	ldr		pc, [ip, #MEMMGR_FAULT]	// call memmgr.fault(&info)

	teq		r0, #0
	beq		__page_fault_deferred	// fault deferred to process time

	teq		r0, #1
	moveq	r0, #0					// handled the fault
	movne	r0, r8					// didn't handle the fault
	add		sp, sp, #SIZEOF_FAULT_INFO
	ldmia	sp!, {pc}

__page_fault_deferred:
	/*
	 * Call PageFaultWait to deliver pulse to handle fault at process time
	 */
	mov		r0, sp					// &fault_info
	bl		PageFaultWait
	teq		r0, #0
	ldrne	r0, [sp, #FI_SIGCODE]	// PageFaultWait failed
	add		sp, sp, #SIZEOF_FAULT_INFO
	ldmia	sp!, {pc}

/*
 * ---------------------------------------------------------------------
 * Reset Exception or Vector exception.
 * This is the result of a branch through 0 or 0x14
 * ---------------------------------------------------------------------
 */
__rsv_entry:
#ifdef	FIXME
	switch to kernel stack and crash
#else
0:	b		0b
#endif

/*
 * ---------------------------------------------------------------------
 * FIQ interrupt
 * ---------------------------------------------------------------------
 */
__fiq_entry:
#ifdef	FIXME
	switch to kernel stack and crash
#else
0:	b		0b
#endif

/*
 * ---------------------------------------------------------------------
 * Test for special SWI codes used emulate the following:
 *
 *	0xff000000	- smp_cmpxchg emulation
 *	0xfe000000	- ClockCycles emulation
 * ---------------------------------------------------------------------
 */
__emu_decode:
	teq		ip, #0xff000000
	beq		__emu_cmpxchg
	teq		ip, #0xfe000000
	bhs		__emu_clockcycles
	b		__ker_entry_save

/*
 * ---------------------------------------------------------------------
 * Emulate the x86 cmpxchg to support libc sync primitives.
 * On entry, interrupts are disabled:
 *	r0   - ptr to word being tested
 *	r1   - value to compare
 *	r2   - value to set
 *	lr   - saved return pc
 *	spsr - saved cpsr
 *
 * On return:
 *	r0	- original value in dest
 *	r1	- preserved
 *	r2	- preserved
 *	ip	- trashed
 *	Z	- set if successful
 * ---------------------------------------------------------------------
 */
__emu_cmpxchg:
	ldr		ip, [r0]
	teq		ip, r1
	streq	r2, [r0]
	mov		r0, ip
	mrs		ip, spsr
	bicne	ip, ip, #ARM_CPSR_Z
	orreq	ip, ip, #ARM_CPSR_Z
	msr		spsr, ip
	movs	pc, lr

/*
 * ---------------------------------------------------------------------
 * Emulate the ClockCycles counter
 * On entry, interrupts are disabled.
 * On return:
 *	r0 - low order 32 bits of cycle counter
 *	r1 - high order 32 bits of cycle counter
 *
 * WARNING: we save lr on the stack.
 *			If we came from usr/sys mode, this is the top of thread regs.
 *			If we came from svc mode, this is a proper stack.
 *			We assume that the callout_timer_value callout does not use
 *			any stack (or if it does, it only uses < sizeof CPU_REGISTERS)
 * ---------------------------------------------------------------------
 */
__emu_clockcycles:
	str		lr, [sp, #-4]	// save user return address

	/*
	 * Get timer counter value
	 */
	ldr		r0, =_syspage_ptr
	ldr		r1, =qtimeptr
	ldr		ip, =callout_timer_value
	ldr		r0, [r0]
	ldr		r1, [r1]
	mov		lr, pc
	ldr		pc, [ip]
	/*
	 * Calculate new timestamp from cycles
	 */
	ldr		r2, =cycles
	ldr		lr, =last_cycles
	ldmia	r2, {r2,r3}
.ifdef	VARIANT_le
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
	 * Update last_cycles and return to user mode
	 */
1:	stmia	lr, {r0,r1}
	ldr		lr, [sp, #-4]
	movs	pc, lr

/*
 * This is a dummy reference to __divsi3 which is needed by the APS
 * scheduler. We should come back and fix this in a cleaner way.
 */
.word	__divsi3
