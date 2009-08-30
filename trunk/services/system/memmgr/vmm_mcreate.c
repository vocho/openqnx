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
#include <spawn.h>

int 
vmm_mcreate(PROCESS *prp) {
	ADDRESS 	*adp;
	OBJECT		*anon;
	int			r;
	uintptr_t	end;
#if CPU_USER_VADDR_START == 0
	struct map_set	ms;
	struct map_set	repl;
#endif

	r = ENOMEM;	// Start by assuming out of memory.	
	adp = object_to_data(prp, address_cookie);

	// The -mP procnto option was _not_ specified, mark any executable
	// that the user has specifically said can handle physical memory
	// above 4G.
	if(!(mm_flags & MM_FLAG_PADDR64_SAFE_SYS) && (prp->lcp != NULL)) {
		if(prp->lcp->flags & SPAWN_PADDR64_SAFE) {
			adp->flags |= MM_ASFLAG_PADDR64_SAFE;
		} else if((prp->lcp->state & LC_STATE_MASK) == LC_FORK) {
			PROCESS *parent = lookup_pid(prp->lcp->ppid);

			CRASHCHECK(parent == NULL);
			adp->flags |= parent->memory->flags & MM_ASFLAG_PADDR64_SAFE;
		}
	}

	anon = object_create(OBJECT_MEM_ANON, adp, prp, sys_memclass_id);

	if(anon == NULL) goto fail1;
	adp->anon = pathmgr_object_clone(anon);
	// Nip off a couple of the vaddrs at the end to use as a last chance
	// area for temp mappings. See mm_temp_map.c for details.
	end = ((CPU_USER_VADDR_END) - ((colour_mask_shifted | (__PAGESIZE-1))+1)) & ~colour_mask_shifted;
	r = map_init(&adp->map, CPU_USER_VADDR_START, end);
	if(r != EOK) goto fail2;
#if CPU_USER_VADDR_START == 0
		// pre-allocate the zero vaddr so user doesn't get it from a mmap()
		// unless he explicitly asks for it.

		r = map_create(&ms, &repl, &adp->map, 0, __PAGESIZE, 0, MAP_FIXED);
		if(r != EOK) goto fail3;
		r = map_add(&ms);
		if(r != EOK) goto fail4;
		adp->rlimit.vmem = __PAGESIZE;
#endif
	prp->memory = adp;
	r = cpu_vmm_mcreate(prp);
	if(r != EOK) goto fail5;

	if(mm_flags & MM_FLAG_SUPERLOCKALL) {
		adp->flags |= MM_ASFLAG_ISR_LOCK|MM_ASFLAG_LOCKALL;
	} else if(mm_flags & MM_FLAG_LOCKALL) {
		adp->flags |= MM_ASFLAG_LOCKALL;
	}
	return EOK;

fail5:
	prp->memory = NULL;

#if CPU_USER_VADDR_START == 0
	map_remove(&ms);

fail4:	
	map_destroy(&ms);

fail3:
#endif	
	(void)map_fini(&adp->map);

fail2:
	pathmgr_object_done(anon);

fail1:
	return r;
}

__SRCVERSION("vmm_mcreate.c $Rev: 209133 $");
