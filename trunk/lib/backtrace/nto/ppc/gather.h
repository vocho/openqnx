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
			uint32_t sp, bt_addr_t *pc, int pc_ary_sz, int *count,
			bt_addr_t stack_limit, bt_addr_t last_pc);
#ifndef _BT_LIGHT
int
_bt_gather_other(bt_accessor_t *acc,
				 mem_reader_t *rdr,
				 bt_addr_t *pc, int pc_ary_sz, int *count);
#endif

/*
 * return address is 4 bytes passed the branch
 */
#define BT_GATHER_ADJUST_PC 4

#define BT_GATHER_SELF(err,rdr,pc,pc_ary_sz,count,stack_limit)\
	do { uint32_t sp, caller_sp;                              \
	bt_addr_t last_pc;                                        \
	/* Get bt_get_backtrace's own stack pointer */            \
	asm volatile("stw %%r1,%0" : "=m"(sp));                   \
	/* Get caller's stack pointer */                          \
	caller_sp=*((uint32_t*)sp);                               \
	/* get caller's pc.  We assume the lr has been saved */   \
	/* since bt_get_backtrace isn't a leaf function */        \
	last_pc=((uint32_t*)caller_sp)[1]-BT_GATHER_ADJUST_PC;    \
	if (stack_limit == 0) {                                   \
		/* No need to check caller_sp against stack_limit,    \
	     * because from within bt_get_backtrace, it is        \
	     * certain that caller_sp < stack_limit (if           \
	     * stack_limit is specified) */                       \
		pc[0] = last_pc;                                      \
		count=1;                                              \
	}                                                         \
	if (_BT(gather)(rdr,caller_sp,pc,pc_ary_sz,&count,        \
					stack_limit, last_pc)==-1)                \
	    err=errno;                                            \
	} while(0)

#endif
