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
vmm_validate(PROCESS *prp, uintptr_t vaddr, size_t len, int flags) {
	ADDRESS					*adp;
	int						r;
	struct map_set			ms;
	struct mm_map			*mm;
	struct mm_map			*next;
	uintptr_t				end;

	adp = prp->memory;
	end = vaddr + len - 1;

	if(flags & VV_RANGE) {
#if defined(CPU_GBL_VADDR_START)
		if((end < vaddr) ||
		   ((vaddr < adp->map.start) || (end > adp->map.end)) &&
		   ((vaddr < CPU_GBL_VADDR_START) || (end > CPU_GBL_VADDR_END))) {
#else
		if((end < vaddr) || (vaddr < adp->map.start) || (end > adp->map.end)) {
#endif
			// If VV_MAPPED is off, we're doing munmap(), which wants EINVAL
			return (flags & VV_MAPPED) ? ENOMEM : EINVAL;
		}
	}

	if(!(flags & VV_MAPPED)) return EOK;

	r = map_isolate(&ms, &adp->map, vaddr, len, MI_NONE);
	if(r != EOK) goto fail1;

	r = ENOMEM; // Assume something's going to mess up
	mm = ms.first;
	if(mm == NULL) goto fail2;
	if((vaddr < mm->start) || (end > ms.last->end)) goto fail2;

	for( ;; ) {
		// Attempting to fiddle with the system page address. 
		// No-no unless we're terminating.
		if((mm->extra_flags & EXTRA_FLAG_SPECIAL) && !(prp->flags & _NTO_PF_TERMING)) {
			goto fail2;
		}
		if(mm == ms.last) break;
		next = mm->next;
		if((mm->end + 1) != next->start) goto fail2;
		mm = next;
	}

	r = EOK; // Everything's good	
fail2:
	map_coalese(&ms);
fail1:
	return r;
}

__SRCVERSION("vmm_validate.c $Rev: 153052 $");
