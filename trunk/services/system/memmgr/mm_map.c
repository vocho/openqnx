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

#include <string.h>
#include <atomic.h>
#include "vmm.h"

#define MAP_WRITE_LOCK	0x1000

//RUSH2: Need more sophisticated structure for quick vaddr => mm_map translation.
//RUSH2: Btree perhaps?

//RUSH1: Register a purger to release unneeded memory....
struct mm_map_block {
	struct mm_map_block		*next;
	struct mm_map_internal	*free;
};
//#define MAP_TO_BLOCK(mm)	((struct mm_map_block *)(ROUNDDOWN((uintptr_t)(mm), MAP_BLOCK_SIZE)+MAPS_PER_BLOCK*sizeof(struct mm_map_internal)))
#define MAP_BLOCK_SIZE		QUANTUM_SIZE
#define MAPS_PER_BLOCK		((MAP_BLOCK_SIZE-sizeof(struct mm_map_block)) / sizeof(struct mm_map_internal))

struct mm_map_internal {
	struct mm_map			map;
	//RUSH2: other stuff
};

intrspin_t						map_spin;

static struct mm_map_internal	*mm_free_list;


static struct mm_map_internal *
map_alloc(void) {
	struct mm_map_internal	**owner;
	struct mm_map_internal	*mm;
	struct mm_map_internal	*tmp;
	void					*addr;
	unsigned				size;
	unsigned				i;

	INTR_LOCK(&map_spin);
	owner = &mm_free_list;
	for( ;; ) {
		mm = *owner;
		if(mm == NULL) goto need_more;
		if(!mm->map.inuse) break;
		owner = (struct mm_map_internal **)&mm->map.next;
	}
	*owner = (struct mm_map_internal *)mm->map.next;
	INTR_UNLOCK(&map_spin);
	memset(mm, 0, sizeof(*mm));
	return mm;

need_more:	
	INTR_UNLOCK(&map_spin);
	// We go directly to mmap() for the storage rather than using the
	// heap because this memory will never be released and can cause
	// fragmentation and a long free list.
	if(memmgr.mmap(0, 0, MAP_BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE,
					0, 0, 0, 0, NOFD, &addr, &size, mempart_getid(NULL, sys_memclass_id)) != EOK) {
		return NULL;
	}
	mm = addr;
	tmp = &mm[1];
	for(i = 2; i < MAPS_PER_BLOCK; ++i, ++tmp) {
		tmp[0].map.inuse = 0;
		tmp[0].map.next = &tmp[1].map;
	}
	tmp->map.inuse = 0;
	INTR_LOCK(&map_spin);
	tmp->map.next = (struct mm_map *)mm_free_list;
	mm_free_list = &mm[1];
	INTR_UNLOCK(&map_spin);
	memset(mm, 0, sizeof(*mm));
	return mm;
}


#define map_free(first, last) \
			INTR_LOCK(&map_spin); \
			(last)->map.next = (struct mm_map *)mm_free_list; \
			mm_free_list = (first);	\
			INTR_UNLOCK(&map_spin)



// For the locking code, we could use the VARIANT_smp version in all
// cases and avoid the #if's, but there are certain processors (SH, ARM,
// NEC MIPS 41xx's, etc) that don't have atomic operations in hardware,
// so the _smp_cmpxchg() would end up trapping into the kernel :-(.

#if defined(VARIANT_smp)
static void rdecl
map_write_lock(struct mm_map_head *mh) {
#ifndef NDEBUG	
	unsigned	count = 0;
#endif	

	do {

		for( ;; ) {
			if(mh->lock == 0) break;
			__cpu_membarrier(); // give somebody else a chance at the bus
#ifndef NDEBUG	
			if(++count == 0) crash();
#endif			
		}
	} while(_smp_cmpxchg(&mh->lock, 0, MAP_WRITE_LOCK) != 0);
}
#else
#define map_write_lock(mh) 	((mh)->lock = MAP_WRITE_LOCK)
#endif


#define map_write_unlock(mh) 	((mh)->lock = 0)


