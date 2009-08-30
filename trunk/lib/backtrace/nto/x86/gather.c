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
#include "debug.h"
int printf(const char *, ...);

/*
 * x86 standard stack frame:
 *
 * Position             Contents                | Frame
 * -------------------+-------------------------+-----------
 * 4n+8(%ebp)         | arg word n              |            high addresses
 *                    | ...                     | Previous
 *                    | arg word 0              |
 * -------------------+-------------------------+-----------
 *    4(%ebp)         | return address          |
 *                    +-------------------------+
 *    0(%ebp)         | previous %ebp           |
 *                    +-------------------------+ Current
 *   -4(%ebp)         | unspecified             |
 *                    | ...                     |
 *    0(%esp)         | variable size           |            low addresses
 *--------------------+-------------------------+-----------
 *
 *
 *
 *
 * x86 __signalstub stack
 *
 * Position             Contents                | Frame
 * -------------------+-------------------------+-----------
 * 4n+8(%ebp)         | arg word 2              |            high addresses
 *                    | arg word 1              | Previous
 *                    | arg word 0              |
 * -------------------+-------------------------+-----------
 *                    | return address          |
 *                    +-------------------------+
 *    0(%ebp)         | previous %ebp           |
 *                    +-------------------------+ Current
 *                    | unspecified             |
 *                    | ...                     |
 *                    | variable size           |            low addresses
 *--------------------+-------------------------+-----------
 * Note:
 *  - previous %ebp is not relevant for __signal stub.
 *  - In order to unwind the stack to obtain the value of %esp prior
 *  to __signalstub pushing the args and calling the signal handler
 *  function it is necessary to count all the push, and add them to
 *  the sig handler function's %ebp.
 *
 *
 *
 *
 * x86 System call stack
 *
 * Position             Contents                | Frame
 * -------------------+-------------------------+-----------
 *                    | return address          | Current
 *--------------------+-------------------------+-----------
 *
 * - A system call does not set a %ebp, so whatever the current value
 *   of %ebp is while in a system call, it is the %ebp of the caller.
 * - For a system call, %esp is necessary to fetch the return address.
 * - So, caller_pc = return address - adjust
 *       caller_ebp = ebp
 *
 */ 


/*
 * On x86, the preceding call instruction could be 3 bytes (16bits
 * relative offset call) or 5 bytes (32bits absolute).  The adjustment
 * must be accordingly.
 */ 
int
_BT(gather_adjust_x86_pc) (mem_reader_t *rdr, bt_addr_t pc)
{
	uint8_t byte;
	MEM_READ_VAR(byte,pc-5);

	/* if 5th byte before pc is e8, then it has to be a 5bytes call,
	 * since if it wasn't then it would mean a 2 bytes instruction
	 * starting with e8, followed by a 3bytes call, and a 2 bytes
	 * instruction starting ff doesn't exist */
	if (byte == 0xe8)
		return 5;
	else
		return 3;
}

static int
is_syscall (mem_reader_t *rdr, bt_addr_t pc)
{
	bt_signature_u16_t sig[] = {
		{.val=0x340f, .mask=0xffff} // sysenter
	};
	BT_SIG_LEN(sig);
	MEM_BUF(uint16_t,sig_len);

	MEM_GET(pc-2,MEM_BUF_SZ);

	return BT_SIG16_MATCH(sig, MEM_P);
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
#if 0
	const char path_self[] = "/proc/self/as";
	const bt_addr_t __signalstub_size = 0x40; /* Empirically determined by 
												 disassembling __signalstub */
	debug_process_t status;
	int ret = -1;
	int fd_self = open(path_self, O_RDWR);
	_Uint64t sigstub;
	if (fd_self == -1)
	  return ret;

	if (devctl(fd_self, DCMD_PROC_STATUS, &status, sizeof(status), 0) != EOK)
	  goto exit_error;

	if (status.sigstub < pc 
		&& (status.sigstub + __signalstub_size) > pc)
	  ret = 1;
	else
	  ret = 0;

exit_error:
	close(fd_self);
	return ret;
#else

#if 0
	/* Assembler code in _signalstub.
	 *  This is obviously fuzzy match, at best...
	 */
	static bt_signature_u32_t sig[] = {
		/* x86 is little endian, so when dealing with 32 bit
		 * quantities, the order of bytes are reversed compared to,
		 * say, what objdump gives */
		{.val=0x560056ff, .mask=0xff00ffff}, /* call *0x28(%esi)
											  * push %esi */
		{.val=0x788bf889, .mask=0xffffffff}, /* mov %edi,%eax
											  * etc. */
		{.val=0x1c708b18, .mask=0xffffffff},
		{.val=0x8b20688b, .mask=0xffffffff},
		{.val=0x508b2858, .mask=0xffffffff},
		{.val=0x30488b2c, .mask=0xffffffff},
	};
	BT_SIG_LEN(sig);
	MEM_BUF(uint32_t,sig_len);

	if (pc == 0) return 0;

	MEM_GET(pc,MEM_BUF_SZ);

	return BT_SIG32_MATCH(sig, MEM_P);
#else
	/* The following assumes the PC is return address, i.e. address
	   just after the call to signal handler. If the bytes match (bytes
	   are taken directly from objdump of __signalstub), we conclude
	   the frame belongs to the signal trampoline.  */
	const unsigned char sig[] = { 0x56, 0x89, 0xf8, 0x8b, 
								  0x78, 0x13, 0x8b, 0x70, 
								  0x1c, 0x8b, 0x68, 0x20,
								  0x8b, 0x58, 0x28, 0x8b,
								  0x50, 0x2c, 0x86, 0x48 };
	const unsigned char * const pc_ = (unsigned char const *) pc;
	unsigned int i;
	for (i = 0; i != sizeof(sig)/sizeof(sig[0]); ++i)
	{
	  if (sig[i] != pc_[i])
		return 0;
	}
	return 1;
#endif
#endif
}

