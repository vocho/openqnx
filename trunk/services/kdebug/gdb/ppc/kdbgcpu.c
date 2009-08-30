//#define DEBUG_GDB
#include <signal.h>
#include <stdarg.h>
#include <string.h>
#include <sys/fault.h>
#include "kdebug.h"

static int	single_step;


void
gdb_prep_reboot() {
	//Nothing to do
}


uintptr_t
gdb_location(CPU_REGISTERS *ctx) {
	return ctx->iar;
}


paddr_t
gdb_image_base(paddr_t base) {
	return base;
}

/*
 * gdb_show_exception_info:
 * Print out some information on the exception taken
 */
void
gdb_show_exception_info(ulong_t signal, CPU_REGISTERS *ctx) {
    gdb_printf("signal=%d, msr=%x, r1=%x, iar=%x\n",
		signal, ctx->msr, ctx->gpr[1], ctx->iar );
}

/*
 * gdb_get_cpuregs:
 * Write the cpu registers into the buffer, and return them 
 * to our gdb client
 */
void
generic_gdb_get_cpuregs(CPU_REGISTERS *ctx, FPU_REGISTERS *ftx) {
    gdb_register_context buf;

#ifdef DEBUG_GDB
	kprintf("get_cpuregs: iar = %x\n", ctx->iar);
#endif
	memcpy(buf.gpr, ctx->gpr, sizeof(buf.gpr));
	memset(buf.fpr, 0, sizeof(buf.fpr));
	buf.pc = ctx->iar;
	buf.ps = ctx->msr;
	buf.cnd = ctx->cr;
	buf.lr = ctx->lr;
	buf.cnt = ctx->ctr;
	buf.xer = ctx->xer;
	buf.mq = ctx->u.mq;

    mem2hex((char*)&buf, outbuf, sizeof(buf));
}

void
protocol1_gdb_get_cpuregs(CPU_REGISTERS *ctx, FPU_REGISTERS *ftx) {
    protocol1_gdb_register_context buf;

#ifdef DEBUG_GDB
	kprintf("protocol1_get_cpuregs: iar = %x\n", ctx->iar);
#endif
	buf.cpu_type = 0;
	memcpy(buf.gpr, ctx->gpr, sizeof(buf.gpr));
	buf.pc = ctx->iar;
	buf.ps = ctx->msr;
	buf.cnd = ctx->cr;
	buf.lr = ctx->lr;
	buf.cnt = ctx->ctr;
	buf.xer = ctx->xer;
	buf.mq = ctx->u.mq;

    mem2hex((char*)&buf, outbuf, sizeof(buf));
}

void
gdb_get_cpuregs(CPU_REGISTERS *ctx, FPU_REGISTERS *ftx) {
	if ( protocol == 0 ) {
		generic_gdb_get_cpuregs(ctx,ftx);
	} else {
		/* protocol1 gdb has a different register context structure,
		 * in that it has a CPU type as the first field, then
		 * the generic regs, and then cpu type specific regs.
		 * We support only the generic regs.
		 */
		protocol1_gdb_get_cpuregs(ctx,ftx);
	}
}

/*
 * gdb_set_cpuregs:
 * Read the cpu registers from the buffer given to us by
 * our gdb client
 */
void
generic_gdb_set_cpuregs(CPU_REGISTERS *ctx, FPU_REGISTERS *ftx) {
    gdb_register_context buf;

    hex2mem(&inbuf[1], (char *)&buf, sizeof(buf));

	memcpy(ctx->gpr, buf.gpr, sizeof(ctx->gpr));
	ctx->iar = buf.pc;
	ctx->msr = buf.ps;
	ctx->cr = buf.cnd;
	ctx->lr = buf.lr;
	ctx->ctr = buf.cnt;
	ctx->xer = buf.xer;
	ctx->u.mq = buf.mq;
#ifdef DEBUG_GDB
	kprintf("set_cpuregs: iar = %x\n", ctx->iar);
#endif
}

void
protocol1_gdb_set_cpuregs(CPU_REGISTERS *ctx, FPU_REGISTERS *ftx) {
    protocol1_gdb_register_context buf;

    hex2mem(&inbuf[1], (char *)&buf, sizeof(buf));

	memcpy(ctx->gpr, buf.gpr, sizeof(ctx->gpr));
	ctx->iar = buf.pc;
	ctx->msr = buf.ps;
	ctx->cr = buf.cnd;
	ctx->lr = buf.lr;
	ctx->ctr = buf.cnt;
	ctx->xer = buf.xer;
	ctx->u.mq = buf.mq;
#ifdef DEBUG_GDB
	kprintf("set_cpuregs: iar = %x\n", ctx->iar);
#endif
}

void
gdb_set_cpuregs(CPU_REGISTERS *ctx, FPU_REGISTERS *ftx) {
	if ( protocol == 0 ) {
		generic_gdb_set_cpuregs(ctx,ftx);
	} else {
		/* Protocol1 gdb has a different register context structure,
		 * in that it has a CPU type as the first field, then
		 * the generic regs, and then cpu type specific regs.
		 * We support only the generic regs.
		 */
		protocol1_gdb_set_cpuregs(ctx,ftx);
	}
}


/*
 * gdb_proc_continue:
 * Resume execution at an optional address
 */
void
gdb_proc_continue(CPU_REGISTERS *ctx, int step) {
    /* 
     * try to read optional parameter, addr unchanged if no parm
     */
    parsehexnum(&inbuf[1], (int *)&ctx->iar);

	single_step = step;

#ifdef DEBUG_GDB
    if(step) kprintf("Step ");
	kprintf("Continue\n");
#endif
}

/*
 * ulong_t handle_exception(ulong_t sigcode, CPU_REGISTERS *ctx)
 *
 * This function is called whenever an exception occurs. The front end
 * has saved the cpu context in stack and is passed to me in ctx.
 * Interrupts have been disabled.
 */

ulong_t
handle_exception(struct kdebug_entry *entry, ulong_t sigcode, CPU_REGISTERS *ctx) {
#ifdef DEBUG_GDB
    kprintf("Enter KDB exception: Sigcode = %x, IP=%x, MSR=%x\n",
				(unsigned)sigcode, ctx->iar, ctx->msr);
#endif

	if(!is_watch_entry(entry, ctx->iar) && !WANT_FAULT(sigcode)) {
#ifdef DEBUG_GDB
		kprintf("Leave KDB exception: return %x\n", (unsigned)sigcode);
#endif
		return sigcode;
    }

	family_stuff(FAM_DBG_ENTRY, ctx);

	restore_break();
    /*
     * Goto GDB command interpreter.
     */
    gdb_interface(entry, ctx, sigcode);

	family_stuff(single_step ? FAM_DBG_EXIT_STEP : FAM_DBG_EXIT_CONTINUE, ctx);

#ifdef DEBUG_GDB
    kprintf("Leave KDB exception: return 0 (iar=%x, msr=%x)\n", ctx->iar, ctx->msr);
#endif

    return 0;
}


int
cpu_handle_alignment(void *p, unsigned size) {
	switch(SYSPAGE_CPU_ENTRY(ppc,kerinfo)->ppc_family) {
	case PPC_FAMILY_600:
		return 1;
	default:
		break;
	}

	return ((uintptr_t)p & (size-1)) == 0;
}
