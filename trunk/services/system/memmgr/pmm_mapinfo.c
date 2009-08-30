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

#include <sys/procfs.h>
#include "pmm.h"

static int
pmm_check(uintptr_t vaddr, struct mem_phys_entry *mem, unsigned memaddr, struct mem_phys_entry **closest, unsigned *addr) {
	if(vaddr >= memaddr) {
		if(vaddr < memaddr + mem->size) {
			*addr = memaddr;
			return 1;
		}
	} else if((!*closest || memaddr < *addr) && !(mem->flags & MAP_ELF)) {
		*closest = mem;
		*addr = memaddr;
	}
	return 0;
}

size_t
pmm_mapinfo(PROCESS *prp, uintptr_t vaddr, struct _procfs_map_info *mip, 
		struct _procfs_debug_info *mdp, size_t dbginfo_pathlen, OBJECT **obpp, int *fdp, size_t *contigp) {
	struct mem_phys_entry			*mem, *closest, text_tmp, data_tmp;
	uintptr_t						addr;

	if(obpp != NULL) *obpp = NULL;
	closest = 0;
	addr = 0;
	for(mem = (struct mem_phys_entry *)prp->memory; mem; mem = mem->next) {
		if(pmm_check(vaddr, mem, (unsigned)(mem + 1), &closest, &addr)) {
			break;
		}
	}

	if(!mem && (!closest || (!(closest->flags & MAP_ELF) && closest->reloc == 0))) {
		text_tmp.flags = MAP_ELF | MAP_PRIVATE;
		text_tmp.reloc = prp->kdebug.text_reloc;
		text_tmp.size = prp->kdebug.text_size;
		if(text_tmp.size && pmm_check(vaddr, &text_tmp, prp->kdebug.text_addr, &closest, &addr)) {
			mem = &text_tmp;
		} else {
			data_tmp.flags = MAP_ELF | MAP_PRIVATE;
			data_tmp.reloc = prp->kdebug.data_reloc;
			data_tmp.size = prp->kdebug.data_size;
			if(data_tmp.size && pmm_check(vaddr, &data_tmp, prp->kdebug.data_addr, &closest, &addr)) {
				mem = &data_tmp;
			}
		}
	}

	if(mem || (mip && (mem = closest))) {
		if(mip) {
			mip->vaddr = addr;
			vaddr = addr;
			mip->flags = mem->flags;
			mip->size = mem->size;
			mip->offset = 0;
			if(mem->reloc || (mem->flags & MAP_ELF)) {
				mip->dev = 1;
				mip->ino = PID_TO_INO(prp->pid, 0);
			} else if((mem->flags & MAP_TYPE) == MAP_PRIVATE) {
				mip->dev = 2;
				mip->ino = 1;
			} else {
				mip->dev = 3;
				mip->ino = 1;
			}
		}
		if(mdp) {
			mdp->vaddr = vaddr - mem->reloc;
			if(mem->reloc || (mem->flags & MAP_ELF)) {
				if (prp->debug_name)
					STRLCPY(mdp->path, prp->debug_name, dbginfo_pathlen);
				else
					mdp->path[0] = '\0';
			} else if((mem->flags & MAP_TYPE) == MAP_PRIVATE) {
				STRLCPY(mdp->path, "/dev/zero", dbginfo_pathlen);
			} else {
				STRLCPY(mdp->path, "/dev/mem", dbginfo_pathlen);
			}
		}
		return (mem->size - vaddr) - (mip ? mip->vaddr : vaddr);
	}
	return 0;
}

__SRCVERSION("pmm_mapinfo.c $Rev: 203538 $");
