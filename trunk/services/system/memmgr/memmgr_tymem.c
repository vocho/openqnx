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

#include <sys/rsrcdbmgr.h>
#include <sys/rsrcdbmsg.h>


static pthread_mutex_t		rl_mux = PTHREAD_MUTEX_INITIALIZER;

static int
match_name(struct asinfo_entry *base_as, struct asinfo_entry *as, 
				const char *name, unsigned name_len) {
	const char				*str = SYSPAGE_ENTRY(strings)->data;
	const char				*curr;
	const char				*p;
	unsigned				len;

	p = &name[name_len];
	for( ;; ) {
	    if(p == name) break;
	    if(p[-1] == '/') break;
	    --p;
	}
	curr = &str[as->name];
	len = name_len - (p - name);

	//Check that this piece matches
	if(strlen(curr) != len) return 0;
	if(memcmp(curr, p, len) != 0) return 0;

	//See if that was the whole given name
	if(p == name) return 1;

	// First character in given name is a "/" => must be top level asinfo.
	if((p == &name[1]) && (as->owner == AS_NULL_OFF)) return 1;

	//More to given name, no owner for current asinfo
	if(as->owner == AS_NULL_OFF) return 0;

	//See if prefix also matches.
	return match_name(base_as, 
				(struct asinfo_entry *)((uint8_t *)base_as + as->owner), 
				name, (name_len - len) - 1);
}


static void
kill_restrict(struct pa_restrict *par) {
	struct pa_restrict	*next;

	while(par != NULL) {
		next = par->next;
		_sfree(par, sizeof(*par));
		par = next;
	}
}


static int
one_restrict_create(const char *name, unsigned name_len, struct pa_restrict **rp) {
	struct asinfo_entry		*base_as = SYSPAGE_ENTRY(asinfo);
	struct asinfo_entry		*as;
	struct asinfo_entry		*try_as;
	unsigned				num;
	struct pa_restrict		*par;
	struct pa_restrict		*new;
	struct pa_restrict		**owner;
	struct pa_restrict		*try;

	// We're hiding the asinfo pointer inside the checker field of the
	// restrict entry so we can remember both the priority & alloc_checker
	// field in this routine. It all gets fixed up at the end.

	as = base_as;
	par = NULL;
	num = _syspage_ptr->asinfo.entry_size / sizeof(*as);
	while(num != 0) {
		if(match_name(base_as, as, name, name_len)) {
			owner = &par;
			for( ;; ) {
			    try = *owner;
			    if(try == NULL) break;
			    try_as = (struct asinfo_entry *)try->checker;
			    if(try_as->priority < as->priority) break;
			    owner = &try->next;
			}
			new = _smalloc(sizeof(*new));
			if(new == NULL) {
				kill_restrict(par);
				return ENOMEM;
			}
			new->start = as->start;
			new->end = as->end;
			new->checker = (void *)as;
			new->next = *owner;
			*owner = new;
		}
		++as;
		--num;
	}
	if(par == NULL) return ENOENT;
	//RUSH3: coalese overlapping/adjacent entries where possible
	for(try = par; try != NULL; try = try->next) {
		try_as = (void *)try->checker;
		try->checker = try_as->alloc_checker;
	}
	*rp = par;
	return EOK;
}


