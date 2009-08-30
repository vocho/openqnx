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
#include "vmm.h"

struct vaddr_range	system_code_vaddr;
struct vaddr_range	system_data_vaddr;

struct pmem_data {
	off64_t		off;
	paddr_t		paddr_next;
	unsigned	flags;
	size_t		len;
	void		*mip;
	int			need_contig;
};


static int
pmem_info(OBJECT *obp, off64_t off, struct pa_quantum *pq, unsigned num, void *d) {
	struct pmem_data	*data = d;
	paddr_t				paddr;
	unsigned			len;
	unsigned			flags;
	unsigned			curr_flags;
	unsigned			i;

	len = NQUANTUM_TO_LEN(num);

	if(pq == NULL) {
		if(data->len == 0) {
			data->len = len;
		}
		return -1;
	}

	paddr = pa_quantum_to_paddr(pq);
	if((data->len != 0) && data->need_contig && (paddr != data->paddr_next)) {
		return -1;
	}
	if(pq->blk == PAQ_BLK_FAKE) {
		flags = PG_HWMAPPED|PG_REFERENCED|PG_MODIFIED;
		if(data->len == 0) {
			unsigned skip = data->off - off;

			//We only have a single pa_quantum_fake to cover the whole
			//of the run, so we need to adjust the paddr with the
			//offset from the start.
			len -= skip;
			paddr += skip;
		}
	} else {
#ifdef FULL_PG_FLAGS
		flags = 0;
#else
		flags = PG_HWMAPPED|PG_REFERENCED|PG_MODIFIED;
#endif
		if(data->mip != NULL) {
			for(i = 0; i < num; ++i, ++pq) {
				curr_flags = 0;
				if(pq->flags & PAQ_FLAG_INITIALIZED) curr_flags |= PG_HWMAPPED|PG_REFERENCED;
				if(pq->flags & PAQ_FLAG_MODIFIED)    curr_flags |= PG_MODIFIED;
#ifdef FULL_PG_FLAGS
				if(i == 0) flags = curr_flags;
				if(flags != curr_flags) break;
#endif
			}
			len = NQUANTUM_TO_LEN(i);
		}
	}
	if(data->len == 0) {
		data->flags = flags;
	}
	if(flags != data->flags) {
		return -1;
	}
	data->len += len;
	data->paddr_next = paddr + len;
	return EOK;
}


/*
Entry:
	prp		locked process pointer to lookup mapping on
	vaddr	virtual address to lookup within process
	mip		map info pointer (If non-zero vaddr matching will match next higher entry. i.e. next valid vaddr for process)
	mdp		map debug pointer (stuffed with debugger vaddr and debugger file name)
	obpp    stuffed with OBJECT pointer for the region
	fdp     stuffed with file descriptor for the region
	contigp stuffed with max contiguous length of the region
Returns:
	number of valid bytes from vaddr (or start of returned mapping if vaddr is not within any mappings) to end of mapping
*/

