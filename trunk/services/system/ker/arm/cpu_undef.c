/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 * 
 * You must obtain a written license from and pay applicable license fees to QNX 
 * Software Systems before you may reproduce, modify or distribute this software, 
 * or any work that includes all or part of this software.   Free development 
 * licenses are available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *  
 * This file may contain contributions from others.  Please review this entire 
 * file for other proprietary rights or license notices, as well as the QNX 
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
 * for other information.
 * $
 */

/*
 * ARM undefined instruction handling
 */

#include "externs.h"
#include <arm/opcode.h>

int
fpemu_handler(void *t, unsigned inst, CPU_REGISTERS *reg, unsigned *signo)
{
	/*
	 * Bounce into FP emulator to emulate the instruction.
	 * Set signo to default value in case fpemu.so doesn't exist.
	 */
	*signo = MAKE_SIGCODE(SIGFPE, FPE_NOFPU, FLTNOFPU);
	return ARM_COPROC_EMULATE;
}

static int (*coproc_handler[16])(void *, unsigned, CPU_REGISTERS *, unsigned *) = {
	0,				// CP0
	0,				// CP1
	0,				// CP2
	0,				// CP3
	0,				// CP4
	0,				// CP5
	0,				// CP6
	0,				// CP7
	0,				// CP8
	0,				// CP9
	fpemu_handler,	// CP10
	fpemu_handler,	// CP11
	0,				// CP12
	0,				// CP13
	0,				// CP14
	0,				// CP15
};

static int
coproc_exception(THREAD *thp, unsigned inst, CPU_REGISTERS *reg, unsigned *signo)
{
	ARM_FPEMU_CONTEXT	*ctx;
	int					(*func)(void *, unsigned, CPU_REGISTERS *, unsigned *);
	int					status;

	/*
	 * Invoke coprocessor specific handler.
	 * This may do one of the following:
	 *
	 * 1. deliver a signal to the thread:
	 *    - possibly modify signo if required
	 *    - return ARM_COPROC_FAULT
	 *    This will deliver the signal via usr_fault().
     *
	 * 2. request allocation of coprocessor state:
	 *    - set _NTO_ATF_FPUSAVE_ALLOC in thp->async_flags
	 *    - return ARM_COPROC_HANDLED
	 *    We will return through __ker_exit to allocate thp->fpudata and
	 *    restart the instruction. Since the coprocessor is still disabled
	 *    this will cause another exception to handle context switching
	 *
	 * 3. handle coprocessor context switching:
	 *    - enable the coprocessor
	 *    - save context to actives_fpu[RUNCPU]->fpudata
	 *    - load context from thp->fpudata and set actives_fpu[RUNCPU]
	 *    - return ARM_COPROC_HANDLED
	 *    We will return through __ker_exit to restart the instruction with
	 *    the coprocessor now enabled.
	 *
	 * 4. bounce the instruction to fpemu.so:
	 *    - possibly modify signo if required
	 *    - return ARM_COPROC_EMULATE
	 *    We will set up the fpemu.so state and return through __ker_exit
	 *    into the emulator entry point. The emulator will call SignalFault()
	 *    to deliver the signal or resume execution of the thread.
	 */
	status = ARM_COPROC_FAULT;
	func = coproc_handler[ARM_CPNUM(inst)];
	if (func == 0 || (status = func(thp, inst, reg, signo)) != ARM_COPROC_EMULATE) {
		return status;
	}

	/*
	 * Copy user context onto user stack
	 */
	ctx = (void *)fpu_emulation_prep(reg, thp, sizeof *ctx);

	/*
	 * Perform any coprocessor specific emulation context setup
	 */
	fpu_fpemu_prep(ctx);

	/*
	 * Lock kernel to prevent preemption whilst fiddling with register context
	 *
	 * Set return pc to execute mathemulator(signo, &tls->__fpuemu_data, reg)
	 */
	lock_kernel();
	reg->gpr[ARM_REG_R0] = *signo;
	reg->gpr[ARM_REG_R1] = (unsigned)&thp->un.lcl.tls->__fpuemu_data;
	reg->gpr[ARM_REG_R2] = (unsigned)ctx;
	reg->gpr[ARM_REG_SP] = (unsigned)ctx;
	reg->gpr[ARM_REG_PC] = (unsigned)thp->process->pls->__mathemulator;

	if(begin_fp_emulation(thp)) {
		reg->gpr[ARM_REG_R0] = *signo | SIGCODE_SSTEP;
	}

	return ARM_COPROC_EMULATE;
}

