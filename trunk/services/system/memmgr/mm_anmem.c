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

#include "vmm.h"



int
anmem_create(OBJECT *obp, void *extra) {
	ADDRESS *adp = extra;
	//RUSH2: initialize available offset tracking fields...

	//RUSH2: need a way to inherit/modify the restrict list.

	if(adp->flags & MM_ASFLAG_PADDR64_SAFE) {
		obp->mem.mm.restriction = restrict_user_paddr64;
	} else {
		obp->mem.mm.restriction = restrict_user;
	}
	obp->mem.mm.pmem_cache = &obp->mem.mm.pmem;
	return EOK;
}

int
anmem_done(OBJECT *obp) {
	if(obp->mem.mm.refs != NULL) return 0;
	if(obp->mem.mm.pmem != NULL) {
#ifndef NDEBUG		
		// We can't lock since the aspace might be 
		// being destroyed and the __tls()->owner for mutex acquisition will
		// not get the right value, so we turn on the MM_MEM_SKIPLOCKCHECK
		// to prevent crash()'s. It's safe since nobody else is
		// referencing this object.
		obp->mem.mm.flags |= MM_MEM_SKIPLOCKCHECK; 
#endif		
		memobj_pmem_del_len(obp, 0, obp->mem.mm.size);
	}
	return 1;
}


size_t
anmem_name(OBJECT *obp, size_t max, char *name) {
	return 0;
}


off64_t
anmem_offset(OBJECT *obp, uintptr_t va, size_t size) {
	uintptr_t	end = va + size;

	VERIFY_OBJ_LOCK(obp);
	//RUSH2: Using the vaddr as the offset won't work once we
	//RUSH2: start supporting /proc/<pid>/as, since the pmem for the
	//RUSH2: va might still be mapped via another process.
	//RUSH2: When we switch, have to deal with MAP_LAZY somehow.
	//RUSH2: Maybe keep the offset, but create multiple anon objects instead?

	//RUSH2: There is code in ms_unmap() (vmm_munmap.c) that assumes that
	//RUSH2: there's only one anonymous object and that anmem_offset()
	//RUSH2: returns the same offset every time we pass in the same 'va'
	//RUSH2: (the code for freeing the preallocated ~MAP_LAZY, MAP_PRIVATE
	//RUSH2: privatize() memory). If those assumptions change, we have
	//RUSH2: to modify that code. Ditto with privatize() in mm_reference.c
	if(end > obp->mem.mm.size) obp->mem.mm.size = end;
	return va;
}


void
anmem_unoffset(OBJECT *obp, off64_t off, size_t size) {
	VERIFY_OBJ_LOCK(obp);

	//RUSH2: nothing to do right now. See above.
	//RUSH2: Might turn out better passing the va in, rather than the offset.
}

__SRCVERSION("mm_anmem.c $Rev: 211761 $");
