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


#define HASH_PAGEBITS	12
#define HASH_PAGESIZE  (1 << HASH_PAGEBITS)
#define HASH_FUNC(hash, val)	(((val) >> HASH_PAGEBITS) & hash.mask)
#define SEC_HASH_BINS	(0x40)
#define SEC_HASH(addr)	(((addr)>>3) & (SEC_HASH_BINS-1))


SYNC * rdecl
sync_create(PROCESS *bill_prp, sync_t *sync, unsigned flags) {
	SYNC			*syp;
	OBJECT			*obp;
	unsigned		addr;
	THREAD			*act = actives[KERNCPU];
	PROCESS			*prp = act->process;

	// Allocate a sync entry.
	if((syp = object_alloc(prp, &sync_souls)) == NULL) {
		kererr(act, EAGAIN);
		return(NULL);
	}

	// Map the virtual address of the users sync to a physical addr.
	// (flags & PTHREAD_PROCESSSHARED_MASK) may be useful to memmgr
	// Note that we tell vaddr_to_memobj to mark this page as containing a
	// sync object.
	if((obp = memmgr.vaddr_to_memobj(prp, sync, &addr, 1)) == (OBJECT *)-1) {
		kererr(act, EINVAL);
		return NULL;
	}

	// Add the entry to the sync vector.
	if(synchash_add(obp, addr, syp) == -1) {
		object_free(prp, &sync_souls, syp);
		kererr(act, EBUSY);
		return(NULL);
	}

	if (sync_create_hook) { 
		sync_create_hook(bill_prp);
	}

	return(syp);
}

int rdecl
sync_destroy(PROCESS  *prp, sync_t *sync) {
	OBJECT		*obp;
	unsigned	addr;
	SYNC		*syp;
	THREAD		*thp;
	unsigned	owner;
	THREAD		*act = actives[KERNCPU];

	CRASHCHECK(!am_inkernel());

	// Map the virtual address of the users sync to a physical addr.
	if((obp = memmgr.vaddr_to_memobj(act->process, sync, &addr, 0)) == (OBJECT *)-1) {
		return EINVAL;
	}

	// Verify the sync exists.
	if((syp = synchash_lookup(obp, addr)) == NULL) {
		return EINVAL;
	}

	lock_kernel();

	// Make it invalid incase someone tried to use it in the future.
	// The owner must be zero or we are busy. If it is zero atomicly
	// set it to _NTO_SYNC_DESTROYED (-2) to indicate it is destroyed.
	// If the sync is a mutex owned by the caller, it can be destroyed.
	// If it is not a mutex also destroy the sync.
	if ((owner = _smp_cmpxchg(&sync->__owner, 0, _NTO_SYNC_DESTROYED)) != 0 &&
		SYNC_OWNER(act) != (owner & ~(_NTO_SYNC_WAITING)) &&
		owner <= _NTO_SYNC_DEAD) {
		return EBUSY;
	}
	sync->__owner = _NTO_SYNC_DESTROYED;

	// Force ready any threads blocked on the sync.
	// Note: This will only happen for semaphore since they have no concept
	//       of ownership.
	while((thp = pril_first(&syp->waiting))) {
		if(TYPE_MASK(thp->type) != TYPE_THREAD) {
			break;
		}
		force_ready(thp, EINVAL);
	}



	if (sync_destroy_hook) { 
		sync_destroy_hook(prp, sync);
	}
	
	// Remove from the sync hash and release object back to sync_souls.
	synchash_rem(addr, obp, addr, addr, NULL, NULL);

	/* since synchash_rem() does not have any return code ... */
	CRASHCHECK(synchash_lookup(obp, addr) != NULL);
	
	return EOK;
}


