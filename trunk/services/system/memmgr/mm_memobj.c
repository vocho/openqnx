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

#define OFF_TO_QUANTUM(o)	((o) >> QUANTUM_BITS)



//RUSH3: memobj_pmem_add and memobj_pmem_del_end should use memobj_pmem_walk
//RUSH3: to do their work...

int
memobj_pmem_del_len(OBJECT *obp, off64_t start, uint64_t len) {
	return memobj_pmem_del_end(obp, start, start + len + (QUANTUM_SIZE-1));
}




/***************************************
I don't like the end parameter to memobj_pmem_del_end...  The way it's
coded it, the caller has to pass in 1 greater than the end of what they
want deleted, which is confusing.

Why not have:

     int
     memobj_pmem_del(OBJECT *obp, off64_t start, uint64_t len) {
          return memobj_pmem_del_end(obp, start, start + len - 1);
     }

     int
     memobj_pmem_del_end(OBJECT *obp, off64_t start, off64_t end) {

        [ . . . ]

        qend = OFF_TO_QUANTUM(end) + 1;

Then the end parameter makes sense.

.... Deferred above change until the edge case ramifications can be
considered....
**************************************/
int
memobj_pmem_del_end(OBJECT *obp, off64_t start, off64_t end) {
	struct pa_quantum	*pq;
	struct pa_quantum	*next;
	struct pa_quantum	**owner;
	struct pa_quantum	**new_owner;
	struct pa_quantum	*tmp;
	unsigned			qstart;
	unsigned			qend;
	unsigned			this_end;
	unsigned			this_start;
	unsigned			run_start;
	unsigned			run_end;
	unsigned			num;
	unsigned			i;
	unsigned			skip;
	paddr_t				paddr;
	off_t				off;

	VERIFY_OBJ_LOCK(obp);

	qstart = OFF_TO_QUANTUM(start);
	qend = OFF_TO_QUANTUM(end);

	owner = obp->mem.mm.pmem_cache;
	pq = *owner;

	if((pq == NULL) || (pq->u.inuse.qpos > qstart)) {
		owner = &obp->mem.mm.pmem;
	} 
	for(;;) {
		pq = *owner;
		if(pq == NULL) break;
		CRASHCHECK(!(pq->flags & PAQ_FLAG_INUSE));
		new_owner = &pq->u.inuse.next;
		//FUTURE: When/If we start paging things out, we might
		//FUTURE: have sync objects in the holes of an object's
		//FUTURE: physical memory list. Should we just call MemobjDestroyed
		//FUTURE: for the whole region?
		run_start = pq->u.inuse.qpos;
		if(run_start >= qend) break;
		run_end = run_start + pq->run;
		this_start = max(run_start, qstart);
		this_end = min(run_end, qend);
		if(this_start < this_end) {
			next = pq->u.inuse.next;
			skip = this_start - run_start;
			if((skip == 0) && (obp->mem.mm.pmem_cache == &pq->u.inuse.next)) {
				obp->mem.mm.pmem_cache = owner;
			}
			num = this_end - this_start;
			if(pq->blk == PAQ_BLK_FAKE) {
				//FUTURE: If we start turning on MM_MEM_HAS_SYNC
				//FUTURE: in vmm_vaddr_to_memobj(), we can check
				//FUTURE: it here and avoid the individual page checks
				paddr = pa_quantum_to_paddr(pq) + (skip << QUANTUM_BITS);
				off = PADDR_TO_SYNC_OFF(paddr);
				// MAP_PHYS mapping's don't keep track of PAQ_FLAG_HAS_SYNC 
				// so if we've granted write permission to anything, we
				// have to assume there's a sync object in it.
				if(pq->flags & PAQ_FLAG_MODIFIED) {
					MemobjDestroyed(PADDR_TO_SYNC_OBJ(paddr), 
								off, off + (num << QUANTUM_BITS) - 1,
								NULL, NULL);
				}
				if(pq->flags & PAQ_FLAG_RDB) {
					tymem_rdb_free(obp, paddr, NQUANTUM_TO_LEN(num));
				}
				if((skip != 0) && (this_end < run_end)) {
					// Freeing a piece in the middle
					tmp = pa_alloc_fake(paddr + ((num+skip) << QUANTUM_BITS), 
								(pq->run - (num+skip)) << QUANTUM_BITS);
					if(tmp == NULL) return ENOMEM;
					tmp->u.inuse.next = next;
					tmp->u.inuse.qpos = this_end;
					pq->u.inuse.next = tmp;
					pq->run = skip;
				} else if(this_end < run_end) {
					// Keeping a bit at the end
					((struct pa_quantum_fake *)pq)->paddr += num << QUANTUM_BITS;
					pq->run -= num; 
					pq->u.inuse.qpos = this_end;
				} else if(skip == 0) {
					// Freeing the whole thing
					pa_free_fake(pq);
					*owner = next;
					new_owner = owner;
				} else {
					// Keeping a bit at the start
					pq->run = skip;
				} 
			} else {
				//FUTURE: If we start turning on MM_MEM_HAS_SYNC
				//FUTURE: in vmm_vaddr_to_memobj(), we can check
				//FUTURE: it here and avoid the individual page checks
				tmp = pq + skip;
				paddr = pa_quantum_to_paddr(tmp);
				for(i = 0; i < num; ++i, ++tmp) {
					if(tmp->flags & PAQ_FLAG_HAS_SYNC) {
						tmp->flags &= ~PAQ_FLAG_HAS_SYNC;
						off = PADDR_TO_SYNC_OFF(paddr);
						MemobjDestroyed(PADDR_TO_SYNC_OBJ(paddr), 
								off, off + (QUANTUM_SIZE-1),
								NULL, NULL);
					}
					paddr += QUANTUM_SIZE;
				}
				if (pmem_stats_hook != NULL) {
					pmem_stats_hook(obp, NQUANTUM_TO_LEN(num), PMEM_STATS_FREE);
				}
				pa_free(pq + skip, num, MEMPART_DECR(obp->hdr.mpid, NQUANTUM_TO_LEN(num)));
				MEMCLASS_PID_FREE_INKER(obp->hdr.creator, obp->hdr.mpid, NQUANTUM_TO_LEN(num));

				if((skip != 0) && (this_end < run_end)) {
					// Freeing a piece in the middle
					tmp = pq + skip + num;
					pq->u.inuse.next = tmp;
					tmp->u.inuse.next = next;
					tmp->u.inuse.qpos = this_end;
				} else if(this_end < run_end) {
					// Keeping a bit at the end
					tmp = pq + num;
					*owner = tmp;
					new_owner = &tmp->u.inuse.next;
					tmp->u.inuse.next = next;
					tmp->u.inuse.qpos = this_end;
				} else if(skip == 0) {
					// Freeing the whole thing
					*owner = next;
					new_owner = owner;
				}
			}
		}
		owner = new_owner;
	}

	return EOK;
}


