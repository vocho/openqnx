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
#include <ucontext.h>
#include <sys/neutrino.h>
#include <stdlib.h>
#include <stdio.h>
#include "backtrace.h"
#include "get_backtrace.h"
#include "signature.h"
#include "debug.h"

#define SH_MIN_CODE_ADDR    0x08040000
#define SH_INSTR_SZ         2
#define CODE_BYTE0(p)       (*(((uint8_t*)p)+0))
#define CODE_BYTE1(p)       (*(((uint8_t*)p)+1))


static inline bt_addr_t
get_low_limit (bt_addr_t pc)
{
	/* Here is the place where obtaining the start address of a
	 * function would be helpful.  That way, a correct low_limit could
	 * be establish.  In the meantime, just guess... */

	/* NOTE: This is VERY expensive, performance wise.  It's hard to
	   pick a number... if the number is too low, and the function is
	   large, then the backtrace might fail to search back far enough
	   to find the prolog.  On the other hand, if the number is too
	   large, then the backtrace might search back to far for no
	   reason and waste a lot of time, given that searching for the
	   prolog is expensive */
	return pc-1024;
}


/* Count the pushes prior to the start of the prolog up until the push
 * that saves the frame pointer on the stack... (It is assumed that
 * all instructions preceding the start of prolog are pushes.
 * However, some tolerance to non-pushes is factored).  Also
 * estimate the total number of pushes in the prolog.
 * 
 * Returns:
 * 1 if a push of the frame pointer is found
 * 0 if no push of the frame pointer is found
 * 
 * Returned args:
 * 
 *  push_cnt_to_fp_push - Number of pushes up to but exlcuding the
 *  saving of fp to stack
 *  
 *  push_cnt_total - estimate of how many pushes in total where made
 *  in the prolog.  This can be used to estimate the caller's stack
 *  pointer if no fp gets saved.
 */
static inline int
count_pushes (mem_reader_t *rdr, bt_addr_t addr, bt_addr_t lowlimit,
			  int *push_cnt_to_fp_push, int *push_cnt_total)
{
	static const bt_signature_u16_t push_gpr[] = { /* gen purpose regs */
		{.val=0x2f00, .mask=0xff00}, // mov.l  ...,@-r15
	};
	BT_SIG_LEN(push_gpr);
	static const bt_signature_u16_t push_fpr[] = { /* floating point regs */
		{.val=0xff0b, .mask=0xff0f}, // fmov ... ,@-r15
	};
	BT_SIG_LEN(push_fpr);
	static const bt_signature_u16_t push_frame_pointer[] = {
		{.val=0x2fe6, .mask=0xffff}, // mov.l r14,@-r15
	};
	BT_SIG_LEN(push_frame_pointer);
	int non_push_cnt = 0;
	int push_cnt = 0;
	MEM_BUF(uint16_t,1);
	int ret = 0;                /* push of fp not found yet. */

	MEM_GET(addr,MEM_BUF_SZ);

	while (MEM_ADDR >= lowlimit) {
		/* No need to check for bad memory access, because the mem
		 * area to count in is bound by the start/end prolog
		 * identified in _bt_find_prolog */

		if (BT_SIG16_MATCH(push_frame_pointer, MEM_P)) {
			ret = 1; /* Found what we're looking for... */
			(*push_cnt_to_fp_push) += push_cnt; /* Note: add because
												 * caller may have
												 * factored in other
												 * pushes */
			push_cnt ++;
		} else if (BT_SIG16_MATCH(push_gpr, MEM_P) || 
				   BT_SIG16_MATCH(push_fpr, MEM_P)) {
			push_cnt++;
		} else {
			non_push_cnt ++;
			/* Tolerate some non-pushes prior to
			 * start of prolog (numbers are
			 * arbitrary...) */
			if (non_push_cnt > 10 ||
				(non_push_cnt && push_cnt > 16)) {
				break;          /* That's enough... */
			}
		}

		MEM_SLIDE_BACKWARD(SH_INSTR_SZ);
	}

	(*push_cnt_total) += push_cnt; /* Note: add because caller may
									* have factored in other pushes */

	return ret;
}

