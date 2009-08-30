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

#include <atomic.h>
#include "vmm.h"

//RUSH3: Duping a [data=uip] executable from an IFS don't work too well.
//RUSH3: Can we detect that and error out?

/*
 * This call dups the address space from process 'pprp' into process 'prp'
 */

struct dup_data {
	off64_t			off;
	struct mm_map	*mm;
	struct mm_map	*pmm;
	ADDRESS			*adp;
	uintptr_t		va;
	size_t			va_off;
};


static int
data_dup_run(void *src, size_t len, void *d) {
	struct dup_data		*data = d;

	memcpy((void *)(data->va + data->va_off), src, len);
	data->va_off += len;
	return EOK;
}


static int 
data_dup(OBJECT *obp, off64_t off, struct pa_quantum *pq, 
				unsigned num, void *d) {
	struct dup_data			*data = d;
	struct mm_map			*mm;
	struct mm_object_ref	*or;
	int						r;
	unsigned				orig_flags;
	uintptr_t				start;
	uintptr_t				end;
	unsigned				run;

	mm = data->mm;
	orig_flags = mm->mmap_flags;
	for( ;; ) {
		// Skip over quantum's that haven't been modified yet (we know
		// they're all zeros).
		for( ;; ) {
			if(num == 0) return EOK;
			if(pq->flags & PAQ_FLAG_MODIFIED) break;
			++pq;
			off += QUANTUM_SIZE;
			--num;
		}
		// Find the length of the next contiguous run of modified memory
		run = 0;
		for( ;; ) {
			++run;
			if(!(pq[run].flags & PAQ_FLAG_MODIFIED)) break;
			if(run == num) break;
		}
		// Copy the modified memory to the child
		data->va_off = off - data->pmm->offset;
		start = data->va + data->va_off;
		end   = start + NQUANTUM_TO_LEN(run) - 1;

		// Add PROT_WRITE to make sure we can write the storage.
		mm->mmap_flags = orig_flags | PROT_WRITE;

		r = memory_reference(&mm, start, end, MR_WRITE, NULL);
		if(r != EOK) return r;
		r = pte_temp_map(data->adp, start, pq, data->pmm, (end - start) + 1, 
							data_dup_run, data);
		or = mm->obj_ref;
		if(!(orig_flags & PROT_WRITE)) {
			(void)pte_prot(or->adp, start, end, orig_flags, or->obp);
		}
		if(orig_flags & PROT_EXEC) {
			CPU_CACHE_CONTROL(or->adp, (void *)start, (end - start) + 1, MS_INVALIDATE_ICACHE);
		}
		mm->mmap_flags = orig_flags;
		if(r != EOK) return r;
		off += run << QUANTUM_BITS;
		pq += run;
		num -= run;
	}
}

 
int 
vmm_dup(PROCESS *pprp, PROCESS *prp) {
	ADDRESS					*padp;
	struct map_set			pms;
	struct mm_map			*pmm;
	struct mm_object_ref	*por;
	OBJECT					*pobp;
	ADDRESS					*adp;
	struct map_set			ms;
	struct mm_map			*mm;
	OBJECT					*anon;
	struct map_set			repl_ms;
	int						r;
	uintptr_t				vaddr;
	size_t					len;
	struct dup_data			data;

	padp = pprp->memory;
	if(padp == NULL) return EOK;

	adp  =  prp->memory;
	if(adp == NULL) return ENOMEM;

	// Clean up all the entries created by the vmm_mcreate() code
	// except for EXTRA_FLAG_SPECIAL one(s) (e.g. MIPS syspage).
	// We'll recreate the ones we want from the parent.
	// We have to leave the EXTRA_FLAG_SPECIAL one(s) in place so
	// that the temporary mapping code doesn't use those vaddr
	// regions. They'll get cleaned up in the copy loop when we
	// get to that point in the address space.
	vaddr = 0;
	for( ;; ) {
		r = map_isolate(&ms, &adp->map, vaddr, 0, MI_NEXT);
		CRASHCHECK(r != EOK);
		mm = ms.first;
		if(mm == NULL) break;
		vaddr = mm->end + 1;
		if(!(mm->extra_flags & EXTRA_FLAG_SPECIAL)) {
			r = ms_unmap(adp, &ms, 0);
			CRASHCHECK(r != EOK);
		}
	}

	// CacheControl() (in data_dup()) might cause page faults, so let 
	// fault_pulse() know that it doesn't have to grab the lock for 
	// this reference
	//RUSH3: data_dup() only ever copies modified pages now,
	//RUSH3: so the CacheControl won't ever fault (hopefully). Can
	//RUSH3: this call be removed?
	proc_lock_owner_mark(prp);

	memset(&adp->rlimit, 0, sizeof(adp->rlimit));
	data.adp = adp;
	vaddr = 0;
	for( ;; ) {
		r = map_isolate(&pms, &padp->map, vaddr, 0, MI_NEXT);
		if(r != EOK) goto fail1;
		pmm = pms.first;
		if(pmm == NULL) break;
		// Yeah, it's a double negative for the test, but doing it this
		// way saves code in mm_map.c since the default has the flag off.
		if(!(pmm->extra_flags & EXTRA_FLAG_NOINHERIT)) {
			len = (pmm->end - pmm->start) + 1;
			r = map_isolate(&repl_ms, &adp->map, pmm->start, len, MI_SPLIT);
			if(r != EOK) goto fail1;
			if(repl_ms.first != NULL) {
				CRASHCHECK(!(repl_ms.first->extra_flags & EXTRA_FLAG_SPECIAL));
				// Clean up an EXTRA_FLAG_SPECIAL entry in the child that
				// was created by vmm_mcreate(). We're going to transfer
				// the one from the parent now.

				r = ms_unmap(adp, &repl_ms, 0);
				CRASHCHECK(r != EOK);
			}
			r = map_create(&ms, NULL, &adp->map, pmm->start, len, 0, MAP_FIXED);
			if(r != EOK) goto fail1;			
			mm = ms.first;
			mm->mmap_flags = pmm->mmap_flags;
			mm->extra_flags = pmm->extra_flags & ~EXTRA_FLAG_LOCK;
			mm->offset = pmm->offset;
			mm->reloc = pmm->reloc;
			mm->last_page_bss = pmm->last_page_bss;
			//MAPFIELDS: copy mm_map fields from 'pmm' to 'mm'

			adp->rlimit.vmem += len;

			pobp = NULL;
			anon = NULL;
			r = map_add(&ms);
			if(r != EOK) goto fail3;
			por = pmm->obj_ref;
			if(por != NULL) {
				if (pmm->mmap_flags & EXTRA_FLAG_RLIMIT_DATA) {
					adp->rlimit.data += len;
				} else if(pmm->extra_flags & EXTRA_FLAG_PRIMARY_STK) {
					adp->rlimit.stack += len;
				}
				pobp = por->obp;
				memobj_lock(pobp);
				if((pobp->hdr.type == OBJECT_MEM_ANON) && ((pmm->mmap_flags & MAP_TYPE) == MAP_PRIVATE)) {
					anon = adp->anon;
					memobj_lock(anon);
					r = memref_add(mm, anon, adp, NOFD);
				} else {
					// For ~MAP_LAZY, MAP_PRIVATE, non anonymous memory, pre-allocate
					// the all anonymous memory that we might end up needing.
					// This will avoid overcommiting the memory and having processes
					// randomly dying with SIGBUS's when we find that we need to create
					// a private copy of a page and we're out of physical memory.
					if((pmm->mmap_flags & (MAP_LAZY|MAP_PRIVATE)) == MAP_PRIVATE) {
						off64_t	anmem_off;

						memobj_lock(adp->anon);
						anmem_off = anmem_offset(adp->anon, pmm->start, len);
						r = memobj_pmem_add(adp->anon, anmem_off, len, pmm->mmap_flags);
						memobj_unlock(adp->anon);
						if(r != EOK) goto fail4;
					}
					r = memref_add(mm, pobp, adp, por->fd);
				}
				if(r != EOK) goto fail4;
			}
			if(anon != NULL) {
				mm->offset = anmem_offset(anon, mm->start, len);
				data.mm  = mm;
				data.pmm = pmm;
				data.va = mm->start;
				data.off = mm->offset;
				if(!(mm->mmap_flags & MAP_LAZY)) {
					r = memobj_pmem_add(anon, mm->offset, len, mm->mmap_flags);
					if(r != EOK) goto fail2;
				}
				//RUSH3: Eventually we should be able to (optionally) delay
				//RUSH3: the copying of the data until first write....
				//RUSH3: Remember to deal with the parent having an ISR.
				r = memobj_pmem_walk(MPW_SYSRAM, pobp, pmm->offset, pmm->offset + len - 1, data_dup, &data);
				if(r != EOK) goto fail2;
				memobj_unlock(anon);
			}
			if(pobp != NULL) {
				if(!(mm->mmap_flags & MAP_LAZY)) {
					// Make sure that we have room for the L2 page table(s)
					r = pte_prealloc(adp, mm->start, mm->end);
					if(r != EOK) goto fail2;

					//RUSH3: Make sure we map any extant MAP_SHARED pages.
					//RUSH3: This ensures that we don't fault during munmap in
					//RUSH3: MemobjDestroyed() if those pages contain sync
					//RUSH3: objects and we hadn't yet accessed the pages
					//RUSH3: through the child.
					//RUSH3: The way things are currently set up, this will
					//RUSH3: deadlock since the munmap has both the aspace and
					//RUSH3: object locked in clean_pmem so fault_pulse will
					//RUSH3: block trying to acquire them.
					//RUSH3: Brian thinks we may be able to use some tweaked
					//RUSH3: version of setup_mappings() to do this work as it
					//RUSH3: has other logic that might come in useful.
					if ((mm->mmap_flags & MAP_TYPE) == MAP_SHARED) {
						r = memory_reference(&mm, mm->start, mm->end, MR_NOINIT, NULL);
						if(r != EOK) goto fail2;
					}
				}
				memobj_unlock(pobp);
			}
			map_coalese(&ms);
			map_coalese(&pms);
		}
		vaddr = pmm->end + 1;
	}

	return EOK;

fail4:
	map_remove(&ms);

fail3:	
	map_destroy(&ms);

fail2:
	if(pobp != NULL) memobj_unlock(pobp);
	if(anon != NULL) memobj_unlock(anon);

fail1:
	return r;
}

__SRCVERSION("vmm_dup.c $Rev: 209328 $");
