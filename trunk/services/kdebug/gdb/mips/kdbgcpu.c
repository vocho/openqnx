//#define DEBUG_GDB
#include <signal.h>
#include <stdarg.h>
#include <string.h>
#include <sys/fault.h>
#include "kdebug.h"

/*
 * mips_cause2sig_map:
 *
 * Maps an MIPS Cause code to a signal number.
 * The first 32 define the actual EXCODE of the
 * cause registers. Values >= 32 cover exceptions
 * other than general exceptions.
 */

#define MK(signo,code,fault)		MAKE_SIGCODE(SIG##signo,signo##_##code,FLT##fault)

#define BAD(cause)	MAKE_SIGCODE(SIGKILL,0,64+cause)

const unsigned long __mips_cause2sig_map[] = {
    BAD(0),						/* 00 - CAUSE_INTERRUPT                 */
    MK(SEGV,ACCERR,ACCESS),		/* 01 - CAUSE_TLB_MOD                   */
    MK(SEGV,MAPERR,BOUNDS),		/* 02 - CAUSE_TLB_LOAD                  */
    MK(SEGV,MAPERR,BOUNDS),		/* 03 - CAUSE_TLB_SAVE                  */
    MK(BUS,ADRALN,ACCESS),		/* 04 - CAUSE_ADDR_LOAD                 */
    MK(BUS,ADRALN,ACCESS),		/* 05 - CAUSE_ADDR_SAVE                 */
    MK(BUS,ADRERR,BOUNDS),		/* 06 - CAUSE_BUS_INSTR                 */
    MK(BUS,ADRERR,BOUNDS),		/* 07 - CAUSE_BUS_DATA                  */
    BAD(8),						/* 08 - CAUSE_SYSCALL                   */
    MK(TRAP,BRKPT,BPT),			/* 09 - CAUSE_BP                        */
    MK(ILL,ILLOPC,ILL),			/* 10 - CAUSE_ILLOP                     */
    MK(ILL,PRVOPC,PRIV),		/* 11 - CAUSE_CP_UNUSABLE               */
    MK(FPE,INTOVF,IOVF),		/* 12 - CAUSE_OVFLW                     */
    MK(FPE,INTOVF,IOVF), 		/* 13 - CAUSE_TRAP                      */
    BAD(14),    				/* 14 - CAUSE_VIRT_COHERENCY_INSTR      */
    MK(FPE,NOFPU,FPE), 			/* 15 - CAUSE_FPE                       */
    BAD(16),    				/* 16 - Reserved, should never happen   */
    BAD(17),    				/* 17 - Reserved, should never happen   */
    BAD(18),    				/* 18 - Reserved, should never happen   */
    BAD(19),    				/* 19 - Reserved, should never happen   */
    BAD(20),    				/* 20 - Reserved, should never happen   */
    BAD(21),    				/* 21 - Reserved, should never happen   */
    BAD(22),    				/* 22 - Reserved, should never happen   */
    BAD(23),    				/* 23 - Reserved, should never happen   */
    BAD(24),    				/* 24 - Reserved, should never happen   */
    BAD(25),    				/* 25 - Reserved, should never happen   */
    BAD(26),    				/* 26 - Reserved, should never happen   */
    BAD(27),    				/* 27 - Reserved, should never happen   */
    BAD(28),    				/* 28 - Reserved, should never happen   */
    BAD(29),   					/* 29 - Reserved, should never happen   */
    BAD(30),    				/* 30 - Reserved, should never happen   */
    BAD(31),    				/* 31 - Reserved, should never happen   */
    BAD(32),    				/* 32 - MIPS extension, Soft reset      */
    BAD(33),    				/* 33 - MIPS extension, NMI interrupt   */
    MK(BUS,ADRERR,CACHERR),		/* 34 - MIPS extension, Cache error     */
    MK(SEGV,MAPERR,UTLBREFILL),	/* 35 - MIPS extension, 32B TLB refill  */
    MK(SEGV,MAPERR,XTLBREFILL),	/* 36 - MIPS extension, 64B TLB refill  */
};


/*
 * gdb_prep_reboot
 *
 * Prepare to reboot the system
 */

