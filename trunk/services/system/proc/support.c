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
#include <sys/mman.h>
#include <sys/storage.h>

struct proc_mux_lock {
	struct proc_mux_lock	*next;
	unsigned				requestors;
	pthread_mutex_t			mux;
};

static struct proc_mux_lock	*mux_free_list;
static pthread_mutex_t	mux_mutex = PTHREAD_MUTEX_INITIALIZER;
struct proc_mux_lock 	fallback_lock = { NULL, ~0U, PTHREAD_RMUTEX_INITIALIZER };

static int rdecl
acquire_lock(struct proc_mux_lock **lpp) {
	struct proc_mux_lock	*lp;

	lp = *lpp;
	if(lp == NULL) {
		lp = mux_free_list;
		if(lp != NULL) {
			mux_free_list = lp->next;
		} else {
			if ( fallback_lock.requestors == ~0U ) {
				pthread_mutexattr_t	attr;
				int					err;
				(void)pthread_mutexattr_init(&attr);
				(void)pthread_mutexattr_setrecursive(&attr, PTHREAD_RECURSIVE_ENABLE);
				err = pthread_mutex_init(&fallback_lock.mux, &attr);
				CRASHCHECK(err != EOK);
				(void)pthread_mutexattr_destroy(&attr);
				fallback_lock.requestors = 0;
			}
			lp = _smalloc(sizeof(*lp));
			if(lp == NULL) {
				/* devolve to a single lock if we can't allocate a new lock structure */
				lp = &fallback_lock;
				goto fallback;
			}
			lp->requestors = 0;
			(void)pthread_mutex_init(&lp->mux, NULL);
		}
	}
fallback:
	*lpp = lp;
	lp->requestors++;
	_mutex_unlock(&mux_mutex);
#ifdef NDEBUG
	_mutex_lock(&lp->mux);
#else
	if (pthread_mutex_lock(&lp->mux) != EOK) {
		crash();
	}
#endif
	return EOK;
}

static void rdecl
release_lock(struct proc_mux_lock **lpp) {
	struct proc_mux_lock	*lp = *lpp;

#ifdef NDEBUG
	_mutex_unlock(&lp->mux);
#else
	if(lp == NULL) crash();
	if (pthread_mutex_unlock(&lp->mux) != EOK) {
		crash();
	}
#endif
	_mutex_lock(&mux_mutex);
	lp->requestors--;
	if(lp->requestors == 0 && lp != &fallback_lock) {
		*lpp = NULL;
		lp->next = mux_free_list;
		mux_free_list = lp;
	}
	_mutex_unlock(&mux_mutex);
}


PROCESS *
proc_lookup_pid(pid_t pid) {
	PROCESS	*prp;

	prp = QueryObject(_QUERY_PROCESS, pid, _QUERY_PROCESS_VECTOR, 0, 0, 0, 0);
	if(prp != NULL) {
		// We can say the query is done early because the code calling
		// this function knows that the returned process is in a state
		// where it can't terminate on us.
		QueryObjectDone(prp);
	}
	return prp;
}

PROCESS *
proc_lock_pid(pid_t pid) {
	PROCESS					*prp;
	int						r;

	while(1) {
		_mutex_lock(&mux_mutex);
		prp = QueryObject(_QUERY_PROCESS, pid, _QUERY_PROCESS_VECTOR, 0, 0, 0, 0);
		if(prp == NULL) {
			_mutex_unlock(&mux_mutex);
			return NULL;
		}
		r = acquire_lock(&prp->lock);
		
		// Can say the query is done here because we've acquired the
		// process lock and that will keep the terminator thread from 
		// releasing the PROCESS structure (or acquire_lock failed and we 
		// don't need the prp).
		QueryObjectDone(prp);

		if(r != EOK) {
#ifndef NDEBUG
			crash();
			/* NOTREACHED */
#endif
			return NULL;
		}

		/*
		 We have to check to see if the entry matches what we asked for before we 
		 actually locked the pid. There are two cases here that concern us.  The 
		 first is that the prp/pid value switched while we weren't looking or that 
		 the pid is terminating, both caused by exec/ProcessSwap().  As a result we 
		 loop until we get a NULL reply from the QueryObject() which is the definitive 
		 answer that a pid is gone.  We get into this condition running wh1_exec where 
		 a large number of execs and context switches occur.
		*/
		if(prp->pid == pid) {
			break;
		}
		release_lock(&prp->lock);
	}
	return prp;
}

