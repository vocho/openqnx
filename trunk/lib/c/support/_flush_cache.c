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




#include <sys/mman.h>

/*
 * This function is called by GCC when it generates some code on the
 * fly (trampoline code for nested functions). It needs to make sure
 * the data & instruction caches are all up to date.
 */

void
_flush_cache(const void *addr, size_t len, unsigned flags) {
	// In the future we should find out which of the flag bits indicate
	// a flush of the dcache & which is the icache. We don't care right
	// now since MS_INVALIDATE_ICACHE knows to flush the dcache only
	// when needed.
	(void)msync((void *)addr, len, MS_INVALIDATE_ICACHE);
}

__SRCVERSION("_flush_cache.c $Rev: 153052 $");
