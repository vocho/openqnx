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
#include <ppc/opcode.h>

#define	IT_OFFSET(code)	( ((code) & _ONEBIT32B(6))? 					\
		((code) & _BITFIELD32B(29, 0xffffff)) | _BITFIELD32B(5, 0x3f):	\
		(code) & _BITFIELD32B(29, 0xffffff) )

#define	BT_OFFSET(code)	( ((code) & _ONEBIT32B(6))? 					\
		((code) & _BITFIELD32B(29, 0x3fff)) | _BITFIELD32B(15, 0xffff):  \
		(code) & _BITFIELD32B(29, 0x3fff) )

static int go_branch(CPU_REGISTERS *ctx , int bo, int bi) {
int ctr = ctx->ctr - (bo & 0x4)?0:1;
int ctr_ok = (bo & 0x4) || ( (ctr==0) == ((bo & 0x2) != 0) );
int cond_ok = (bo & 0x10) || ( ((ctx->cr & (0x80000000>>bi)) != 0) == ((bo & 0x8) != 0) );

//kprintf("\ngo_branch: bo=%x,bi=%x,cr=%x,ctr_ok = %x,cond_ok = %x\n", bo,bi,ctx->cr,ctr_ok,cond_ok);

return (ctr_ok && cond_ok);

}

/*
 * next_instruction:
 *	Find out the address of next instructions.
 *	Four types of PPC instructions may change the program flow, they are:
 *		bx, bcx, bcctrx and bclrx.
 */
uintptr_t
next_instruction(CPU_REGISTERS *ctx) {
	union ppc_instr *op;
	uint16_t 		opcode1, opcode2;
	uintptr_t 		ret;
	paddr_t			paddr;

	/* @@@ This will not work for 64 bit paddrs */
	if(memmgr.vaddrinfo(aspaces_prp[KERNCPU], ctx->iar, &paddr, NULL, VI_PGTBL) == PROT_NONE) return -(uintptr_t)1;
	op = (void *)(uint32_t)paddr;

	ret = ctx->iar + 4;
	opcode1 = op->d_t.op;

//kprintf("\nnext_instruction: op = %x, original ret = %x\n", op->op_code, ret);

	switch(opcode1) {
	case PPCOPCODE_BC:
		if(go_branch(ctx, op->b_t.bo, op->b_t.bi))
			ret = (uint32_t)BT_OFFSET(op->op_code) + (uint32_t)((op->b_t.aa)?0:ctx->iar);
		break;
	case PPCOPCODE_B:
//kprintf("\nnext_instruction: it_off = %x, base = %x\n", IT_OFFSET(op->op_code), (op->i_t.aa)?0:ctx->iar);
			ret = (uint32_t)(IT_OFFSET(op->op_code)) + (uint32_t)((op->i_t.aa)?0:ctx->iar);
		break;
	case PPCOPCODE_XLFORM:
		opcode2 = op->xl_t.xo;
		switch(opcode2) {
		case PPCXOCODE_BCLR:
			if(go_branch(ctx, op->xl_t.bo_d, op->xl_t.bi_a))
				ret = (uint32_t) ctx->lr & _BITFIELD32B(29, 0x3fffffff);
			break;
		case PPCXOCODE_BCCTR:
			if(go_branch(ctx, op->xl_t.bo_d, op->xl_t.bi_a))
				ret = (uint32_t) (ctx->ctr-(op->xl_t.bo_d&0x4)?0:1) & _BITFIELD32B(29, 0x3fffffff);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	};

//kprintf("\nnext_instruction: ret = %x\n", ret);

	return ret;
}


__SRCVERSION("next_instruction.c $Rev: 153052 $");
