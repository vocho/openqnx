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

#include <sys/procmgr.h>
#include "externs.h"
#include "pathmgr_proto.h"
#include "procmgr_internal.h"

static int termer_done(resmgr_context_t *ctp, union proc_msg_union *msg, PROCESS *prp) {
	struct loader_context				*lcp;
	PROCESS								*parent;
	struct wait_entry					*wap, **pwap;
	int									state;
	int									fd;
	int									pid;

	if(prp->siginfo.si_signo != SIGCHLD) {
		kprintf("termer thread for pid %d failed with signo=%d at 0x%p\n", prp->pid, prp->siginfo.si_signo, prp->siginfo.si_fltip);
	}

	lcp = prp->lcp;
	state = lcp->state & LC_STATE_MASK;

#ifndef NKDEBUG
	if(prp->kdebug.prev) {
		(void)kdebug_unlink(&prp->kdebug);
	}
#endif

	fd = -1;
	if(state != LC_EXEC_SWAP) {
		if(prp->session && prp->session->pgrp == PROCMGR_PID) {
			procmgr_trigger(PROCMGR_EVENT_DAEMON_DEATH);
		}
		if(prp->flags & _NTO_PF_SLEADER) {
			fd = procmgr_sleader_detach(prp);
		}
	}

	if(prp->root) {
		pathmgr_node_detach(prp->root);
		prp->root = NULL;
	}
	if(prp->cwd) {
		pathmgr_node_detach(prp->cwd);
		prp->cwd = NULL;
	}

	if(prp->debug_name) {
		free(prp->debug_name);
		prp->debug_name = NULL;
	}
	
	parent = proc_lock_parent(prp);

	// If we are the parents guardian, remove it
	if(parent->guardian == prp) {
		parent->guardian = 0;
	}

	if(state != LC_EXEC_SWAP) {
		for(pwap = &parent->wap; (wap = *pwap);) {
			int			status;

			if((status = procmgr_wait_check(prp, parent, wap, WEXITED)) > 0) {
				*pwap = wap->next;
				proc_object_free(&wait_souls, wap);
				if(status == WEXITED) {
					break;
				}
			} else {
				pwap = &wap->next;
			}
		}
	}

	if(prp->vfork_info) {
		struct vfork_info		*vip = prp->vfork_info;

		ProcessBind(prp->parent->pid);
		ProcessRestore(vip->rcvid, &vip->tls, vip->frame, vip->frame_base, vip->frame_size);
		ProcessBind(0);
	}
	prp->flags |= _NTO_PF_ZOMBIE;
	prp->flags &= ~(_NTO_PF_TERMING | _NTO_PF_RING0);

	pid = prp->pid;

	if(state == LC_EXEC_SWAP) {
		lcp = procmgr_exec(ctp, msg, prp);
	}
	proc_unlock(parent);

	if(prp->vfork_info) {
		MsgReplyv(prp->vfork_info->rcvid, pid, 0, 0);
		//
		// We need to use _kfree() here because this structure
		// was allocated by the kernel.  This insures that the
		// memory goes back on to the correct free list.
		//
		_kfree(prp->vfork_info);
		prp->vfork_info = 0;
	}

	if(lcp) {
		procmgr_context_free(lcp);
		prp->lcp = 0;
	}

	return fd;
}

static void termer_start(resmgr_context_t *ctp, union proc_msg_union *msg, PROCESS *prp) {
	struct loader_context				*lcp;
	struct _thread_attr					attr;
	struct wait_entry					*wap;

#ifndef NKDEBUG
	(void)kdebug_detach(&prp->kdebug);
#endif

	// setup process so it can run the terminator thread
	prp->flags |= _NTO_PF_RING0;

	// clear any pending wait's
	while((wap = prp->wap)) {
		prp->wap = wap->next;
		// Should do better cleanup @@@
		proc_object_free(&wait_souls, wap);
	}

	// start the terminator thread
	if((lcp = prp->lcp) || (lcp = procmgr_context_alloc(0, LC_TERM))) {
		procmgr_thread_attr(&attr, lcp, NULL);
		prp->lcp = lcp;
		lcp->process = prp;
		lcp->info = prp->siginfo;		// save this so terminator can reload it
		if(ThreadCreate(prp->pid, terminator, lcp, &attr) != -1) {
			return;
		}
		if(lcp->state == LC_TERM) {
			procmgr_context_free(lcp);
			prp->lcp = NULL;
		}
	}
	//RUSH3: Should we try unmapping the memory for the process to
	//RUSH3: free up some space? Not if vforked!

	// Wait for some more memory to appear and then send the pulse again.
	procmgr_context_wait(prp);
}

