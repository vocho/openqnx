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

#include "externs.h"
#include <mips/opcode.h>

#if defined(VARIANT_r3k)
	#define VERIFY_VADDR(ctx, vaddr, fail_lbl) /*lint -save -e568 */	\
		if(!MIPS_IS_KUSEG((uintptr_t)(vaddr))) {							\
			/* not user mode address */										\
			if(((ctx)->regs[MIPS_CREG(MIPS_REG_SREG)] & MIPS3K_SREG_KUp)) { \
				goto fail_lbl;												\
			}																\
		/*lint -restore */	\
		}
#else 
	#define VERIFY_VADDR(ctx, vaddr, fail_lbl) /*lint -save -e568 */	\
		if(!MIPS_IS_KUSEG((uintptr_t)(vaddr))) {							\
			/* not user mode address */										\
			switch((ctx)->regs[MIPS_CREG(MIPS_REG_SREG)] & MIPS_SREG_KSU) {	\
			case MIPS_SREG_MODE_KERNEL:										\
				break;														\
			case MIPS_SREG_MODE_SUPER:										\
				if(MIPS_IS_KSSEG((uintptr_t)(vaddr))) break;				\
				/* fall through */											\
			default:														\
				/* address space error */									\
				goto fail_lbl;												\
			}																\
		/*lint -restore */	\
		}
#endif

#define MIPS_R4K_HI_ADDR_MASK	0xF0000000

/*
 * r4k_get_cpu_reg:
 *
 * Return a register from the context
 */
static unsigned long
r4k_get_cpu_reg(CPU_REGISTERS *ctx, int index) {
    /*
     * Make sure zero is 0. It's used during next_instruction
     * but not saved by kernel as part of context.
     */
	if(index == 0) return(0);
	return(ctx->regs[MIPS_CREG(index)]);
}

/*
 * r4k_set_cpu_reg:
 *
 * Set a register in the context
 */
static void
r4k_set_cpu_reg(CPU_REGISTERS *ctx, int index, unsigned long value) {
    /*
     * Make sure zero is 0. It's used during next_instruction
     * but not saved by kernel as part of context.
     */
	if(index != 0) MIPS_REG_SETx64(ctx, index, value);
}

/*
 * r4k_memref:
 *
 * Return the address being referenced by a load/store style instruction
 */
static uintptr_t
r4k_memref(CPU_REGISTERS *ctx, union r4k_instr *op) {
	return((r4k_get_cpu_reg(ctx, op->i_t.rs) + op->i_t.s_imd));
}

/*
 * next_instruction:
 *
 * If EPC register points to a jump instruction, return
 * the destination PC of the jump. Note, even if the original
 * intruction that caused the exception was in the branch delay slot,
 * the EPC register will point to the branch instruction rather than
 * the branch delay instruction
 */