int 
map_fault_lock(struct mm_map_head *mh) {
#if defined(VARIANT_smp)
	unsigned	l;

	do {

		l = mh->lock;
		if(l & MAP_WRITE_LOCK) return 0;
	} while(_smp_cmpxchg(&mh->lock, l, l + 1) != l);
#else 
	if(mh->lock & MAP_WRITE_LOCK) return 0;
	mh->lock = 1;
#endif
	return 1;
}

void 
map_fault_unlock(struct mm_map_head *mh) {
#if defined(VARIANT_smp)
	atomic_sub(&mh->lock, 1);
#else 
	mh->lock = 0;
#endif
}


static struct mm_map_internal * rdecl
split_one(struct mm_map_head *mh, struct mm_map_internal *mm, uintptr_t split) {
	struct mm_map_internal	*new;

	new = map_alloc();
	if(new == NULL) return NULL;
	map_write_lock(mh);
	new->map.next = mm->map.next;
	new->map.offset = mm->map.offset + (split - mm->map.start);
	new->map.start = split;
	new->map.end = mm->map.end;
	mm->map.end = split - 1;
	new->map.obj_ref = mm->map.obj_ref;
	if(new->map.obj_ref != NULL) {
		new->map.ref.next = mm->map.ref.next;
		if(mm->map.ref.next != NULL) {
			mm->map.ref.next->ref.owner = &new->map.ref.next;
		}
		new->map.ref.owner = &mm->map.ref.next;
		mm->map.ref.next = &new->map;
	}
	new->map.mmap_flags = mm->map.mmap_flags;
	new->map.extra_flags = mm->map.extra_flags;
	new->map.last_page_bss = mm->map.last_page_bss;
	mm->map.last_page_bss = 0;
	new->map.reloc = mm->map.reloc;
	//MAPFIELDS: copy other mm_map fields. 
	mm->map.next = &new->map;
	map_write_unlock(mh);
	memref_walk_restart(mh);
	return new;
}


/*
 * This function populates a map_set structure with all the memory mappings
 * the process has between 'start' and 'end'.
 *
 * If MI_NEXT is specified, the map_set will include the first mapping past 'start',
 * even if that is past 'end'.
 *
 * If MI_SPLIT is specified, the mappings will be split up so that the boundaries
 * of the map_set exactly match 'start' and 'end'.
 */
 
int
map_isolate(struct map_set *ms, struct mm_map_head *mh, 
				uintptr_t start, size_t size, int flags) {
	struct mm_map_internal	*prev;
	struct mm_map_internal	*mm;
	uintptr_t				end;

	if(size == 0) size = 1;

	// find the end of the last page
	end = (start + size - 1) | (__PAGESIZE-1);
	start = ROUNDDOWN(start, __PAGESIZE);
	ms->flags = flags;
	ms->head = mh;
	ms->first = NULL; // In case of no matching entries
	ms->prev = NULL;

	prev = (struct mm_map_internal *)mh->rover;
	if(prev == NULL || (mm = (struct mm_map_internal *)prev->map.next) == NULL ||
	    mm->map.start > start) {
		// If rover is too high, start searching at the beginning
		prev = NULL;
		mm = (struct mm_map_internal *)mh->head;
	}

	for( ;; ) {
		if(mm == NULL) {
			// No more mappings to examine
			if(ms->first != NULL) {
				break;
			}
			// Empty set
			return EOK;
		}
		if(ms->first == NULL) {
			if(start <= mm->map.end) {
				if(flags & MI_NEXT) {
					flags &= ~MI_NEXT; // MI_NEXT only applies for first map
					if(end < mm->map.start) {
						// Move up 'end' to overlap this chunk
						end = mm->map.end;
					}
				}
				if(end >= mm->map.start) {
					// Found our first entry
					ms->first = &mm->map;
					if(prev != NULL) {
						ms->prev = &prev->map;
						mh->rover = &prev->map;
					}
				}
			}
		} 
		if(!(flags & MI_NEXT) && (end < mm->map.start)) {
			// this map is past our range and MI_NEXT is off; we're done
			break;
		}
		prev = mm;
		mm = (struct mm_map_internal *)mm->map.next;
	}
	// How can prev ever be null here?
	// prev points to the last map included in the target region
	if((prev == NULL) || (end < prev->map.start)) {
		// If something went wrong??? Shouldn't get here
		prev = (struct mm_map_internal *)ms->first;
	}
	ms->last = &prev->map;
	if((flags & MI_SPLIT) && (ms->first != NULL)) {
		if(ms->first->start < start) {
			// Start mapping crosses 'start' boundary; slice it up
			mm = split_one(mh, (struct mm_map_internal *)ms->first, start);
			if(mm == NULL) {
				(void)map_coalese(ms);
				return ENOMEM;
			}
			ms->prev = ms->first;
			ms->first = &mm->map;
			if(ms->prev == ms->last) {
				ms->last = ms->first;
			}
		}
		if(ms->last->end > end) {
			// End mapping crosses 'end' boundary; slice it up
			mm = split_one(mh, (struct mm_map_internal *)ms->last, end + 1);
			if(mm == NULL) {
				(void)map_coalese(ms); 
				return ENOMEM;
			}
		}
	}
	return EOK;
}


