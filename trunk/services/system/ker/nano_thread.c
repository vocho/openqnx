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
/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA


    Authors: Jerome Stadelmann (JSN), Pietro Descombes (PDB), Daniel Rossier (DRE)
    Emails: <firstname.lastname@heig-vd.ch>
    Copyright (c) 2009 Reconfigurable Embedded Digital Systems (REDS) Institute from HEIG-VD, Switzerland
*/

#include "externs.h"
#include "apm.h"
#include <sys/procmsg.h>
#include "mt_kertrace.h"

#define chk_lock() \
	CRASHCHECK((get_inkernel() & (INKERNEL_NOW|INKERNEL_LOCK)) == INKERNEL_NOW)


static void		**procmgr_stacks;
static size_t	procmgr_stack_size;
#define NO_PURGER	(~(size_t)0)

static int procmgr_stack_purge(size_t amount);


static int
procmgr_stack_alloc(THREAD *thp) {
	if((procmgr_stacks == NULL) && procmgr_stack_size != NO_PURGER) {
		procmgr_stack_size = thp->un.lcl.stacksize;
	} else if(thp->un.lcl.stacksize == procmgr_stack_size) {
		thp->un.lcl.stackaddr = procmgr_stacks;
		procmgr_stacks = *procmgr_stacks;
		return EOK;
	}

	return memmgr.mmap(NULL, 0, thp->un.lcl.stacksize, PROT_READ|PROT_WRITE,
			MAP_ANON|MAP_PRIVATE|MAP_STACK, 0, 0, __PAGESIZE, 0, NOFD,
			&thp->un.lcl.stackaddr, &thp->un.lcl.stacksize,
			mempart_getid(procmgr_prp, sys_memclass_id));
}


static void
procmgr_stack_free(THREAD *thp) {
	void		**stk;

	stk = thp->un.lcl.stackaddr;

	if(thp->un.lcl.stacksize == procmgr_stack_size) {
		*stk = procmgr_stacks;
		procmgr_stacks = stk;
		return;
	}

	(void)memmgr.munmap(NULL, (uintptr_t)stk, thp->un.lcl.stacksize, 0,
					mempart_getid(procmgr_prp, sys_memclass_id));
}



static void
kerext_stack_purge(void *p) {
	int		r;

	r = procmgr_stack_purge((size_t)p);
	SETKSTATUS(actives[KERNCPU], r);
}


static int
procmgr_stack_purge(size_t amount) {
	void	**stk;

	if(!am_inkernel()) {
		return __Ring0(kerext_stack_purge, (void *)amount);
	}

	if(procmgr_stacks == NULL) return 0;

	lock_kernel();
	for( ;; ) {
		stk = procmgr_stacks;
		if(stk == NULL) break;
		procmgr_stacks = *stk;

		(void)memmgr.munmap(NULL, (uintptr_t)stk, procmgr_stack_size, 0,
						mempart_getid(procmgr_prp, sys_memclass_id));
	}
	return 1;
}


void rdecl
thread_init(void) {
	if(purger_register(procmgr_stack_purge, 10) != EOK) {
		procmgr_stack_size = NO_PURGER;
	}
}