/*
 * Called by optional coprocessor module to attach exception handler
 */
int
coproc_attach(int cpnum, int (*hdl)(void *, unsigned, CPU_REGISTERS *, unsigned *))
{
	if (cpnum < 0 || cpnum >= 15) {
		return -1;
	}
	if (ker_verbose && coproc_handler[cpnum] != 0) {
		kprintf("coproc_attach(%d): replacing %p with %p\n", cpnum, coproc_handler[cpnum], hdl);
	}
	coproc_handler[cpnum] = hdl;
	return 0;
}

/*
 * Called from kernel.S undefined instruction exception handler
 * - check for special undefined instructions
 * - check for coprocessor exceptions and lazy context management
 */
void
arm_undef(THREAD *thp, CPU_REGISTERS *reg)
{
	unsigned				pc    = reg->gpr[ARM_REG_PC];
	unsigned				instr = *(unsigned *)pc;
	unsigned				signo;
	PROCESS					*prp;
	struct kdebug_callback	*call;

	switch (instr) {
	case OPCODE_KDOUTPUT:	// DebugKDOutput
		/*
		 * Invoke kernel debugger message routine
		 */
		call = privateptr->kdebug_call;
		if (call && call->msg_entry) {
			call->msg_entry((char *)reg->gpr[ARM_REG_R0], reg->gpr[ARM_REG_R1]);
		}

		/*
		 * Skip over instruction and restore context
		 */
		reg->gpr[ARM_REG_PC] += 4;
		return;

	case OPCODE_KDBREAK:	// DebugKDBreak
		/*
		 * Invoke kernel debugger
		 */
		signo = MAKE_SIGCODE(SIGTRAP, TRAP_BRKPT, FLTBPT) | SIGCODE_KERNEL;
		prp = (inkernel & INKERNEL_INTRMASK) ? 0 : aspaces_prp[KERNCPU];
		(void) kdebug_enter(prp, signo, reg);

		/*
		 * Skip over instruction and restore context
		 */
		reg->gpr[ARM_REG_PC] += 4;
		return;
		
	case OPCODE_BREAK:		// breakpoint
		signo = MAKE_SIGCODE(SIGTRAP, TRAP_BRKPT, FLTBPT);
		break;

	case OPCODE_DIV0:		// divide by zero trap
		signo = MAKE_SIGCODE(SIGFPE, FPE_INTDIV, FLTIZDIV);
		break;

	default:
		signo = MAKE_SIGCODE(SIGILL, ILL_ILLOPC, FLTILL);
		break;
	}

	/*
	 * If fault happened in kernel, call the kernel debugger.
	 * Otherwise, deliver signal to user process.
	 */
#ifdef	VARIANT_smp
	/*
	 * We acquire the kernel on entry for SMP, so check the entry mode
	 */
	if ((reg->spsr & ARM_CPSR_MODE_MASK) == ARM_CPSR_MODE_SVC) {
#else
	if (inkernel) {
#endif
		/*
		 * Crash unless the kernel debugger handles the fault.
		 */
		signo |= SIGCODE_FATAL;
		prp = (inkernel & INKERNEL_INTRMASK) ? 0 : aspaces_prp[KERNCPU];
		(void) kdebug_enter(prp, signo, reg);
	} else {
#ifndef	VARIANT_smp
		/*
		 * Set inkernel and enable interrupts. We return through __ker_exit.
		 * INKERNEL_EXIT ensures faulting instruction will be restarted if
		 * we get preempted after the InterruptEnable().
		 */
		inkernel = INKERNEL_NOW | INKERNEL_EXIT;
#endif
		InterruptEnable();
		
		/*
		 * Check for coprocessor instructions
		 */
		switch (ARM_OPC(instr)) {
		case 0xc:	// coprocessor load/store, post-indexed
		case 0xd:	// coprocessor load/store, pre-indexed
		case 0xe:	// coprocessor data processing/register transfer
			if (coproc_exception(thp, instr, reg, &signo) != ARM_COPROC_FAULT) {
				/*
				 * Return through __ker_exit to:
				 * - bounce into emulator
				 * - handle _NTO_ATF_FPUSAVE_ALLOC
				 * - restart instruction with coprocessor enabled
				 */
				return;
			}
			/* fall through */
		default:
			/* deliver fault to thread */
			usr_fault(signo, thp, pc);
			break;
		}
	}
}

__SRCVERSION("cpu_undef.c $Rev: 153052 $");
