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

#ifndef _GATHER_H_INCLUDED
#define _GATHER_H_INCLUDED

int
_BT(gather)(mem_reader_t *rdr,
			uint32_t esp,
			uint32_t ebp, bt_addr_t *pc, int pc_ary_sz, int *count,
			bt_addr_t stack_limit, bt_addr_t last_pc);
#ifndef _BT_LIGHT
int
_bt_gather_other(bt_accessor_t *acc,
				 mem_reader_t *rdr,
				 bt_addr_t *pc, int pc_ary_sz, int *count);
#endif
int
_BT(gather_adjust_x86_pc)(mem_reader_t *rdr, bt_addr_t pc);

#define BT_GATHER_SELF(err,rdr,pc,pc_ary_sz,count,stack_limit)\
	do { register uint32_t ebp;   /* base pointer */          \
	uint32_t caller_ebp;                                      \
	int adjust;                                               \
	bt_addr_t last_pc;                                        \
	/* Get bt_get_backtrace's own stack pointer */            \
	asm volatile("mov %%ebp,%0" : "=r"(ebp));                 \
	/* Get caller's stack pointer */                          \
	caller_ebp=*((uint32_t*)ebp);                             \
	/* get caller's pc. */                                    \
	last_pc=((uint32_t*)ebp)[1];                              \
	adjust = _BT(gather_adjust_x86_pc)(rdr, last_pc);         \
	last_pc -= adjust;                                        \
	if (stack_limit == 0) {                                   \
		/* No need to check ebp against stack_limit,          \
	     * because from within bt_get_backtrace, it is        \
	     * certain that ebp < stack_limit */                  \
	    pc[0]=last_pc;                                        \
		count=1;                                              \
	}                                                         \
	if (_BT(gather)(rdr,0,caller_ebp,pc,pc_ary_sz,&count,     \
	                stack_limit,last_pc)==-1)                 \
		err=errno;                                            \
	} while(0)

#endif