void
gdb_prep_reboot(void) {

#if !defined(VARIANT_r3k)
	unsigned	one_tenth;
	unsigned	start;
	unsigned	curr;

	// Wait for 1/10th of a second to make sure the acknowledgment
	// of the kill packet has time to make it to GDB.
	one_tenth = (SYSPAGE_ENTRY(qtime)->cycles_per_sec >> 32) / 10;
	start = getcp0_count();
	do {
		curr = getcp0_count();
	} while((curr - start) < one_tenth);
#endif	

	setcp0_sreg(getcp0_sreg() & ~MIPS_SREG_IE);
	setcp0_sreg(getcp0_sreg() & ~MIPS_SREG_IMASK);
	/*
	 * Flush our caches, so the monitor doesn't have to deal
	 * with flotsam appearing in memory
	 */
	cache_flush(0UL, ~0UL);
}

/*
 * r4k_get_cpu_reg:
 *
 * Return a register from the context
 */

ulong_t
r4k_get_cpu_reg (CPU_REGISTERS *ctx, int index) {
   return(ctx->regs[MIPS_CREG(index)]);
}

uintptr_t
gdb_location(CPU_REGISTERS *ctx) {
   return(ctx->regs[MIPS_CREG(MIPS_REG_EPC)]);
}

paddr_t
gdb_image_base(paddr_t base) {
	return(MIPS_PHYS_TO_KSEG0(base));
}

/*
 * gdb_show_exception_info:
 * Print out some information on the exception taken
 */
void
gdb_show_exception_info(ulong_t signal, CPU_REGISTERS *ctx) {
    uint32_t *regs = ctx->regs;

    gdb_printf("signal=%d, sr=%x, sp=%x, gp=%x, epc=%x\n", 
		signal,
		regs[MIPS_CREG(MIPS_REG_SREG)],
		regs[MIPS_CREG(MIPS_REG_SP)],
		regs[MIPS_CREG(MIPS_REG_GP)],
		regs[MIPS_CREG(MIPS_REG_EPC)]);
}

/*
 * gdb_get_cpuregs:
 * Write the cpu registers into the buffer, and return them 
 * to our gdb client
 */
void
gdb_get_cpuregs(CPU_REGISTERS *ctx, FPU_REGISTERS *ftx) {
    gdb_register_context buf;
    int i;

    for (i=0; i<32; i++) buf.regs[i] = ctx->regs[MIPS_CREG(i)];

    buf.sr = ctx->regs[MIPS_CREG(MIPS_REG_SREG)];
    buf.lo = ctx->regs[MIPS_CREG(MIPS_REG_LO)];
    buf.hi = ctx->regs[MIPS_CREG(MIPS_REG_HI)];
    buf.bad = ctx->regs[MIPS_CREG(MIPS_REG_BADVADDR)];
    buf.cause = getcp0_cause();
    buf.pc = ctx->regs[MIPS_CREG(MIPS_REG_EPC)];
    buf.fp = ctx->regs[MIPS_CREG(MIPS_REG_SP)];

    mem2hex((char*)&buf, outbuf, sizeof(buf));
}

/*
 * gdb_set_cpuregs:
 * Read the cpu registers from the buffer given to us by
 * our gdb client
 */
void
gdb_set_cpuregs(CPU_REGISTERS *ctx, FPU_REGISTERS *ftx) {
    gdb_register_context buf;
    int i;

    hex2mem(&inbuf[1], (char *)&buf, sizeof(buf));

    for (i=0; i<32; i++) {
		ctx->regs[MIPS_CREG(i)] = buf.regs[i];
		ctx->regs[MIPS_HCREG(i)] = (buf.regs[i] & 0x80000000) ? -1 : 0;
    }

    ctx->regs[MIPS_CREG(MIPS_REG_SREG)] = buf.sr;
    ctx->regs[MIPS_CREG(MIPS_REG_LO)] = buf.lo;
    ctx->regs[MIPS_HCREG(MIPS_REG_LO)] = (buf.lo & 0x80000000) ? -1 : 0;
    ctx->regs[MIPS_CREG(MIPS_REG_HI)] = buf.hi;
    ctx->regs[MIPS_HCREG(MIPS_REG_HI)] = (buf.hi & 0x80000000) ? -1 : 0;
    ctx->regs[MIPS_CREG(MIPS_REG_BADVADDR)] = buf.bad;
    ctx->regs[MIPS_HCREG(MIPS_REG_BADVADDR)] = (buf.bad & 0x80000000) ? -1 : 0;
    ctx->regs[MIPS_CREG(MIPS_REG_EPC)] = buf.pc;
    ctx->regs[MIPS_HCREG(MIPS_REG_EPC)] = (buf.pc & 0x80000000) ? -1 : 0;
}

