//#define DEBUG_GDB
#include <signal.h>
#include <stdarg.h>
#include <string.h>
#include <sys/fault.h>
#include "sh/opcodes.h"
#include "kdebug.h"

static uintptr_t
branch_predict(CPU_REGISTERS *ctx) {
union sh_instr *op;
int disp, delay = 0;
ulong_t pc, pc_valid = 0;

	op = (union sh_instr *)ctx->pc;
	pc = ctx->pc;

#ifdef DEBUG_GDB
	kprintf("type %x sr %x\n", op->i_d12.op,ctx->sr);
#endif
	switch( op->i_d12.op ) {
	case OPCODE_B1:
#ifdef DEBUG_GDB
		kprintf("type d8: op=%hx, disp=%x\n", op->i_d8.op, op->i_d8.disp );
#endif
		if ( (op->i_d8.disp & 0x80) == 0 )
			disp = (0x000000ff & op->i_d8.disp);
		else
			disp = (0xffffff00 | op->i_d8.disp);
		switch( op->i_d8.op ) {
		case OPCODE_BF_S:
			delay = 2;
		case OPCODE_BF:
			if ( !(ctx->sr & 0x00000001) )
				pc = pc+4+(disp<<1);
			else
				pc+= 2 + delay;
			pc_valid = 1;
			break;
		case OPCODE_BT_S:
			delay = 2;
		case OPCODE_BT:
			if ( (ctx->sr & 0x00000001) )
				pc = pc+4+(disp<<1);
			else
				pc+= 2 + delay;
			pc_valid = 1;
			break;
		}
		break;
	case OPCODE_BRA:
	case OPCODE_BSR:
#ifdef DEBUG_GDB
		kprintf("type d12: op=%hx, disp=%x\n", op->i_d12.op, op->i_d12.disp );
#endif
		if ( (op->i_d12.disp & 0x800) == 0 )
			disp = (0x00000fff & op->i_d12.disp);
		else
			disp = (0xfffff000 | op->i_d12.disp);
		pc = pc + 4 + (disp<<1);
		pc_valid = 1;
		break;
	case OPCODE_B2:
	case OPCODE_J:
		if ( op->op_code == OPCODE_RTS ) {
			pc = ctx->pr;
			pc_valid = 1;
			break;
		}
		switch( op->i_f.func ) {
		case OPCODE_BRAF:
		case OPCODE_BSRF:
#ifdef DEBUG_GDB
			kprintf("branch to pc + 4 + r%d = %08x\n", op->i_f.reg, ctx->gr[op->i_f.reg]);
#endif
			pc = pc + 4 + ctx->gr[op->i_f.reg];
			pc_valid = 1;
			break;
		case OPCODE_JMP:
		case OPCODE_JSR:
#ifdef DEBUG_GDB
			kprintf("Jump to r%d = %08x\n", op->i_f.reg, ctx->gr[op->i_f.reg]);
#endif
			pc = ctx->gr[op->i_f.reg];
			pc_valid = 1;
			break;
		}
		break;
	}
	if ( pc_valid == 0 )
		pc += 2;
#ifdef DEBUG_GDB
	kprintf("op = %04hx, pc = %08x, target pc = %08x\n", 
		op->op_code, ctx->pc, pc );
#endif
	return pc;
}

void
gdb_prep_reboot() {
	//NIY
}


uintptr_t
gdb_location(CPU_REGISTERS *ctx) {
	return( ctx->pc );
}


paddr_t
gdb_image_base(paddr_t base) {
	return(SH_PHYS_TO_P1(base));
}

/*
 * gdb_show_exception_info:
 * Print out some information on the exception taken
 */
void
gdb_show_exception_info(ulong_t signal, CPU_REGISTERS *ctx) {
    gdb_printf("signal=%d, sr=%x, r15=%x, pc=%x\n",
		signal, ctx->sr, ctx->gr[15], ctx->pc );
}

/*
 * gdb_get_cpuregs:
 * Write the cpu registers into the buffer, and return them 
 * to our gdb client
 */
