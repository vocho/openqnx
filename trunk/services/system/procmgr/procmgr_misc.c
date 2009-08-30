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
#include "procmgr_internal.h"

// Number of loader contexts to keep around, including the spare
#define MAX_LOADER_CONTEXT			3

static pthread_mutex_t				procmgr_context_alloc_mutex = PTHREAD_MUTEX_INITIALIZER;
static struct loader_context		*lcp_list;
static int							lcp_num;
static unsigned						lcp_waiting;
static unsigned						lcp_inuse;


struct loader_context *
procmgr_context_alloc(unsigned msgsize, int state) {
	struct loader_context				*lcp;
	size_t								size;
	int									r;

	if(msgsize < sizeof(union proc_msg_union)) {
		msgsize = sizeof(union proc_msg_union);
	}
	size = offsetof(struct loader_context, msg) + msgsize;

	lcp = NULL;
	pthread_mutex_lock(&procmgr_context_alloc_mutex);
	if(size <= sizeof(*lcp)) {
		lcp = lcp_list;
		if(lcp != NULL) {
			lcp_list = lcp->next;
			lcp_num--;
			size = lcp->size;
		}
	}
	if(lcp == NULL) {
		pthread_mutex_unlock(&procmgr_context_alloc_mutex);
		r = memmgr.mmap(NULL, 0, size, PROT_READ|PROT_WRITE|PROT_EXEC, 
				MAP_PRIVATE|MAP_ANON, 0, 0, __PAGESIZE, 0, NOFD, (void *)&lcp, &size, mempart_getid(procnto_prp, sys_memclass_id));
		if(r != EOK) {	
			return NULL;
		}
		pthread_mutex_lock(&procmgr_context_alloc_mutex);
	} 
	++lcp_inuse;	
	pthread_mutex_unlock(&procmgr_context_alloc_mutex);
	// We don't have to zero the whole thing - just the first part.
	memset(lcp, 0, sizeof(struct loader_context_prefix));
#ifndef NDEBUG
	/* Ensure no code assumes this is zeroed */
	memset(&lcp->msg, 0xee, sizeof(lcp->msg));
#endif
	lcp->size = size;
	lcp->state = state;
	lcp->msgsize = msgsize;
	return lcp;
}


void
procmgr_context_free(struct loader_context *lcp) {
	unsigned	pid;
	PROCESS		*prp;

	pthread_mutex_lock(&procmgr_context_alloc_mutex);
	--lcp_inuse;
	if((lcp_num < MAX_LOADER_CONTEXT) && (lcp->msgsize <= sizeof(lcp->msg))) {
		lcp->next = lcp_list;
		lcp_list = lcp;
		lcp_num++;
	} else {
		pthread_mutex_unlock(&procmgr_context_alloc_mutex);
		(void)memmgr.munmap(NULL, (uintptr_t)lcp, lcp->size, 0, mempart_getid(procnto_prp, sys_memclass_id));
		pthread_mutex_lock(&procmgr_context_alloc_mutex);
	}
	// Scan through the process table looking for processes waiting for
	// a loader context (they want to terminate) now that we've freed
	// up some memory for them.
	pid = 1;
	for( ;; ) {
		unsigned	flags;

		if(lcp_waiting == 0) break;
		prp = QueryObject(_QUERY_PROCESS, pid+1, _QUERY_PROCESS_VECTOR, 0, &pid ,0,0);
		if(prp == NULL) break;
		flags = prp->flags;
		// We can terminate the query early because if the _NTO_PF_TERM_WAITING
		// flag is on, we know that the process isn't going to terminate
		// on us while we're inside the 'if' - it can't possibly be in the
		// termination code. If it's off, we don't do anything more with
		// the process pointer.
		QueryObjectDone(prp);
		if(flags & _NTO_PF_TERM_WAITING) {
			prp->flags &= ~_NTO_PF_TERM_WAITING;
			MsgSendPulse(PROCMGR_COID, prp->terming_priority, PROC_CODE_TERM, prp->pid);
			--lcp_waiting;
		}
	}
	pthread_mutex_unlock(&procmgr_context_alloc_mutex);
}


void
procmgr_context_wait(PROCESS *prp) {
	pthread_mutex_lock(&procmgr_context_alloc_mutex);
	if(lcp_inuse != 0) {
		prp->flags |= _NTO_PF_TERM_WAITING;
		++lcp_waiting;
	} else {
		// We can't wait for a loader context to become available
		// since there aren't any in use. Only thing to do right
		// now is send a low prio pulse and hope that things work
		// better later.
		MsgSendPulse(PROCMGR_COID, 1, PROC_CODE_TERM, prp->pid);
	}
	pthread_mutex_unlock(&procmgr_context_alloc_mutex);
}


static int
procmgr_context_purge(size_t amount) {
	struct loader_context	*lcp;
	int						purged = 0;

	// If we're being called from the kernel, we can't get
	// the lock we need
	if(!KerextAmInKernel()) {
		for( ;; ) {
			pthread_mutex_lock(&procmgr_context_alloc_mutex);
			// Always leave at least one entry on the list 
			// to avoid deadlocks when terminating processes
			if(lcp_num <= 1) break;
			lcp = lcp_list;
			lcp_list = lcp->next;
			lcp_num--;
			pthread_mutex_unlock(&procmgr_context_alloc_mutex);
			(void)memmgr.munmap(NULL, (uintptr_t)lcp, lcp->size, 0, 
								mempart_getid(procnto_prp, sys_memclass_id));
			purged = 1;
		}
		pthread_mutex_unlock(&procmgr_context_alloc_mutex);
	}
	return purged;
}


void
procmgr_context_init(void) {
	(void)purger_register(procmgr_context_purge, 20);
}


void
procmgr_thread_attr(struct _thread_attr *attr, struct loader_context *lcp, resmgr_context_t *ctp) {
	memset(attr, 0x00, sizeof *attr);
	attr->__stackaddr = lcp->stack;
	attr->__stacksize = sizeof(lcp->stack);
	if(ctp != NULL) {
		attr->__flags = PTHREAD_EXPLICIT_SCHED;
		attr->__policy = SchedGet(ctp->info.pid, ctp->info.tid, (struct sched_param *)&attr->__param);
	}
}

__SRCVERSION("procmgr_misc.c $Rev: 164352 $");