/*
 * branch_predict :
 * If EPC register points to a jump instruction, return
 * the destination PC of the jump so the caller can fill
 * in the "stepping" breakpoint. Note, even if the original
 * gdb breakpoint was in the branch delay slot, the EPC
 * register will point to the branch instruction rather than
 * the branch delay instruction
 */
static uintptr_t
branch_predict (CPU_REGISTERS *ctx) {
    union r4k_instr *op;
    uintptr_t 		epc = ctx->regs[MIPS_CREG(MIPS_REG_EPC)];
	uint32_t		data;
	uint32_t		*p;
	size_t			valid;
 
#ifdef DEBUG_GDB
    kprintf("\nPredict: %x\n", epc);
#endif

    /*
     * Make sure zero is 0. It's used during predict
     * but not saved by kernel as part of context.
     */
    ctx->regs[MIPS_CREG(MIPS_REG_ZERO)] = 0;

	p = mapping_add(epc, sizeof(uint32_t), PROT_READ, &valid);
	if(p == NULL) {
#ifdef DEBUG_GDB
    	kprintf("Couldn't access opcode at: 0x%x\n", epc);
#endif
		return(epc + 4);
	};
	data = *p;
	mapping_del(p, valid);
    op = (union r4k_instr *)&data;

    switch( op->i_t.op ) {

    case OPCODE_SPECIAL:
	
		switch ( op->r_t.func ) {
			
		case OPCODE_JALR:
		case OPCODE_JR:
			return(r4k_get_cpu_reg(ctx, op->r_t.rs));
	
		default:
			return(epc + 4);
		}
		break;

    case OPCODE_REGIMM:

		switch ( op->r_t.rt ) {
	
		case OPCODE_BGEZAL:
		case OPCODE_BGEZALL:
		case OPCODE_BGEZ:
		case OPCODE_BGEZL:
			if ((int)r4k_get_cpu_reg(ctx, op->i_t.rs) < 0)
				return(epc + 8);
			else
				return(epc + (4 + (op->i_t.s_imd << 2)));
	
		case OPCODE_BLTZAL:
		case OPCODE_BLTZALL:
		case OPCODE_BLTZ:
		case OPCODE_BLTZL:
			if ((int)r4k_get_cpu_reg(ctx, op->i_t.rs) >= 0)
				return(epc + 8);
			else
				return(epc + (4 + (op->i_t.s_imd << 2)));
	
		default:
			return(epc + 4);
		}
		break;

    case OPCODE_JAL:
    case OPCODE_J:
		return((epc & MIPS_R4K_HI_ADDR_MASK) + (op->j_t.target << 2));
	
    case OPCODE_BEQ:
    case OPCODE_BEQL:
		if (r4k_get_cpu_reg(ctx, op->i_t.rs) != r4k_get_cpu_reg(ctx, op->i_t.rt))
			return(epc + 8);
		else
			return(epc + (4 + (op->i_t.s_imd << 2)));

    case OPCODE_BGTZ:
    case OPCODE_BGTZL:
		if ((int)r4k_get_cpu_reg(ctx, op->i_t.rs) <= 0)
			return(epc + 8);
		else
			return(epc + (4 + (op->i_t.s_imd << 2)));
	
    case OPCODE_BLEZ:
    case OPCODE_BLEZL:
		if ((int)r4k_get_cpu_reg(ctx, op->i_t.rs) > 0)
			return(epc + 8);
		else
			return(epc + (4 + (op->i_t.s_imd << 2)));

    case OPCODE_BNE:
    case OPCODE_BNEL:
		if (r4k_get_cpu_reg(ctx, op->i_t.rs) == r4k_get_cpu_reg(ctx, op->i_t.rt))
			return(epc + 8);
		else
			return(epc + (4 + (op->i_t.s_imd << 2)));

    default:
		return(epc + 4);
    }
}

/*
 * gdb_proc_continue:
 * Resume execution at an optional address
 */
