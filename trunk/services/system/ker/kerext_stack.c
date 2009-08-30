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
#include <sys/memmsg.h>

struct kerargs_stack_cont {
	pid_t				pid;
	int					tid;
	int					err;
	struct _thread_attr	*attr;
};

static void kerext_stack_cont(void *data) {
	struct kerargs_stack_cont *kap = data;
	PROCESS	*prp;
	THREAD	*act = actives[KERNCPU], *thp;

	// Verify the target process and thread exists.
	if((prp = lookup_pid(kap->pid)) == NULL  ||
	   (thp = vector_lookup2(&prp->threads, kap->tid-1)) == NULL  ||
	   (thp->state != STATE_STACK)) {
		kererr(act, ESRCH);
		return;
	}

	lock_kernel();
	if(kap->err) {
		thp->status = (void *)kap->err;
		thp->flags |= (_NTO_TF_KILLSELF | _NTO_TF_ONLYME);
	} else {
		struct _thread_attr		*attr = kap->attr;

		thp->un.lcl.stackaddr = attr->__stackaddr;
		thp->un.lcl.stacksize = attr->__stacksize;
		if(thp->flags & _NTO_TF_WAAA) {
			thp->flags |= _NTO_TF_ALLOCED_STACK;
		} else {
			thp->flags &= ~_NTO_TF_ALLOCED_STACK;
		}
	}
	ready(thp);
	SETKSTATUS(act, EOK);
}

int StackCont(pid_t pid, int tid, int err, struct _thread_attr *attr) {
	struct kerargs_stack_cont	data;

	data.pid = pid;
	data.tid = tid;
	data.err = err;
	data.attr = attr;
	return __Ring0(kerext_stack_cont, &data);
}

struct kerargs_stack_wait_info {
	union sigval		value;
	pid_t				*ppid;
	int					*ptid;
	struct _thread_attr	*attr;
};

static void kerext_stack_wait_info(void *data) {
	struct kerargs_stack_wait_info	*kap = data;
	int								id;
	PROCESS							*prp;
	THREAD							*thp;
	int								ret;

	ret = -1;
	if((id = SYNC_PINDEX(kap->value.sival_int)) < process_vector.nentries  &&
			(prp = VECP(prp, &process_vector, id))  &&
			SYNC_PINDEX(kap->value.sival_int) == PINDEX(prp->pid) &&
			(thp = vector_lookup2(&prp->threads, SYNC_TID(kap->value.sival_int))) &&
			(thp->state == STATE_STACK)) {
		struct _thread_attr				*attr = kap->attr;

		*kap->ppid = prp->pid;
		*kap->ptid = thp->tid + 1;
        attr->__stackaddr = thp->un.lcl.stackaddr;
        attr->__stacksize = thp->un.lcl.stacksize;
		if(thp->flags & _NTO_TF_WAAA) {
			attr->__flags = (uintptr_t)thp->next.thread;
			attr->__guardsize = (uintptr_t)thp->prev.thread;
			attr->__prealloc  = (uintptr_t)thp->status;
		} else {
			attr->__flags = 0;
			attr->__prealloc = 0;
			attr->__guardsize = ~0U;
		}

		ret = 0;
	}
	lock_kernel();
	SETKSTATUS(actives[KERNCPU], ret);
}

int StackWaitInfo(union sigval value, pid_t *ppid, int *ptid, struct _thread_attr *attr) {
	struct kerargs_stack_wait_info		data;

	data.value = value;
	data.ppid = ppid;
	data.ptid = ptid;
	data.attr = attr;
	return __Ring0(kerext_stack_wait_info, &data);
}

__SRCVERSION("kerext_stack.c $Rev: 169208 $");