uintptr_t
map_find_va(struct mm_map_head *mh, uintptr_t va, uintptr_t size, 
			uintptr_t mask, unsigned flags) {
	uintptr_t				hole_start;
	uintptr_t				hole_end;
	uintptr_t				end;
	uintptr_t				start;
	uintptr_t				va_mask_bits;
	struct mm_map_internal	*mm;
	uintptr_t				try_align;
	uintptr_t				check;
	uintptr_t				*pgsz;

	end = va + size - 1;
	if(end < va) {
		// wrapped around, have to shift things down.
		end = ~(uintptr_t)0;
		va  = (end - size) + 1;
	}

	if(mask & 1) {
		mask &= ~1;
		// Find the largest supported page size that's not bigger than
		// the requested size and try for that alignment (but do not require).
		// Makes it more likely that we'll be able to use big pages
		// for the allocation.
		pgsz = pgszlist;
		for( ;; ) {
			try_align = *pgsz;
			if(size >= try_align) break;
			++pgsz;
		}
		CRASHCHECK(try_align == 0);
	} else {
		try_align = 1;
	}

	va_mask_bits = va & mask;
	start = VA_INVALID;
	hole_start = mh->start;
	mm = (struct mm_map_internal *)mh->head;
	for( ;; ) {
		//RUSH1: Don't allow allocs outside of mh->start/end.
		if((mm == NULL) || (mm->map.start > mh->end)) {
			hole_end = mh->end;
		} else {
			hole_end = mm->map.start;

			//This "if" is for the first entry, 
			// where mh->start == mm->map.start == 0
			if(hole_start < hole_end) hole_end -= 1;
		}
		//adjust start for colour and/or alignment
		if(flags & MAP_BELOW) {
			uintptr_t	tmp = (hole_end - size) + 1;

			hole_end = (tmp - ((tmp - va_mask_bits) & mask)) + size - 1;
		} else {
			hole_start = hole_start + ((va_mask_bits - hole_start) & mask);
		}
		if(hole_start < hole_end) {
			if(size < (hole_end - hole_start)) {
				if(flags & MAP_BELOW) {
					if((hole_start > va) && (start != VA_INVALID)) break;
					if((va >= hole_start) && (end <= hole_end)) {
						start = va;
						break;
					}
					start = (hole_end - size) + 1;
				} else {
					start = hole_start;
					check = ROUNDUP(start, try_align);
					if(size < (hole_end - check)) {
						start = check;
					}
					if(start >= va) break;
					if(end < hole_end) {
						start = va;
						break;
					}
				}
			}
		}
		if(mm == NULL) break;
		hole_start = mm->map.end + 1;			
		mm = (struct mm_map_internal *)mm->map.next;
	}
	return start;
}