size_t 
vmm_mapinfo(PROCESS *prp, uintptr_t vaddr, struct _procfs_map_info *mip, 
					struct _procfs_debug_info *mdp, size_t dbginfo_pathlen, OBJECT **obpp, 
					int *fdp, size_t *contigp) {
	ADDRESS						*adp;
	struct _procfs_map_info		info;
	struct map_set				ms;
	struct mm_map				*mm;
	struct mm_map				*name_mm;
	struct mm_object_ref		*or;
	OBJECT						*obp;
	struct pmem_data			data;
	uintptr_t					chk_vaddr;

	if(obpp != NULL) *obpp = NULL;
	if(fdp != NULL) *fdp = NOFD;
	adp = prp->memory;
	if(adp == NULL) return 0;

	memset(&info, 0, sizeof(info));

	if(prp->pid == SYSMGR_PID) {
		if(vaddr == prp->base_addr) {
			vaddr = 0;
		}
		info.dev = 1;
		info.ino = PID_TO_INO(SYSMGR_PID, 0);
		//RUSH1: need special handling to get info on system address space.
		//RUSH1: Information contained in system_heap_vaddr, system_code_vaddr
		//RUSH1: and system_data_vaddr variables - for the heap, additional
		//RUSH1: inspection of the physical allocator structures is required
/*
Things to fill in:
			info (*mip):
	_Uint64t					vaddr;
	_Uint64t					size;
	_Uint32t					flags;
	dev_t						dev;
	off_t						offset;
	ino_t						ino;

		if(mdp != NULL) {
			mdp->vaddr = vaddr;
			if(mem from heap) {	
				mdp->path[0] = '\0';
			} else {
				STRLCPY(mdp->path, procnto_prp->debug_name, dbginfo_pathlen);
			}
		}
*/	
		return 0;
	}
	chk_vaddr = vaddr;
	for( ;; ) {
		map_isolate(&ms, &adp->map, chk_vaddr, 0, MI_NEXT);
		mm = ms.first;
		if(mm == NULL) return 0;
		or = mm->obj_ref;
		if(or != NULL) break;
		if(mm->mmap_flags & MAP_STACK) break; // Need to report guard pages
		map_coalese(&ms);
		chk_vaddr = mm->end + 1;
	}
	if(vaddr < mm->start) {
		vaddr = mm->start;
	}
	info.vaddr = vaddr;
	info.size = mm->end + 1 - vaddr;
	info.flags = mm->mmap_flags;
	if(or != NULL) {
		obp = or->obp;
		memobj_lock(obp);
		if(obpp != NULL) *obpp = obp;
		if(fdp != NULL) *fdp = or->fd;
		name_mm = mm;
		if((mm->mmap_flags & MAP_ELF) && (mm->mmap_flags & PROT_WRITE)) {
			struct map_set				pms;
			struct mm_map				*prev_mm;
			/*
			 * For MAP_ELF objects, we need to set the ino & name to the base
			 * of the text segment so pidin/qconn/IDE can match up text/data
			 * of a given shared lib.
			 * RUSH - as pointed below, we really need to keep a 
			 * reference to the object/shared lib for the data segment.
			 * This code is just heuristics and not guaranteed
			 * to always work.
			 */

			/* This is the data segment - text should be before */
			prev_mm = mm;
			for( ;; ) {
				map_isolate(&pms, &adp->map, prev_mm->start - 1, 0, MI_NEXT);
				if(pms.first == prev_mm) break;
				prev_mm = pms.first;
				if(prev_mm == NULL) break;
				if(prev_mm->obj_ref == NULL) break;
				if(!(prev_mm->mmap_flags & MAP_ELF)) break;
				if((prev_mm->mmap_flags & (PROT_WRITE|PROT_EXEC)) == PROT_EXEC) {
					name_mm = prev_mm;
					break;
				}
				map_coalese(&pms);
			}
		}

		if(mdp != NULL) {
			OBJECT	*nobp = name_mm->obj_ref->obp;

			//RUSH3: A difference between old and new memmgrs. The old
			//RUSH3: one would assign R/W data for a shared object to
			//RUSH3: the shared object name. The new one, since it
			//RUSH3: privatize()'s the data, treats it the same as an
			//RUSH3: a normal anonymous mapping (if it's been written to).
			//RUSH3: The end result is that it shows up as belonging to
			//RUSH3: the process, with nothing to say that it originally
			//RUSH3: came from the shared object. Fixing that would
			//RUSH3: entail adding data to the mm_map that allowed us
			//RUSH3: to trace back to the underlying shared object. I don't
			//RUSH3: want to do that unless I'm forced to.
			mdp->vaddr = vaddr - mm->reloc;
			if((nobp->hdr.type == OBJECT_MEM_SHARED) && (nobp->mem.mm.flags & MM_SHMEM_IFS)) {
				(void) imagefs_fname(nobp, name_mm->offset, dbginfo_pathlen, mdp->path);
			} else if(object_name(nobp, dbginfo_pathlen, mdp->path) == 0) {
				if((name_mm->extra_flags & EXTRA_FLAG_LOADER) && (name_mm->mmap_flags & MAP_ELF) && (prp->debug_name != NULL)) {
					STRLCPY(mdp->path, prp->debug_name, dbginfo_pathlen);
				} else if((nobp->hdr.type == OBJECT_MEM_ANON) && (mm->mmap_flags & MAP_SYSRAM)) {
					STRLCPY(mdp->path, "/dev/zero", dbginfo_pathlen);
				//RUSH2: Do something with OBJECT_MEM_TYPED here, or in tymem_name?
				} else {
					STRLCPY(mdp->path, "/dev/mem", dbginfo_pathlen);
				}
			}
		}
		info.offset = mm->offset + (vaddr - mm->start);
		if((contigp != NULL) || (mip != NULL)) {
			data.mip   = mip;
			data.flags = 0;
			data.len   = 0;
			data.off   = info.offset;
			data.need_contig = (contigp != NULL);
			memobj_pmem_walk(MPW_SYSRAM|MPW_PHYS|MPW_HOLES, obp,
					info.offset, info.offset + info.size - 1, pmem_info, &data);

			info.size = data.len - ADDR_OFFSET(vaddr);
			if(contigp != NULL) {
				*contigp = data.len;
			}
			if(mip != NULL) {
				if(mm->mmap_flags & MAP_ELF) {
					info.dev = 4;
					info.ino = name_mm->start;
					info.offset = name_mm->offset + (vaddr - name_mm->start);
				} else if(obp->hdr.type == OBJECT_MEM_SHARED) {
					info.dev = 3;
					info.ino = 1;
				//RUSH2: Do something with OBJECT_MEM_TYPED here?	
				} else {
					info.dev = 2;
					if((mm->mmap_flags & MAP_STACK) && (data.flags == 0)) {
						// Override ino so pidin displays the lazy part correctly
						info.ino = 0;
					} else {
						info.ino = 1;
					}

					// Anonymous mappings don't have an offset
					if(obp->hdr.type == OBJECT_MEM_ANON) info.offset = 0;
				}
				info.flags |= data.flags;
			}
		}
		memobj_unlock(obp);
	} else if(mdp != NULL) {
		// This is a stack guard page.
		mdp->vaddr = vaddr;
		mdp->path[0] = '\0';
	}
	map_coalese(&ms);

	if(mip != NULL) *mip = info;

	return info.size;
}

__SRCVERSION("vmm_mapinfo.c $Rev: 207313 $");
