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

#include "externs.h"
#include <sys/mman.h>
#include <unistd.h>
// FIX ME #include <sys/mempart.h>
#include "kernel/mempart.h"


static unsigned
_dummy_alloc(unsigned size, void **addr) {
	// Can't get anything more from the system, since it's all been
	// put in the heap.
	return 0;
}

int
emm_mmap(PROCESS *prp, uintptr_t addr, size_t len, int prot, int flags, 
		OBJECT *obpe, uint64_t boff, unsigned align, unsigned preload, int fd, 
		void **vaddr, size_t *size, part_id_t mpart_id) {
	unsigned		off = (unsigned)boff;

//NYI: Doesn't handle PROT_NOCACHE	
	// Physical can't do guardpages, so remove them from the len
	if(flags & MAP_STACK) {
		if(off >= len) {
			return EINVAL;
		}
		len -= off;
	}

	// Allocate memory
	if(flags & MAP_ANON) {
		len = (len + memmgr.pagesize - 1) & ~(memmgr.pagesize - 1);
		for( ;; ) {
			*vaddr = _sreallocfunc(0, 0, len, _dummy_alloc);
			if(*vaddr != NULL) break;
			if(!purger_invoke(len)) {
				return ENOMEM;
			}
		}
		*size = len;
		return EOK;
	}

	// since physical == virtual, return offset
	if((flags & (MAP_PHYS | MAP_TYPE)) == (MAP_PHYS | MAP_SHARED)) {
 		*vaddr = (void *)CPU_P2V(off);
		*size = len;
		return EOK;
	}

	return EINVAL;
}

__SRCVERSION("emm_mmap.c $Rev: 168445 $");
