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
#include <sh/opcodes.h>



/*
 * next_instruction:
 *	Find out the address of next instructions.
 */
uintptr_t
next_instruction(CPU_REGISTERS *ctx) {
	const union sh_instr  *op;
	union sh_instr	opcode;
	int				disp, delay = 0;
	ulong_t			pc, pc_valid = 0;
	paddr_t			phys;
	PROCESS*		prp = aspaces_prp[KERNCPU];

	if(memmgr.vaddrinfo(prp, ctx->pc, &phys, NULL, VI_PGTBL) == PROT_NONE) {
		return ~0U;
	}
	
	if ( SH_PHYS_IN_1TO1(phys) ) {
		// We can read the pointer through the 1TO1 area
		op = (union sh_instr *)PHYS_TO_PTR(phys);
	} else {
		// The pointer isn't visible through the 1TO1 area, so we need
		// to make sure it's mapped into this address space.
		// Run with interrupts disabled to ensure that the TLB doesn't 
		// get swapped out before we get a chance to use it.  Map it 
		// in as a 4K page
		unsigned ptel = (phys & (~(__PAGESIZE-1))) | SH_CCN_PTEL_SZ0 | SH_CCN_PTEL_V | SH_CCN_PTEL_PR(2);
		uintptr_t vaddr = ctx->pc;
		
		InterruptDisable();
		
		// Ensure there aren't any mappings for vaddr already in the TLBs
		tlb_flush_va( prp->memory, vaddr, vaddr+2 );

		// Add the new map.
		sh4_update_tlb( vaddr & (~(__PAGESIZE-1)), ptel );
		
		// Read the code
		opcode = *((union sh_instr *)vaddr);
		op = &opcode;
		
		// Clear the TLB just to be safe.
		tlb_flush_va( prp->memory, vaddr, vaddr+2 );
		
		InterruptEnable();
		
	}
	pc = ctx->pc;

#ifdef DEBUG_GDB
	kprintf("type %x sr %x\n", op->i_d12.op,ctx->sr);
#endif
	switch( op->i_d12.op ) {
	case OPCODE_B1:
#ifdef DEBUG_GDB
		kprintf("type d8: op=%hx, disp=%x\n", op->i_d8.op, op->i_d8.disp );
#endif
		if ( (op->i_d8.disp & 0x80) == 0 ) {
			disp = (0x000000ff & op->i_d8.disp);
		} else {
			disp = (0xffffff00 | op->i_d8.disp);
		}
		switch( op->i_d8.op ) {
		case OPCODE_BF_S:
			delay = 2;
			/* fall through */
		case OPCODE_BF:
			if ( !(ctx->sr & 0x00000001) ) {
				pc = pc+4+(disp<<1);
			} else {
				pc+= 2 + delay;
			}
			pc_valid = 1;
			break;
		case OPCODE_BT_S:
			delay = 2;
			/* fall through */
		case OPCODE_BT:
			if ( (ctx->sr & 0x00000001) ) {
				pc = pc+4+(disp<<1);
			} else {
				pc+= 2 + delay;
			}
			pc_valid = 1;
			break;
		default: break;
		}
		break;
	case OPCODE_BRA:
	case OPCODE_BSR:
#ifdef DEBUG_GDB
		kprintf("type d12: op=%hx, disp=%x\n", op->i_d12.op, op->i_d12.disp );
#endif
		if ( (op->i_d12.disp & 0x800) == 0 ) {
			disp = (0x00000fff & op->i_d12.disp);
		} else {
			disp = (0xfffff000 | op->i_d12.disp);
		}
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
		default: break;
		}
		break;
		default: break;
	}
	if ( pc_valid == 0 ) {
		pc += 2;
	}
#ifdef DEBUG_GDB
	kprintf("op = %04hx, pc = %08x, target pc = %08x\n", 
		op->op_code, ctx->pc, pc );
#endif
	return pc;
}

__SRCVERSION("next_instruction.c $Rev: 199396 $");
