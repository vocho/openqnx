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
#include <sh/cpu.h>

#define SH4_INT_OP1			0x2
#define SH4_INT_OP3			0x400e
#define SH4_INT_ENABLE			0x2009
#define SH4_INT_DISABLE			0xcbf0
#define SH4_INT_ENABLE_MASKED(x)	(x & ~SH_SR_IMASK)
#define SH4_INT_ENABLE_VALUE		~SH_SR_IMASK

uintptr_t
next_instruction(CPU_REGISTERS *ctx);

/*
 * Illegal instruction/Address Error exceptions have a higher priority
 * than Data TLB protection violation, so we can potentially encounter
 * a TLB protection violation in any user address accesses performed by
 * the emulation code. If this happens, deliver a signal to the thread.
 */
static void
emulate_fault(THREAD *thp, CPU_REGISTERS *regs, unsigned sig)
{
	usr_fault(sig, actives[KERNCPU], 0);
	__ker_exit();
}

static const struct fault_handlers emulate_fault_handlers = {
	emulate_fault, 0
};


/*
 * emulate_instruction:
 *
 * Emulate unimplemented instructions that silly chip designers left
 * out.
 *
 * SH unique: emulate interrupt handling when process has IO_PRIV.
 *
 * Called when illegal instruction exception and alignment emulation.
 *
 * Inputs:
 *	flags -- the exception code
 */
uint32_t
emulate_instruction(THREAD *thp, uint32_t flags) {
	CPU_REGISTERS 	*ctx = &thp->reg;
	union sh_instr 	inst;
	uint32_t		ret = flags;

	/*
	 * Set fault handler and unlock the kernel in case of a TLB fault
	 * Set INKERNEL_EXIT so we restart the faulting instruction on preemption.
	 */
	SET_XFER_HANDLER(&emulate_fault_handlers);
	bitset_inkernel(INKERNEL_EXIT);
	unlock_kernel();

	/* get the instruction in fault */
	inst.op_code = *(uint16_t *)ctx->pc;

	/* emulate privileged instructions for IO_PRIV processes */
	if( ((flags >> 16) == FLTPRIV) && (thp->flags & _NTO_TF_IOPRIV) ) {
		if(inst.i_f.op == OPCODE_STC) {
			/* stc */
			if(inst.i_f.func == FCODE_STC_SR_RN) {
				/* stc sr,Rn */
				ctx->gr[inst.i_f.reg] = ctx->sr;
				ret = 0;
			}
		} else if(inst.i_f.op == OPCODE_LDC) {
			/* ldc */
			if(inst.i_f.func == FCODE_LDC_RM_SR) {
				/* ldc Rm,sr */
				/* Not allowed to change privilege mode and BL bit  */
				ctx->sr = ctx->gr[inst.i_f.reg] & ~(SH_SR_BL | SH_SR_MD);
				ret = 0;
			}
		}
	}


	lock_kernel();
	if(ret == 0) {
		ctx->pc += 2;
	}

	SET_XFER_HANDLER(0);
	return ret;
}


/*
 * emulate_alignment:
 *
 * Taking care of memory alignment exception (data).
 *
 * Inputs:
 *	flags -- the exception code
 */
