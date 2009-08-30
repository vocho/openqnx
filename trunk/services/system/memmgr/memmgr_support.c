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

//RUSH3: Should we stick with the one condvar, or have one per aspace?
//RUSH3: Having only one reduces the number of sync objects that the
//RUSH3: kernel has to keep track of, at the cost of possibly more procnto
//RUSH3: threads waking up when we pthread_cond_broadcast(). On the other
//RUSH3: hand, we probably don't have too many times where we have multiple
//RUSH3: threads blocked waiting for multiple aspaces.

//RUSH3: However, if we _do_ use one condvar per aspace, we can keep track
//RUSH3: if there are any threads requesting R/W locks (similar to 
//RUSH3: _PROMOTE_REQ) and use that information in the unlock code to
//RUSH3: sometimes do a pthread_cond_signal() rather than broadcast
//RUSH3: (when there are write requests, but no read). That would allow us
//RUSH3: to avoid waking up a whack of threads for no purpose.

static pthread_mutex_t mm_mux  = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  mm_cond = PTHREAD_COND_INITIALIZER;


/*
 * R/W lock
 * we use a single 32-bit unsigned for this.
 * the bottom 28 bits are used as reader count
 * top bits are a writer lock and promotion request
 *
 * The lock makes no attempt at fairness, and probably should not be used for 
 * heavily contested resources.
 *
 */

#define _RLOCK_MASK			0x0fffffff
#define _WLOCK_BIT			0x10000000
#define _PROMOTE_REQ		0x20000000
#define _PENDING			0x80000000


int rdecl
proc_rlock_adp(PROCESS *prp) {
	ADDRESS	*adp = prp->memory;
	int		r;

	if(adp == NULL) return -1;

	r = pthread_mutex_lock(&mm_mux);
	CRASHCHECK(r != EOK);
	//RUSH3: Should this wait for the _PROMOTE_REQ bit to be off as well?
	while(adp->rwlock.lock & _WLOCK_BIT) {
		adp->rwlock.lock |= _PENDING;
		r = pthread_cond_wait(&mm_cond, &mm_mux);
		CRASHCHECK(r != EOK);
		CRASHCHECK((mm_mux.__owner & ~_NTO_SYNC_WAITING) != __tls()->__owner);
	}

	adp->rwlock.lock++;
	if(adp->rwlock.lock & _WLOCK_BIT) crash();

	CRASHCHECK((mm_mux.__owner & ~_NTO_SYNC_WAITING) != __tls()->__owner);
	r = pthread_mutex_unlock(&mm_mux);
	CRASHCHECK(r != EOK);
	return 0;
}


int rdecl
proc_wlock_adp(PROCESS *prp) {
	ADDRESS	*adp = prp->memory;
	int		r;

	if(adp == NULL) return -1;

	r = pthread_mutex_lock(&mm_mux);
	CRASHCHECK(r != EOK);
	while(adp->rwlock.lock != 0) {
		adp->rwlock.lock |= _PENDING;
		r = pthread_cond_wait(&mm_cond, &mm_mux);
		CRASHCHECK(r != EOK);
		CRASHCHECK((mm_mux.__owner & ~_NTO_SYNC_WAITING) != __tls()->__owner);
	}
	adp->rwlock.lock = _WLOCK_BIT;

	CRASHCHECK((mm_mux.__owner & ~_NTO_SYNC_WAITING) != __tls()->__owner);
	r = pthread_mutex_unlock(&mm_mux);
	CRASHCHECK(r != EOK);
	return 0;
}


int rdecl
proc_rlock_promote_adp(PROCESS *prp) {
	int		r;
	int		ret;
	ADDRESS	*adp = prp->memory;

	if(adp == NULL) return -1;

	r = pthread_mutex_lock(&mm_mux);
	CRASHCHECK(r != EOK);

	if(adp->rwlock.lock & _PROMOTE_REQ) {
		ret = -1;
	} else {
		adp->rwlock.lock |= _PROMOTE_REQ;
		adp->rwlock.lock--;
		while((adp->rwlock.lock & (_WLOCK_BIT|_RLOCK_MASK)) != 0) {
			adp->rwlock.lock |= _PENDING;
			r = pthread_cond_wait(&mm_cond, &mm_mux);
			CRASHCHECK(r != EOK);
			CRASHCHECK((mm_mux.__owner & ~_NTO_SYNC_WAITING) != __tls()->__owner);
		}
		adp->rwlock.lock = _WLOCK_BIT;
		ret = 0;
	}

	CRASHCHECK((mm_mux.__owner & ~_NTO_SYNC_WAITING) != __tls()->__owner);
	r = pthread_mutex_unlock(&mm_mux);
	CRASHCHECK(r != EOK);
	return ret;
}

int rdecl
proc_unlock_adp(PROCESS *prp) {
	ADDRESS		*adp = prp->memory;
	unsigned	locked;

	if(adp == NULL) crash();

	adp->fault_owner = 0;
	pthread_mutex_lock(&mm_mux);

	locked = adp->rwlock.lock;
	if(locked & _WLOCK_BIT) {
		locked &= ~_WLOCK_BIT;
	} else {
		unsigned count = (locked & _RLOCK_MASK);

		if(count != 0) {
			locked = (locked & ~_RLOCK_MASK) | (count - 1);
		}
		CRASHCHECK(adp->fault_owner != 0);
	}
	if((locked & (_WLOCK_BIT|_RLOCK_MASK|_PENDING)) == _PENDING) {
		locked &= ~_PENDING;
		pthread_cond_broadcast(&mm_cond);
	}
	adp->rwlock.lock = locked;

	CRASHCHECK((mm_mux.__owner & ~_NTO_SYNC_WAITING) != __tls()->__owner);
	pthread_mutex_unlock(&mm_mux);
	return 0;
}

void rdecl
proc_lock_owner_mark(PROCESS *prp) {
	ADDRESS		*adp;

	adp = prp->memory;
	CRASHCHECK(!(adp->rwlock.lock & _WLOCK_BIT));
	if(adp->fault_owner == 0) {
		adp->fault_owner = __tls()->__owner;
	}

}

int rdecl
proc_lock_owner_check(PROCESS *prp, pid_t pid, unsigned tid) {
	return prp->memory->fault_owner == KerextSyncOwner(pid, tid);
}


void rdecl
memobj_lock(OBJECT *obp) {
	pathmgr_object_clone(obp);
	proc_mux_lock(&obp->mem.mm.mux);
}

int rdecl
memobj_cond_lock(OBJECT *obp) {
	struct proc_mux_lock **ml = &obp->mem.mm.mux;

	if(proc_mux_haslock(ml, 0)) return 1;
	pathmgr_object_clone(obp);
	proc_mux_lock(ml);
	return 0;
}


void rdecl
memobj_unlock(OBJECT *obp) {
	VERIFY_OBJ_LOCK(obp);
	proc_mux_unlock(&obp->mem.mm.mux);
	pathmgr_object_done(obp);
}

__SRCVERSION("memmgr_support.c $Rev: 174147 $");