/*
 * 	The prolog of a function looks something like:
 *
 * 	        ... pushes...
 * 	        mov.l r14, @-r15    # push caller's fp to stack
 * 	        ... pushes...
 *			sts.l   pr,  @-r15  # save return address
 *			... changes to r15 for local variables...
 *			mov   r15, r14      # setup new frame pointer
 *
 *	Changes to r15 are either via add instructions, mov.w/sub or mov.l/sub
 *	sequence of instructions.
 *
 *	The general search is as follows:
 *	   - Search for start of prolog
 *	   - Search for end of prolog (counting any changes to r15),
 *	     while at the same time counting for the changes to r15
 *	   - Count backward the number of pushes made up to the saving
 *	     of fp.  This count can then be used to get the caller's fp
 *	     from the stack.
 */
int
_BT(find_prolog) (mem_reader_t *rdr,
				  bt_addr_t fp, bt_addr_t pc, bt_addr_t lowlimit,
				  bt_addr_t *caller_fp, bt_addr_t *caller_pc,
				  bt_addr_t *alternate_fp)
{
	static const bt_signature_u16_t prolog_start[] = {
		{.val=0x4f22, .mask=0xffff}, // sts.l pr, @-r15
	};
	BT_SIG_LEN(prolog_start);
	static const bt_signature_u16_t prolog_end[] = {
		{.val=0x6ef3, .mask=0xffff}, // mov   r15, r14
	};
	BT_SIG_LEN(prolog_end);
	static const bt_signature_u16_t incr_stack[] = {
		{.val=0x7f80, .mask=0xff80}, // add   #-xx, r15
	};
	BT_SIG_LEN(incr_stack);
	// Variant A of a stack decrement
	static const bt_signature_u16_t stack_decr_a[] = {
		{.val=0xd000, .mask=0xf000}, // mov.l @(x*,PC),Rn  1101nnnnxxxxxxxx
		{.val=0x3f08, .mask=0xff0f}, // sub   Rm,r15       00111111mmmm1000
		// and Rn should be the same as Rm
	};
	BT_SIG_LEN(stack_decr_a);
	// Variant B of a stack decrement
	static const bt_signature_u16_t stack_decr_b[] = {
		{.val=0x9000, .mask=0xf000}, // mov.w @(x*,PC),Rn  1101nnnnxxxxxxxx
		{.val=0x3f08, .mask=0xff0f}, // sub Rm,r15         00111111mmmm1000
		// and Rn should be the same as Rm
	};
	BT_SIG_LEN(stack_decr_b);
	static const bt_signature_u16_t delayed_branch[] = {
		{.val=0x400b, .mask=0xf0ff}, // jsr @Rn            0100nnnn00001011
	};
	BT_SIG_LEN(delayed_branch);
	int ret;
	/* should be large enough to accomodate
	 * the largest signature to match against */
#define DATA_LEN 2
	MEM_BUF(uint16_t,DATA_LEN);
	bt_addr_t addr = pc+BT_GATHER_ADJUST_PC;
	bt_addr_t prolog_start_addr, prolog_end_addr;
	bt_addr_t sp;
	int push_cnt_to_fp_push=1; /* the sts.l (start of prolog) counts
								* as one push */
	int push_cnt_total=push_cnt_to_fp_push;
	int subs_total=0;
	int adds_total=0;        /* add to r15 between prolog start/end */

	MEM_GET(addr,MEM_BUF_SZ);

	lowlimit=max(lowlimit,SH_MIN_CODE_ADDR);

	while (MEM_ADDR && MEM_ADDR >= lowlimit) {
		if (BT_SIG16_MATCH(prolog_start, MEM_P)) {
			prolog_start_addr=MEM_ADDR;

			while (1) {
				if (BT_SIG16_MATCH(prolog_end, MEM_P)) {
					int back, back_sz;
					/* found the prolog end */
					prolog_end_addr=MEM_ADDR;

					/* If the pc isn't after the prolog_end, then it
					 * means the prolog is only partially executed,
					 * and none of the math done to locate the
					 * caller's fp can be trusted... so just pretend
					 * the prolog couldn't be found.
					 *
					 * There's however one exception, and that is if
					 * the prolog end is in the slot. e.g.:
					 * 
					 *   jsr @rn
					 *   mov r15,r14
					 *
					 * In that particular case, pc<prolog_end_addr,
					 * but the prolog is completely executed, because
					 * the mov is executed before the jsr instruction.
					 */

					if (pc <= prolog_end_addr) {
						if (pc != (prolog_end_addr-SH_INSTR_SZ)) {
							/* definitely not in the slot */
							return 0;
						} else {
							/* WARNING: Careful here... we're
							   overshadowing the top level
							   buffer... */
							MEM_BUF(uint16_t,delayed_branch_len);
							MEM_GET(prolog_end_addr-MEM_BUF_SZ,MEM_BUF_SZ);
							if (! BT_SIG16_MATCH(delayed_branch, MEM_P))
								return 0; /* not in the slot, so the
										   * prolog was not executed
										   * to the end */

							/* else, it is a delayed branch, and
							 * therefore the prolog_end was
							 * executed. */

						}

					}

					/* Look at the instructions prior to the prolog_end
					 * to figure out how much the stack was
					 * augmented */
					back = stack_decr_a_len; /* CHEAT: assume
											  * stack_decr_a is same
											  * len as stack_decr_b */
					back_sz = SH_INSTR_SZ*back;

					MEM_SLIDE_BACKWARD(back_sz);

					if (BT_SIG16_MATCH(stack_decr_a, MEM_P)) {
						// extract the sub value (32bits) from an
						// indirect mov.l
						bt_addr_t opcode_pc=prolog_end_addr-back_sz;
						bt_addr_t read_addr=
							CODE_BYTE0(MEM_P)*4+(opcode_pc>>2)<<2+4;
						int32_t decr;
						MEM_READ_VAR(decr, read_addr);
						subs_total = decr;
					} else if (BT_SIG16_MATCH(stack_decr_b, MEM_P)) {
						// extract the sub value (signed 16bits) from an
						// indirect mov.w, then sign extend.
						bt_addr_t opcode_pc=prolog_end_addr-back_sz;
						bt_addr_t read_addr=CODE_BYTE0(MEM_P)*2+opcode_pc+4;
						int16_t decr;
						MEM_READ_VAR(decr, read_addr);
						subs_total = decr;
					}

					/* Adjust fp, and get caller pc/fp ... */
					if (subs_total && adds_total) {
						// Stack modif should be either subs, or
						// negative adds, but not both?
						return 0;
					}

					/* caller's fp is pushed first on the stack,
					 * the return address is pushed last on the stack,
					 * and then some extra space is allocated before
					 * establishing the new fp.
					 */
					sp = fp + subs_total - adds_total; /* location of
														* return
														* address */
					/* read the return address... */
					MEM_READ_VAR(*caller_pc, sp);

					/* We're assuming that the caller's pc is is just
					   BT_GATHER_ADJUST_PC prior to the return
					   address.  */
					if ((*caller_pc)&0x1) exit(67);
					(*caller_pc) -= BT_GATHER_ADJUST_PC;
					if ((*caller_pc)&0x1) exit(68);

					ret = count_pushes(rdr, prolog_start_addr-SH_INSTR_SZ,
									   lowlimit,
									   &push_cnt_to_fp_push, &push_cnt_total);

					/* The alternate_fp is used if the fp (r14) isn't
					 * pushed on the stack.
					 *
					 * Also, in the event that the caller's pc is in
					 * _signalstub, then that would also serve as the
					 * starting point to search for the saved context
					 * on the stack because _signalstub does not setup
					 * a correct fp (r14).  So, whatever might have
					 * been saved on the stack is actually useless.
					 * For this reason, the alternate fp is returned
					 * back to the caller of _bt_find_prolog, just in
					 * case pc turn's out to be in _signalstub.
					 */
					(*alternate_fp) = sp + 4*push_cnt_total;

					if (ret == 1) {
						/* caller's fp is pushed on the stack first ... */
						MEM_READ_VAR(*caller_fp,
									 sp + 4 * push_cnt_to_fp_push);
					} else {
						/* Couldn't find a push of the fp to the
						 * stack, so just use the alternate_sp ... */
						(*caller_fp) = (*alternate_fp);
					}

					return 1;

				} else if (BT_SIG16_MATCH(incr_stack, MEM_P)) {
					/* Extract the signed value from the add instruction... */
					int8_t incr = ((int8_t*)MEM_P)[0];
					adds_total += incr;
				}

				if (MEM_ADDR >= pc) {
					/* If the prolog end wasn't found, then the pc
					   is in the middle of the prolog, and this case isn't
					   handled. */
					return 0;
				}

				MEM_SLIDE_FORWARD(MEM_SZ_1_ELT);
			}
		}

		MEM_SLIDE_BACKWARD(MEM_SZ_1_ELT);
	}
	return 0;
}

