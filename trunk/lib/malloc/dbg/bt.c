/*
 * $QNXtpLicenseC:
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




#include <stdlib.h>
#include <sys/types.h>
#include <sys/storage.h>

ulong_t bt_stack_from = 0;

int
backtrace (void **buffer, int size, int caller_addr)
{
	bt_addr_t _bt[size+10];
	int i;
	int save;
	int cnt;
	int realcnt = 0;

	// assume size >= 2...
	
	buffer[realcnt++] = caller_addr;
	if (caller_addr == 0)
		return realcnt;

	/* Collect a backtrace larger than 'size', and then exclude
	   internal functions from the trace back... */
	cnt = btl_get_backtrace(&btl_acc_self, _bt, (sizeof(_bt)/sizeof(bt_addr_t)));
	if (cnt > 0) {
		for (save=0,i=1; i<cnt && realcnt<size; i++) {
			if (save) {
				buffer[realcnt++] = _bt[i]; /* caller addr and beyond... */
			} else if (_bt[i] == (bt_addr_t)buffer[0]) {
				save=1;
			}
		}
	}
	/* always have the last entry be NULL.  libmalloc code relies on
	 * this, since the actual backtrace count isn't given back to the
	 * caller of backtrace()
	 *
	 * NOTE: Also see MALLOC_GETBT... unfortunately, the handling of
	 * when a NULL is needed is confusing.  It appears that there
	 * should be one, unless there are exactly CALLERPCDEPTH entry on
	 * the trace back... this needs to be looked into...
	 */
	if (realcnt < size)
		_bt[realcnt] = 0;
	return realcnt;
}

// Used for C++
void *
libmalloc_caller_fn (void)
{
	// Since this is a function, the function calling the caller of
	// libmalloc_caller_fn() is the 3rd entry in the backtrace.
	// e.g.:
	//   userFunction()                    _bt[2]
	//   -> malloc()                       _bt[1]
	//      -> libmalloc_caller_fn()       _bt[0]
	//         -> btl_get_backtrace()
	//      
	bt_addr_t _bt[3]={0,0,0};
	btl_get_backtrace(&btl_acc_self,_bt,3);
	return _bt[2];
}
