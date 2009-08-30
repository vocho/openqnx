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


// Support routines for the kernel.
void rdecl
kererr(THREAD *thp, int err) {

	if(TYPE_MASK(thp->type) == TYPE_VTHREAD) {
		return;
	}

	if(get_inkernel() & INKERNEL_NOW) {
		lock_kernel();
	}

	// Last error wins.
	// The last error winning is used when the condvar
	// code calls the mutex code so the condvar code
	// can override the error.
	SETKSTATUS(thp, err);

	// Check for multiple errors in one kernel call.
	if(err != EOK && (thp->flags & _NTO_TF_KERERR_SET) == 0) {
		thp->flags |= _NTO_TF_KERERR_SET;
		thp->restart = NULL;
		timeout_stop(thp);
		SETKIP(thp, KIP(thp)+KERERR_SKIPAHEAD);
	}
}


void rdecl
kerunerr(THREAD *thp) {

	// Ignore unless an error is already set;
	if(thp->flags & _NTO_TF_KERERR_SET) {
		thp->flags &= ~_NTO_TF_KERERR_SET;
		SETKIP(thp, KIP(thp)-KERERR_SKIPAHEAD);
	}
}


int rdecl
kerisroot(THREAD *thp) {

	if(thp->process->cred->info.euid == 0) {
		return(1);
	}

	kererr(thp, EPERM);
	return(0);
}


int rdecl
kerisusr(THREAD *srcthp, PROCESS *dstprp) {
	CREDENTIAL	*src = srcthp->process->cred;
	CREDENTIAL	*dst = dstprp->cred;

	if(src->info.euid == 0  ||
	   src->info.ruid == dst->info.ruid  ||
	   src->info.ruid == dst->info.suid  ||
	   src->info.euid == dst->info.ruid  ||
	   src->info.euid == dst->info.suid) {
		return(1);
	}

	kererr(srcthp, EPERM);
	return(0);
}

int rdecl
keriskill(THREAD *srcthp, PROCESS *dstprp, int signo) {
	if(signo == SIGCONT && dstprp->session == srcthp->process->session) {
		return(1);
	}
	return kerisusr(srcthp, dstprp);
}

int rdecl
kerschedok(THREAD *thp, int policy, const struct sched_param *param) {
	if(param->sched_priority < 0 || param->sched_priority >= NUM_PRI) return EINVAL;
	if(param->sched_priority == 0 && (thp->process->flags & (_NTO_PF_LOADING | _NTO_PF_RING0)) != _NTO_PF_RING0) return EINVAL;
	if(thp->process->cred->info.euid != 0 && param->sched_priority >= priv_prio) return EPERM;
	if(policy < SCHED_NOCHANGE || policy > SCHED_SPORADIC) return EINVAL;
	if(policy != SCHED_SPORADIC) return EOK;
	if(param->sched_ss_max_repl < 1 || param->sched_ss_max_repl > SS_REPL_MAX) return EINVAL;
	if(param->sched_ss_low_priority <= 0 || param->sched_ss_low_priority > param->sched_priority) return EINVAL;
	if(timespec2nsec(&param->sched_ss_repl_period) < timespec2nsec(&param->sched_ss_init_budget)) return EINVAL;
	return EOK;
}

int
kercall_attach(unsigned callnum, int kdecl (*func)()) {

	if(callnum >= __KER_BAD)
		return(-1);

	ker_call_table[callnum] = func;
#if defined(VARIANT_instr)
	_trace_ker_call_table[callnum] = func;
	_trace_call_table[callnum] = func;
#endif

	return(0);
}

/*
 * This function is there to reference
 * the get_cpunum() and send_ipi stub functions
 * in the non-SMP kernel. This is needed for linking optional
 * modules such as APS.
 */
void __reference_smp_funcs(void) {
	(void)get_cpunum();
	send_ipi(0, 0);
}

// Can be set to one by the kernel debugger so we can continue on
// from a crash() message
int kdebug_walkout;

void rdecl
(crash)(const char *file, int line) {

	InterruptDisable();
	kprintf("Crash[%d,%d] at %s line %d.\n", RUNCPU, KERNCPU, file, line);
	for(;;) {
		InterruptDisable();
		DebugKDBreak();
		if(privateptr->kdebug_call == NULL) {
			RebootSystem(1);
		}
		if(kdebug_walkout) break;
	}
}


//
// Routines to free any unrequired memory when the physical allocator
// can't satisfy an allocation
//

struct purger {
	struct purger	*next;
	unsigned		prio;
	int				(*purger)(size_t);
};
static struct purger *purge_list;
SMP_SPINVAR(static, purge_slock);

int
purger_invoke(size_t amount) {
	struct purger	*purge;

	purge = purge_list;
	for( ;; ) {
		if(purge == NULL) return 0;
		if(purge->purger(amount)) return 1;
		purge = purge->next;
	}
}