int
map_create(struct map_set *ms, struct map_set *repl, struct mm_map_head *mh, 
				uintptr_t va, uintptr_t size, uintptr_t mask, unsigned flags) {
	struct mm_map_internal	*new;
	struct mm_map			*mm;
	uintptr_t				end;
	int						r;

	if((ADDR_OFFSET(va) != 0) || (ADDR_OFFSET(size) != 0)) crash();

	new = map_alloc();
	if(new == NULL) return ENOMEM;

	new->map.mmap_flags = flags & ~IMAP_GLOBAL;
	ms->head = mh;
	ms->first = &new->map;
	ms->last = &new->map;
	ms->prev = NULL;
	ms->flags = 0;

	if(flags & (MAP_FIXED|IMAP_GLOBAL)) {
		if(!(flags & IMAP_GLOBAL)) {
			if((va < mh->start) || (va >= mh->end)) {
				r = EINVAL;
				goto fail1;
			}
			end = va + size - 1;
			if((end > mh->end) || (end < va)) {
				r = ENOMEM;
				goto fail1;
			}
		}
		// If repl is NULL, we're being called from a location that knows
		// that there can't be anything to replace (e.g. vmm_dup()).
		if(repl != NULL) {
			r = map_isolate(repl, mh, va, size, MI_SPLIT);
			if(r != EOK) goto fail1;
			mm = repl->first;
			if(mm != NULL) {
				for( ;; ) {
					if(mm->extra_flags & EXTRA_FLAG_SPECIAL) return EINVAL;
					if(mm == repl->last) break;
					mm = mm->next;
				} 
			}
		}
	} else {
		repl->first = NULL;

		va = map_find_va(mh, va, size, mask, flags);
		if(va == VA_INVALID) {
			r = ENOMEM;
			goto fail1;
		}
	}
	new->map.start = va;
	new->map.end = va + size - 1;
	return EOK;

fail1:	
	map_free(new, new);
	return r;
}


int
map_split(struct map_set *ms, size_t size) {
	uintptr_t				point;
	struct mm_map			*mm;
	struct mm_map_internal	*new;

	//Code can't handle splitting at zero. Probably doesn't need to.
	if(size == 0) crash();

	mm = ms->first;
	point = mm->start + size;
	for( ;; ) {
		if(mm == NULL) crash();
		if((point >= mm->start) && (point < mm->end)) break;
		mm = mm->next;
	}
	new = map_alloc();
	if(new == NULL) return ENOMEM;

	// Don't have to acquire the write lock because this map_set isn't
	// linked into the map_head structure yet.
	new->map.next = mm->next;
	mm->next = &new->map;
	new->map.end = mm->end;
	new->map.start = point;
	mm->end = point - 1;
	new->map.mmap_flags = mm->mmap_flags;
	new->map.extra_flags = mm->extra_flags;
	new->map.last_page_bss = mm->last_page_bss;
	mm->last_page_bss = 0;
	//MAPFIELDS: anything else?
	if(mm == ms->last) ms->last = &new->map;
	return EOK;
}


int
map_destroy(struct map_set *ms) {
	// ms->last == NULL in the case where we did a map_remove() with
	// only EXTRA_FLAG_SPECIAL entries in the list being removed....
	if(ms->last != NULL) {
		map_free((struct mm_map_internal *)ms->first, (struct mm_map_internal *)ms->last);
	}
	return EOK;
}


static int rdecl
coalese_one(struct mm_map_head *mh, struct mm_map *mm) {
	struct mm_map_internal	*next;

	next = (struct mm_map_internal *)mm->next;
	if(next == NULL) return 0;
	if(mm->last_page_bss != 0) return 0;
	if(mm->obj_ref != next->map.obj_ref) return 0;
	if((mm->end + 1) != next->map.start) return 0;
	if((mm->offset + ((mm->end - mm->start) + 1)) != next->map.offset) return 0;
	if(mm->mmap_flags != next->map.mmap_flags) return 0;
	if(mm->extra_flags != next->map.extra_flags) return 0;
	//MAPFIELDS: check other mmap fields for match
	memref_del(&next->map);
	map_write_lock(mh);
	mm->end = next->map.end;
	mm->next = next->map.next;
	mm->last_page_bss = next->map.last_page_bss;
	if(mh->rover == &next->map) {
		mh->rover = mm;
	}
	map_write_unlock(mh);
	memref_walk_restart(mh);
	map_free(next, next);
	return 1;
}