void
gdb_get_cpuregs(CPU_REGISTERS *ctx, FPU_REGISTERS *ftx) {
    gdb_register_context buf;

#ifdef DEBUG_GDB
	kprintf("get_cpuregs: pc = %x\n", ctx->pc);
#endif
	memcpy( buf.gr, ctx->gr, sizeof( buf.gr ) );
	if(ftx != NULL) {	
		buf.fpul = ftx->fpul;
		buf.fpscr = ftx->fpscr;
		if(ftx->fpscr & SH_FPSCR_FR)
			memcpy( buf.fpr_bank0, ftx->fpr_bank1, sizeof( buf.fpr_bank0 ) );
		else
			memcpy( buf.fpr_bank0, ftx->fpr_bank0, sizeof( buf.fpr_bank0 ) );
	} else {
		buf.fpul = 0;
		buf.fpscr = 0;
		memset( buf.fpr_bank0, 0, sizeof( buf.fpr_bank0 ) );
	}
		buf.pc = ctx->pc;
		buf.sr = ctx->sr;

//	buf.dbr = ctx->dbr;
	buf.vbr = (shint)_syspage_ptr->un.sh.exceptptr;
	buf.ssr = buf.sr; /* heh :) */
	buf.spc = buf.spc;

	buf.gbr = ctx->gbr;
	buf.mach = ctx->mach;
	buf.macl = ctx->macl;
	buf.pr = ctx->pr;

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

	memcpy( ctx->gr, buf.gr, sizeof( ctx->gr ) );
	if(ftx != NULL) {	
		ftx->fpul = buf.fpul;
		ftx->fpscr = buf.fpscr;
		if(ftx->fpscr & SH_FPSCR_FR)
			memcpy( ftx->fpr_bank1, buf.fpr_bank0, sizeof( buf.fpr_bank0 ) );
		else
			memcpy( ftx->fpr_bank0, buf.fpr_bank0, sizeof( buf.fpr_bank0 ) );
	}

	ctx->pc = buf.pc;
	ctx->sr = buf.sr;
//	ctx->dbr = buf.dbr;
	ctx->gbr = buf.gbr;
	ctx->mach = buf.mach;
	ctx->macl = buf.macl;
	ctx->pr = buf.pr;
	_syspage_ptr->un.sh.exceptptr = (uint32_t *)buf.vbr;
#ifdef DEBUG_GDB
	kprintf("set_cpuregs: pc = %x\n", ctx->pc);
#endif
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
    parsehexnum(&inbuf[1], (int *)&ctx->pc);

    if (step) {
		addr = branch_predict(ctx);
#ifdef DEBUG_GDB
		kprintf("Step. %x\n",addr);
#endif
		set_break(addr);
    }
#ifdef DEBUG_GDB
    else
		kprintf("Continue\n");
#endif
}

/*
 * ulong_t handle_exception(ulong_t sigcode, CPU_REGISTERS *ctx)
 *
 * This function is called whenever an exception occurs. The front end
 * has saved the cpu context in stack and is passed to me in ctx.
 * ERL/EXL have been de-asserted and interrupts have been disabled.
 */

ulong_t
handle_exception(struct kdebug_entry *entry, ulong_t sigcode, CPU_REGISTERS *ctx) {
#ifdef DEBUG_GDB
    kprintf("Enter KDB exception: Sigcode = %x, IP=%x\n", sigcode, ctx->pc);
    kprintf("ctx=%x,gr0=%x, SP=%x, pc=%x, SR=%x\n", ctx, ctx->gr[0], ctx->gr[15], ctx->pc, ctx->sr);
#endif

    if(!is_watch_entry(entry, ctx->pc) && !WANT_FAULT(sigcode)) {
#ifdef DEBUG_GDB
		kprintf("Leave KDB exception: return %x\n", sigcode);
#endif
		return sigcode;
    }


	restore_break();
    /*
     * Goto GDB command interpreter.
     */
    gdb_interface(entry, ctx, sigcode);

#ifdef DEBUG_GDB
    kprintf("Leave KDB exception: return 0 (iar=%x)\n",ctx->pc);
#endif

    return (0);
}


int
cpu_handle_alignment(void *p, unsigned size) {
	return ((uintptr_t)p & (size-1)) == 0;
}