static int
multi_restrict_create(const char *name, OBJECT *obp, memclass_attr_t *attr) {
	int					is_sysram;
	struct pa_restrict	*try;
	struct pa_restrict 	*new;
	struct pa_restrict	*par;
	struct pa_restrict	**owner;
	struct pa_restrict	*check1;
	struct pa_restrict	*check2;
	const char			*curr;
	char				last_op;
	int					r;

	obp->mem.mm.restriction = NULL;
	par = NULL;
	curr = name;
	last_op = ' ';
	do {
		switch(*curr) {
		case '\0':	
		case '|':
		case '&':
			if(curr == name) {
				kill_restrict(par);
				return EINVAL;
			}
			r = one_restrict_create(name, curr - name, &new);
			if(r != EOK) {
				kill_restrict(par);
				return r;
			}
			switch(last_op) {
			case ' ':
				// first time, just set par to new
				par = new;
				break;
			case '|':	
				// unioning the two names together, just tack the new
				// restriction list to the end of the first
				owner = &par;
				for( ;; ) {
					try = *owner;
					if(try == NULL) break;
					owner = &try->next;
				}
				*owner = new;
				break;
			case '&':
				// want the intersection of the two restriction lists
				try = NULL;
				owner = &try;
				for(check1 = par; check1 != NULL; check1 = check1->next) {
					// walk the second list and look for overlapping entries
					for(check2 = new; check2 != NULL; check2 = check2->next) {
						paddr_t		start = ~(paddr_t)0;
						paddr_t		end   = (paddr_t)0;

						if((check2->start >= check1->start) && (check2->end <= check1->end)) {
							// 'check2' fully contined within 'check1'
							start = check2->start;
							end   = check2->end;
						} else if((check1->start >= check2->start) && (check1->end <= check2->end)) {
							// 'check1' fully contained within 'check2'
							start = check1->start;
							end   = check1->end;
						} else if((check2->start < check1->start) && (check2->end > check1->start)) {
							// start of 'check1' included in 'check2'
							start = check1->start;
							end   = check2->end;
						} else if((check2->start < check1->end) && (check2->end > check1->end)) {
							// end of 'check1' included in 'check2'
							start = check2->start;
							end   = check1->end;
						}
						if(start < end) {
							// found an overlap
							struct pa_restrict 	*p;

							p = _smalloc(sizeof(*p));
							if(p == NULL) {
								kill_restrict(par);
								kill_restrict(new);
								return ENOMEM;
							}
							p->next = NULL;
							p->checker = check1->checker;
							p->start = start;
							p->end = end;
							*owner = p;
							owner = &p->next;
						}
					}
				}
				kill_restrict(par);
				kill_restrict(new);
				par = try;
				break;
			default:
				break;
			}
			last_op = *curr;
			name = curr + 1;
			break;
		default:
			break;
		}
		++curr;
	} while(last_op != '\0');
	if(par == NULL) {
		// empty intersection....
		return ENODEV;
	}
	attr->size = 0;
	attr->limits.alloc.size.min = __PAGESIZE;			// FIX ME
	attr->limits.alloc.size.max = memsize_t_INFINITY;	// FIX ME
	is_sysram = 0;
	for(try = par; try != NULL; try = try->next) {
		// If any piece of the restriction list overlaps with a
		// system ram entry, we're only going to allocate out of the
		// system ram piece. If a POSIX_TYPED_MEM_MAP_ALLOCATABLE request
		// is used for that region, it's the user's lookout.
		is_sysram |= pa_sysram_overlap(try);
		//RUSH2: This is actually wrong since there can be overlapping
		//RUNS2: entries in the restriction list
		attr->size += (try->end - try->start) + 1;
	}
	if(!is_sysram) obp->mem.mm.flags |= MM_MEM_RDB;
	obp->mem.mm.restriction = par;
	return EOK;
}


int
tymem_create(OBJECT *obp, void *extra) {
	return shmem_create(obp, extra);
}


int
tymem_done(OBJECT *obp) {
	if(obp->mem.mm.refs != NULL) return 0;

	/* FIX ME - following required as per comments on per open() OBJECT creation */
	if(obp->tymem.name != NULL) {
		_sfree(obp->tymem.name, strlen(obp->tymem.name + 1));
	}

	return 1;
}

