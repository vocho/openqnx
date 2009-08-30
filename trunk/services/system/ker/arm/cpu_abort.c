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
 * ARM7 Data abort fixup and unaligned access emulation
 */

#include "externs.h"
#include <arm/opcode.h>

// opcodes
#define	OPCODE(i)			(((i) >> 24) & 15)
#define		HSB_POST		0
#define		HSB_PRE			1
#define		LSR_IMM_POST	4
#define		LSR_IMM_PRE		5
#define		LSR_REG_POST	6
#define		LSR_REG_PRE		7
#define		LSM_POST		8
#define		LSM_PRE			9
#define		LSC_POST		12
#define		LSC_PRE			13

#define	LSH_H	(1 << 5)	// halfword load/store
#define	LSH_S	(1 << 6)	// signed   load/store

/*
 * Calculate addressing mode offset for load/store with register shift
 */
static unsigned
arm_lsr_reg_offset(CPU_REGISTERS *reg, unsigned instr)
{
	int			shift = ARM_SHIFT_IMM(instr);
	unsigned	val = reg->gpr[ARM_RM(instr)];
	unsigned	off;

	switch (ARM_SHIFT(instr)) {

	case ARM_SHIFT_LSL:
		return val << shift;

	case ARM_SHIFT_LSR:
		if (shift == 0)
			shift = 32;
		return val >> shift;

	case ARM_SHIFT_ASR:
		if (shift == 0)
			shift = 32;
		return (int)val >> shift;

	case ARM_SHIFT_ROR:
		if (shift == 0) {
			off = val >> 1;
			if (reg->spsr & ARM_CPSR_C)
				off |= 1 << 31;
		} else {
			off = val >> shift;
			off |= val << (32-shift);
		}
		return off;
	default: break;
	}
	return 0;
}

/*
 * Fix up modified base registers for data abort
 * This is currently used only for ARM720 processors (see init_cpu.c)
 */
void
arm_abort_fixup(CPU_REGISTERS *reg)
{
	unsigned	instr = *(unsigned *)(reg->gpr[ARM_REG_PC]);
	unsigned	off   = 0;
	int			wb    = instr & ARM_LS_W;

	switch (OPCODE(instr)) {

	/*
	 * WARNING: the following two cases match a number of other instructions.
	 *			The following assumptions may not be true in ARMv5 and above:
	 *			- swp/swpb are pre-indexed with W=0 (no register update)
	 *			- other instruction encodings are not load/store operations
	 *			  and hence cannot cause a data abort.
	 */
	case HSB_POST:
		wb = 1;
	//fall through
	case HSB_PRE:
		if (instr & ARM_LSH_I) {
			off = ARM_LSH_IMML(instr) | (ARM_LSH_IMMH(instr) << 4);
		} else {
			off = reg->gpr[ARM_RM(instr)];
		}
		break;

	case LSR_IMM_POST:
		wb = 1;
	// fall through
	case LSR_IMM_PRE:
		off = ARM_LSR_IMM(instr);
		break;

	case LSR_REG_POST:
		wb = 1;
	// fall through
	case LSR_REG_PRE:
		off = arm_lsr_reg_offset(reg, instr);
		break;

	case LSM_POST:
		wb = 1;
	// fall through
	case LSM_PRE:
	{
		unsigned	rlist = ARM_LSM_RL(instr);

		for (off = 0; rlist; rlist >>= 1) {
			off += rlist & 1;
		}
		off <<= 2;
		break;
	}

	case LSC_POST:
		wb = 1;
	// fall through
	case LSC_PRE:
		off = ARM_LSC_IMM(instr) << 2;
		break;
	default: break;
	}

	if (wb) {
		if (instr & ARM_LS_U)
			reg->gpr[ARM_RN(instr)] -= off;
		else
			reg->gpr[ARM_RN(instr)] += off;
	}
}


/*
 * --------------------------------------------------------------------------
 * Unaligned access emulation
 * --------------------------------------------------------------------------
 */

static inline unsigned
get_byte(unsigned ptr)
{
	unsigned	tmp;

	asm volatile(
		"ldrbt	%0, [%1]"
		: "=&r" (tmp)
		: "r" (ptr)
	);
	return tmp;
}

static inline void
put_byte(unsigned ptr, unsigned val)
{
	asm volatile(
		"strbt	%0, [%1]"
		:
		: "r" (val), "r" (ptr)
	);
}

static void
load_store(unsigned *reg, unsigned ptr, unsigned instr)
{
	unsigned	val;

	if (instr & ARM_LS_L) {
#if defined(__LITTLEENDIAN__)
		val  = get_byte(ptr);
		val |= get_byte(ptr+1) << 8;
		val |= get_byte(ptr+2) << 16;
		val |= get_byte(ptr+3) << 24;
#else
		val  = get_byte(ptr)   << 24;
		val |= get_byte(ptr+1) << 16;
		val |= get_byte(ptr+2) << 8;
		val |= get_byte(ptr+3);
#endif
		*reg = val;
	}
	else {
		val = *reg;
#if defined(__LITTLEENDIAN__)
		put_byte(ptr,   val);
		put_byte(ptr+1, val >> 8);
		put_byte(ptr+2, val >> 16);
		put_byte(ptr+3, val >> 24);
#else
		put_byte(ptr,   val >> 24);
		put_byte(ptr+1, val >> 16);
		put_byte(ptr+2, val >> 8);
		put_byte(ptr+3, val);
#endif
	}
}

