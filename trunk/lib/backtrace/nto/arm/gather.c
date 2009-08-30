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
#include "backtrace.h"
#include "get_backtrace.h"
#include "signature.h"


/*
 * Assume that a prolog is:
 *   mov   ip, sp
 *   stmdb sp!,{...., fp, ip, lr, ...}
 *   sub   fp, ip, #4
 *
 * Where fp=r11, ip=r12, sp=r13, lr=r14, pc=r15
 *
 * Search back until lowlimit is passed, or until
 * something looking like an epilog is found.
 *
 * If a prolog is found, then the lr and fp are obtained from the stack.
 *
 * stack is moving from high addr to low addr on pushes.
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
				  bt_addr_t fp, bt_addr_t pc, bt_addr_t lowlimit,
				  bt_addr_t *caller_sp,
				  bt_addr_t *caller_fp, bt_addr_t *caller_pc)
{
	static const bt_signature_u32_t prolog[] = {
		{.val=0xe1a0c00d, .mask=0xffffffff}, // mov ip, sp
		{.val=0xe92dd800, .mask=0xffffd800}, // stmdb sp!,{...,fp,ip,...,lr,pc}
		{.val=0xe24cb004, .mask=0xffffffff}, // sub fp, ip, #4
	};
	BT_SIG_LEN(prolog);
	MEM_BUF(uint32_t, prolog_len);
	int prolog_size = MEM_BUF_SZ;
	bt_addr_t addr = pc;

	if (pc == 0) {
		return 0;
	}

	MEM_GET(addr, MEM_BUF_SZ);

	while (MEM_ADDR && MEM_ADDR >= lowlimit) {
		/*
		 * If found the prolog...
		 */
		if (BT_SIG_MATCH(prolog, MEM_P)) {
			// Each bit corresponds to one saved register.  See arm's
			// stmdb instruction.
			unsigned saved_reg_list = ((uint32_t*)MEM_P)[1]&0xffff;
			int rnum=15;
			uint32_t val;
			int i=0;
			int found = 0;

			/* pc might be before, or in the prolog... */
			if ((pc-MEM_ADDR) < prolog_size) {
				// In the midle of running the prolog.  This can
				// happen if an interrupt occur, or when backtracing
				// another thread.
				return 0;
			}

			while (rnum >= 0) {
				if (saved_reg_list&(1<<rnum)) {
					switch (rnum) {
						case 11: /* caller's fp */
							found ++;
							MEM_READ_VAR(val, fp-(i*4));
							(*caller_fp) = val;
							break;

						case 12: /* caller's sp */
							/*
							 * When backtracing through __signalstub,
							 * the sp, rather than fp is needed to
							 * fetch the ucontext.  So, in case
							 * caller_pc is in sigstub, save the
							 * caller_sp (refer to _gather() to see
							 * how this gets used).
							 */
							found ++;
							MEM_READ_VAR(val, fp-(i*4));
							(*caller_sp)= val;
							break;

						case 14: /* caller's pc */
							found ++;
							MEM_READ_VAR(val, fp-(i*4));
							/* The value on the stack is really the
							 * return address.  Adjust to have the
							 * address of the actual assembly
							 * instruction being executed at that
							 * stack frame */
							(*caller_pc) = val-BT_GATHER_ADJUST_PC;
							break;

						default:
							break;
					}

					i++;
				}
				rnum --;
			}

			/* Should have found 2 stored registers... */
			if (found == 3) {
				return 1;
			} else {
				errno=EINVAL;
				return -1;
			}
		}

		if (MEM_ADDR <= 4)
			return 0;
		MEM_SLIDE_BACKWARD(4);
	}

	return 0;
}

static inline int
is_syscall (mem_reader_t *rdr, bt_addr_t pc)
{
	static const bt_signature_u32_t sig[] = {
		{.val=0xe3a0c000, .mask=0xfffff000}, /* mov ip, #<n> */
		{.val=0xef000000, .mask=0xffffffff}, /* swi 0x00000000 */
		{.val=0xe8bd8000, .mask=0xffff8000}, /* ldmia sp!, {..., pc} */
	};
	BT_SIG_LEN(sig);
	MEM_BUF(uint32_t, sig_len);

	/* Check one instruction prior to current pc (hence the -4) ... */
	MEM_GET(pc-4,MEM_BUF_SZ);
	
	return BT_SIG32_MATCH(sig, MEM_P);
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
	static bt_signature_u32_t sig[] = {
		{.val=0xe594f028, .mask=0xffffffff}, /* ldr   pc, [r4, #40] */
		{.val=0xe1a00004, .mask=0xffffffff}, /* mov   r0, r4        */
		{.val=0xe8950ffe, .mask=0xffffffff}, /* ldmia r5, {r1, r2, r3, r4, r5, r6, r7, r8, r9, sl, fp} */
		{.val=0xea000000, .mask=0xff000000}, /* b     <addr>        */
	};
	BT_SIG_LEN(sig);
	MEM_BUF(uint32_t, sig_len);

	if (pc == 0) return 0;

	MEM_GET(pc, MEM_BUF_SZ);

	return BT_SIG32_MATCH(sig, MEM_P);
}