/*
 * tymem_register
 * 
 * this routine ties the memclass table to the resource database. Its main
 * purpose is to check for the existence of <name> and add it to the class table
 * with the startup defined attributes if it does exist.
 * 
 * The default access functions will be used hence NULL is passed for the
 * 'allocator_accessfncs_t *' parameter
 * 
 * On error, errno will be returned and the entry will not be added
 * 
 * Should be reworked so that there is one memory class repository
*/ 
int
tymem_register(const char *name) {

	OBJECT  		tmp_obj;
	memclass_attr_t	attr;
	int  			r;

	r = multi_restrict_create(name, &tmp_obj, &attr);
	if((r != EOK) || (memclass_add(name, &attr, NULL) == memclass_id_t_INVALID)) {
		r = (r == EOK) ? ENOENT : r;
	}

/*
 * FIX ME
 * Currently what I am doing is using restrict_create() with a temporary OBJECT
 * only to setup the 'attr' structure for the memclass_add() call since its knows
 * about sizes, etc. Then, I delete the restriction and the temporary OBJECT.
 * When an open() on the memory class is done, I recreate the restriction so that
 * can have an OBJECT per open() similar to a processes anonymous memory OBJECT.
 * As a minimum, I should eliminate the restrict_create() call for every open since
 * it is wasteful. Not positve I even need the per open() OBJECT. Could just account
 * the OBJECT (and its name) to the system heap. 
 * 
*/
	kill_restrict(tmp_obj.mem.mm.restriction);
	return r;
}

//RUSH1: permission checking? 
int
memmgr_tymem_open(const char *name, int oflags, OBJECT **obpp, PROCESS *prp) {
	OBJECT				*obp;
	unsigned			name_len;
	int					r;
	memclass_attr_t  	dummy_attr;
	memclass_entry_t  	*e;
	memclass_id_t  		mclass_id = sys_memclass_id;

	pthread_mutex_lock(&rl_mux);
	if(MEMPART_INSTALLED()) {
		r = tymem_register(name);
		if(r != EOK) {
			goto fail1;
		}
		e = memclass_find(name, memclass_id_t_INVALID);
		if(e == NULL) {
			r = ENOENT;
			goto fail1;
		}
		/*
		 * if memory partitioning is installed and the caller is trying to open the
		 * typed memory for allocation, then the caller is required to be associated
		 * with a partition of the class 
		*/
		if(oflags & (POSIX_TYPED_MEM_ALLOCATE | POSIX_TYPED_MEM_ALLOCATE_CONTIG)) {
			if(mempart_getid(prp, e->data.info.id) == part_id_t_INVALID) {
				r = ENOENT;
				goto fail1;
			}
			mclass_id = e->data.info.id;
		}
	}
	r = ENOMEM;
	/*
	 * although this OBJECT will be created in order to represent a partition
	 * of a different class of memory, the OBJECT itself will be allocated from
	 * the callers sysram memory partition
	*/
	obp = object_create(OBJECT_MEM_TYPED, NULL, prp, mclass_id);
	if(obp == NULL) goto fail1;

	name_len = strlen(name) + 1;
	obp->tymem.name = _smalloc(name_len);
	if(obp->tymem.name == NULL) {
		goto fail2;
	}
	memcpy(obp->tymem.name, name, name_len);
	r = multi_restrict_create(name, obp, &dummy_attr);
	if(r != EOK) goto fail2;

	*obpp = obp;
	pthread_mutex_unlock(&rl_mux);
	return EOK;
	
fail2:
	object_done(obp);

fail1:	
	pthread_mutex_unlock(&rl_mux);
	return r;
}


void
memmgr_tymem_close(OBJECT *obp) {
	memobj_lock(obp);
	kill_restrict(obp->mem.mm.restriction);
	(void)shmem_done(obp); // Free up pmem list if possible
	memobj_unlock(obp);
}