uint32_t
emulate_alignment(CPU_REGISTERS *ctx, uint32_t flags) {
	union sh_instr 	inst;
	uint32_t		ret = flags, next_pc;
	uint8_t			*addr;
	union	{
		uint32_t	i;
		struct	{
			uint32_t b0:8;
			uint32_t b1:8;
			uint32_t b2:8;
			uint32_t b3:8;
		} bytes;
	} val32;
	union	{
		uint16_t	i;
		struct	{
			uint16_t b0:8;
			uint16_t b1:8;
		} bytes;
	} val16;

	/*
	 * Set fault handler and unlock kernel in case of TLB fault.
	 * Set INKERNEL_EXIT so we restart the faulting instruction on preemption.
	 */
	SET_XFER_HANDLER(&emulate_fault_handlers);
	bitset_inkernel(INKERNEL_EXIT);
	unlock_kernel();

	next_pc = next_instruction(ctx);
	if(next_pc != (ctx->pc + 2)) {
		/* delayed slot */
		inst.op_code = *(uint16_t *)(ctx->pc + 2);
	} else {
		/* get the instruction in fault */
		inst.op_code = *(uint16_t *)ctx->pc;
	}

	/* no need to check memory boundry here. Boundry error has been catched before jump to this function. */
	if( ((flags & 0xff) == SIGBUS) && (((flags >> 8) & 0xff) == BUS_ADRALN) ) {
		/* make sure no instructions shared the same opcode */
		if(inst.i_d4.op == OPCODE_MOVW_R0_ADRN) {
			/* mov.w R0,@(disp,Rn) */
			val16.i = ctx->gr[0];
			addr = (uint8_t*) (ctx->gr[inst.i_d4.reg] + (inst.i_d4.disp << 1));
			*addr++ = val16.bytes.b0;
			*addr = val16.bytes.b1;
			ret = 0;
		}
		else if(inst.i_d4.op == OPCODE_MOVW_ADRM_R0) {
			/* mov.w @(disp,Rm),R0 */
			// sign extended
			addr = (uint8_t*)(ctx->gr[inst.i_d4.reg] + (inst.i_d4.disp << 1));
			val16.bytes.b0 = *addr++;
			val16.bytes.b1 = *addr;
			ret = 0;
			lock_kernel();
			ctx->gr[0] = val16.i | (val16.i & 0x8000) ? 0xffff0000:0;
		}
		else if(inst.i_d4r.op == OPCODE_MOVL_RM_ADRN) {
			/* mov.l Rm,@(disp,Rn) */
			val32.i = ctx->gr[inst.i_d4r.reg2];
			addr = (uint8_t*)(ctx->gr[inst.i_d4r.reg1] + (inst.i_d4r.disp << 2));
			ret = 0;
			*addr++ = val32.bytes.b0;
			*addr++ = val32.bytes.b1;
			*addr++ = val32.bytes.b2;
			*addr = val32.bytes.b3;
		}
		else if(inst.i_d4r.op == OPCODE_MOVL_ADRM_RN) {
			/* mov.l @(disp,Rm),Rn */
			addr = (uint8_t*)(ctx->gr[inst.i_d4r.reg2] + (inst.i_d4r.disp << 2));
			val32.bytes.b0 = *addr++;
			val32.bytes.b1 = *addr++;
			val32.bytes.b2 = *addr++;
			val32.bytes.b3 = *addr;
			ret = 0;
			lock_kernel();
			ctx->gr[inst.i_d4r.reg1] = val32.i;
		}
		else if(inst.i_r.op == OPCODE_MOV_RM_AR0RN) {
			/* mov Rm,@(R0,Rn);mov @(R0,Rm),Rn;movt Rn */
			if(inst.i_r.func == FCODE_MOVW_RM_AR0RN) {
				/* mov.w Rm,@(R0,Rn) */
				val16.i = ctx->gr[inst.i_r.reg2];
				addr = (uint8_t*)(ctx->gr[inst.i_r.reg1] + ctx->gr[0]);
				ret = 0;
				*addr++ = val16.bytes.b0;
				*addr = val16.bytes.b1;
			} else if(inst.i_r.func == FCODE_MOVL_RM_AR0RN) {
				/* mov.l Rm,@(R0,Rn) */
				val32.i = ctx->gr[inst.i_r.reg2];
				addr = (uint8_t*)(ctx->gr[inst.i_r.reg1] + ctx->gr[0]);
				ret = 0;
				*addr++ = val32.bytes.b0;
				*addr++ = val32.bytes.b1;
				*addr++ = val32.bytes.b2;
				*addr = val32.bytes.b3;
			} else if(inst.i_r.func == FCODE_MOVW_AR0RM_RN) {
				/* mov.w @(R0,Rm),Rn */
				// sign extended
				addr = (uint8_t*)(ctx->gr[inst.i_r.reg2] + ctx->gr[0]);
				val16.bytes.b0 = *addr++;
				val16.bytes.b1 = *addr;
				ret = 0;
				lock_kernel();
				ctx->gr[inst.i_r.reg1] = val16.i | (val16.i & 0x8000) ? 0xffff0000:0;
			} else if(inst.i_r.func == FCODE_MOVL_AR0RM_RN) {
				/* mov.l @(R0,Rm),Rn */
				addr = (uint8_t*)(ctx->gr[inst.i_r.reg2] + ctx->gr[0]);
				val32.bytes.b0 = *addr++;
				val32.bytes.b1 = *addr++;
				val32.bytes.b2 = *addr++;
				val32.bytes.b3 = *addr;
				ret = 0;
				lock_kernel();
				ctx->gr[inst.i_r.reg1] = val32.i;
			}

		}
		else if(inst.i_r.op == OPCODE_MOV_RM_ARN) {
			/* mov Rm,@Rn;mov Rm,@-Rn;xtrct Rm,Rn */
			if(inst.i_r.func == FCODE_MOVW_RM_ARN) {
				/* mov.w Rm,@Rn */
				val16.i = ctx->gr[inst.i_r.reg2];
				addr = (uint8_t*)ctx->gr[inst.i_r.reg1];
				*addr++ = val16.bytes.b0;
				*addr = val16.bytes.b1;
				ret = 0;
			} else if(inst.i_r.func == FCODE_MOVL_RM_ARN) {
				/* mov.l Rm,@Rn */
				val32.i = ctx->gr[inst.i_r.reg2];
				addr = (uint8_t*)ctx->gr[inst.i_r.reg1];
				*addr++ = val32.bytes.b0;
				*addr++ = val32.bytes.b1;
				*addr++ = val32.bytes.b2;
				*addr = val32.bytes.b3;
				ret = 0;
			}
		}
		else if(inst.i_r.op == OPCODE_MOV_ARM_RN) {
			/* mov @Rm,Rn;mov @Rm+,Rn;swap */
			if(inst.i_r.func == FCODE_MOVW_ARM_RN) {
				/* mov.w @Rm,Rn */
				// sign extended
				addr = (uint8_t*)ctx->gr[inst.i_r.reg2];
				val16.bytes.b0 = *addr++;
				val16.bytes.b1 = *addr;
				ret = 0;
				lock_kernel();
				ctx->gr[inst.i_r.reg1] = val16.i | (val16.i & 0x8000) ? 0xffff0000:0;
			} else if(inst.i_r.func == FCODE_MOVL_ARM_RN) {
				/* mov.l @Rm,Rn */
				addr = (uint8_t*)ctx->gr[inst.i_r.reg2];
				val32.bytes.b0 = *addr++;
				val32.bytes.b1 = *addr++;
				val32.bytes.b2 = *addr++;
				val32.bytes.b3 = *addr;
				ret = 0;
				lock_kernel();
				ctx->gr[inst.i_r.reg1] = val32.i;
			}
		}
	}

	if(ret == 0) {
		lock_kernel();
		ctx->pc = next_pc;
	}

	SET_XFER_HANDLER(0);
	return ret;
}

__SRCVERSION("emulate.c $Rev: 163913 $");