int
purger_register(int (*purger)(size_t), unsigned prio) {
	struct purger	*new;
	struct purger	**owner;
	struct purger	*chk;

	// Something to note. If we ever start allowing unregistering
	// purgers (why, I don't know, but play along), there's going
	// to be a problem freeing the struct purger entries. This function
	// might be called from either the kernel or process manager, so
	// the _smalloc might come from either the kernel or procnto heap.
	// the _sfree will have to be careful to free it back to the
	// right place. My take: do a _Ring0() call and always allocate
	// from the kernel heap. It'd also let you get rid of the INTR_[UN]LOCK
	// below when inserting into the list.
	new = _smalloc(sizeof(*new));
	if(new == NULL) return ENOMEM;
	new->prio = prio;
	new->purger = purger;
	owner = &purge_list;
	for( ;; ) {
		chk = *owner;
		if(chk == NULL) break;
		if(chk->prio > prio) break;
		owner = &chk->next;
	}
	if(alives[0]) {
		INTR_LOCK(&purge_slock);
	}
	new->next = *owner;
	*owner = new;
	if(alives[0]) {
		INTR_UNLOCK(&purge_slock);
	}
	return EOK;
}


//
// The PRIL (PRIority List) data structure is a modified doubly linked
// list. The next pointers are the usual. The prev pointer field is
// as per usual *except* for the first element on the list of a particular
// priority (known as a HEAD entry). In that case, what's normally the prev 
// field instead points at the last element on the list of the same priority
// (the TAIL entry). In this particular implementation, HEAD entries are
// marked with FLAG_HEAD in the 'type' field, though we could actually
// store the information in the bottom bit of the 'prev' field if we didn't
// have another field to put it in. TAIL entries have FLAG_TAIL set in the
// the 'type' field - we don't really need that bit since you can tell you're
// a TAIL entry if (p->next == NULL || p->priority != p->next->priority),
// but it simplifies the code in pril_rem() to maintain the information
// as a bit. In the diagram below, the number in the centre of the boxes
// is the priority of the entry, the lines at the top are the next field
// and the prev field are the bottom lines. The "H" and "T" on top of the
// boxes indicate the HEAD and TAIL flags.
//
//    H                   T      H T      H          T
//    ### ---> ### ---> ### ---> ### ---> ### ---> ###
//    #9#      #9#      #9#      #7#      #6#      #6#
// +- ### <--- ### <--- ###   +- ###   +- ### <--- ###
// |                    ^     |  ^     |           ^
// +--------------------+     +--+     +-----------+
//

#define FLAG_TAIL		(TYPE_FLAG_FIRST << 1)
#define FLAG_HEAD		(TYPE_FLAG_FIRST << 2)
#define FLAG_UPDATER	(TYPE_FLAG_FIRST << 3)

// The pulse and signal queues have the ability to say that they want something
// inserted at the front, no matter what their priority is :-(. We handle
// that by setting the TYPE_FLAG_FIRST and treating that as an extra
// priority bit
#define PRIL_PRIORITY(p)	((p)->priority + (((p)->type & TYPE_FLAG_FIRST) ? NUM_PRI : 0))

static struct pril_update	*update_list;

//#define DEBUG_PRIL
#if defined(DEBUG_PRIL)
void
pril_validate(PRIL_HEAD *ph, PRIL *present, PRIL *not_present) {
	PRIL	**owner;
	PRIL	*p;
	PRIL	*n;

	owner = &ph->data;
	for( ;; ) {
		p = *owner;
		if(p == NULL) break;
		if(p == not_present) crash();
		if(p == present) {
			present = NULL;
		}
		if(p->type & FLAG_HEAD) {
			n = p->prev.prio_tail;
			if(PRIL_PRIORITY(p) != PRIL_PRIORITY(n)) crash();
		} else {
			if(p->prev.pril != owner) crash();
		}
		n = p->next.pril;
		if(p->type & FLAG_TAIL) {
			if((n != NULL) && PRIL_PRIORITY(p) <= PRIL_PRIORITY(n)) crash();
		} else {
			if(PRIL_PRIORITY(p) != PRIL_PRIORITY(n)) crash();
		}
		owner = &p->next.pril;
	}
	// There was either no verification entry to check, or it was
	// on the list if present == NULL.
	if(present != NULL) crash();
}
#else
#define pril_validate(h, p, np)
#endif


#if !defined(pril_first)
void * rdecl
pril_first(PRIL_HEAD *ph) {
	return ph->data;
}
#endif


#if !defined(pril_next)
void * rdecl
pril_next(void *p) {
	return ((PRIL *)p)->next.pril;
}
#endif


void * rdecl
pril_next_prio(void *d) {
	PRIL	*p = d;

	CRASHCHECK(!(p->type & FLAG_HEAD));
	return p->prev.prio_tail->next.pril;
}


void * rdecl
pril_find_insertion(PRIL_HEAD *ph, unsigned priority) {
	PRIL		*head;
	PRIL		*tail;
	PRIL		**owner;
	unsigned	n_prio;
	unsigned	h_prio;

	pril_validate(ph, NULL, NULL);
	n_prio = (uint8_t)priority;
	if(priority & _PULSE_PRIO_BOOST) n_prio += NUM_PRI;
	owner = &ph->data;
	for( ;; ) {
		head = *owner;
		if(head == NULL) break;
		h_prio = PRIL_PRIORITY(head);
		if(h_prio < n_prio) break;
		tail = head->prev.prio_tail;
		if(h_prio == n_prio) {
			return (priority & _PULSE_PRIO_HEAD) ? head : tail;
		}
		owner = &tail->next.pril;
	}
	return NULL;
}