/**
//
// This routine is used to start a thread. It will block the parent
// to hold off reporting the thread was created until all the user
// parameters are verified (e.g. stack was created, attributes are valid).
// It is called from kernel mode, so it can't examine any of the user
// attributes itself. It just set the WAAA flag and lets thread_specret()
// do the looking while it can fault...
//*/
int rdecl
thread_create(THREAD *act, PROCESS *prp, const struct sigevent *evp, unsigned thread_create_flags) {
// thread_create_flags are an or-ing of kermacros.h : THREAD_CREATE_*_FLAG
	THREAD	*thp;
	int		tid;
	int		align;

chk_lock();
	// Allocate a thread object.
	if((thp = object_alloc(prp, &thread_souls)) == NULL) {
		return EAGAIN;
	}
	thp->process = prp;
	thp->type = TYPE_THREAD;
	thp->last_chid = -1;
	thp->syscall = -1;
	thp->client = 0;
	snap_time(&thp->start_time, 1);
	//no need to init thp->timestamp_since_block since we start READY

	// If not a sysmanager process then inherit the address space
	// of the calling thread. If first thread initialize to prp;
	if(prp->pid == SYSMGR_PID) {
		thp->aspace_prp = NULL;
	} else {
		thp->aspace_prp = act->process == prp ? act->aspace_prp : prp;
	}

	// Inherit signal mask and thread priv.
	thp->sig_blocked = act->sig_blocked;
	SIGMASK_SPECIAL(&thp->sig_blocked);
	SIGMASK_NO_KILLSTOP(&thp->sig_blocked);

	// Start the new thread at the current threads priority.
	// Because we can block active, we don't want priority
	// inversion if the new thread is a lower priority.
	thp->priority = thp->real_priority = act->real_priority;
	thp->policy = act->policy;

	/*
	 *  Inherit scheduler partition.
         *
	 *  If the creating thread(act) and child thread(thp) are in the same
	 *  process, the child inherits both the dynamic partition (->dpp) and
	 *  the home partition(->orig_dpp) from the creating thread.
         *
	 *  Otherwise, we assume we are Proc and either:
	 *  1) Proc is creating the main thread of a process (result of msg send), or
	 *  2) Proc is creating the terminator thread in a process (result of pulse).
	 *  So we set both dynamic and home partitions of the child to the
	 *  default partition of the parent's process.
	 *
	 */
	if (prp == act->process) {
		thp->dpp = act->dpp;
		thp->orig_dpp = act->orig_dpp;
	} else {
		thp->dpp = prp->default_dpp;
		thp->orig_dpp = prp->default_dpp;
	}

	//set child to start running critical if triggered by critical sigev_thread. Child's critical state will
	//last only up until it goes received blocked.
	thp->sched_flags = 0;
	if ( thread_create_flags & THREAD_CREATE_APS_CRITICAL_FLAG) AP_MARK_THREAD_CRITICAL(thp);

	// Inherit parent runmask
	thp->runmask = thp->default_runmask = act->default_runmask;

	/*
	 Add a new schedinfo.ss_info object for every thread
     we create, even if it isn't needed (delete later).  We are
     temporarily inheriting the priority, so it only makes sense.
	*/
	if(IS_SCHED_SS(thp)) {
		if(!(thp->schedinfo.ss_info = object_alloc(prp, &ssinfo_souls))) {
			object_free(prp, &thread_souls, thp);
			return EAGAIN;
		}
		memset(thp->schedinfo.ss_info, 0, sizeof(*thp->schedinfo.ss_info));
		//Transfer the inherited attributes across, may be reset later
		thp->schedinfo.ss_info->init_budget = act->schedinfo.ss_info->init_budget;
		thp->schedinfo.ss_info->repl_period = act->schedinfo.ss_info->repl_period;
		thp->schedinfo.ss_info->max_repl = act->schedinfo.ss_info->max_repl;
		thp->schedinfo.ss_info->low_priority = act->schedinfo.ss_info->low_priority;
		thp->schedinfo.ss_info->replenishment.thp = thp;

		//Ensure that we have enough information set-up to allow us to run with a valid
 		//sporadic configuration (ie non-zero execution budget, snapped activation time)
		//Leaving the other fields (repl_count, consumed) as 0 is fine
		snap_time(&thp->schedinfo.ss_info->activation_time, 0);
		thp->schedinfo.ss_info->curr_budget = thp->schedinfo.ss_info->init_budget;
	} else {
		RR_RESET_TICK(thp);
	}

	// Add the object to the process thread vector.
	if((tid = vector_add(&prp->threads, thp, 0)) == -1) {
		object_free(prp, &thread_souls, thp);
		return EAGAIN;
	}
	if(prp->valid_thp == NULL) {
		prp->valid_thp = thp;
	}
	thp->tid = tid;


	/* Give the vendor extension a chance to take first dibs */
	if ( kerop_thread_create_hook != NULL ) {
		int r = kerop_thread_create_hook(act, prp, evp, thread_create_flags, thp);
		if ( r != EOK ) {
			object_free(prp, &thread_souls, thp);
			return r;
		}
	}

	// Figure out the default aligment
	if(prp != act->process) {
		align = align_fault;
	} else if (act->flags & _NTO_TF_ALIGN_FAULT) {
		align = +1;
	} else {
		align = -1;
	}
	cpu_thread_init(act, thp, align);
	thp->flags |= (act->flags & _NTO_TF_IOPRIV);

	// Stash some stuff for use in the WAAA special return code
	thp->args.wa.not_sigev_thread = (thread_create_flags & THREAD_CREATE_BLOCK_FLAG)!=0;
	thp->args.wa.attr = evp->sigev_notify_attributes;
	thp->args.wa.real_attr = thp->args.wa.attr;
	SETKIP_FUNC(thp, evp->sigev_notify_function);
	thp->args.wa.arg = evp->sigev_value.sival_ptr;

	// We cannot initalize the thread private data at this time because
	// we may not have addressability to it. We set a flag which will
	// initalize it when it runs for the first time (its birth cry).
	thp->flags |= _NTO_TF_WAAA | _NTO_TF_DETACHED;

	if(prp->num_active_threads == 0) {
		//get a default value for process/termination priority
		prp->terming_priority = act->priority;
		prp->process_priority = 0;	//Set later on in the specret
	}

	++prp->num_active_threads;

	// Make thread lookups invalid
	vector_flag(&prp->threads, thp->tid, 1);
#ifdef _mt_LTT_TRACES_	/* PDB */
	mt_trace_task_create(prp->pid, tid, thp->priority);
#endif
	_TRACE_TH_EMIT_CREATE(thp);
	if(thread_create_flags & THREAD_CREATE_BLOCK_FLAG) {
		// If the parent needs to know if the create succeeded, we block it.
		act->state = STATE_WAITTHREAD;
		_TRACE_TH_EMIT_STATE(act, WAITTHREAD);
		block_and_ready(thp);
		act->blocked_on = thp;
		SETKSTATUS(act, tid + 1);
		thp->join = act;
	} else {
		ready(thp);
	}

	return ENOERROR;
}
/**
//
// By design this function will only be called by a thread in the same
// process as the one being destroyed. It might very well be the same
// thread destroying itself. The code does not currently require this
// but it is good to remember this for the future.
//*/
void rdecl
thread_destroy(THREAD *thp) {
	PROCESS 	*prp = thp->process;
	PULSE 		*pup;
	int 		i;
	SYNC 		*syp;

chk_lock();

	_TRACE_TH_EMIT_DESTROY(thp);
	if(thp->state != STATE_DEAD) {
		if(thp->state == STATE_RUNNING  &&  thp != actives[KERNCPU]) {
			// The thread is running on another processor in an SMP system.
			thp->flags |= (_NTO_TF_KILLSELF | _NTO_TF_ONLYME);
			SENDIPI(thp->runcpu, IPI_RESCHED);
			return;
		}
		// Remove thread from any queues.
		if(!force_ready(thp, _FORCE_KILL_SELF)) {
			// Couldn't kill this thread right now, do it later
			return;
		}

		// Remove any fpu buffer before the thread becomes dead
		if(thp->fpudata) {
			FPU_REGISTERS	*fpudata = FPUDATA_PTR(thp->fpudata);

			if(FPUDATA_INUSE(thp->fpudata)) {
				if(FPUDATA_CPU(thp->fpudata) != KERNCPU) {
					// Context still in use on another CPU; need to flush it out
					thp->flags |= (_NTO_TF_KILLSELF | _NTO_TF_ONLYME);
					SENDIPI(FPUDATA_CPU(thp->fpudata), IPI_CONTEXT_SAVE);
					return;
				}
			}

			if(actives_fpu[KERNCPU] == thp) {
				actives_fpu[KERNCPU] = NULL;
			}
			atomic_order(thp->fpudata = NULL);
			object_free(NULL, &fpu_souls, fpudata);
		}

		SIGMASK_ONES(&thp->sig_blocked);

		if(thp->flags & _NTO_TF_RCVINFO) {
			thp->args.ri.thp->restart = 0;
		}
		if(thp->flags & _NTO_TF_SHORT_MSG) {
			((THREAD *)thp->blocked_on)->restart = 0;
		}

		thp->timeout_flags = 0;
		if(thp->timeout) {
			timer_free(prp, thp->timeout);
			thp->timeout = NULL;
		}


		// Now remove thread from the ready queue.
		// Call unready() instead of block() the thread may not be active.
		unready(thp, STATE_DEAD);


        // The thread is definitely no longer in actives[].

		/* Give the vendor extension a chance to take first dibs */
		if ( kerop_thread_destroy_hook != NULL ) {
			kerop_thread_destroy_hook(thp);
		}

		// purge mutex hold list
		while((syp = thp->mutex_holdlist)) {
			THREAD *waiting = pril_first(&syp->waiting);
			CRASHCHECK(waiting->args.mu.owner == 0);
			mutex_holdlist_rem(syp);
			waiting->args.mu.owner = 0;
		}

		// clear client field
		if(thp->client != 0) {
			/* need to clear client's server field */
			thp->client->args.ms.server = 0;
			thp->client = 0;
		}

		// Release the stack if it was dynamically allocated
		if(thp->flags & _NTO_TF_ALLOCED_STACK) {
			if(prp->pid != PROCMGR_PID && procmgr.process_stack_code) {
				if(thp->state != STATE_STACK) {
					struct sigevent		event;

					// Must do modification of user address spaces at process time
					thp->state = STATE_STACK;

					_TRACE_TH_EMIT_STATE(thp, STACK);
					thp->flags |= (_NTO_TF_KILLSELF | _NTO_TF_ONLYME);
					thp->flags &= ~(_NTO_TF_TO_BE_STOPPED | _NTO_TF_WAAA);

					event.sigev_notify = SIGEV_PULSE;
					event.sigev_coid = PROCMGR_COID;
					event.sigev_value.sival_int = SYNC_OWNER(thp);
					event.sigev_priority = thp->priority;
					event.sigev_code = procmgr.process_stack_code;

					if(sigevent_proc(&event)) {
						// Very bad; we are out of pulses... This means
						// there's absolutely no memory left in the system.
						CRASHCHECK(1);
					}
				}
				return;
			}
			procmgr_stack_free(thp);
			thp->flags &= ~_NTO_TF_ALLOCED_STACK;
		}

		// Make thread lookups invalid
		vector_flag(&prp->threads, thp->tid, 1);

		// If there is still active threads retarget valid_thp
		if(--prp->num_active_threads  &&  prp->valid_thp == thp) {
			THREAD				*thp2, *thp3 = NULL;

			for(i = 0; i < prp->threads.nentries; i++) {

				// VECP() will not match our thread.
				if(VECP(thp2, &prp->threads, i)) {
					prp->valid_thp = thp2;
					break;
				} else if((thp2 = VECP2(thp2, &prp->threads, i)) && thp2->state != STATE_DEAD) {
					thp3 = thp2;
				}
			}

			// Could happen if we only have threads starting up left
			if(prp->valid_thp == thp) {
				prp->valid_thp = thp3;
			}
		} else if(!prp->num_active_threads) {
			prp->valid_thp = NULL;
		}

		// If killing thread that last processed a signal, update
		// signal tid cache.
		if(thp->tid == prp->sigtid_cache) {
			THREAD	*vthp = prp->valid_thp;

			prp->sigtid_cache = (vthp != NULL) ? vthp->tid : 0;
		}

		// Deactivate any timers targeting this thread
		for(i = 0; i < prp->timers.nentries; ++i) {
			TIMER	 *tip;

			if(VECP(tip, &prp->timers, i) && (tip->flags & _NTO_TI_ACTIVE) &&
					(prp->num_active_threads == 0 || tip->thread == thp)) {
				if((tip->flags & _NTO_TI_TARGET_PROCESS) && (prp->valid_thp != NULL)) {
					// have to retarget to a new thread
					tip->thread = prp->valid_thp;
				} else {
					timer_deactivate(tip);
				}
			}
		}
		if(prp->alarm != NULL) {
			TIMER *tip = prp->alarm;

			// Alarm timers are always process based.
			if(tip->thread == thp) {
				tip->thread = prp->valid_thp;
			}
		}

		// Clean up after a SPORADIC thread
		if(thp->policy == SCHED_SPORADIC) {
			sched_ss_cleanup(thp);
		}

		// Release any attached interrupts
		if(prp->flags & _NTO_PF_CHECK_INTR) {
			INTERRUPT	*itp;

			for(i = 0; i < interrupt_vector.nentries; ++i) {
				if(VECP(itp, &interrupt_vector, i) && itp->thread->process == prp) {
					// Only detach interrupt if bound to thread, or all threads are inactive
					if(prp->num_active_threads == 0) {
						interrupt_detach_entry(prp, i);
					} else if(itp->thread != thp) {
						/* nothing to do */
					} else if(itp->flags & _NTO_INTR_FLAGS_PROCESS) {
						itp->thread = prp->valid_thp;
					} else {
						interrupt_detach_entry(prp, i);
					}
				}
			}
		}

		// timers/interrupts have been detached, so flush the interrupt
		// queues to get rid of anything pending that's pointing at
		// this thread
		intrevent_flush();

		// Purge any enqueued signals
		for( ;; ) {
			pup = pril_first(&thp->sig_pending);
			if(pup == NULL) break;
			pril_rem(&thp->sig_pending, pup);
			object_free(prp, &pulse_souls, pup);
		}

		// If requested send a death pulse to a channel.
		if(prp->death_chp && (prp->death_chp->flags & _NTO_CHF_THREAD_DEATH)) {
			pulse_deliver(prp->death_chp, thp->real_priority, _PULSE_CODE_THREADDEATH, thp->tid+1, -1, 0);
		}
	}

	// A zombie if nobody waiting, not detached and not last thread.
	if(thp->join == NULL  &&  (thp->flags & _NTO_TF_DETACHED) == 0  &&
	   (prp->threads.nentries-1 != prp->threads.nfree)) {
		return;
	}


	// Wakeup any thread waiting to join on me.
	if(thp->join) {
		THREAD *jthp = thp->join;

		if(jthp->state == STATE_WAITTHREAD) {
			// This thread was being created....
			kererr(jthp, (intptr_t)thp->status);
		} else {
			// This is a normal thread death...
			jthp->args.jo.status = thp->status;
			jthp->flags |= _NTO_TF_JOIN;
			SETKSTATUS(jthp, EOK);
		}
		ready(jthp);
	}

	// FPU buffer should be freed already
	if(thp->fpudata) crash();

	// Remove thread from process thread vector.
	thp = vector_rem(&prp->threads, thp->tid);
	if(thp == NULL) {
		crash();
	}

	// Remove any CPU-specific save buffers
	cpu_thread_destroy(thp);
#ifdef _mt_LTT_TRACES_	/* PDB */
	//mt_TRACE_DEBUG("2 !");
	mt_trace_task_delete(thp->process->pid, thp->tid, 0);
#endif

	// Remember the priority for the pulses below
	i = thp->real_priority;

	thp->schedinfo.rr_ticks = 0; /* sanity check before we return it to the pool */

	// Free the name if it was allocated
	if(thp->name != NULL) {
		int name_len = strlen(thp->name) + 1;
		if(name_len >= THREAD_NAME_FIXED_SIZE) {
   		     _sfree(thp->name, name_len);
   		 } else {
   		     object_free(thp->process, &threadname_souls, thp->name);
		}
		thp->name = NULL;
	}

	// Release thread object back to the free queue.
	object_free(prp, &thread_souls, thp);

	if(prp->threads.nentries == prp->threads.nfree) {
		// Purge any enqueued signals pending on the process.
		for( ;; ) {
			pup = pril_first(&prp->sig_pending);
			if(pup == NULL) break;
			pril_rem(&prp->sig_pending, pup);
			object_free(prp, &pulse_souls, pup);
		}

		// setup process so it can run as a terminator thread
		prp->flags |= _NTO_PF_TERMING;
		prp->boundry_addr = VM_KERN_SPACE_BOUNDRY;
		SIGMASK_ONES(&prp->sig_ignore);

		// If debugging, don't tell debugger, not process manager
		if(prp->debugger  &&  (*debug_process_exit)(prp, i)) {
			return;
		}

		// If last thread gone, tell the process manager.
		if(procmgr.process_threads_destroyed) {
			struct sigevent		ev;
			/* PDB process_delete */
			_TRACE_DESTROY_EH(prp);
			(*procmgr.process_threads_destroyed)(prp, &ev);
			sigevent_proc(&ev);
		}
	}
}
/**
//
// Called if we get a fault while touching the thread's stack.
// Let the thread creation fail.
//*/