SYNC * rdecl
sync_lookup(sync_t *sync, unsigned create) {
	SYNC			*syp;
	THREAD			*act = actives[KERNCPU];
	unsigned		addr;
	unsigned		mem_addr;
	OBJECT			*obp;
	SYNC			**owner;

	//Watch out for sneaky QA types trying to get at kernel memory.
	if(!WITHIN_BOUNDRY((uintptr_t)sync, (uintptr_t)(sync+1), act->process->boundry_addr)) {
		kererr(act, EFAULT);
		return NULL;
	}

	// Reference sync so it will fault if not-present or invalid.
	// We do this with atomic_set rather than the usual
	// "WR_PROBE_INT(act, &sync->__owner, 1)" because another CPU in an
	// SMP system might be playing around with the memory at the same time
	// and the WR_PROBE_INT accessing is *not* SMP safe. Don't do this to
	// us again!
	//
	// We also have to probe the 'count' field so that it will
	// get privatized before we do the memmgr.vaddr_to_memobj() call.
	// Since the 'count' field is at the the start of the sync object,
	// its address is used for the memmgr.vaddr_to_memobj return - if
	// it happens to fall on a different page than the 'owner' field,
	// we could have it's paddr move on us after the vaddr_to_memobj
	// has returned it and we've entered the original value in the hash 
	// table.
	// 
#if defined(VARIANT_smp)
	atomic_set((unsigned *)&sync->__count, 0);
	atomic_set(&sync->__owner, 0);
#else
	//
	// Do two probes rather than a single
	// WR_PROBE_INT(act, sync, sizeof(*sync)/sizeof(int)) because
	// it'll expand to a more efficient code sequence that way.
	//
	WR_PROBE_INT(act, &sync->__count, 1);
	WR_PROBE_INT(act, &sync->__owner, 1);
#endif

	obp = memmgr.vaddr_to_memobj(act->process, sync, &mem_addr, 0);
	// The following assignment is for improved code generation.
	// By doing it, the compiler knows that it can cache the 'addr'
	// value in a register (harder for 'mem_addr', since it's address
	// was taken above).
	addr = mem_addr;

	owner = (SYNC **)sync_hash.table[HASH_FUNC(sync_hash, addr)];
	if(owner != NULL) {
		owner += SEC_HASH(addr);
		for( ;; ) {
			syp = *owner;
			if(syp == NULL) break;
			if((syp->addr == addr) && (syp->obj == obp)) goto found_it;
			owner = &syp->next;
		}
	}

	// If sync object not found, then we autoinit a sync
	//UNLESS we said don't autocreate (create:_NTO_SYNC_INITIALIZER)
	if((sync->__owner != _NTO_SYNC_INITIALIZER) || (create == _NTO_SYNC_INITIALIZER)) {
		kererr(act, EINVAL);
		return(NULL);
	}

	lock_kernel();
	if((syp = sync_create(act->process, sync, PTHREAD_PROCESS_PRIVATE)) == NULL) {
		return(NULL);
	}

	sync->__owner = create;
	if(create == _NTO_SYNC_COND) {
		sync->__count = CLOCK_REALTIME;
	}

	unlock_kernel();
	KER_PREEMPT(act, NULL);

found_it:	
	if(sync->__owner == _NTO_SYNC_INITIALIZER) {
		kererr(act, EINVAL);
		return(NULL);
	}

	return(syp);
}


void rdecl
sync_wakeup(SYNC *syp, int all) {
	THREAD 		*thp;
	int			first;
	unsigned	orig_runmask;
	unsigned	new_runmask;
	THREAD		*act = actives[KERNCPU];

	first = 1;
	while((thp = pril_first(&syp->waiting))) {
		if(TYPE_MASK(thp->type) != TYPE_THREAD) {
			thp = pril_next(thp);
			if(!thp || TYPE_MASK(thp->type) != TYPE_THREAD) {
				break;
			}
		}
		pril_rem(&syp->waiting, thp);

// This screws up the ETIMEDOUT return code from sync_condvar_wait.
// Instead, we EOK the status when the thread goes onto the waiting list...
//		SETKSTATUS(thp,EOK);

		orig_runmask = thp->runmask;
		switch(thp->state) {
		case STATE_MUTEX:	
		case STATE_CONDVAR:
			thp->flags |= _NTO_TF_ACQUIRE_MUTEX;
			if(first) {
				if(thp->priority > act->priority) {
					// Make sure that the first (highest priority) blocked
					// thread gets run on _this_ CPU. That way, it'll have
					// first crack at the mutex and avoid an SMP priority
					// inversion problem (PR 8591).
					new_runmask = 1 << KERNCPU;
					if((thp->runmask & new_runmask) == 0) {
						thp->runmask = ~new_runmask;
					}
				}
				first = 0;
			}
			break;
		default:
			break;
		}
		ready(thp);
		thp->runmask = orig_runmask;
		if(!all) {
			break;
		}
	}
}