void
proc_unlock(PROCESS *prp) {
	release_lock(&prp->lock);
}

PROCESS *
proc_lock_parent(PROCESS *prp) {
	PROCESS		*parent;
	uint64_t	sleepl;
	unsigned	count;

	// Parent could be in a death transition
	count = 0;
	for( ;; ) {
		parent = proc_lock_pid(prp->parent->pid);
		if(parent != NULL) break;
		if(count > 100) {
			// Sleep for one tick to give the transition more time to complete
			sleepl = 1;
			TimerTimeout(CLOCK_MONOTONIC, _NTO_TIMEOUT_NANOSLEEP, NULL, 
							&sleepl, NULL);
		} else if(count > 50) {
			// Give other guys a chance to run
			(void)SchedYield();
		}
		++count;
		CRASHCHECK(count == 0);
	}
	return parent;
}

int rdecl
proc_mux_lock(struct proc_mux_lock **mp) {
	_mutex_lock(&mux_mutex);
	return acquire_lock(mp);
}

int rdecl
proc_mux_unlock(struct proc_mux_lock **mp) {
	release_lock(mp);
	return 0;
}

int rdecl
proc_mux_haslock(struct proc_mux_lock **mp, int owner) {
	int		hasit;

	hasit = 0;
	_mutex_lock(&mux_mutex);
	if(*mp != NULL) {
		if(owner == 0) owner = __tls()->__owner;
		if(((*mp)->mux.__owner & ~_NTO_SYNC_WAITING) == owner) {
			hasit = 1;
		}
	}
	_mutex_unlock(&mux_mutex);
	return hasit;
}


pthread_mutex_t object_allocator_mutex = PTHREAD_MUTEX_INITIALIZER;

void * 
proc_object_alloc(SOUL *soulp) {
	void *ptr;

	if (KerextAmInKernel()) {
		return object_alloc(0, soulp);
	}

	pthread_mutex_lock(&object_allocator_mutex);
	ptr = object_alloc(0, soulp);
	pthread_mutex_unlock(&object_allocator_mutex);

	return ptr;
}


void
proc_object_free(SOUL *soulp, void *ptr) {
	if (KerextAmInKernel()) {
		object_free(0, soulp, ptr);
		return;
	}
	
	pthread_mutex_lock(&object_allocator_mutex);
	object_free(0, soulp, ptr);
	pthread_mutex_unlock(&object_allocator_mutex);
}

int
proc_error(int ret, PROCESS *prp) {
	if (prp != NULL)
		proc_unlock(prp);

	return ret;
}

int
proc_isaccess(PROCESS *prp, struct _msg_info *rcvinfo) {
	struct _client_info					info;
  	
	return ConnectClientInfo(rcvinfo->scoid, &info, 0) == -1 ? 0 :
		(info.cred.euid == 0 || (prp && prp->cred->info.euid == info.cred.euid));
}

int
proc_status(resmgr_context_t *ctp, int status) {

	if(status == (int)_RESMGR_NOREPLY) {
		// nothing to do
	} else if (status > 0) {
		MsgError(ctp->rcvid, status);
	} else {
		MsgReplyv(ctp->rcvid, ctp->status, ctp->iov + 0, -status);
	}

	return 0;
} 

__SRCVERSION("support.c $Rev: 201354 $");
