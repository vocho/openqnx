/*
 * $QNXLicenseC:
 * Copyright 2007,2008, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable
 * license fees to QNX Software Systems before you may reproduce,
 * modify or distribute this software, or any work that includes
 * all or part of this software.   Free development licenses are
 * available for evaluation and non-commercial purposes.  For more
 * information visit http://licensing.qnx.com or email
 * licensing@qnx.com.
 *
 * This file may contain contributions from others.  Please review
 * this entire file for other proprietary rights or license notices,
 * as well as the QNX Development Suite License Guide at
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

#include "common.h"

#include <assert.h>
#include <errno.h>
#include <alloca.h>
#include <ucontext.h>
#include <sys/neutrino.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "backtrace.h"
#include "get_backtrace.h"
#include "signature.h"



/*
 * Assume that a prolog is:
 *   addiu sp,sp,-<n>
 *   sw    ra,<m>(sp)
 *
 * Search back until lowlimit is passed, or until
 * something looking like an epilog is found.
 *
 * An epilogue is assumed to look like:
 *   jr ra
 *   addiu sp,sp,<n>
 *
 * If a prologue is found, then the value of <n> is exctraced and stored
 * in stack_sz_change (as an absolute number).
 *
 * Notes:
 *
 * All sorts of problems are to be expected:
 * - there might not be a prolog, and no preceding epilog
 *   immediately before the actual start of the function
 * - the prolog or epilogue might look different than what we expect
 * - the function might change sp (e.g. alloca()) and thus, the calculation
 *   of the previous stack frame might not be possible.
 * return:
 * 0: no prolog found
 * 1: prolog found, and stored in arg 'prolog'
 * -1: error occured. errno set.
 */

int
_BT(find_prolog) (mem_reader_t *rdr,
				  bt_addr_t pc, bt_addr_t lowlimit,
				  int *stack_incr, int *ra_offset)
{
	static const bt_signature_u32_t prolog[] = {
		{.val=0x27bdffe8, .mask=0xffff0000}, /* addiu sp,sp,<-n> */
		{.val=0xafbf0014, .mask=0xffff0000}, /* sw    ra,<m>(sp) */
	};
	BT_SIG_LEN(prolog);
	int prolog_size = prolog_len*sizeof(uint32_t);
	static const bt_signature_u32_t epilog[] = {
		{.val=0x03e00008, .mask=0xffffffff}, /* jr    ra */
	};
	BT_SIG_LEN(epilog);
	MEM_BUF(uint32_t, max(prolog_len, epilog_len));

	if (pc == 0 || pc < lowlimit)
		return 0;

	MEM_GET(pc, prolog_size);

	while (1) {
		if (BT_SIG_MATCH(prolog, MEM_P)) {

			/* pc might be before, or in the prolog... */
			if ((pc-MEM_ADDR) < prolog_size) {
				// In the midle of running the prolog.  This can
				// happen if an interrupt occur, or when backtracing
				// another thread.
				return 0;
			}

			/* Extract increment and offset from byte code */
			(*stack_incr)=((uint32_t*)MEM_P)[0]&0xffff;
			(*ra_offset)=((uint32_t*)MEM_P)[1]&0xffff;
			// Sign extend...
			if ((*stack_incr) & 0x00008000)
				(*stack_incr) |= 0xffff0000;
			if ((*ra_offset) & 0x00008000)
				(*ra_offset) |= 0xffff0000;
			return 1;
		}
		/* If what looks like an epilog is found, then end the search for
		 * the prolog... */
		if (BT_SIG_MATCH(epilog, MEM_P)) {
			return 0;
		}

		if ((MEM_ADDR <= 4) || ((MEM_ADDR-4) < lowlimit)) {
			return 0;
		}

		MEM_SLIDE_BACKWARD(4);
	}

	return 0;
}

static inline int
is_syscall (mem_reader_t *rdr, bt_addr_t pc)
{
	uint32_t opcode;
	MEM_READ_VAR(opcode, pc);
	if (opcode == 0xc)
		return 1;
	return 0;
}

/*
 * Returns:
 * -1 on error
 * 0 if pc NOT in sigstub
 * 1 if pc APPEARS to be in sigstub
 */ 
static inline int
is_in_sigstub (mem_reader_t *rdr, bt_addr_t pc)
{
	static const bt_signature_u32_t sig[] = {
		{.val=0x02201021, .mask=0xffffffff}, /* move v0,s1 */
		{.val=0x02002021, .mask=0xffffffff}, /* move a0,s0 */
		{.val=0x04100001, .mask=0xffffffff},
		{.val=0x00000000, .mask=0xffffffff},
		{.val=0x3c170007, .mask=0xffffffff},
		{.val=0x26f71668, .mask=0xffffffff},
		{.val=0x02ffb821, .mask=0xffffffff},
		{.val=0x8ee18b7c, .mask=0xffffffff},
	};
	BT_SIG_LEN(sig);
	MEM_BUF(uint32_t, sig_len);

	if (pc == 0) return 0;

	pc+=BT_GATHER_ADJUST_PC;    /* reverse the adjustment */

	MEM_GET(pc, MEM_BUF_SZ);

	return BT_SIG32_MATCH(sig, MEM_P);
}

