//#define DEBUG_GDB
#include <signal.h>
#include <stdarg.h>
#include <string.h>
#include <sys/fault.h>
#include "kdebug.h"
#include "mmu.h"

void
machine_single_step(CPU_REGISTERS *reg ) {
	reg->efl |= X86_PSW_TF;
}

/*
 * gdb_prep_reboot
 *
 * Prepare to reboot the system
 */

void
gdb_prep_reboot(void) {
}

uintptr_t
gdb_location(CPU_REGISTERS *ctx) {
	return ctx->eip;
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
    gdb_printf("signal=%d, eip=%x\n", signal, ctx->eip);
}

/*
 * gdb_get_cpuregs:
 * Write the cpu registers into the buffer, and return them 
 * to our gdb client
 */
void
gdb_get_cpuregs(CPU_REGISTERS *ctx, FPU_REGISTERS *ftx) {
    gdb_register_context buf;

	buf.eax = ctx->eax;
	buf.ebx = ctx->ebx;
	buf.ecx = ctx->ecx;
	buf.edx = ctx->edx;
	buf.esi = ctx->esi;
	buf.edi = ctx->edi;
	buf.ebp = ctx->ebp;
	buf.uesp = ctx->esp;
	buf.efl = ctx->efl;
	buf.eip = ctx->eip;
	buf.cs = ctx->cs;
	buf.ds = __ds;
	buf.ss = ctx->ss;
	buf.es = __es;
	buf.fs = __fs;
	buf.gs = __gs;
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

    hex2mem(&inbuf[1], (char *)&buf, sizeof(buf));

	ctx->eax = buf.eax;
	ctx->ebx = buf.ebx;
	ctx->ecx = buf.ecx;
	ctx->edx = buf.edx;
	ctx->esi = buf.esi;
	ctx->edi = buf.edi;
	ctx->ebp = buf.ebp;
	ctx->esp = buf.uesp;
	ctx->efl = buf.efl;
	ctx->eip = buf.eip;
	ctx->cs = buf.cs;
	__ds = buf.ds;
	ctx->ss = buf.ss;
	__es = buf.es;
	__fs = buf.fs;
	__gs = buf.gs;
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
    parsehexnum(&inbuf[1], (int *)&ctx->eip);

    if (step) {
		machine_single_step(ctx);
    }
#ifdef DEBUG_GDB
	kprintf("%s: %x\n", step ? "Step" : "Continue", ctx->eip);
#endif
}

/*
 * ulong_t handle_exception(ulong_t sigcode, CPU_REGISTERS *ctx)
 *
 * This function is called whenever an exception occurs. The front end
 * has saved the cpu context in stack and is passed to me in ctx.
 * ERL/EXL have been de-asserted and interrupts have been disabled.
 */

ulong_t handle_exception(ulong_t sigcode, CPU_REGISTERS *ctx, struct kdebug_entry *entry) {
#ifdef DEBUG_GDB
    kprintf("Enter KDB exception: Sigcode = %x (EIP=%x)\n", (unsigned)sigcode, ctx->eip);
#endif

	if(!is_watch_entry(entry, ctx->eip) && !WANT_FAULT(sigcode)) {
#ifdef DEBUG_GDB
		kprintf("Skip KDB exception: return %x\n", (unsigned)sigcode);
#endif
		return sigcode;
    }

	ctx->efl &= ~X86_PSW_TF;
	restore_break();
    /*
     * Goto GDB command interpreter.
     */
    gdb_interface(entry, ctx, sigcode);

#ifdef DEBUG_GDB
    kprintf("Leave KDB exception: return 0\n");
#endif

    return (0);
}

int
outside_watch_entry(struct kdebug_entry *entry, unsigned vaddr) {
	return watch_entry(entry, vaddr);
}

void
outside_msg_entry(const char *msg, unsigned len) {
	msg_entry(msg, len);
}

void
outside_update_plist(struct kdebug_entry *mod) {
}


int
cpu_handle_alignment(void *p, unsigned size) {
	return 1;
}