int procmgr_termer(message_context_t *mctp, int code, unsigned flags, void *handle) {
	resmgr_context_t				*ctp = (resmgr_context_t *)mctp;
	union sigval					value = ctp->msg->pulse.value;
	pid_t							pid = value.sival_int;
	union proc_msg_union			*msg = (union proc_msg_union *)(void *)ctp->msg;
	PROCESS							*prp;
	int								fd = -1;

	if((prp = proc_lock_pid(pid))) {
		if(prp->threads.nentries == prp->threads.nfree) {
			if(prp->flags & _NTO_PF_LOADING) {
				loader_exit(ctp, msg, prp);
				// Let termer clean up any resources that may have been allocated
				prp->flags &= ~(_NTO_PF_LOADING | _NTO_PF_RING0);
				prp->flags |= _NTO_PF_NOZOMBIE;
			}
			if(prp->flags & _NTO_PF_TERMING) {
				if((prp->lcp == NULL) || !(prp->lcp->state & LC_TERMER_FINISHED)) {
					termer_start(ctp, msg, prp);		// start the termination thread
				} else {
					fd = termer_done(ctp, msg, prp);	// cleanup termer context, and send SIGCHLD's
				}
			} 
			if(!(prp->flags & (_NTO_PF_LOADING | _NTO_PF_TERMING | _NTO_PF_WAITINFO))) {
				pid_t			pid2;
				PROCESS			*parent;
				uint64_t		sleepl = 1;
				unsigned		count;

				pid2 = prp->pid;
#ifndef NDEBUG
				if(pid2 == 0) {
					/* 
					 The pid test is historical and should not be needed.  At
					 some point we should remove it but for now put this in 
					 to guarantee that the test is not required.
					*/
					crash();
				}
#endif


				if(prp->ocb_list) {
					(void)proc_debug_destroy(ctp, prp);
				}

				parent = proc_lock_parent(prp);

				prp->pid = 0;
				proc_unlock(prp);

				// Wait for any threads that were attempting to
				// get the lock to figure out that the process isn't
				// there anymore (Yuck).
				// The ProcessDestroy() returns zero if some other
				// proc thread is in the middle of a QueryObject() on
				// the prp. We have to wait for that to finish
				// before we can free the prp memory.
				count = 0;
				while(prp->lock || !ProcessDestroy(pid2)) {
					TimerTimeout(CLOCK_MONOTONIC, _NTO_TIMEOUT_NANOSLEEP,
						NULL, &sleepl, NULL);
					if(++count == 100) {
						struct sched_param	param;

						// lower the priority of this thread
						(void)SchedGet(0,0, &param);
						if(param.sched_curpriority == 1) {
							crash();
						}
						param.sched_priority = param.sched_curpriority - 1;
						(void)SchedSet(0, 0, SCHED_NOCHANGE, &param);
						count = 0;
					}
				}

				prp = parent;		// common code will unlock it this way
			}
		}
		proc_unlock(prp);
	}

	// close the fd's if nessessary while there is no proc_lock() in effect
	if(fd != -1) {
		if(proc_thread_pool_reserve() != 0) {
			return EAGAIN;
		}
		close(fd);
		proc_thread_pool_reserve_done();
	}

	return 0;
}

__SRCVERSION("procmgr_termer.c $Rev: 153052 $");