static int
pq_insert(OBJECT *obp, off64_t off, unsigned numq, 
		struct pa_quantum *(*func)(OBJECT *, unsigned, void *), void *d) {
	struct pa_quantum	**owner;
	struct pa_quantum	**chk;
	struct pa_quantum	*ins;
	struct pa_quantum	*prev;
	struct pa_quantum	*new;
	unsigned			hole_start;
	unsigned			hole_end;
	unsigned			this_start;
	unsigned			this_end;
	unsigned			add_start;
	unsigned			add_end;
	int					first;

	first = 1;
	add_start = OFF_TO_QUANTUM(off);
	add_end = add_start + numq;
	hole_start = 0;
	owner = &obp->mem.mm.pmem;
	chk = obp->mem.mm.pmem_cache;
	if(chk != owner) {
		// If the pmem_cache isn't pointing at the head of the list,
		// it's pointing at a pq->u.inuse.next, so by doing the following
		// funkiness, we can retrieve one entry earlier in the list
		// than normal. That lets us use the cache pointer more often
		// when doing a mmap/munmap/mmap/munmap/etc... sequence.
		prev = (void *)((uint8_t *)chk - offsetof(struct pa_quantum, u.inuse.next));
	} else {
		prev = *chk;
	}
	if(prev != NULL) {
		this_end = prev->u.inuse.qpos + prev->run;
		if(this_end <= add_start) {
			owner = &prev->u.inuse.next;
			hole_start = this_end;
		}
	}
	for( ;; ) {
		ins = *owner;
		if(ins != NULL) {
			CRASHCHECK(!(ins->flags & PAQ_FLAG_INUSE));
			hole_end = ins->u.inuse.qpos;
		} else {
			hole_end = ~0;
		}
		this_start = max(hole_start, add_start);
		this_end = min(hole_end, add_end);

		if(this_start < this_end) {
			new = func(obp, this_end - this_start, d);	
			if(new == NULL) return ENOMEM;
			CRASHCHECK(!(new->flags & PAQ_FLAG_INUSE));
			if(first) {
				obp->mem.mm.pmem_cache = owner;
				first = 0;
			}
			*owner = new;
			do {
				new->u.inuse.qpos = this_start;
				this_start += new->run;
				prev = new;
				new = new->u.inuse.next;
			} while(new != NULL);	
			prev->u.inuse.next = ins;
		}
		if(hole_end >= add_end) return EOK;
		CRASHCHECK(ins == NULL);
		hole_start = hole_end + ins->run;
		owner = &ins->u.inuse.next;
	}
}