static inline int
get_sigstub_ebp_pc (mem_reader_t *rdr,
					bt_addr_t sp,
					uint32_t *caller_esp,
					uint32_t *caller_ebp, uint32_t *caller_pc)
{
	ucontext_t uc;
	bt_addr_t uc_ptr_addr;
	uint32_t uc_addr;
	uint32_t eip;
	int adjust, ret;
	static struct _sighandler_info si = {};

	/*
	 * Locate the offset of the pointer to the ucontext_t structure:
	 * sp: current stack pointer
	 * 20: offset of SIGSTACK from sp
	 *     __signalstub pushes 3 args, + ret addr, and called function
	 *     pushes ebp => 5*4bytes => 20
	 *     (_sighandler_info is the first field of SIGSTACK)
	 *   : offset of context in _sighandler_info)
	 */
	uc_ptr_addr = sp + 20 /*stack offset*/ +
		((bt_addr_t)&si.context - (bt_addr_t)&si);

	MEM_READ_VAR(uc_addr, uc_ptr_addr);

	/* Read entire ucontext_t from stack */
	MEM_READ_VAR(uc, uc_addr);

	(*caller_ebp) = uc.uc_mcontext.cpu.ebp;
	eip = uc.uc_mcontext.cpu.eip;
	if ((ret=is_syscall(rdr, eip)) == -1)
		return -1;
	if (ret) {                  /* syscall */
		adjust = 2;
		(*caller_esp) = uc.uc_mcontext.cpu.esp;
	} else {                    /* not syscall */
		adjust = _BT(gather_adjust_x86_pc)(rdr, eip);
		(*caller_esp) = 0;
	}
	(*caller_pc) = eip - adjust;

	return 1;
}

int
_BT(gather) (mem_reader_t *rdr,
			 uint32_t esp, uint32_t ebp,
			 bt_addr_t *pc, int pc_ary_sz, int *count,
			 bt_addr_t stack_limit, bt_addr_t last_pc)
{
	uint32_t caller_esp = 0;
	uint32_t caller_ebp;
	uint32_t old_ebp = 0;
	uint32_t caller_pc;
	uint32_t return_addr;
	int adjust, ret;
	int store_frame = (stack_limit == 0);
 
	if (BT(gather_hook)) {
		BT(gather_hook)(ebp);
	}
	while (ebp) {
		if ((*count) >= pc_ary_sz) {
			break;
		}

		if ((ret=is_in_sigstub(rdr, last_pc)) != 0) {
			if (ret == -1) return -1;

			if (!old_ebp)
				return -1;

			/* Latest return address is to the sigstub.  fetch
			 * caller's stack frame and pc from the saved context (put
			 * by the kernel on the stack) */
			if (get_sigstub_ebp_pc(rdr, old_ebp,
								   &caller_esp,
								   &caller_ebp, &caller_pc) == -1) {
				return -1;
			}
		} else {
			/* +2 because is_syscall assumes pc is after the sysenter */
			if ((ret=is_syscall(rdr, last_pc+2))==-1)
				return -1;
			if (ret) {          /* system call */
				/* system calls do not push ebp, so the ebp is that of
				 * the caller, and the return addr (aka caller_pc) is
				 * obtained via esp */
				MEM_READ_VAR(return_addr, (bt_addr_t)((uint32_t*)esp));
				caller_ebp = ebp;
			} else {
				MEM_READ_VAR(caller_ebp, (bt_addr_t)((uint32_t*)ebp));
				MEM_READ_VAR(return_addr, (bt_addr_t)(((uint32_t*)ebp)+1));
			}
			adjust = _BT(gather_adjust_x86_pc)(rdr, return_addr);
			caller_pc = return_addr - adjust;
		}

		last_pc = caller_pc;

		if (store_frame) {
			pc[*count] = caller_pc;
			(*count)++;
		} else if (caller_ebp > stack_limit) {
			/* first time that ebp is > stack_limit means we're in the
			 * last frame to exclude.  Next frame will be stored */
			store_frame = 1;
		}

		old_ebp = ebp;
		ebp = caller_ebp;
		esp = caller_esp;

	}
	return 0;
}

#ifndef _BT_LIGHT

int
_bt_gather_other (bt_accessor_t *acc,
				  mem_reader_t *rdr,
				  bt_addr_t *pc, int pc_ary_sz, int *count)
{
	bt_addr_t ebp, eip, esp;
	procfs_greg greg;
	int greg_size, ret, adjust;
	if (_bt_get_greg(rdr->fd, acc, &greg, &greg_size) == -1)
		return -1;
	ebp=GETREG(&greg,2);
	eip=GETREG(&greg,8);
	esp=GETREG(&greg,11);
	pc[0]=eip; (*count)=1;
	if ((*count) == pc_ary_sz)
		return 0;
	if ((ret=is_syscall(rdr, pc[0])) == -1)
		return -1;
	if (ret) {                  /* syscall */
		adjust = 2;
	} else {                    /* not syscall */
		adjust = _bt_gather_adjust_x86_pc(rdr, pc[0]);
	}
	pc[0] -= adjust;

	return _bt_gather(rdr, esp, ebp, pc, pc_ary_sz, count,
					  acc->stack_limit, pc[(*count)-1]);
}

#endif