static inline int
get_sigstub_sp_pc (mem_reader_t *rdr,
				   bt_addr_t sp, uint32_t *caller_sp, uint32_t *caller_pc,
				   uint32_t *caller_ra)
{
	ucontext_t uc;
	bt_addr_t uc_ptr_addr;
	uint32_t uc_addr;
	uint32_t return_addr;
	static struct _sighandler_info si = {};

	/*
	 * Locate the offset of the pointer to the ucontext_t structure:
	 * sp: current stack pointer
	 * 16: offset of SIGSTACK from sp
	 *     (_sighandler_info is the first field of SIGSTACK)
	 *   : offset of context in _sighandler_info)
	 */
	uc_ptr_addr = sp + 16/*stack offset*/ +
		((bt_addr_t)&si.context - (bt_addr_t)&si);

	MEM_READ_VAR(uc_addr, uc_ptr_addr);

	/* Read entire ucontext_t from stack */
	MEM_READ_VAR(uc, uc_addr);

	(*caller_sp) = GET_REGSP(&(uc.uc_mcontext.cpu));
	return_addr = GET_REGIP(&(uc.uc_mcontext.cpu));

	if (is_syscall(rdr, return_addr-4)) {
		(*caller_pc) = return_addr - 4; /* no pipeline on 'syscall' */
		*caller_ra = ((&(uc.uc_mcontext.cpu))->regs[MIPS_CREG(MIPS_REG_RA)]);
	} else {
		(*caller_pc) = return_addr - BT_GATHER_ADJUST_PC;
		*caller_ra = 0;
	}

	return 1;
}

int
_BT(gather) (mem_reader_t *rdr,
			 uint32_t sp, bt_addr_t *pc, int pc_ary_sz, int *count,
			 bt_addr_t stack_limit, bt_addr_t last_pc)
{
	uint32_t caller_sp;
	uint32_t caller_pc;
	uint32_t caller_ra = 0;
	uint32_t return_addr;
	int ret;
	int stack_incr, ra_offset;

	if (BT(gather_hook)) {
		BT(gather_hook)(sp);
	}
	while (sp) {
		if ((*count) >= pc_ary_sz) {
			break;
		}
		if (caller_ra) {
			/* The previous entry on the stack was a syscall, which
			   means that there's no prolog to search, and the previously
			   obtained caller_ra is the next entry on the stack */
			caller_pc = caller_ra - BT_GATHER_ADJUST_PC;
			caller_sp = sp;
			caller_ra = 0;      /* clear it after use */
		} else if ((ret=is_in_sigstub(rdr, last_pc)) != 0) {
			if (ret == -1) return -1;

			/* Latest return address is to the sigstub.  fetch
			 * caller's stack frame and pc from the saved context (put
			 * by the kernel on the stack) */
			if (get_sigstub_sp_pc(rdr, sp,
								  &caller_sp, &caller_pc, &caller_ra) == -1) {
				return -1;
			}
		} else {
			ret=_BT(find_prolog)(rdr, last_pc, last_pc-1024,
								 &stack_incr, &ra_offset);
			if (ret == -1)
				return -1;
			if (ret == 0)
				return 0;
			MEM_READ_VAR(return_addr, sp+ra_offset);
			caller_sp = sp-stack_incr;
			caller_pc = return_addr - BT_GATHER_ADJUST_PC;
		}

		last_pc = caller_pc;
		
		if (stack_limit == 0 ||caller_sp > stack_limit) {
			pc[*count] = caller_pc;
			(*count)++;
		}

		sp = caller_sp;

	}
	return 0;
}

#ifndef _BT_LIGHT

int
_bt_gather_other (bt_accessor_t *acc,
				  mem_reader_t *rdr,
				  bt_addr_t *pc, int pc_ary_sz, int *count)
{
	uint32_t epc;
	uint32_t sp;
	uint32_t ra;
	procfs_greg greg;
	int greg_size;
	if (_bt_get_greg(rdr->fd, acc, &greg, &greg_size) == -1)
		return -1;
	sp=GETREG(&greg,29);
	ra=GETREG(&greg,31); /* this is iffy */
	epc=GETREG(&greg,37);
	if (is_syscall(rdr,epc-4)) {
		pc[0]=(bt_addr_t)epc-4;
		(*count)=1;
		if ((*count) == pc_ary_sz)
			return 0;
		pc[1]=ra-BT_GATHER_ADJUST_PC;
		(*count)++;
	} else {
		pc[0]=(bt_addr_t)epc-BT_GATHER_ADJUST_PC;
		(*count)=1;
	}
	if ((*count) == pc_ary_sz)
		return 0;
	return _bt_gather(rdr, sp, pc, pc_ary_sz, count,
					  acc->stack_limit, pc[*count-1]);
}

#endif