static inline int
is_syscall (mem_reader_t *rdr, bt_addr_t pc)
{
	static const bt_signature_u16_t syscall[] = {
		{.val=0x2fe6, .mask=0xffff}, /* mov.l    r14,@-r15 */
		{.val=0x6ef3, .mask=0xffff}, /* mov      r15,r14   */
		{.val=0xc300, .mask=0xff00}, /* trapa    #<n>      */
	};
	BT_SIG_LEN(syscall);
	MEM_BUF(uint16_t,syscall_len);
	bt_addr_t addr, limit;

	addr = pc - (syscall_len-1)*SH_INSTR_SZ;

	/* Limit the search back.  A system call is usually very short */
	limit = addr-SH_INSTR_SZ*20;

	MEM_GET(addr, MEM_BUF_SZ);
	if (BT_SIG16_MATCH(syscall, MEM_P))
		return 1;

	while (MEM_ADDR > limit) {
		MEM_SLIDE_BACKWARD(SH_INSTR_SZ);
		if (BT_SIG16_MATCH(syscall, MEM_P)) {
			return 1;
		}
	}
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
	static const bt_signature_u16_t sig[] = {
		{.val=0x410b, .mask=0xffff}, /* jsr     @r1         */
		{.val=0x6503, .mask=0xffff}, /* mov     r0,r5       */
		{.val=0x6083, .mask=0xffff}, /* mov     r8,r0       */
		{.val=0x6493, .mask=0xffff}, /* mov     r9,r4       */
		{.val=0xfff9, .mask=0xffff}, /* fmov    @r15+, fr15 */
		{.val=0xfef9, .mask=0xffff}, /* fmov    @r15+, fr14 */
	};
	BT_SIG_LEN(sig);
	MEM_BUF(uint16_t,sig_len);

	if (pc == 0) return 0;

	MEM_GET(pc,MEM_BUF_SZ);

	return BT_SIG16_MATCH(sig, MEM_P);
}