static void
threadstack_fault(THREAD *thp, CPU_REGISTERS *reg, unsigned flags) {

chk_lock();
	// This will cause the parent thread to return from ThreadCreate
	// with an ENOMEM or EINVAL error.
	thp->status = (thp->flags & _NTO_TF_ALLOCED_STACK) ? (void *)ENOMEM : (void *)EINVAL;
	thp->flags |= (_NTO_TF_KILLSELF | _NTO_TF_ONLYME);
	__ker_exit();
}

static const struct fault_handlers threadstack_fault_handlers = {
	threadstack_fault, 0
};
/**
//
// Handle thread creation during when address space is available
// and we can recover from faults (from bad user pointers...)
//*/
void rdecl
thread_specret(THREAD *thp) {
	struct _thread_local_storage	*tsp;
	const struct _thread_attr		*attr;
	void							*init_cc;
	uintptr_t						 stack_top;
	uintptr_t						 new_sp;
	int								 verify;

	thp->status = (void *)EFAULT;
	if((attr = thp->args.wa.attr)) {
		//RD_VERIFY_PTR(act, attr, sizeof(*attr));
		//RD_PROBE_INT(act, attr, sizeof(*attr) / sizeof(int));

		// Check for attributes which we do not support.
		// If there is a stack addr there must be a stack size.
		// If there is a stack size it must be at lease PTHREAD_STACK_MIN.
		// If EXPLICIT sched, make sure policy and priority are valid.
		//                    add validation of the sporadic server attributes
		if(attr->__flags & PTHREAD_SCOPE_PROCESS) {
			verify = ENOTSUP;
		} else if((attr->__stackaddr || attr->__stacksize) && attr->__stacksize < PTHREAD_STACK_MIN) {
			verify = EINVAL;
		} else if(attr->__flags & PTHREAD_EXPLICIT_SCHED) {
			verify = kerschedok(thp, attr->__policy, (struct sched_param *)&attr->__param);
		} else {
			verify = EOK;
		}

		if(verify != EOK) {
			lock_kernel();
			thp->status = (void *)verify;
			thp->flags |= (_NTO_TF_KILLSELF | _NTO_TF_ONLYME);
			return;
			// RUSH3: this comes out in loader_exit() but EINTR overridden
		}
	}

	// Check if we need to allocate a stack
	if(!(thp->flags & _NTO_TF_ALLOCED_STACK)) {
		uintptr_t					guardsize = 0;
		unsigned					lazystate = 0;
		unsigned					prealloc  = 0;

		if(attr) {
			// Get the user requested values.
			thp->un.lcl.stackaddr = attr->__stackaddr;
			thp->un.lcl.stacksize = attr->__stacksize;
			if(attr->__stackaddr != NULL &&
			  !WR_PROBE_PTR(thp, thp->un.lcl.stackaddr, thp->un.lcl.stacksize)) {
				lock_kernel();
				thp->status = (void *)EINVAL;
				thp->flags |= (_NTO_TF_KILLSELF | _NTO_TF_ONLYME);
				return;
			}
			guardsize = attr->__guardsize;
			prealloc = attr->__prealloc;
			lazystate = attr->__flags & PTHREAD_NOTLAZYSTACK_MASK;
		}
		if(thp->un.lcl.stacksize == 0) {
			if(__cpu_flags & CPU_FLAG_MMU) {
				thp->un.lcl.stacksize = DEF_VIRTUAL_THREAD_STACKSIZE;
			} else {
				thp->un.lcl.stacksize = DEF_PHYSICAL_THREAD_STACKSIZE;
			}
		}
		if(!thp->un.lcl.stackaddr) {
			lock_kernel();

			if(thp->process->pid != PROCMGR_PID && procmgr.process_stack_code) {
				unspecret_kernel();

				if(thp->state != STATE_STACK) {
					// Must do modification of user address spaces at process time
					struct sigevent		event;

					CRASHCHECK(thp != actives[KERNCPU]);

					event.sigev_notify = SIGEV_PULSE;
					event.sigev_coid = PROCMGR_COID;
					event.sigev_value.sival_int = SYNC_OWNER(thp);
					event.sigev_priority = thp->priority;
					event.sigev_code = procmgr.process_stack_code;

					if(sigevent_proc(&event)) {
						// Pulse failed...
						thp->status = (void *)EAGAIN;
						thp->flags |= (_NTO_TF_KILLSELF | _NTO_TF_ONLYME);
						return;
					}

					// we may not be running after sigevent_proc()
					unready(thp, STATE_STACK);
					thp->prev.thread = (void *)guardsize;
					thp->next.thread = (void *)lazystate;
					thp->status = (void *)prealloc;
				}
				return;
			}

			guardsize = 0;
			if(procmgr_stack_alloc(thp) != EOK) {
				thp->status = (void *)EAGAIN;
				thp->flags |= (_NTO_TF_KILLSELF | _NTO_TF_ONLYME);
				return;
			}
			thp->flags |= _NTO_TF_ALLOCED_STACK;
			unlock_kernel();
			SPECRET_PREEMPT(thp);
		}
	}

	// Inherit or assign a scheduling policy and params.
	if(attr) {
		if(attr->__flags & PTHREAD_MULTISIG_DISALLOW) {
			thp->flags |= _NTO_TF_NOMULTISIG;
		}
		thp->args.wa.exitfunc = attr->__exitfunc;
	}

	// Clear detach state if there is a parent

	// Get the *real* attribute structure pointer - we may have
	// NULL'd out thp->args.wa.attr and then been preempted
	attr = thp->args.wa.real_attr;
	if(thp->join && (!attr || !(attr->__flags & PTHREAD_CREATE_DETACHED))) {
		thp->flags &= ~_NTO_TF_DETACHED;
	}

	// Make thread lookups valid
	lock_kernel();
	vector_flag(&thp->process->threads, thp->tid, 0);
	thp->args.wa.attr = 0;

	if(actives[KERNCPU] != thp) {
		return;
	}

	// Load the necessary registers for the thread to start execution.
	stack_top = STACK_INIT((uintptr_t)thp->un.lcl.stackaddr, thp->un.lcl.stacksize);
	STACK_ALLOC(thp->un.lcl.tls, new_sp, stack_top, sizeof *thp->un.lcl.tls);
	STACK_ALLOC(init_cc, new_sp, new_sp, STACK_INITIAL_CALL_CONVENTION_USAGE);
	SETKSP(thp, new_sp);

	// Could fault again while setting tls in stack...
	unlock_kernel();
	SPECRET_PREEMPT(thp);

	SET_XFER_HANDLER(&threadstack_fault_handlers);

	tsp = thp->un.lcl.tls;
	memset(tsp, 0, sizeof(*tsp));
	// Set the inital calling convention usage section to zero - will
	// help any stack traceback code to determine when it has hit the
	// top of the stack.
	memset(init_cc, 0, STACK_INITIAL_CALL_CONVENTION_USAGE);

	if(attr) {
		tsp->__flags = attr->__flags & (PTHREAD_CSTATE_MASK|PTHREAD_CTYPE_MASK);
	}
	tsp->__arg = thp->args.wa.arg;
	tsp->__exitfunc = thp->args.wa.exitfunc;
	if(tsp->__exitfunc == NULL && thp->process->valid_thp != NULL) {
		/*
			We don't have thread termination (exitfunc) for this thread.
			Likely it was created with SIGEV_THREAD. Use the same one
			as for the valid_thp's. This mostly works since all threads
			created via pthread_create have the same exit function.
		*/
		tsp->__exitfunc = thp->process->valid_thp->un.lcl.tls->__exitfunc;
	}

	tsp->__errptr = &tsp->__errval;
	if(thp->process->pid == PROCMGR_PID) {
		tsp->__stackaddr = (uint8_t *)thp->un.lcl.stackaddr;
	} else {
		tsp->__stackaddr = (uint8_t *)thp->un.lcl.stackaddr + ((attr == NULL) ? 0 : attr->__guardsize);
	}
	tsp->__pid = thp->process->pid;
	tsp->__tid = thp->tid + 1;
	tsp->__owner = SYNC_OWNER(thp);

	// Touch additional stack if requested in attr
	// @@@ NYI
	// if(attr->guaranteedstacksize) ...

	SET_XFER_HANDLER(NULL);

	cpu_thread_waaa(thp);

	// Let the parent continue. The tid was stuffed during thread_create().
	if(thp->join && thp->join->state == STATE_WAITTHREAD) {
		lock_kernel();
		ready(thp->join);
		thp->join = NULL;
	}

	//
	// Don't change priority until parent thread freed to run again
	// - we might get a priority inversion otherwise.
	//
	if((attr != NULL) && (attr->__flags & PTHREAD_EXPLICIT_SCHED)) {
		lock_kernel();

		if(sched_thread(thp, attr->__policy, (struct sched_param *)&attr->__param) != EOK) {
			/* We should have some error handling if sched_thread() fails ...
			thp->status = (void *)EAGAIN;
			thp->flags |= (_NTO_TF_KILLSELF | _NTO_TF_ONLYME);
			return;
			*/
		}
	}

	/* Only done once for the first thread running */
	if(thp->process->process_priority == 0) {
		thp->process->process_priority = thp->priority;
	}

	/* a thread is born unto a STOPPED process - make sure it stops too! */
	if ( thp->process->flags & (_NTO_PF_DEBUG_STOPPED|_NTO_PF_STOPPED) ) {
		thp->flags |= _NTO_TF_TO_BE_STOPPED;
	}
	thp->flags &= ~_NTO_TF_WAAA;
}

