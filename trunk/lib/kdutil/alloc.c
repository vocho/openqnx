/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
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

#include "kdintl.h"

#define ROUND(v, r)		((v) + ((r)-1) & ~((r)-1))

struct meminfo {
	struct asinfo_entry	*as;
	unsigned			size;
	paddr64_t			align;
	paddr64_t			start;
};


static int
carve_mem(struct asinfo_entry *as, char *name, void *data) {
	struct meminfo	*mem = data;
	paddr64_t		start;

	start = ROUND(as->start, mem->align);

	if(((as->end - start) > mem->size) && (mem->start > start)) {
		mem->start = start;
		mem->as = as;
	}
	return 1;
}


paddr64_t	
alloc_pmem(unsigned size, unsigned align) {
	struct meminfo	mem;

	if(private->kdebug_info != 0) {
		// if kdebug_info is filled in, the kernel has started and
		// we can't allocate any more memory
		return ~(paddr64_t)0;
	}

	if(align == 0) align = sizeof(uint64_t);
	mem.as = NULL;
	mem.start = ~(paddr64_t)0;
	mem.align = align;
	mem.size = ROUND(size, align);

	walk_asinfo("sysram", carve_mem, &mem);
	if(mem.as == NULL) {
		// no memory
		return ~(paddr64_t)0;
	}

	mem.as->start = mem.start + mem.size;
	return mem.start;
}
