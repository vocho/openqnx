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

#include "debug.h"

int
_BT(gather)(mem_reader_t *rdr,
			uint32_t sp, bt_addr_t *pc, int pc_ary_sz, int *count,
			bt_addr_t stack_limit, bt_addr_t last_pc);
#ifndef _BT_LIGHT
int
_bt_gather_other(bt_accessor_t *acc,
				 mem_reader_t *rdr,
				 bt_addr_t *pc, int pc_ary_sz, int *count);
#endif
int
_BT(find_prolog)(mem_reader_t *rdr,
				 bt_addr_t pc, bt_addr_t lowlimit,
				 int *stack_incr, int *ra_offset);

/*
 * return address is 8 bytes passed the branch (branch instruction +
 * instruction after branch)
 */
#define BT_GATHER_ADJUST_PC 8

#define BT_GATHER_MAKE_LABEL() start_of_get_backtrace:

#define BT_GATHER_SELF(err,rdr,pc,pc_ary_sz,count,stack_limit)       \
	do { uint32_t sp, caller_sp;                                     \
	bt_addr_t my_pc, caller_pc;                                      \
	int ret, stack_incr, ra_offset;                                  \
	if (pc_ary_sz <= 0) { count = 0; break; }                        \
	/* Get bt_get_backtrace's own stack pointer */                   \
	/* May not be accurate, but that's the best */                   \
	/* can be done */                                                \
	asm volatile("sw $sp,%0" : "=m"(sp));                            \
	my_pc=(bt_addr_t)&&start_of_get_backtrace;                       \
	ret=_BT(find_prolog)(rdr, my_pc, (bt_addr_t)BT(get_backtrace),   \
						 &stack_incr, &ra_offset);                   \
	if (ret == 1) {                                                  \
		caller_sp=sp-stack_incr;                                     \
        caller_pc=((uint32_t*)(sp+ra_offset))[0]-BT_GATHER_ADJUST_PC;\
		/* get caller's pc. */                                       \
		if (stack_limit == 0) {                                      \
			/* No need to check caller_sp against stack_limit,       \
	    	 * because from within bt_get_backtrace, it is           \
	    	 * certain that caller_sp < stack_limit (if stack_limit  \
	    	 * is specified) */                                      \
			pc[0]=caller_pc;                                         \
			count=1;                                                 \
		}                                                            \
		if (_BT(gather)(rdr,caller_sp,pc,pc_ary_sz,&count,           \
						stack_limit,caller_pc)==-1)                  \
			err=errno;                                               \
	} else if (ret==-1)                                              \
		err=errno;                                                   \
	/* else can't backtrace, but that's ok... */                     \
	} while(0)

#endif