struct sigstack_entry {
	struct _sighandler_info		info;
	sigset_t					sig_blocked;
	sync_t						*mutex;
	unsigned					old_flags;
	uint64_t					timeout_time;
	unsigned					timeout_flags;
	unsigned					mutex_timeout_flags;
	struct sigevent			 	timeout_event;
};

static inline int
get_sigstub_fp_pc (mem_reader_t *rdr, bt_addr_t sp,
				   bt_addr_t *caller_fp, bt_addr_t *caller_pc,
				   bt_addr_t *next_caller_fp, bt_addr_t *next_caller_pc)
{
	ucontext_t uc;
	bt_addr_t uc_ptr_addr;
	uint32_t uc_addr;
	static const struct _sighandler_info si = {};

	/*
	 * Locate the offset of the pointer to the ucontext_t structure:
	 * fp: current stack pointer
	 * 4*18: offset of SIGSTACK from fp
	 *        There are 18 pushes in sh's _signalstub...
	 *     (_sighandler_info is the first field of SIGSTACK)
	 *   : offset of context in _sighandler_info)
	 */
	/*
	 * Locate the offset of the pointer to the "sigdeliver" structure:
	 */
	uc_ptr_addr = sp + 4*18 /*stack offset*/ +
		((bt_addr_t)&si.context - (bt_addr_t)&si);

	MEM_READ_VAR(uc_addr, uc_ptr_addr);

	/* Read entire ucontext_t from stack */
	MEM_READ_VAR(uc, (bt_addr_t)uc_addr);

	/*who's the caller*/
	(*caller_pc) = GET_REGIP(&(uc.uc_mcontext.cpu));
	(*caller_fp) = uc.uc_mcontext.cpu.gr[14];

	if (is_syscall(rdr,
				   (*caller_pc)-BT_GATHER_ADJUST_PC_TRAP)) {
		(*caller_pc) -= BT_GATHER_ADJUST_PC_TRAP;
		(*next_caller_pc) = uc.uc_mcontext.cpu.pr-BT_GATHER_ADJUST_PC;
		/* When a syscall is made (trapa), the fp is pushed right on
		 * (see is_syscall for assembly code) the stack last, so
		 * reading at caller_fp's address gets us the next caller
		 * fp's */
		MEM_READ_VAR(*next_caller_fp, *caller_fp);
	} else {
		(*caller_pc) -= BT_GATHER_ADJUST_PC;
		(*next_caller_pc) = 0;
	}

	return 1;
}