static void
load_store_halfword(unsigned *reg, unsigned ptr, unsigned instr)
{
	unsigned	val;

	if (instr & ARM_LS_L) {
#if defined(__LITTLEENDIAN__)
		val  = get_byte(ptr);
		val |= get_byte(ptr+1) << 8;
#else
		val  = get_byte(ptr)   << 8;
		val |= get_byte(ptr+1);
#endif
		if (instr & LSH_S) {
			val = ((int)(val << 16)) >> 16;
		}
		*reg = val;
	}
	else {
		val = *reg;
#if defined(__LITTLEENDIAN__)
		put_byte(ptr,   val);
		put_byte(ptr+1, val >> 8);
#else
		put_byte(ptr+2, val >> 8);
		put_byte(ptr+3, val);
#endif
	}
}

static unsigned
load_store_multiple(CPU_REGISTERS *reg, unsigned instr)
{
	unsigned	addr;
	unsigned	off;
	unsigned	rlist;
	unsigned	rn;
	int			saved_rn = 0;
	int			r;
	int			load = instr & ARM_LS_L;

	off = 0;
	for (rlist = ARM_LSM_RL(instr); rlist; rlist >>= 1) {
		if (rlist & 1)
			off += 4;
	}

	/*
	 * Calculate address of lowest numbered register
	 */
	addr = reg->gpr[ARM_RN(instr)];
	if ((instr & ARM_LS_U) == 0) {
		addr -= off;
		if ((instr & ARM_LS_P) == 0) {
			addr += 4;
		}
	}
	else if ((instr & ARM_LS_P) != 0) {
		addr += 4;
	}

	for (rlist = ARM_LSM_RL(instr), r = 0; rlist; rlist >>= 1, r++) {
		if ((rlist & 1) == 0)
			continue;

		/*
		 * If we are about to modify the Rn register, do it to a temp
		 * location. This ensures we preserve the base address in case we
		 * cross a page boundary and take a fault after loading Rn.
		 * If such a fault happens, the page will be brought in and we
		 * will restart the whole instruction.
		 */
		if (load && r == ARM_RN(instr)) {
			load_store(&rn, addr, load);
			saved_rn = 1;
		}
		else {
			load_store(reg->gpr + r, addr, load);
		}
		addr += 4;
	}

	if (saved_rn) {
		reg->gpr[ARM_RN(instr)] = rn;
	}
	return off;
}

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
 * Emulate a user mode misaligned load/store operation.
 *
 * FIXME: doesn't do the new V5 load/store instructions
 */
unsigned
arm_align_emulate(CPU_REGISTERS *reg, unsigned addr)
{
	unsigned	instr = *(unsigned *)(reg->gpr[ARM_REG_PC]);
	unsigned	off   = 0;
	int			wb    = instr & ARM_LS_W;

	/*
	 * Set INKERNEL_EXIT to re-start instruction in case we fault
	 */
	bitset_inkernel(INKERNEL_EXIT);
	SET_XFER_HANDLER(&emulate_fault_handlers);
	unlock_kernel();


	switch (OPCODE(instr)) {

	/*
	 * WARNING: the following two cases match a number of other instructions.
	 *			The following assumptions may not be true in ARMv5 and above:
	 *			- swp/swpb are always pre-indexed with W=0
	 *			- other instruction encodings are not load/store operations
	 *			  and hence cannot cause a data abort.
	 */
	case HSB_POST:
		wb = 1;
	// fall through
	case HSB_PRE:
		/*
		 * Only deal with halfword load/stores
		 */
		if ((instr & LSH_H) == 0) {
			SET_XFER_HANDLER(0);
			return MAKE_SIGCODE(SIGBUS, BUS_ADRALN, FLTACCESS);
		}

		if (instr & ARM_LSH_I) {
			off = ARM_LSH_IMML(instr) | (ARM_LSH_IMMH(instr) << 4);
		} else {
			off = reg->gpr[ARM_RM(instr)];
		}
		load_store_halfword(reg->gpr + ARM_RD(instr), addr, instr);
		break;

	case LSR_IMM_POST:
		wb = 1;
	// fall through
	case LSR_IMM_PRE:
		off = ARM_LSR_IMM(instr);
		load_store(reg->gpr + ARM_RD(instr), addr, instr);
		break;

	case LSR_REG_POST:
		wb = 1;
	// fall through
	case LSR_REG_PRE:
		off = arm_lsr_reg_offset(reg, instr);
		load_store(reg->gpr + ARM_RD(instr), addr, instr);
		break;

	case LSM_POST:
		wb = 1;
	// fall through
	case LSM_PRE:
		off = load_store_multiple(reg, instr);
		break;
	

	case LSC_POST:
	case LSC_PRE:
		/*
		 * FIXME: the N-bit specifies a coprocessor-dependent data size.
		 * 		  Since we don't know what that is here, fall through to
		 *		  the default case to deliver a signal.
		 */

	default:
		/*
		 * Can't emulate the instruction
		 */
		SET_XFER_HANDLER(0);
		return MAKE_SIGCODE(SIGBUS, BUS_ADRALN, FLTACCESS);
	}

	lock_kernel();

	/*
	 * Skip past the instruction and update the base register if necessary
	 */
	reg->gpr[ARM_REG_PC] += 4;

	if (wb) {
		if (instr & ARM_LS_U)
			reg->gpr[ARM_RN(instr)] += off;
		else
			reg->gpr[ARM_RN(instr)] -= off;
	}

	SET_XFER_HANDLER(0);
	return 0;
}

__SRCVERSION("cpu_abort.c $Rev: 160053 $");