struct add_data {
	int					flags;
	struct pa_quantum	*fake;
	struct pa_restrict	*restriction;
};


static struct pa_quantum *
pmem_alloc(OBJECT *obp, unsigned numq, void *d) {
	struct add_data		*data = d;
	struct pa_quantum	*pq;
	unsigned			status;
	memsize_t			resv = 0;

	VERIFY_OBJ_LOCK(obp);

	//FUTURE: Pass in something other than PAQ_COLOUR_NONE so pa_alloc()
	//FUTURE: can try to get the correctly coloured memory for the request.
	//RUSH2: Alignment
	if (MEMPART_CHK_and_INCR(obp->hdr.mpid, NQUANTUM_TO_LEN(numq), &resv) != EOK) {
		return(NULL);
	}
	pq = pa_alloc(NQUANTUM_TO_LEN(numq), 0, PAQ_COLOUR_NONE, 
					data->flags, &status, data->restriction, resv);
	if (pq == NULL) {
		MEMPART_UNDO_INCR(obp->hdr.mpid, NQUANTUM_TO_LEN(numq), resv);
	} else {
		MEMCLASS_PID_USE_INKER(obp->hdr.creator, obp->hdr.mpid, NQUANTUM_TO_LEN(numq));
	}
	return pq;
}


int
memobj_pmem_add(OBJECT *obp, off64_t off, size_t size, int flags) {
	static struct pa_restrict	restrict_16M = {NULL, NULL, 0, 0x00ffffff};
	struct add_data				data;
	int status=0;

	data.flags = 0;
	if(flags & MAP_PHYS) data.flags |= PAA_FLAG_CONTIG;
	if(flags & MAP_NOX64K) data.flags |= PAA_FLAG_NOX64K;
	if((obp->hdr.type == OBJECT_MEM_SHARED) && (obp->mem.mm.flags & SHMCTL_HIGHUSAGE)) {
		data.flags |= PAA_FLAG_HIGHUSAGE;
	}
	if((flags & MAP_BELOW16M)
#if defined(__ARM__)||defined(__PPC__)||defined(__MIPS__)||defined(__SH__)
		&& !(mm_flags & MM_FLAG_BACKWARDS_COMPAT)
#endif		
	){
		data.restriction = &restrict_16M;
	} else { 
		data.restriction = obp->mem.mm.restriction;
	}
	size += (size_t)off & (QUANTUM_SIZE-1);
	status = pq_insert(obp, off, LEN_TO_NQUANTUM(size), pmem_alloc, &data);
	if (status == EOK) {
		if (pmem_stats_hook != NULL) {
			pmem_stats_hook(obp, size, PMEM_STATS_ALLOCATE);
		}
	}
	return(status);
}


static struct pa_quantum *
pmem_pq(OBJECT *obp, unsigned numq, void *d) {
	struct add_data	*data = d;

	CRASHCHECK(data->fake->run > numq);
	return data->fake;
}

int
memobj_pmem_add_pq(OBJECT *obp, off64_t off, struct pa_quantum *pq) {
	struct add_data	data;

	data.fake = pq;
	return pq_insert(obp, off, pq->run, pmem_pq, &data);
}