/**
//
// Kill all siblings of active thread.
// The kernel is locked when this is called.
//*/
void rdecl
thread_destroyall(THREAD *thp) {
	PROCESS	*prp = thp->process;
	THREAD	*thp2;
	int		tid;

chk_lock();
	SIGMASK_ONES(&thp->sig_blocked);
	thp->flags &= ~_NTO_TF_SIG_ACTIVE;

	for(tid = 0 ; tid < prp->threads.nentries ; ++tid) {
		if(VECP(thp2, &prp->threads, tid)) {
			thp2->flags |= _NTO_TF_DETACHED | _NTO_TF_KILLSELF;
			// Knock down the stack flag, faster to do it when we unmap all
			thp2->flags &= ~(_NTO_TF_TO_BE_STOPPED | _NTO_TF_ALLOCED_STACK);
#ifdef _mt_LTT_TRACES_	/* PDB */
	//mt_TRACE_DEBUG("bad_destroy");
	//mt_trace_task_delete(prp->pid, tid, 0);
#endif
		}
	}

	cpu_force_thread_destroyall(thp);
	prp->flags |= _NTO_PF_DESTROYALL;
}


void rdecl
thread_cancel(THREAD *thp) {
chk_lock();
	if(thp->state == STATE_RUNNING  &&  thp != actives[KERNCPU]) {
		// The thread is running on another processor in an SMP system.
		thp->flags |= _NTO_TF_CANCELSELF;
		SENDIPI(thp->runcpu, IPI_RESCHED);
		return;
	}

#if defined(VARIANT_smp) && defined(SMP_MSGOPT)
	if(thp->internal_flags & _NTO_ITF_MSG_DELIVERY) {
		thp->flags |= _NTO_TF_CANCELSELF;
	} else {
		thp->flags |= _NTO_TF_KERERR_SET;
		SETKIP_FUNC(thp, thp->process->canstub);
	}
#else
	thp->flags |= _NTO_TF_KERERR_SET;
	SETKIP_FUNC(thp, thp->process->canstub);
#endif
}

__SRCVERSION("nano_thread.c $Rev: 211554 $");
