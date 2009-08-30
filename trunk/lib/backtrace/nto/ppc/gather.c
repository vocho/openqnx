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

#include <errno.h>
#include <alloca.h>
#include <ucontext.h>
#include <sys/neutrino.h>
#include "backtrace.h"
#include "get_backtrace.h"
#include "signature.h"


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
		{.val=0x7fc3f378, .mask=0xffffffff}, /* mr r3,r30 */
		{.val=0x73bd0400, .mask=0xffffffff}, /* andi. r29,r29,1024 */
		{.val=0x41a20118, .mask=0xffffffff},
		{.val=0x385f00a8, .mask=0xffffffff},
		{.val=0x10028301, .mask=0xffffffff},
		{.val=0x100004c4, .mask=0xffffffff},
		{.val=0x80020004, .mask=0xffffffff},
	};
	BT_SIG_LEN(sig);
	MEM_BUF(uint32_t, sig_len);

	if (pc == 0) return 0;

	pc+=BT_GATHER_ADJUST_PC;    /* reverse the adjustment */

	MEM_GET(pc, MEM_BUF_SZ);

	return BT_SIG32_MATCH(sig, MEM_P);
}

static inline int
is_syscall (mem_reader_t *rdr, bt_addr_t pc)
{
	static const bt_signature_u32_t sig[] = {
		{.val=0x38000000, .mask=0xffffff00}, // li r0,<n>
		{.val=0x44000002, .mask=0xffffffff}, // sc
	};
	BT_SIG_LEN(sig);
	MEM_BUF(uint32_t, sig_len);
	/* Check one instruction prior to current pc (hence the -4) ... */
	MEM_GET(pc-4, MEM_BUF_SZ);
	return BT_SIG32_MATCH(sig, MEM_P);
}

static inline int
get_sigstub_sp_pc (mem_reader_t *rdr,
				   bt_addr_t sp,
				   uint32_t *caller_sp, uint32_t *caller_pc,
				   uint32_t *caller_lr)
{
	ucontext_t uc;
	bt_addr_t uc_ptr_addr;
	uint32_t uc_addr;
	int ret;
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
	(*caller_pc) = GET_REGIP(&(uc.uc_mcontext.cpu)) - BT_GATHER_ADJUST_PC;

	if ((ret=is_syscall(rdr, (*caller_pc))) == -1)
		return -1;
	if (ret) {                  /* syscall */
		/* If caller_pc is a system call, then the LR register is
		 * going to be the the address of the function that called the
		 * system call */
		(*caller_lr) = uc.uc_mcontext.cpu.lr - BT_GATHER_ADJUST_PC;
	} else {                    /* not syscall */
		(*caller_lr) = 0;
	}

	return 1;
}

int
_BT(gather) (mem_reader_t *rdr,
			 uint32_t sp, bt_addr_t *pc, int pc_ary_sz, int *count,
			 bt_addr_t stack_limit, bt_addr_t last_pc)
{
	uint32_t caller_lr=0;
	uint32_t caller_sp;
	uint32_t caller_pc=0;
	uint32_t lr;
	int ret;

	if (BT(gather_hook)) {
		BT(gather_hook)(sp);
	}
	while (sp) {
		if ((*count) >= pc_ary_sz) {
			break;
		}

		if (caller_lr) {
			caller_pc = caller_lr;
			caller_sp = sp;
			caller_lr = 0;      /* reset after use */
		} else if ((ret=is_in_sigstub(rdr, last_pc)) != 0) {
			if (ret == -1) return -1;

			/* Latest return address is to the sigstub.  fetch
			 * caller's stack frame and pc from the saved context (put
			 * by the kernel on the stack).  Also, in the event that
			 * caller_pc is a system call, then caller_lr is going to
			 * be set to the caller of the system call (i.e. the next
			 * function in the backtrace). */
			if (get_sigstub_sp_pc(rdr, sp,
								  &caller_sp, &caller_pc, &caller_lr) == -1) {
				return -1;
			}
		} else {
			MEM_READ_VAR(caller_sp, (bt_addr_t)(((uint32_t*)sp)));

			if (caller_sp) {
				MEM_READ_VAR(lr, (bt_addr_t)(((uint32_t*)caller_sp)+1));
				caller_pc = lr - BT_GATHER_ADJUST_PC;
			}
		}

		/*
		 * On powerpc, there appears to be a last frame which has a
		 * return address of 0.  This isn't put on the backtrace.
		 */
		if (caller_pc == 0)
			break;

		if (caller_sp) {
			last_pc = caller_pc;
			if (stack_limit == 0 || caller_sp > stack_limit) {
				pc[*count] = caller_pc;
				(*count)++;
			}
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
	uint32_t sp;
	uint32_t lr;
	uint32_t iar;
	procfs_greg greg;
	int greg_size;
	int ret;
	if (pc_ary_sz <= 0) {
		(*count) = 0;
		return 0;
	}
	if (_bt_get_greg(rdr->fd, acc, &greg, &greg_size) == -1)
		return -1;
	sp=GETREG(&greg,1);
	lr=GETREG(&greg,33);
	iar=GETREG(&greg,35);
	pc[0]=(bt_addr_t)iar-BT_GATHER_ADJUST_PC;
	(*count)=1;
	if ((*count) == pc_ary_sz)
		return 0;
	if ((ret=is_syscall(rdr, pc[0])) == -1)
		return -1;
	if (ret) {                  /* syscall */
		/* If caller_pc is a system call, then the LR register is
		 * going to be the the address of the function that called the
		 * system call */
		pc[1] = lr - BT_GATHER_ADJUST_PC;
		(*count) ++;
		if ((*count) == pc_ary_sz)
			return 0;
	}

	return _bt_gather(rdr, sp, pc, pc_ary_sz, count,
					  acc->stack_limit, pc[*count-1]);
}

#endif
