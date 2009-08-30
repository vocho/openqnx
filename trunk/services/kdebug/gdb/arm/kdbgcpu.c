#include "kdebug.h"
#include <arm/opcode.h>

static uintptr_t	branch_predict(CPU_REGISTERS *ctx);


void
gdb_prep_reboot()
{
}

uintptr_t
gdb_location(CPU_REGISTERS *ctx)
{
	return ctx->gpr[ARM_REG_PC];
}

paddr_t
gdb_image_base(paddr_t base)
{
#ifdef	FIXME
	/*
	 * Need to translate address?
	 */
#else
	return base;
#endif
}

/*
 * gdb_show_exception_info:
 * Print out some information on the exception taken
 */
void
gdb_show_exception_info(ulong_t signal, CPU_REGISTERS *ctx)
{
    gdb_printf("signal=%d, eip=%x\n", signal, ctx->gpr[ARM_REG_PC]);
}

void
gdb_get_cpuregs(CPU_REGISTERS *ctx, FPU_REGISTERS *ftx)
{
	gdb_register_context	buf;
	int						i;

	for (i = 0; i < 16; i++)
		buf.gpr[i] = ctx->gpr[i];
	buf.ps = ctx->spsr;

	mem2hex((char*)&buf, outbuf, sizeof(buf));
}

void
gdb_set_cpuregs(CPU_REGISTERS *ctx, FPU_REGISTERS *ftx)
{
	gdb_register_context	buf;
	int						i;

	hex2mem(&inbuf[1], (char *)&buf, sizeof(buf));

	for (i = 0; i < 16; i++)
		ctx->gpr[i] = buf.gpr[i];
	ctx->spsr = buf.ps;
}

void
gdb_proc_continue(CPU_REGISTERS *ctx, int step)
{
	parsehexnum(&inbuf[1], (int *)&ctx->gpr[ARM_REG_PC]);
	if (step) {
		set_break(branch_predict(ctx));
	}
}

ulong_t
handle_exception(struct kdebug_entry *entry, ulong_t sigcode, CPU_REGISTERS *ctx)
{
	if(!is_watch_entry(entry, ctx->gpr[ARM_REG_PC]) && !WANT_FAULT(sigcode)) {
		return sigcode;
	}

	restore_break();
	gdb_interface(entry, ctx, sigcode);
	return 0;
}


static uintptr_t
branch_predict(CPU_REGISTERS *ctx)
{
	unsigned	pc  = ctx->gpr[ARM_REG_PC];
	unsigned	ps  = ctx->spsr;
	unsigned	op  = *(unsigned *)pc;
	int			ex  = 0;

	if (ARM_IS_BRANCH(op)) {
		/*
		 * Test branch condition
		 */
		switch (ARM_COND(op)) {
		case ARM_COND_EQ:
			ex = (ps & ARM_CPSR_Z) != 0;
			break;
		case ARM_COND_NE:
			ex = ((ps & ARM_CPSR_Z) == 0);
			break;
		case ARM_COND_CS:
			ex = ((ps & ARM_CPSR_C) != 0);
			break;
		case ARM_COND_CC:
			ex = ((ps & ARM_CPSR_C) == 0);
			break;
		case ARM_COND_MI:
			ex = ((ps & ARM_CPSR_N) != 0);
			break;
		case ARM_COND_PL:
			ex = ((ps & ARM_CPSR_N) == 0);
			break;
		case ARM_COND_VS:
			ex = ((ps & ARM_CPSR_V) != 0);
			break;
		case ARM_COND_VC:
			ex = ((ps & ARM_CPSR_V) == 0);
			break;
		case ARM_COND_HI:
			ex = ((ps & ARM_CPSR_C) == 0 && (ps & ARM_CPSR_Z) == 0);
			break;
		case ARM_COND_LS:
			ex = ((ps & ARM_CPSR_C) == 0 || (ps & ARM_CPSR_Z) != 0);
			break;
		case ARM_COND_GE:
			ex = ((ps & ARM_CPSR_N) == (ps & ARM_CPSR_V));
			break;
		case ARM_COND_LT:
			ex = ((ps & ARM_CPSR_N) != (ps & ARM_CPSR_V));
			break;
		case ARM_COND_GT:
			ex = ((ps & ARM_CPSR_Z) == 0 && (ps & ARM_CPSR_N) == (ps & ARM_CPSR_V));
			break;
		case ARM_COND_LE:
			ex = ((ps & ARM_CPSR_Z) != 0 && (ps & ARM_CPSR_N) != (ps & ARM_CPSR_V));
			break;
		case ARM_COND_AL:
			ex = 1;
			break;
		case ARM_COND_NV:
			ex = 0;
			break;
		}
	}

	/*
	 * If condition passed, target is specified by 24-bit signed offset.
	 * Otherwise, go to next instruction
	 */
	return pc + (ex ? (((int)(op << 8) >> 6) + 8) : 4);
}


int
cpu_handle_alignment(void *p, unsigned size) {
	return ((uintptr_t)p & (size-1)) == 0;
}