struct pa_quantum *
tymem_rdb_alloc(OBJECT *obp, size_t size, unsigned flags) {
	rsrc_request_t		req;
	int					r;
	struct pa_quantum	*head;
	struct pa_quantum	**owner;
	struct pa_quantum	*pq;
	size_t				requested_size;
	struct pa_restrict	*rl;
	memsize_t			resv_size = 0;
	memclass_entry_t *e;
	memclass_sizeinfo_t cur, prev;

	head = NULL;
	owner = &head;
	requested_size = size;

	/* check to see if the partition will allow the requested allocation */
	if (MEMPART_CHK_and_INCR(obp->hdr.mpid, requested_size, &resv_size) != EOK) {
		return NULL;
	}
	e = memclass_find(NULL, mempart_get_classid(obp->hdr.mpid));

#ifndef NDEBUG
	if (e == NULL) crash();
	if ((r = pthread_mutex_lock(&e->lock.mutex)) != EOK) crash();
#else	/* NDEBUG */

	/* this lock won't work if the kernel ever uses non sysram memory for any internal objects */
	CRASHCHECK(KerextAmInKernel());
	pthread_mutex_lock(&e->lock.mutex);
#endif	/* NDEBUG */

	prev = e->data.info.size;

	do {
restart:
		rl = obp->mem.mm.restriction;

		for( ;; ) {
			req.length = size;
			req.start = rl->start;
			req.end = rl->end;
			req.align = __PAGESIZE;
			req.flags = RSRCDBMGR_MEMORY | RSRCDBMGR_FLAG_RANGE | RSRCDBMGR_FLAG_ALIGN;
			
			/* can the allocator satisfy the request ? */
			if ((e->data.info.size.reserved.free < resv_size) ||
				(e->data.info.size.unreserved.free < (size - resv_size))) {
				goto fail1;
			}
			if ((r = rsrcdbmgr_proc_interface(&req, 1, RSRCDBMGR_REQ_ATTACH)) == EOK) {
				if (size <= resv_size) {
					e->data.info.size.reserved.free -= size;
					e->data.info.size.reserved.used += size;
				}
				else {
					e->data.info.size.reserved.free -= resv_size;
					e->data.info.size.reserved.used += resv_size;
					e->data.info.size.unreserved.free -= (size - resv_size);
					e->data.info.size.unreserved.used += (size - resv_size);
				}
				pq = pa_alloc_fake(req.start, size);
				if(pq == NULL) {
					pthread_mutex_unlock(&e->lock.mutex);
					goto fail2;
				}
				pq->u.inuse.next = NULL;
				pq->flags |= PAQ_FLAG_RDB;
				*owner = pq;
				owner = &pq->u.inuse.next;
				break;
			}
			rl = rl->next;
			if(rl == NULL) {
				if(flags & PAA_FLAG_CONTIG) goto fail1;
				if(size <= __PAGESIZE) goto fail1;
				size >>= 1;
				goto restart;
			}
		}
		requested_size -= size;
	} while(requested_size != 0);

	cur = e->data.info.size;
	pthread_mutex_unlock(&e->lock.mutex);
	MEMCLASS_PID_USE_INKER(obp->hdr.creator, obp->hdr.mpid, size);
	MEMCLASSMGR_EVENT(&e->data, memclass_evttype_t_DELTA_TU_INCR, &cur, &prev);
	return head;

fail2:
	tymem_rdb_free(obp, req.start, size);

fail1:
	while(head != NULL) {
		pq = head->u.inuse.next;
		tymem_rdb_free(obp, pa_quantum_to_paddr(head), NQUANTUM_TO_LEN(head->run));
		pa_free_fake(head);
		head = pq;
	}

	MEMPART_UNDO_INCR(obp->hdr.mpid, requested_size, resv_size);
	pthread_mutex_unlock(&e->lock.mutex);
	return NULL;
}


struct pa_quantum *
tymem_rdb_alloc_given(OBJECT *obp, paddr_t paddr, size_t size, int *rp) {
	rsrc_request_t		req;
	int					r;
	struct pa_quantum	*pq;
	memsize_t			resv = 0;

	if (MEMPART_CHK_and_INCR(obp->hdr.mpid, size, &resv) != EOK) {
		return NULL;
	}

	req.length = size;
	req.start = paddr;
	req.end = paddr + size - 1;
	req.flags = RSRCDBMGR_MEMORY | RSRCDBMGR_FLAG_RANGE;
	r = rsrcdbmgr_proc_interface(&req, 1, RSRCDBMGR_REQ_ATTACH);
	if(r != EOK) {
		MEMPART_UNDO_INCR(obp->hdr.mpid, size, resv);
		goto fail1;
	}
	r = ENOMEM;
	pq = pa_alloc_fake(paddr, size);
	if(pq == NULL) goto fail2;

	MEMCLASS_PID_USE_INKER(obp->hdr.creator, obp->hdr.mpid, size);
	pq->flags |= PAQ_FLAG_RDB;
	return pq;

fail2:		
	tymem_rdb_free(obp, paddr, size);

fail1:	
	*rp = r;
	return NULL;
}


