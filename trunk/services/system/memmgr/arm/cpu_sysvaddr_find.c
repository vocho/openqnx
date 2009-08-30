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

#include <vmm.h>

uintptr_t
cpu_sysvaddr_find(uintptr_t start, unsigned size)
{
	unsigned	rover;
	unsigned	next;
	unsigned	found_va;
	unsigned	found_sz;
	unsigned	pg_size;
	int			is_free;
	int			wrapped;
	static unsigned	sysvaddr_end;

	if (sysvaddr_end == 0) {
		extern char	_btext[];

		/*
		 * Allow heap from ARM_STARTUP_BASE to start of procnto text
		 */
		sysvaddr_end = ((unsigned)_btext) & ~ARM_SCMASK;
	}

	if (start == 0 || start >= sysvaddr_end) {
		start = ARM_STARTUP_BASE;
	}
	rover = start;
	wrapped = 0;
	found_va = VA_INVALID;
	found_sz = 0;

	while (1) {
		is_free = 1;
		pg_size = (1 << 20);	// FIXME: need constant here?

		if (*KTOPDIR(rover) != 0) {
			if (*KTOPTEP(rover) != 0) {
				is_free = 0;
				found_sz = 0;
			}
			pg_size = __PAGESIZE;
		}
		next = (rover + pg_size) & ~(pg_size - 1);

		if (found_sz == 0) {
			found_va = rover;
		}
		if (is_free) {
			found_sz += next - rover;
		}
		if (found_sz >= size) {
			return found_va;
		}

		if (next >= sysvaddr_end) {
			wrapped = 1;
			next = ARM_STARTUP_BASE;
			found_sz = 0;
		}
		rover = next;
		if (wrapped && (rover >= start)) {
			return VA_INVALID;
		}
	}
}

__SRCVERSION("cpu_sysvaddr_find.c $Rev: 170757 $");