void
gdb_proc_continue(CPU_REGISTERS *ctx, int step) {
    uintptr_t addr;

    /* 
     * try to read optional parameter, addr unchanged if no parm
     */
    parsehexnum(&inbuf[1], (int *)&ctx->regs[MIPS_CREG(MIPS_REG_EPC)]);

    /*
     * If we are stepping, we must write a "breakpoint" into 
     * the next instruction and note that we have done this. 
     * The breakpoint handler will check to see if we hit one
     * of these stepping breaks and restore the code
     * appropriately.
     *
     * Note : If the original gdb break point was in the place
     *        of a branch instruction or in the branch delay slot
     *        we must decode the branch and set the "stepping"
     *        breakpoint at the destination of the branch.
     */
    if (step) {
		addr = branch_predict(ctx);
#ifdef DEBUG_GDB
		kprintf("Step: addr = %x, ", addr);
#endif
		set_break(addr);
    }
#ifdef DEBUG_GDB
    else
		kprintf("Continue\n");
#endif
}

void mips_specific_exception(CPU_REGISTERS *ctx, ulong_t sigcode) {
	ulong_t fault = SIGCODE_FAULT(sigcode);

    switch( fault ) {
	case FLTCACHERR:
		gdb_printf("****  CACHE ERROR exception  ****\n");
		break;
	case FLTUTLBREFILL:
		gdb_printf("****  32-BIT CACHE REFILL exception  ****\n");
		break;
	case FLTXTLBREFILL:
		gdb_printf("****  64-BIT CACHE REFILL exception  ****\n");
		break;
	default:
		return;
	}
	gdb_printf("    SP = %x, ", ctx->regs[MIPS_CREG(MIPS_REG_SP)]);
	gdb_printf("GP = %x, ", ctx->regs[MIPS_CREG(MIPS_REG_GP)]);
	gdb_printf("SR = %x, ", ctx->regs[MIPS_CREG(MIPS_REG_SREG)]);
	gdb_printf("EPC = %x\n", ctx->regs[MIPS_CREG(MIPS_REG_EPC)]);
	gdb_printf("CAUSE = %x, ", getcp0_cause());
	gdb_printf("BADADDR = %x\n", ctx->regs[MIPS_CREG(MIPS_REG_BADVADDR)]);
}


/*
 * ulong_t handle_exception(struct kdebug_entry *entry, ulong_t sigcode, CPU_REGISTERS *ctx)
 *
 * This function is called whenever an exception occurs. The front end
 * has saved the cpu context in stack and is passed to me in ctx.
 * ERL/EXL have been de-asserted and interrupts have been disabled.
 */

ulong_t handle_exception(struct kdebug_entry *entry, ulong_t sigcode, CPU_REGISTERS *ctx) {
#ifdef DEBUG_GDB
    kprintf("Enter KDB exception: Sigcode=%x EPC=%x\n", sigcode, ctx->regs[MIPS_CREG(MIPS_REG_EPC)]);
#endif

	if(!is_watch_entry(entry, ctx->regs[MIPS_CREG(MIPS_REG_EPC)]) && !WANT_FAULT(sigcode)) {
#ifdef DEBUG_GDB
		kprintf("Leave KDB exception: return %x\n", sigcode);
#endif
		return sigcode;
    }

    /*
     * Do not handle SIGKILL sent to user processes, or else
     * pdebug killing the debugee process will be caught.
     */
    if ((sigcode & SIGCODE_USER) && SIGCODE_SIGNO(sigcode) == SIGKILL) {
		return sigcode;
	}


    /*
     * First check to see if it's one of the MIPS special
     * exceptions that are not recognized and processed by GDB,
     * do an simple exception report if it is.
     */
    mips_specific_exception(ctx, sigcode);

	restore_break();
    /*
     * Goto GDB command interpreter.
     */
    gdb_interface(entry, ctx, sigcode);

#ifdef DEBUG_GDB
    kprintf("Leave KDB exception: EPC=%x\n", ctx->regs[MIPS_CREG(MIPS_REG_EPC)]);
#endif

    return (0);
}


/*
 * mips_signal_handler
 * 
 * Interface between the kdebugger assembly exception wrapper
 * and C entry call. 
 */ 
void mips_signal_handler(ulong_t cause, CPU_REGISTERS *ctx) {
	ulong_t sigcode;

	/* cause code is CauseReg[6..2] */
	sigcode = __mips_cause2sig_map[cause >> 2];

#ifdef DEBUG_GDB
	kprintf("Enter exception. Sigcode = %x\n", sigcode);
#endif

	handle_exception(0, SIGCODE_FATAL|sigcode, ctx);
#ifdef DEBUG_GDB
	kprintf("Exit exception. Sigcode = %x\n", sigcode);
#endif
}


int
cpu_handle_alignment(void *p, unsigned size) {
	return ((uintptr_t)p & (size-1)) == 0;
}