static inline int
get_sigstub_fp_pc (mem_reader_t *rdr,
				   bt_addr_t sp, bt_addr_t *caller_fp, bt_addr_t *caller_pc,
				   bt_addr_t *caller_lr)
{
	ucontext_t uc;
	bt_addr_t uc_ptr_addr;
	uint32_t uc_addr;
	uint32_t return_addr;
	int ret;
	static struct _sighandler_info si = {};

	/*
	 * Locate the offset of the pointer to the ucontext_t structure:
	 * sp: current stack pointer
	 *     (_sighandler_info is the first field of SIGSTACK)
	 *   : offset of context in _sighandler_info)
	 */
	uc_ptr_addr = sp +
		((bt_addr_t)&si.context - (bt_addr_t)&si);
	MEM_READ_VAR(uc_addr, uc_ptr_addr);

	/* Read entire ucontext_t from stack */
	MEM_READ_VAR(uc, uc_addr);

	(*caller_fp) = uc.uc_mcontext.cpu.gpr[ARM_REG_R11];
	return_addr =  uc.uc_mcontext.cpu.gpr[ARM_REG_PC];
	(*caller_pc) = return_addr - BT_GATHER_ADJUST_PC;
	if ((ret=is_syscall(rdr, *caller_pc)) == -1)
		return -1;
	if (ret) {
		*caller_lr = ((&(uc.uc_mcontext.cpu))->gpr[ARM_REG_LR]);
	} else {
		*caller_lr = 0;
	}
	return 1;
}

int
_BT(gather) (mem_reader_t *rdr,
			 bt_addr_t fp, bt_addr_t *pc, int pc_ary_sz, int *count,
			 bt_addr_t stack_limit, bt_addr_t last_pc)
{
	int ret;
	bt_addr_t sp=0, caller_sp, caller_fp, caller_pc, caller_lr=0;

	if (BT(gather_hook)) {
		BT(gather_hook)(fp);
	}
	while (fp) {
		if ((*count) >= pc_ary_sz) {
			break;
		}
		if (caller_lr) {
			/* The previous entry on the stack was a syscall, which
			 * means that there's no prolog to search, and the previously
			 * obtained caller_lr is the next entry on the stack */
			caller_pc = caller_lr - BT_GATHER_ADJUST_PC;
			caller_fp = fp;
			caller_lr = 0;      /* clear it after use */
		} else if ((ret=is_in_sigstub(rdr, last_pc)) != 0) {
			if (ret == -1) return -1;

			/* Latest return address is to the sigstub.  fetch
			 * caller's stack frame and pc from the saved context (put
			 * by the kernel on the stack) */
			ret = get_sigstub_fp_pc(rdr, sp, &caller_fp, &caller_pc, &caller_lr);
			if (ret == -1)
				return -1;
			if (ret == 0)
				return 0;
		} else {
			ret=_BT(find_prolog)(rdr, fp,
								 last_pc, last_pc-4096,
								 &caller_sp, &caller_fp, &caller_pc);
			if (ret == -1)
				return -1;
			if (ret == 0)
				return 0;
		}

		last_pc = caller_pc;
		if (fp > stack_limit) {
			pc[*count] = caller_pc;
			(*count)++;
		}

		fp = caller_fp;
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
	bt_addr_t fp, lr;
	procfs_greg greg;
	int greg_size;
	int ret;

	if (_bt_get_greg(rdr->fd, acc, &greg, &greg_size) == -1)
		return -1;
	fp=GETREG(&greg,11);
	lr=GETREG(&greg,14);
	pc[0]=(bt_addr_t)(GETREG(&greg,15)-BT_GATHER_ADJUST_PC);
	(*count) = 1;
	if ((*count) == pc_ary_sz)
		return 0;
	if ((ret=is_syscall(rdr, pc[0])) == -1)
		return -1;
	if (ret) {                  /* system call */
		pc[1]=(bt_addr_t)lr-BT_GATHER_ADJUST_PC;
		(*count)++;
		if ((*count) == pc_ary_sz)
			return 0;
	}

	return _bt_gather(rdr, fp, pc, pc_ary_sz, count,
					  acc->stack_limit, pc[*count-1]);
}

#endif