uintptr_t
next_instruction(CPU_REGISTERS *ctx) {
    union r4k_instr *op;
    uintptr_t epc = ctx->regs[MIPS_CREG(MIPS_REG_EPC)];

    op = (union r4k_instr *)epc;

    switch(op->i_t.op) {
    case OPCODE_SPECIAL:
		switch(op->r_t.func) {
		case OPCODE_JALR:
		case OPCODE_JR:
			return(r4k_get_cpu_reg(ctx, op->r_t.rs));
		default:
			return(epc + 4);
		}
		break;
    case OPCODE_REGIMM:
		switch(op->r_t.rt) {
		case OPCODE_BGEZAL:
		case OPCODE_BGEZALL:
		case OPCODE_BGEZ:
		case OPCODE_BGEZL:
			if((int)r4k_get_cpu_reg(ctx, op->i_t.rs) < 0)
				return(epc + 8);
			else
				return(epc + (4 + (op->i_t.s_imd << 2)));
		case OPCODE_BLTZAL:
		case OPCODE_BLTZALL:
		case OPCODE_BLTZ:
		case OPCODE_BLTZL:
			if((int)r4k_get_cpu_reg(ctx, op->i_t.rs) >= 0) 
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
		if(r4k_get_cpu_reg(ctx, op->i_t.rs) != r4k_get_cpu_reg(ctx, op->i_t.rt))
			return(epc + 8);
		else
			return(epc + (4 + (op->i_t.s_imd << 2)));
    case OPCODE_BGTZ:
    case OPCODE_BGTZL:
		if((int)r4k_get_cpu_reg(ctx, op->i_t.rs) <= 0)
			return(epc + 8);
		else
			return(epc + (4 + (op->i_t.s_imd << 2)));
    case OPCODE_BLEZ:
    case OPCODE_BLEZL:
		if((int)r4k_get_cpu_reg(ctx, op->i_t.rs) > 0)
			return(epc + 8);
		else
			return(epc + (4 + (op->i_t.s_imd << 2)));
    case OPCODE_BNE:
    case OPCODE_BNEL:
		if(r4k_get_cpu_reg(ctx, op->i_t.rs) == r4k_get_cpu_reg(ctx, op->i_t.rt))
			return(epc + 8);
		else
			return(epc + (4 + (op->i_t.s_imd << 2)));
    default: break;
	}
	return(epc + 4);
}

static void
touch(void *source, unsigned len) {
	volatile uint8_t	*p = source;
	unsigned			dummy;

	/* Make sure the memory is accessable */
	while(len != 0) {
		dummy = *p++;
		--len;
	}
}

static void
load(void *target, void *source, uint32_t len, uint32_t f_sign) {
#if defined(__BIGENDIAN__)
	if(f_sign && (0x80 & *(uint8_t*)source) ) {
		// sign negtive
		memset(target, 0xff, 8 - len);
	} else {
		// unsign load or sign positive
		memset(target, 0, 8 - len);
	}
	memcpy((uint8_t *)target + (8  - len), source, len);
#elif defined(__LITTLEENDIAN__)
	if(f_sign && (0x80 & *(uint8_t*)((uintptr_t)source+len-1)) ) {
		// sign negtive
		memset((uint8_t *)target + len, 0xff, 8 - len);
	} else {
		// unsign load or sign positive
		memset((uint8_t *)target + len, 0, 8 - len);
	}
	memcpy(target, source, len);
#else
	#error ENDIAN Not defined for system
#endif
}

static void
store(void *target, void *source, uint32_t len) {

#if defined(__BIGENDIAN__)
	memcpy(target, (uint8_t *)source + (8 - len), len);
#elif defined(__LITTLEENDIAN__)
	memcpy(target, source, len);
#else
	#error ENDIAN Not defined for system
#endif
}

uintptr_t 		load_linked_addr = ~0;
static THREAD	*load_linked_thread;

struct emu_stack {
	struct emu_stack			*prev;
	uintptr_t					epc;
	unsigned					intr_nest;
	const struct fault_handlers *handler;
};

static struct emu_stack	* volatile emu_stack;

static void
emu_fault(THREAD *thp, CPU_REGISTERS *regs, unsigned sigcode) {
	load_linked_addr = ~0;
	emu_stack = NULL;
	usr_fault(sigcode, actives[KERNCPU], CP0REG_GET(8));
	__ker_exit();
}

static void
emu_preempt(THREAD *thp, CPU_REGISTERS *regs) {
	struct emu_stack	*curr = emu_stack;
	const struct fault_handlers	*handler;

	load_linked_addr = ~0;
	emu_stack = curr->prev;
	handler = curr->handler;
	if((handler != NULL) && (handler->restart != NULL)) {
		handler->restart(thp, regs);
	}
}

const struct fault_handlers emulate_fault_handlers = {
	emu_fault, emu_preempt
};

/*
 * emulate_instruction:
 *
 * Emulate unimplemented instructions that silly chip designers left
 * out.
 * Also, taking care of memory alignment exception.
 */
unsigned
emulate_instruction(CPU_REGISTERS *ctx, unsigned cause, int intrs_enabled) {
    union r4k_instr 	*op;
	uintptr_t			link_addr;
	uintptr_t			next_epc;
	int					success;
	void				*v_badvaddr_reg = (void *)ctx->regs[MIPS_CREG(MIPS_REG_BADVADDR)];
	void				*a_reg;
	unsigned			sigcode;
	struct emu_stack	curr;
	struct emu_stack	*check;

	sigcode = __mips_cause2sig_map[(cause & MIPS_CAUSE_MASK) >> MIPS_CAUSE_SHIFT];

	curr.epc = ctx->regs[MIPS_CREG(MIPS_REG_EPC)];
	// adjust addr of curr instruction
	if(cause & MIPS_CAUSE_BD_SLOT) {
		curr.epc += sizeof(union r4k_instr);
	}
	curr.intr_nest = (get_inkernel() & INKERNEL_INTRMASK);
	for(check = emu_stack; check != NULL; check = check->prev) {
		if((check->epc == curr.epc) && (check->intr_nest == curr.intr_nest)) {
			//Don't attempt to recursively emulate instruction
			return(sigcode);
		}
	}

	curr.handler = GET_XFER_HANDLER();
	curr.prev = emu_stack;
	InterruptDisable();
	emu_stack = &curr;
	SET_XFER_HANDLER(&emulate_fault_handlers);
	if(intrs_enabled) InterruptEnable();

	op = (union r4k_instr *)curr.epc;

	// switch on the cause of exception
	switch(cause & MIPS_CAUSE_MASK) {
	case MIPS_CAUSE_ADDR_LOAD << MIPS_CAUSE_SHIFT:
		// load or instruction fetch exc
		if(op == v_badvaddr_reg)  {
			// Instruction fetch error
			goto fail_instr;
		}
		// fall through 
	case MIPS_CAUSE_ADDR_SAVE << MIPS_CAUSE_SHIFT:
		// is address space error
		VERIFY_VADDR(ctx, v_badvaddr_reg, fail_instr);

		// aligment error
		a_reg = (ctx->regs + MIPS_BAREG(op->i_t.rt));
		switch(op->i_t.op) {
		case	OPCODE_LHU:
			touch(v_badvaddr_reg, 2);
			lock_kernel();
			load(a_reg, v_badvaddr_reg, 2, 0);
			break;
		case	OPCODE_LH:
			touch(v_badvaddr_reg, 2);
			lock_kernel();
			load(a_reg, v_badvaddr_reg, 2, 1);
			break;
		case	OPCODE_LW:
			touch(v_badvaddr_reg, 4);
			lock_kernel();
			load(a_reg, v_badvaddr_reg, 4, 1);
			break;
		case	OPCODE_LWU:
			touch(v_badvaddr_reg, 4);
			lock_kernel();
			load(a_reg, v_badvaddr_reg, 4, 0);
			break;
		case	OPCODE_LD:
			touch(v_badvaddr_reg, 8);
			lock_kernel();
			memcpy(a_reg, v_badvaddr_reg, 8);
			break;
		case	OPCODE_SH:
			store(v_badvaddr_reg, a_reg, 2);
			break;
		case	OPCODE_SW:
			store(v_badvaddr_reg, a_reg, 4);
			break;
		case	OPCODE_SD:
			memcpy(v_badvaddr_reg, a_reg, 8);
			break;
		default:
			goto fail_instr;
		}
		
		break;

#if defined(VARIANT_r3k)
	// Workaround for TX39 bug.
	case MIPS_CAUSE_CP_UNUSABLE << MIPS_CAUSE_SHIFT:
		//fall through
#endif
	case MIPS_CAUSE_ILLOP << MIPS_CAUSE_SHIFT:
		// emulation
		switch(op->i_t.op) {
#if defined(VARIANT_r3k)
		// R3K's don't support these instructions at all, but the
		// compiler generates them and we want the FP emulator
		// to handle them. Change the signal code so that the code
		// in kernel.S knows what to do.
		case OPCODE_LWC1:
		case OPCODE_SWC1:
		case OPCODE_LDC1:
		case OPCODE_SDC1:
    		sigcode = MAKE_SIGCODE(SIGILL,ILL_PRVOPC,FLTPRIV);
    		goto fail_instr;
#endif
		case OPCODE_LL:
			link_addr = r4k_memref(ctx, op);
		
			VERIFY_VADDR(ctx, link_addr, fail_instr);
			
			InterruptDisable();
			load_linked_addr = link_addr;
			load_linked_thread = actives[0];
			r4k_set_cpu_reg(ctx, op->i_t.rt, *(uint32_t *)link_addr);
			//Can delay lock_kernel() since we've got interrupts disabled.
			//Required so that we don't hardcrash() on a bad address.
			lock_kernel();
			if(intrs_enabled) InterruptEnable();
			break;
		case OPCODE_SC:
			success = 0;
			link_addr = r4k_memref(ctx, op);

			VERIFY_VADDR(ctx, link_addr, fail_instr);

			InterruptDisable();
			if((link_addr == load_linked_addr) && (load_linked_thread == actives[0])) {
				*(uint32_t *)link_addr = r4k_get_cpu_reg(ctx, op->i_t.rt);
				success = 1;
			}
			lock_kernel();
			r4k_set_cpu_reg(ctx, op->i_t.rt, success);
			load_linked_addr = ~0;
			if(intrs_enabled) InterruptEnable();
			break;
		default:
			goto fail_instr;
		}
		break;

	default:
		// others
		goto fail_instr;
	}
	
	next_epc = next_instruction(ctx);
	if(next_epc == curr.epc) next_epc += sizeof(union r4k_instr);
	lock_kernel();
	r4k_set_cpu_reg(ctx, MIPS_REG_EPC, next_epc);

	sigcode = 0; // successfully emulated instruction

fail_instr:
	InterruptDisable();
	emu_stack = curr.prev;
	SET_XFER_HANDLER(curr.handler);

	if(intrs_enabled) InterruptEnable();
	return(sigcode);
}

__SRCVERSION("emulate.c $Rev: 160636 $");