int
_BT(gather) (mem_reader_t *rdr,
			 bt_addr_t fp, bt_addr_t *pc, int pc_ary_sz, int *count,
			 bt_addr_t stack_limit, bt_addr_t last_pc)
{
	int ret;
	bt_addr_t caller_fp, caller_pc;
	bt_addr_t next_caller_fp, next_caller_pc = 0;
	bt_addr_t alternate_fp=0;   /* see _bt_find_prolog for explanation */

	while (fp) {
		if ((*count) >= pc_ary_sz) {
			break;
		}
		if (next_caller_pc) {
			/* The previous entry on the stack was a syscall, which
			   means that next caller was obtained by means other than
			   via find_prolog (see get_sigstub_fp_pc) */
			caller_pc = next_caller_pc;
			caller_fp = next_caller_fp;
			next_caller_pc = 0; /* clear it after use */
		} else if ((ret=is_in_sigstub(rdr, last_pc)) != 0) {
			if (ret == -1) return -1;

			/* Latest return address is to the sigstub.  fetch
			 * caller's stack frame and pc from the saved context (put
			 * by the kernel on the stack) */
			if (get_sigstub_fp_pc(rdr, alternate_fp,
								  &caller_fp, &caller_pc,
								  &next_caller_fp, &next_caller_pc) == -1)
				return -1;
			alternate_fp = 0;   /* clear after use */
		} else {
			ret=_BT(find_prolog)(rdr, fp, last_pc,
								 get_low_limit(last_pc),
								 &caller_fp, &caller_pc,
								 &alternate_fp);
			if (ret <= 0)
				return ret;
		}

		last_pc = caller_pc;
		if (stack_limit == 0 || caller_fp > stack_limit) {
			pc[*count] = caller_pc;
			(*count)++;
		}

		fp = caller_fp;

		if((caller_pc == 0) || (caller_fp == 0) ||
		   (caller_pc&0x1 != 0)) {
			return 0;
		}
	}

	return 0;
}

#ifndef _BT_LIGHT
int
_bt_gather_other (bt_accessor_t *acc,
				  mem_reader_t *rdr,
				  bt_addr_t *pc, int pc_ary_sz, int *count)
{
	bt_addr_t fp, pc_reg;
	procfs_greg greg;
	int greg_size;
	int ret;

	if (_bt_get_greg(rdr->fd, acc, &greg, &greg_size) == -1){
		return -1;
	}

	pc_reg = greg.sh.pc;
	fp = greg.sh.gr[14];
	pc[0]=(bt_addr_t)pc_reg; (*count)=1;

	if ((ret=is_syscall(rdr,
						pc_reg-BT_GATHER_ADJUST_PC_TRAP)) == -1)
		return -1;

	if (ret) {                  /* is syscall */
		pc[0] -= BT_GATHER_ADJUST_PC_TRAP;
		if ((*count) == pc_ary_sz)
			return 0;
		/* kernel calls have a different frame */
		pc[(*count)] = greg.sh.pr-BT_GATHER_ADJUST_PC;
		(*count) ++;
		/* When a syscall is made (trapa), the fp is pushed right on
		 * (see is_syscall for assembly code) the stack last, so
		 * reading at caller_fp's address gets us the next caller
		 * fp's */
		MEM_READ_VAR(fp, fp);
	} else {
		pc[0] -= BT_GATHER_ADJUST_PC;
	}
	if ((*count) == pc_ary_sz)
		return 0;

	return _bt_gather(rdr,fp,pc,pc_ary_sz,count,
					  acc->stack_limit,pc[*count-1]);
}

#endif