int
memobj_pmem_split(OBJECT *obp, struct pa_quantum *pq, unsigned split_quanta) {
	struct pa_quantum	*new;
	unsigned			remaining;

	remaining = pq->run - split_quanta;
	new = pa_alloc_fake(pa_quantum_to_paddr(pq) + NQUANTUM_TO_LEN(split_quanta),
					NQUANTUM_TO_LEN(remaining));
	if(new == NULL) return ENOMEM;
	new->u.inuse.qpos = pq->u.inuse.qpos + split_quanta;
	new->u.inuse.next = pq->u.inuse.next;
	pq->u.inuse.next = new;
	pq->run -= split_quanta;
	return EOK;
}


int
memobj_pmem_walk(int flags, OBJECT *obp, off64_t off_start, off64_t off_end, 
					int (*func)(OBJECT *, off64_t, struct pa_quantum *, unsigned, void *), 
					void *d) {

	struct pa_quantum	**owner;
	struct pa_quantum	**next_owner;
	struct pa_quantum	*pq;
	unsigned			qpos;
	unsigned			qnext;
	unsigned			qstart;
	unsigned			qend;
	unsigned			start;
	unsigned			end;
	int					r;
	unsigned			invoked;


	VERIFY_OBJ_LOCK(obp);
	invoked = 0;
	qstart = OFF_TO_QUANTUM(off_start);
	qend   = OFF_TO_QUANTUM(off_end);
	qpos = 0;
	owner = obp->mem.mm.pmem_cache;
	pq = *owner;
	if((pq == NULL) || (pq->u.inuse.qpos > qstart)) {
		owner = &obp->mem.mm.pmem;
	}
	for(;;) {
		pq = *owner;
		if(qpos > qend) break;
		if(pq == NULL) {
			if(flags & MPW_HOLES) {
				++invoked;
				start = max(qpos, qstart);
				r = func(obp, NQUANTUM_TO_LEN(start), NULL, (qend - start) + 1, d);
				if(r != EOK) goto done1;
			}
			break;
		}
		CRASHCHECK(pq->run <= 0);
		next_owner = &pq->u.inuse.next;
		qnext = pq->u.inuse.qpos;
		if((flags & MPW_HOLES) && (qnext > qpos) && (qnext > qstart)) {
			++invoked;
			start = max(qpos, qstart);
			end   = min(qend, qnext-1);
			r = func(obp, NQUANTUM_TO_LEN(start), NULL, (end - start) + 1, d);
			if(r != EOK) goto done1;
		}
		if(qnext > qend) break;
		qpos = qnext;
		qnext += pq->run;
		if(qnext > qstart) {
			end  = min(qend, qnext-1);
			r = EOK;
			if(pq->blk != PAQ_BLK_FAKE) {
				if(flags & MPW_SYSRAM) {
					++invoked;
					start = max(qpos, qstart);
					r = func(obp, NQUANTUM_TO_LEN(start), pq + (start - qpos), (end - start) + 1, d);
				}
			} else if(flags & MPW_PHYS) {
				++invoked;
				r = func(obp, NQUANTUM_TO_LEN(qpos), pq, (end - qpos) + 1, d);
			}
			if(r != EOK) goto done1;
		}
		if(invoked == 1) {
			invoked = 2;
			obp->mem.mm.pmem_cache = owner;
		}
		owner = next_owner;
		qpos = qnext;
	}
	return EOK;

done1:
	if(r < 0) r = EOK;
	return r;
}


int
memobj_pmem_walk_mm(int flags, struct mm_map *mm, 
					int (*func)(OBJECT *, off64_t, struct pa_quantum *, unsigned, void *), void *d) {
	return memobj_pmem_walk(flags, mm->obj_ref->obp, 
				mm->offset, mm->offset + (mm->end - mm->start),
				func, d);
}


//Need to check that the offset we're adding the pmem to doesn't
//exceed the maximum allowed by the data structure (2**(32+12))
int
memobj_offset_check(OBJECT *obp, off64_t off, uint64_t len) {
	off64_t		end;

	end = off + len;
	if(end < off) return ENXIO;
	if(end > ((off64_t)((uint32_t)~0) << QUANTUM_BITS)) return ENXIO;
	return EOK;
}

__SRCVERSION("mm_memobj.c $Rev: 211761 $");