SYNC * rdecl
synchash_lookup(OBJECT *obp, unsigned addr) {
	SYNC		**owner;
	SYNC		*syp;

	owner = (SYNC **)sync_hash.table[HASH_FUNC(sync_hash, addr)];
	if(owner != NULL) {
		owner += SEC_HASH(addr);
		for( ;; ) {
			syp = *owner;
			if(syp == NULL) break;
			if((syp->addr == addr) && (syp->obj == obp)) {
				return(syp);
			}
			owner = &syp->next;
		}
	}
	return(NULL);
}


int rdecl
synchash_add(OBJECT *obp, unsigned addr, SYNC *syp) {
	unsigned	index, sindex;
	SYNC		**table;

	if(synchash_lookup(obp, addr)) {
		return(-1);
	}

	syp->obj = obp;
	syp->addr = addr;

	index = HASH_FUNC(sync_hash, addr);
	table = (SYNC **)(void *)sync_hash.table[index];

	if(table == NULL) {
		table = _scalloc(sizeof (SYNC *) * SEC_HASH_BINS);
		if(table == NULL) return -1;
		sync_hash.table[index] = table;
	}
	sindex = SEC_HASH(addr);
	syp->next = table[sindex];
	table[sindex] = syp;

	return(0);
}


void rdecl
synchash_rem(unsigned addr, OBJECT *obp, unsigned addr1, unsigned addr2, PROCESS *prp, void *vaddr) {
	unsigned	 index, sindex, eindex;
	SYNC		*prev, *syp, **table;

	index = HASH_FUNC(sync_hash, addr);
	table = (SYNC **)(void *)sync_hash.table[index];

	if(table == NULL) return;

	/* We have two cases, either remove one or scan a whole page */
	if(addr1 == addr2) {
		sindex = SEC_HASH(addr);
		eindex = sindex + 1;
	} else {
		sindex = 0;
		eindex = SEC_HASH_BINS;
	}

	for(index = sindex; index < eindex; index ++) {
		for(prev = (SYNC *)(void *)&table[index]; (syp = prev->next); prev = syp) {
			if(syp->obj == obp  &&  syp->addr >= addr1  &&  syp->addr <= addr2) {
				PROCESS 			*prp1;
				THREAD				*thp;
				struct syncevent_entry *syncep;
				struct syncevent_entry *next;
				unsigned			event_state;

				#define EVENT_DELIVER	0x01
				#define EVENT_PROCESSED	0x02

				if(prp) {
					// Are we are unmapping a region that has a mutex in it?
					sync_t				*sync = (sync_t *)((char *)vaddr + (syp->addr - addr1));

					unlock_kernel();
					//
					// Can't use WR_PROBE_INT on the sync->__owner because it is not
					// SMP friendly (on x86 it does a read-modify-write with out a 
					// lock prefix).  Therefore we use atomic_set() to check if we
					// can write to sync->__owner so that we're SMP safe.
					//
#if defined(VARIANT_smp)
					atomic_set(&sync->__owner, 0);
#else
					WR_PROBE_INT(act, &sync->__owner, 1);
#endif
					event_state = 0;

					lock_kernel();

					// Is the mutex currently locked by a thread in the unmapping process
					if(SYNC_PINDEX(sync->__owner) == SYNC_PINDEX(SYNC_OWNER_BITS(prp->pid, 0))) {
						event_state = EVENT_DELIVER;

						thp = pril_first(&syp->waiting);
						if(thp != NULL) {
							unsigned	type = TYPE_MASK(thp->type);

							if((type == TYPE_THREAD && thp->state == STATE_MUTEX) || (type == TYPE_SYNCEVENT && ((struct syncevent_entry *)thp)->subtype == SYNCEVENT_SUBTYPE_PRIORITYCEILING)) {
								if(thp->args.mu.owner) {
									mutex_holdlist_rem(syp);
									thp->args.mu.owner = 0;
								}
							}
						}
						sync->__owner = _NTO_SYNC_DEAD;

					}
					// looking for any event attached to the mutex
					for(syncep = pril_first(&syp->waiting); syncep != NULL; syncep = next) {
						next = pril_next(syncep);
						if(TYPE_MASK(syncep->type) == TYPE_SYNCEVENT && syncep->subtype == SYNCEVENT_SUBTYPE_EVENT) {
							prp1 = syncep->un.ev.process;
							if(prp1 == prp) {
								if(prp->guardian == NULL) {
									// the process is going away, delete the event
									pril_rem(&syp->waiting, syncep);
									object_free(prp, &syncevent_souls, syncep);
#ifdef EVENT_REVIVE_LATER
									if ( event_state & EVENT_DELIVER ) {
										event_state |= EVENT_PROCESSED;
									}
#endif
									continue;
								}
								// guardian inherits the mutex event
								syncep->un.ev.process = prp->guardian;
								syncep->un.ev.tid = 0;
								syncep->un.ev.event.sigev_notify = SIGEV_SIGNAL;
								syncep->un.ev.event.sigev_signo = 0;
							}
							if(event_state & EVENT_DELIVER) {
								// deliver event
								event_state |= EVENT_PROCESSED;
								thp = vector_lookup(&prp1->threads, syncep->un.ev.tid);
								if((thp != NULL) || ((thp = prp1->valid_thp) != NULL)) {
									sigevent_exe(&syncep->un.ev.event, thp, 1);
								}
							}
						}
					}

					if(event_state == EVENT_DELIVER) {
						// supposed to deliver event, but no event found
						sync->__owner = _NTO_SYNC_DESTROYED;
						while((thp = pril_first(&syp->waiting))) {
							if(TYPE_MASK(thp->type) != TYPE_THREAD) {
								thp = pril_next(thp);
								if(!((thp != NULL) && TYPE_MASK(thp->type) == TYPE_THREAD)) {
									break;
								}
							}
							// Kill all threads waiting on the mutex without a timeout
							if((thp->timeout_flags & (_NTO_TIMEOUT_ACTIVE | (1 << STATE_MUTEX))) !=
									(_NTO_TIMEOUT_ACTIVE | (1 << STATE_MUTEX))) {
								(void)signal_kill_thread(thp->process, thp, SIGDEADLK, SI_NOINFO, (intptr_t)sync, prp->pid,0);
								if(thp == pril_first(&syp->waiting)) {
									/* SIGDEADLK must be blocked */
									force_ready(thp, EOWNERDEAD);
								}
							}
						}
					}
				} else {
					// remove the sync from its holding list
					thp = pril_first(&syp->waiting);
					if(thp != NULL) {
						unsigned	type = TYPE_MASK(thp->type);

						if(((type == TYPE_THREAD && thp->state == STATE_MUTEX) || (type == TYPE_SYNCEVENT && ((struct syncevent_entry *)thp)->subtype == SYNCEVENT_SUBTYPE_PRIORITYCEILING))
						  && thp->args.mu.owner) {
							mutex_holdlist_rem(syp);
							thp->args.mu.owner = 0;
						}
					
						// Force ready any threads blocked on the sync
						do {
							if(TYPE_MASK(thp->type) == TYPE_THREAD) {
								force_ready(thp, EINVAL);
							} else {
								syncep = (struct syncevent_entry *)thp;
								pril_rem(&syp->waiting, syncep);
								if(syncep->subtype == SYNCEVENT_SUBTYPE_EVENT) {
									object_free(syncep->un.ev.process, &syncevent_souls, syncep);
								} else { /* SYNCEVENT_SUBTYPE_PRIORITYCEILING */
									object_free(NULL, &thread_souls, syncep);
								}
							}
							thp = pril_first(&syp->waiting);
						} while(thp != NULL);
					}

					prev->next = syp->next;
					object_free(NULL, &sync_souls, syp);
					syp = prev;
					// Optimize the case of single delete
					if(addr1 == addr2) return;
				}
			}
		}
	}

#if 0
// Check code to make sure things are consistant...
	for(syp = sync_hash.table[index] ; syp ; syp = syp->next)
		if(syp->addr >= addr1  &&  syp->addr <= addr2)
			crash();
#endif
}


