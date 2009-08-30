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
vmm_debuginfo(PROCESS *prp, struct _mem_debug_info *info) {
	struct map_set			ms;
	struct mm_map			*mm;
	struct mm_object_ref	*or;
	OBJECT					*obp;
	int						r;

	r = map_isolate(&ms, &prp->memory->map, info->vaddr, info->size, MI_NONE);
	if(r == EOK) {
		mm = ms.first;
		if(mm != NULL) {
			if(mm->mmap_flags & MAP_ELF) {
				or = mm->obj_ref;
				if(or != NULL) {
					//RUSH3: Do we actually need to store this?
					//RUSH3: It just appears to be used by the old vmm_mapinfo().
					mm->reloc = info->vaddr - info->old_vaddr;
					obp = or->obp;
					if(obp->hdr.type == OBJECT_MEM_FD) {
						(void)memmgr_fd_setname(obp, info->path);
					}
				}
			}
		}
		map_coalese(&ms);
	}
	return r;
}

__SRCVERSION("vmm_debuginfo.c $Rev: 153052 $");