void rdecl
pril_add(PRIL_HEAD *ph, void *d) {
	PRIL		*new = d;
	PRIL		*head;
	PRIL		*tail;
	PRIL		**owner;
	unsigned	n_prio;
	unsigned	h_prio;

	pril_validate(ph, NULL, new);
	n_prio = PRIL_PRIORITY(new);
	owner = &ph->data;
	for( ;; ) {
		head = *owner;
		if(head == NULL) goto new_level;
		h_prio = PRIL_PRIORITY(head);
		if(h_prio < n_prio) goto new_level;
		tail = head->prev.prio_tail;
		if(h_prio == n_prio) break;
		owner = &tail->next.pril;
	}

	// add to current level
	head->prev.prio_tail = new;
	new->next.pril = tail->next.pril;
	new->type = (new->type & ~FLAG_HEAD) | FLAG_TAIL;
	tail->type &= ~FLAG_TAIL;
	tail->next.pril = new;
	new->prev.pril = &tail->next.pril;
	pril_validate(ph, new, NULL);
	return;

new_level:	
	// create new level
	new->next.pril = head;
	new->type |= FLAG_HEAD|FLAG_TAIL;
	new->prev.prio_tail = new;
	*owner = new;
	pril_validate(ph, new, NULL);
	return;
}


void rdecl
pril_rem(PRIL_HEAD *ph, void *d) {
	PRIL		*p = d;
	PRIL		*next;
	PRIL		*t;
	PRIL		*head;
	PRIL		*tail;
	PRIL		**owner;
	unsigned	f;
	unsigned	p_prio;

	pril_validate(ph, p, NULL);
	next = p->next.pril;
	f = p->type;
	if(f & (FLAG_HEAD|FLAG_TAIL)) {
		p_prio = PRIL_PRIORITY(p);
		owner = &ph->data;
		for( ;; ) {
			head = *owner;
			if(PRIL_PRIORITY(head) == p_prio) break;
			tail = head->prev.prio_tail;
			owner = &tail->next.pril;
		}
		if(!(f & FLAG_TAIL)) {
			// just removing a head
			*owner = next;
			next->prev.prio_tail = p->prev.prio_tail;
			next->type |= FLAG_HEAD;
		} else if(!(f & FLAG_HEAD)) {
			// just removing a tail
			owner = p->prev.pril;
			*owner = next;
			t = (PRIL *)((uintptr_t)owner - offsetof(PRIL, next.pril));
			head->prev.prio_tail = t;
			t->type |= FLAG_TAIL;
		} else {
			// totally removing a level
			*owner = next;
		}
		p->type &= ~(FLAG_HEAD|FLAG_TAIL);
	} else {
		owner = p->prev.pril;
		next->prev.pril = owner;
		*owner = next;
	}
	pril_validate(ph, NULL, p);
	if(f & FLAG_UPDATER) {
		struct pril_update	*upp;

		// We're deleting an entry that somebody is using - update to
		// the next one
		for(upp = update_list; upp != NULL; upp = upp->next) {
			if(upp->pril == p) {
				upp->pril = next;
				next->type |= FLAG_UPDATER;
			}
		}
	}
}


void rdecl
pril_update_register(PRIL_HEAD *ph, struct pril_update *upp) {
	struct pril_update	*prev;
	struct pril_update	*chk;
	PRIL				*pril;

	pril = upp->pril;
	prev = NULL;
	chk = update_list;
	for( ;; ) {
		if(chk == NULL) break;
		// We're keeping the list sorted so that we can
		// keep all updaters working on the same pril together. 
		// That way it's easier for the pril_update_unregister
		// routine to know when to turn off FLAG_UPDATER.
		if((uintptr_t)chk->pril >= (uintptr_t)pril) break;
		prev = chk;
		chk = chk->next;
	}
	upp->next = chk;
	upp->prev = prev;
	if(chk != NULL) {
		chk->prev = upp;
	}
	if(prev != NULL) {
		prev->next = upp;
	} else {
		update_list = upp;
	}
	if(pril != NULL) {
		pril->type |= FLAG_UPDATER;
	}
}


void rdecl
pril_update_unregister(PRIL_HEAD *ph, struct pril_update *upp) {
	struct pril_update	*next;
	struct pril_update	*prev;
	PRIL				*pril;
	int					more_updaters = 0;

	pril = upp->pril;
	next = upp->next;
	prev = upp->prev;

	if(next != NULL) {
		next->prev = prev;
		if(next->pril == pril) more_updaters = 1;
	}
	if(prev != NULL) {
		prev->next = next;
		if(prev->pril == pril) more_updaters = 1;
	} else {
		update_list = next;
	}
	if((pril != NULL) && !more_updaters) {
		pril->type &= ~FLAG_UPDATER;
	}
}

__SRCVERSION("nano_misc.c $Rev: 199085 $");