int
map_coalese(struct map_set *ms) {
	struct mm_map	*mm;
	struct mm_map	*prev;
	struct mm_map	*next;

	if(ms->flags & MI_SPLIT) {
		mm = ms->first;
		prev = ms->prev;
		if(prev != NULL) {
			if(coalese_one(ms->head, prev)) {
				if(mm == ms->last) ms->last = prev;
				mm = prev;
			}
		}
		for( ;; ) {
			next = mm->next;
			if(!coalese_one(ms->head, mm)) break;
			if(next == ms->last) ms->last = mm;
		}
		ms->first = mm;
		if(mm != ms->last) {
			(void)coalese_one(ms->head, ms->last);
		}
	}

	return EOK;
}


void
map_set_update(struct map_set *ms, struct mm_map *old, struct mm_map *new) {
	if(ms->first == old) {
		ms->prev = NULL; // we can't trust it anymore
		ms->first = new;
	}
	if(ms->last == old) {
		ms->last = new;
	}
}


int
map_add(struct map_set *ms) {
	struct mm_map		*prev;
	struct mm_map		*mm;
	struct mm_map_head	*mh;

	mh = ms->head;
	prev = mh->rover;
	if((prev != NULL) && (prev->start < ms->first->start)) {
		mm = prev->next;
	} else {
		prev = NULL;
		mm = mh->head;
	}

	for( ;; ) {
		if(mm == NULL) break;
		if(mm->start > ms->first->start) break;
		prev = mm;
		mm = mm->next;
	}
	ms->last->next = mm;
	map_write_lock(mh);
	if(prev == NULL) {
		mh->head = ms->first;
	} else {
		prev->next = ms->first;
		mh->rover = prev;
	}
	map_write_unlock(mh);
	ms->flags |= MI_SPLIT;
	ms->prev = prev;
	mh->rover = prev;
	return EOK;
}


int
map_remove(struct map_set *ms) {
	struct mm_map	**owner;
	struct mm_map	**rem_owner;
	struct mm_map	*mm;
	struct mm_map	*rover;
	uintptr_t		rover_start;

	if(ms->prev != NULL) {
		owner = &ms->prev->next;
	} else {
		owner = &ms->head->head;
	}
	map_write_lock(ms->head);
	rover = ms->head->rover;
	if(rover != NULL &&
	    (rover_start = rover->start) >= ms->first->start &&
	    rover_start <= ms->last->start) {
		ms->head->rover = ms->prev;
	}
	*owner = ms->last->next;
	ms->last->next = NULL;
	if(ms->flags & MI_SKIP_SPECIAL) {
		ms->last = NULL; // In case there are only MI_SKIP_SPECIAL's...
		rem_owner = &ms->first;
		for( ;; ) {
			mm = *rem_owner;
			if(mm == NULL) break;
			if(mm->extra_flags & EXTRA_FLAG_SPECIAL) {
				*rem_owner = mm->next;
				mm->next = *owner;
				*owner = mm;
				owner = &mm->next;
			} else {
				rem_owner = &mm->next;
				ms->last = mm;
			}
		}
	}
	map_write_unlock(ms->head);
	return EOK;
}


int
map_init(struct mm_map_head *mh, uintptr_t start, uintptr_t end) {
	mh->head = NULL;
	mh->rover = NULL;
	mh->lock = 0;
	mh->start = start;
	mh->end = end;
	return EOK;
}


int
map_fini(struct mm_map_head *mh) {
	struct mm_map	*first;
	struct mm_map	*last;

	first = mh->head;
	if(first != NULL) {
		for(last = first; last->next != NULL; last = last->next) {
			CRASHCHECK(last->obj_ref != NULL);
		}
		map_free((struct mm_map_internal *)first, (struct mm_map_internal *)last);
	}
	return EOK;
}

__SRCVERSION("mm_map.c $Rev: 174317 $");