void
tymem_rdb_free(OBJECT *obp, paddr_t paddr, size_t size) {
	rsrc_request_t	req;
	int  ret;

	//RUSH2: handle partial frees
	req.start = paddr;
	req.end = paddr + size - 1;
	req.length = size;
	req.flags = RSRCDBMGR_MEMORY;
	ret = rsrcdbmgr_proc_interface(&req, 1, RSRCDBMGR_REQ_DETACH);
	if (ret == EOK) {
		memsize_t  resv_size = MEMPART_DECR(obp->hdr.mpid, size);
		memclass_sizeinfo_t cur, prev;
		memclass_id_t memclass_id = mempart_get_classid(obp->hdr.mpid);
		memclass_entry_t *e = memclass_find(NULL, memclass_id);

		prev = e->data.info.size;

		CRASHCHECK(size < resv_size);

		/* LOCK */		
		/* this lock won't work if the kernel ever uses non sysram memory for any internal objects */
		CRASHCHECK(KerextAmInKernel());
		if (pthread_mutex_lock(&e->lock.mutex) != EOK) crash();

		e->data.info.size.reserved.free += resv_size;
		e->data.info.size.reserved.used -= resv_size;
		e->data.info.size.unreserved.free += (size - resv_size);
		e->data.info.size.unreserved.used -= (size - resv_size);

		cur = e->data.info.size;
		/* UNLOCK */
		pthread_mutex_unlock(&e->lock.mutex);
		MEMCLASS_PID_FREE_INKER(obp->hdr.creator, obp->hdr.mpid, size);
		MEMCLASSMGR_EVENT(&e->data, memclass_evttype_t_DELTA_TU_DECR, &cur, &prev);
	}
}

static void
tymem_rdb_free_info(struct pa_restrict *rlp, paddr_t *totalp, paddr_t *contigp) {
	struct pa_restrict	*rp;
	int					num;
	unsigned			idx;
	unsigned			i;
	rsrc_alloc_t		list[20];
	paddr_t				start;
	paddr_t				end;
	paddr_t				total;
	paddr_t				contig;

	total = 0;
	contig = 0;

	idx = 0;
	for( ;; ) {
		num = rsrcdbmgr_proc_query(list, NUM_ELTS(list), idx, RSRCDBMGR_MEMORY);
		if(num <= 0) break;
		idx += num;
		for(i = 0; i < num; ++i) {
			if(!(list[i].flags & RSRCDBMGR_FLAG_USED)) {
				for(rp = rlp; rp != NULL; rp = rp->next) {
					//RUSH3: If the list is in ascending order, kick out early
					//RUSH3: if list[i].end < rp->start
					start = max(list[i].start, rp->start);
					end   = min(list[i].end, rp->end);		
					if(start < end) {
						paddr_t		size;

						size = (end - start) + 1;
						total += size;
						if(size > contig) contig = size;
					}
				}
			}
		}
	}
	if(totalp != NULL) *totalp = total;
	if(contigp != NULL) *contigp = contig;
}


void
tymem_free_info(OBJECT *obp, paddr_t *total, paddr_t *contig) {
	if(obp->mem.mm.flags & MM_MEM_RDB) {
		tymem_rdb_free_info(obp->mem.mm.restriction, total, contig);	
	} else {
		//RUSH3: This doesn't work for a physical system - the pa_*
		//RUSH3: routines are only used by the virtual memmgr code.
		pa_free_info(obp->mem.mm.restriction, total, contig);
	}
}

size_t rdb_reserve(size_t size)
{
	return size;
}