struct kerargs_synchash_rem {
	unsigned		addr;
	OBJECT			*obp;
	off_t			start;
	off_t			end;
	PROCESS			*prp;
	void			*vaddr;
};

static void
ker_synchash_rem(void *data) {
	struct kerargs_synchash_rem	*kap = data;
	unsigned	ker_lock;

	ker_lock = get_inkernel() & INKERNEL_LOCK;
	lock_kernel();
	if (kap->addr > (kap->end | (HASH_PAGESIZE - 1))) {
		KerextStatus(NULL, 0);
		return;
	}
	for(;;) {
		synchash_rem(kap->addr, kap->obp, kap->start, kap->end, kap->prp, kap->vaddr);
		if((kap->addr & ~(HASH_PAGESIZE - 1)) == (kap->end & ~(HASH_PAGESIZE - 1)))
			break;
		kap->addr += HASH_PAGESIZE;
		if(ker_lock == 0) {
			unlock_kernel();
			KEREXT_PREEMPT(actives[KERNCPU]);
			lock_kernel();
		}
	}
	KerextStatus(NULL, 0);
}

void
MemobjDestroyed(OBJECT *obp, off_t start, off_t end, PROCESS *prp, void *vaddr) {
	struct kerargs_synchash_rem	data;
	int ret;

	data.addr = start;
	data.obp = obp;
	data.start = start;
	data.end = end;
	data.prp = prp;
	data.vaddr = vaddr;

	if (am_inkernel()) {
		// should never been reached
		ker_synchash_rem(&data);
	} else {
		if(data.addr > (data.end | (HASH_PAGESIZE - 1)))
			return;
		for(;;) {
			ret = __Ring0(ker_synchash_rem, &data);

			if ((data.addr & ~(HASH_PAGESIZE - 1)) ==
			    (data.end & ~(HASH_PAGESIZE - 1)))
				break;

			if (ret != 0) {
				// If Ring0() returns a failure indication (errno == EFAULT),
				// synchash_rem() faulted during the test to see if sync->__owner
				// is writable. In that case, addvance data.addr to the next
				// page to avoid an infinite loop
				data.addr += HASH_PAGESIZE;
			}
		}
	}
}


void
mutex_holdlist_add(THREAD *thp, SYNC *syp) {
	SYNC	**owner;
	SYNC	*curr;
	THREAD	*new;

	new = pril_first(&syp->waiting);
	owner = &thp->mutex_holdlist;
	for( ;; ) {
		curr = *owner;
		if(curr == NULL) break;
		thp = pril_first(&curr->waiting);
		if(thp->priority < new->priority) break;
		owner = &thp->args.mu.next;
	}
	new->args.mu.next = curr;
	new->args.mu.prev = owner;
	if(curr != NULL) {
		thp->args.mu.prev = &new->args.mu.next;
	}
	*owner = syp;
}


void
mutex_holdlist_rem(SYNC *syp) {	
	THREAD	*thp;
	THREAD	*next_thp;
	SYNC	*next_syp;


	thp = pril_first(&syp->waiting);
	next_syp = thp->args.mu.next;
	*thp->args.mu.prev = next_syp;
	if(next_syp != NULL) {
		next_thp = pril_first(&next_syp->waiting);
		next_thp->args.mu.prev = thp->args.mu.prev;
	}
}

__SRCVERSION("nano_sync.c $Rev: 193078 $");
