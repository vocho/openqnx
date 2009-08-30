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
#include "apm.h"
#include "aps.h"

/* remove this when we no longer support WATCOM, or WATCOM supports inlines */
#if (defined(__WATCOMC__) || !defined(NDEBUG))
static DISPATCH *select_dpp(THREAD *act, PROCESS *prp, part_id_t id)
#else	/* (defined(__WATCOMC__) || !defined(NDEBUG)) */ 
static __inline__ DISPATCH *select_dpp(THREAD *act, PROCESS *prp, part_id_t id)
#endif	/* (defined(__WATCOMC__) || !defined(NDEBUG)) */ 
{
	DISPATCH *d = (DISPATCH *)SCHEDPART_SELECT_DPP(prp, id);
	return (d == NULL) ? act->dpp : d;
}
#define SELECT_DPP(_a_, _p_, _spid_)		select_dpp((_a_), (_p_), (_spid_))

#define MUTEX_INIT(p, m) \
		do { \
			const pthread_mutex_t  tmp = { 0, _NTO_SYNC_MUTEX_FREE }; \
			(m)->mutex = tmp; \
			(m)->spin.value = 0; \
			(void)sync_create(p, &(m)->mutex, 0); \
		} while(0)
#define MUTEX_DESTROY(p, m)	sync_destroy(p, &(m)->mutex)

struct kerargs_process_create {
	pid_t	parent_pid;
	void	*lcp;
	proc_create_attr_t *extra;
};

static void
kerext_process_create(void *data) {
	struct kerargs_process_create *kap = data;
	pid_t							pid;
	PROCESS							*prp, *parent;
	THREAD							*act = actives[KERNCPU];
	int								status, i;
	struct _cred_info				info;

	if((parent = lookup_pid(kap->parent_pid)) == NULL) {
		kererr(act, ESRCH);
		return;
	}

	if (parent->num_processes >= parent->rlimit_vals_soft[RLIMIT_NPROC]) {
		kererr(act, EAGAIN);
		return;
	}

	lock_kernel();

	// Check that we haven't run out of process vector entries
	// The process index & PID_MASK should never be all zeros
	// or all ones. All ones could cause SYNC_*() to return
	// valid looking numbers from an uninitialized sync.
	if((process_vector.nentries - process_vector.nfree) >= PID_MASK - 1) {
		kererr(act, EAGAIN);
		return;
	}

	// Alloc a process entry.
	if((prp = object_alloc(NULL, &process_souls)) == NULL) {
		kererr(act, ENOMEM);
		return;
	}

	if(kap->parent_pid) {
		prp->flags = _NTO_PF_LOADING | _NTO_PF_NOZOMBIE | _NTO_PF_RING0;
		prp->lcp = kap->lcp;
	}
	snap_time(&prp->start_time, 1);

	MUTEX_INIT(prp, &prp->mpartlist_lock);
	MUTEX_INIT(prp, &prp->spartlist_lock);

	CRASHCHECK((kap->extra == NULL) || (kap->extra->mpart_list == NULL) || 
				((kap->extra->spart_list == NULL) && SCHEDPART_INSTALLED()));
	{
		part_list_t  *mpart_list = kap->extra->mpart_list;
		part_list_t  *spart_list = kap->extra->spart_list;

		/* first thing is to associate with all specified partitions */
		for (i=0; i<mpart_list->num_entries; i++)
		{
			if ((status = MEMPART_ASSOCIATE(prp, mpart_list->i[i].id, mpart_list->i[i].flags)) != EOK)
			{
				(void)MEMPART_DISASSOCIATE(prp, part_id_t_INVALID);
				(void)MUTEX_DESTROY(prp, &prp->mpartlist_lock);
				(void)MUTEX_DESTROY(prp, &prp->spartlist_lock);
				object_free(NULL, &process_souls, prp);
				kererr(act, status);
				return;
			}
		}
		if (SCHEDPART_INSTALLED())
		{
			for (i=0; i<spart_list->num_entries; i++)
			{
				if ((status = SCHEDPART_ASSOCIATE(prp, spart_list->i[i].id, spart_list->i[i].flags)) != EOK)
				{
					(void)MEMPART_DISASSOCIATE(prp, part_id_t_INVALID);
					(void)MUTEX_DESTROY(prp, &prp->mpartlist_lock);
					(void)MUTEX_DESTROY(prp, &prp->spartlist_lock);
					object_free(NULL, &process_souls, prp);
					kererr(act, status);
					return;
				}
			}
		}
	}

	// Allocate a vector for 1 thread but don't get a thread entry.
	if(vector_add(&prp->threads, NULL, 1) == -1) {
		(void)SCHEDPART_DISASSOCIATE(prp, part_id_t_INVALID);
		(void)MEMPART_DISASSOCIATE(prp, part_id_t_INVALID);
		(void)MUTEX_DESTROY(prp, &prp->mpartlist_lock);
		(void)MUTEX_DESTROY(prp, &prp->spartlist_lock);
		object_free(NULL, &process_souls, prp);
		kererr(act, ENOMEM);
		return;
	}

	// Add process to the process table vector.
	if((pid = vector_add(&process_vector, prp, 0)) == -1) {
		(void)SCHEDPART_DISASSOCIATE(prp, part_id_t_INVALID);
		(void)MEMPART_DISASSOCIATE(prp, part_id_t_INVALID);
		(void)MUTEX_DESTROY(prp, &prp->mpartlist_lock);
		(void)MUTEX_DESTROY(prp, &prp->spartlist_lock);
		vector_free(&prp->threads);
		object_free(NULL, &process_souls, prp);
		kererr(act, ENOMEM);
		return;
	}

	prp->boundry_addr = VM_KERN_SPACE_BOUNDRY;
	prp->pid = pid | pid_unique; 	// adjust pid_unique during process destroy
	SIGMASK_SPECIAL(&prp->sig_queue);

	// Call out to allow memory manager to initialize the address space
	if((status = memmgr.mcreate(prp)) != EOK) {
		(void)SCHEDPART_DISASSOCIATE(prp, part_id_t_INVALID);
		(void)MEMPART_DISASSOCIATE(prp, part_id_t_INVALID);
		(void)MUTEX_DESTROY(prp, &prp->mpartlist_lock);
		(void)MUTEX_DESTROY(prp, &prp->spartlist_lock);
		vector_rem(&process_vector, PINDEX(pid));
		vector_free(&prp->threads);
		object_free(NULL, &process_souls, prp);
		kererr(act, status);
		return;
	}

	// Inherit parents information
	info = parent->cred->info;
	info.sgid = parent->cred->info.egid;
	info.suid = 0;			// The loader will set to euid after loading...
	cred_set(&prp->cred, &info);
	prp->seq = 1;

	// inherit setrlimit/getrlimit settings 
	for(i=0; i < RLIM_NLIMITS; i++) {
		prp->rlimit_vals_soft[i] = parent->rlimit_vals_soft[i];
		prp->rlimit_vals_hard[i] = parent->rlimit_vals_hard[i];
	}
	prp->max_cpu_time = parent->max_cpu_time;

	// stop core file generation if RLIMIT_CORE is 0
	if (prp->rlimit_vals_soft[RLIMIT_CORE] == 0) {
		prp->flags |= _NTO_PF_NOCOREDUMP;
	}

	// Inherit default scheduling partition
	// from creating thread.
	prp->default_dpp = SELECT_DPP(act, prp, schedpart_getid(prp));

	prp->pgrp = parent->pgrp;
	prp->umask = parent->umask;
	prp->sig_ignore = parent->sig_ignore;
	SIGMASK_NO_KILLSTOP(&prp->sig_ignore);

	if((prp->limits = lookup_limits(parent->cred->info.euid))  ||
	   (prp->limits = parent->limits)) {
		prp->limits->links++;
	}

	if((prp->session = parent->session)) {
		atomic_add(&prp->session->links, 1);
	}

	// Link the new process in as a child of its creator.
	prp->child = NULL;
	prp->parent = parent;
	prp->sibling = parent->child;
	parent->child = prp;
	++parent->num_processes;
	_TRACE_PR_EMIT_CREATE(prp);
	SETKSTATUS(act, prp);
}

/*
 * The addition parameter <extra> is used to pass in a list of partitions
 * that the newly created process should be associated with as well as any
 * kernel object memory class overrides 
*/
PROCESS *
ProcessCreate(pid_t parent_pid, void *lcp, proc_create_attr_t *extra) {
	struct kerargs_process_create	data;

	data.parent_pid = parent_pid;
	data.lcp = lcp;
	data.extra = extra;

	return((PROCESS *)__Ring0(kerext_process_create, &data));
}


static void
kerext_process_destroyall(void *data) {
	pid_t	pid = (pid_t)data;
	THREAD	*act = actives[KERNCPU];
	THREAD	*thp;
	PROCESS	*prp;

	if((prp = lookup_pid(pid)) == NULL) {
		kererr(act, ESRCH);
		return;
	}

	lock_kernel();
	if((thp = prp->valid_thp) != NULL) {
		thread_destroyall(thp);
		if(thp->state == STATE_STOPPED)
			ready(thp);
	}

	SETKSTATUS(act, 0);
}

int
ProcessDestroyAll(pid_t pid) {
	return(__Ring0(kerext_process_destroyall, (void *)pid));
}


static void
kerext_process_destroy(void *data) {
	PROCESS		*prp, *parent, *child;
	pid_t		pid = (pid_t)data;
	THREAD		*act = actives[KERNCPU];

	lock_kernel();

	if(!(prp = vector_lookup(&process_vector, PINDEX(pid)))) {
		kererr(act, ESRCH);
		return;
	}

	if(prp->querying) {
		// Some proc thread is looking at the process fields
		// (a QueryOject() has been done on it). We have to wait
		// before we can destroy the process and free the memory
		SETKSTATUS(act, 0);
		return;
	}

	_TRACE_PR_EMIT_DESTROY(prp, pid);

	// retarget all children to PROC
	if((child = prp->child)) {
		PROCESS		*proc = sysmgr_prp;

		do {
			//Loop might take a while - give intr queue a chance to drain.
			KEREXT_PREEMPT(act);

			prp->child = child->sibling;
			if(procmgr.process_threads_destroyed) {
				if (((child->flags & (_NTO_PF_LOADING | _NTO_PF_TERMING | _NTO_PF_ZOMBIE)) == _NTO_PF_ZOMBIE) || (child->flags & _NTO_PF_NOZOMBIE)) {
				struct sigevent		ev;

				child->flags &= ~(_NTO_PF_ZOMBIE | _NTO_PF_WAITINFO);
				(*procmgr.process_threads_destroyed)(child, &ev);
				sigevent_proc(&ev);
				}
			}
			child->parent = proc;
			child->sibling = proc->child;
			proc->child = child;
			--prp->num_processes; ++proc->num_processes;
		} while((child = prp->child));
	}

	vector_rem(&process_vector, PINDEX(pid));

	// Remove the thread vector
	if(prp->threads.nentries && prp->threads.nentries == prp->threads.nfree)
		vector_free(&prp->threads);

	// Remove the timer vector
	if(prp->timers.nentries && prp->timers.nentries == prp->timers.nfree)
		vector_free(&prp->timers);

	// Remove the fd vector
	if(prp->fdcons.nentries && prp->fdcons.nentries == prp->fdcons.nfree)
		vector_free(&prp->fdcons);

	// Remove the channel vector
	if(prp->chancons.nentries && prp->chancons.nentries == prp->chancons.nfree)
		vector_free(&prp->chancons);

	// Unlink the credential
	if(prp->cred) {
		cred_set(&prp->cred, NULL);
	}

	// undo all session stuff
	if(prp->session && atomic_sub_value(&prp->session->links, 1) == 1) {
		_sfree(prp->session, sizeof *prp->session);
	}
	prp->session = 0;

	// Unlink the limits
	if(prp->limits  &&  --prp->limits->links == 0) {
		LINK1_REM(limits_list, prp->limits, LIMITS);
		object_free(NULL, &limits_souls, prp->limits);
	}

	if(prp->limits  &&  prp->limits->links == ~0U) crash();
	if(prp->pid)				crash();
	if(prp->cred)				crash();
	if(prp->alarm)				crash();
	if(pril_first(&prp->sig_pending))		crash();
	if(prp->sig_table)			crash();
	if(prp->nfds)				crash();
	if(prp->chancons.vector)	crash();
	if(prp->fdcons.vector)		crash();
	if(prp->threads.vector)		crash();
	if(prp->timers.vector)		crash();
	if(prp->memory)				crash();
	if(prp->join_queue)			crash();
//	if(prp->session)			crash();
	if(prp->debugger)			crash();
	if(prp->lock)				crash();
	if(prp->num_active_threads)	crash();
	if(prp->vfork_info)			crash();
// FIX ME - this is not NULL now ... why?	if(prp->rsrc_list)			crash();
	if(prp->conf_table)			crash();

	// Unlink from parent
	parent = prp->parent;
	--parent->num_processes;
	if(prp == parent->child) {
		parent->child = prp->sibling;
	} else {
		for(parent = parent->child ; parent ; parent = parent->sibling) {
			if(parent->sibling == prp) {
				parent->sibling = prp->sibling;
				break;
			}
		}
	}

	//Keep pids as positive numbers
	pid_unique = (pid_unique + (PID_MASK + 1)) & INT_MAX;

	/* diassociate prp from all partitions */
/*
	apparently prp->pid gets looked at even though we are about to
	object_free(prp).
	prp->pid = pid;	// stick it back in so we have the info for the disassociate event
*/
	(void)SCHEDPART_DISASSOCIATE(prp, part_id_t_INVALID);
	(void)MEMPART_DISASSOCIATE(prp, part_id_t_INVALID);
	(void)MUTEX_DESTROY(prp, &prp->mpartlist_lock);
	(void)MUTEX_DESTROY(prp, &prp->spartlist_lock);
		
	CRASHCHECK(prp->mpart_list.vector != NULL);
	CRASHCHECK(prp->spart_list != NULL);	// no vector for sched partitions since only 1

	object_free(NULL, &process_souls, prp);
	SETKSTATUS(act, 1);
}

int
ProcessDestroy(pid_t pid) {
	return(__Ring0(kerext_process_destroy, (void *)pid));
}

struct kerargs_process_restore {
	int								rcvid;
	struct _thread_local_storage	*tsp;
	void							*frame;
	void							*frame_base;
	unsigned						frame_size;
};

static void
kerext_process_restore(void *data) {
	struct kerargs_process_restore	*kap = data;
	THREAD							*thp;
	
	if(!(lookup_rcvid(0, kap->rcvid, &thp)) || thp->state != STATE_REPLY) {
		return;
	}
	*thp->un.lcl.tls = *kap->tsp;
	memcpy(kap->frame_base, kap->frame, kap->frame_size);
}

void
ProcessRestore(int rcvid, struct _thread_local_storage *tsp, void *frame, void *frame_base, unsigned frame_size) {
	struct kerargs_process_restore	data;

	data.rcvid = rcvid;
	data.tsp = tsp;
	data.frame = frame;
	data.frame_base = frame_base;
	data.frame_size = frame_size;
	__Ring0(kerext_process_restore, &data);
}


struct kerargs_process_startup {
	pid_t	pid;
	int		start;
};

static void
kerext_process_startup(void *data) {
	struct kerargs_process_startup *kap = data;
	THREAD						*act = actives[KERNCPU];
	PROCESS						*prp;
	struct _cred_info			info;
	void						*lcp;
	
	if((prp = lookup_pid(kap->pid)) == NULL || (prp->flags & _NTO_PF_LOADING) == 0) {
		kererr(act, ESRCH);
		return;
	}

	lcp = prp->lcp;

	if(kap->start) {
		int							status;
		uintptr_t					start_ip;
		THREAD						*thp;

		status = procmgr.process_start ? procmgr.process_start(prp, &start_ip) : ENOSYS;
		if(status != EOK) {
			kererr(act, status);
			return;
		}

		lock_kernel();

		info = prp->cred->info;
		info.suid = info.euid;
		info.sgid = info.egid;
		cred_set(&prp->cred, &info);
		thp = prp->valid_thp;
		if(!(prp->flags & _NTO_PF_FORKED)) {
			SETKIP_FUNC(thp, start_ip);
			SETKSP(thp, prp->initial_esp);
		}
		cpu_process_startup(thp, prp->flags & _NTO_PF_FORKED);
		_TRACE_NOARGS(thp);

		/* @@@ This should be examined in more detail later!!! */
		prp->boundry_addr = WITHIN_BOUNDRY(prp->initial_esp, prp->initial_esp, user_boundry_addr) ? user_boundry_addr : VM_KERN_SPACE_BOUNDRY;
		prp->flags &= ~(_NTO_PF_LOADING | _NTO_PF_RING0);
		prp->lcp = 0;
		_TRACE_PR_EMIT_CREATE_NAME(prp);
	}

	lock_kernel();
	SETKSTATUS(act, lcp);
}

void *
ProcessStartup(pid_t pid, int start) {
	struct kerargs_process_startup	data;

	data.pid = pid;
	data.start = start;
	return((void *)__Ring0(kerext_process_startup, &data));
}


struct kerargs_process_exec {
	void	*lcp;
	int		*prcvid;
	int		rcvid;
	pid_t	ppid;
};

static void
kerext_process_exec(void *data) {
	struct kerargs_process_exec *kap = data;
	THREAD							*act = actives[KERNCPU];
	THREAD							*thp;
	CONNECT							*cop;
	PROCESS							*child, *prp;

	// Do the lookup through the child
	if(!(cop = lookup_rcvid(NULL, kap->rcvid, &thp)) || thp->state != STATE_REPLY || thp->blocked_on != cop) {
		kererr(act, ESRCH);
		return;
	}

	child = thp->process;
	prp = child->parent;

	if(prp->pid != kap->ppid || !prp->valid_thp) {
		kererr(act, ESRCH);
		return;
	}

	lock_kernel();

	if(prp->lcp) {
		// make exec safe for multithreaded process
		kererr(act, EBUSY);
		return;
	}

	/* @@@ Need to copy pending signals! */
	if(child->lcp != kap->lcp) {
		// Should never happen
		crash();
	}
	child->lcp = NULL;

	/*
	 * Both masks are propagated as is across exec() (replacement
	 * vs parent / child relationship).  Stash them both here to
	 * propagate them properly when we finally do the proc_start()
	 * which looks at the thread prcvid will point to.
	 */
	thp->runmask = prp->valid_thp->runmask;
	thp->default_runmask = prp->valid_thp->default_runmask;

	/* No need to force ready this thread, since we're going to reply to it in procmgr_spawn */
	prp->lcp = kap->lcp;
	*kap->prcvid = kap->rcvid;
	thread_destroyall(prp->valid_thp);
	SETKSTATUS(act, 0);
}

int
ProcessExec(void *lcp, int *prcvid, int rcvid, int ppid) {
	struct kerargs_process_exec	data;

	data.lcp = lcp;
	data.prcvid = prcvid;
	data.rcvid = rcvid;
	data.ppid = ppid;
	return(__Ring0(kerext_process_exec, &data));
}


static void process_swap(PROCESS *child) {
	PROCESS			*prp, **pprp;
	PROCESS			*parent = child->parent;

	if((prp = child->parent = parent->parent)) {
		for(pprp = &prp->child; (prp = *pprp); pprp = &prp->sibling) {
			if(prp == parent) {
				*pprp = child;
				break;
			}
		}
	}
	parent->parent = child;
	prp = parent->sibling;
	parent->sibling = child->sibling;
	child->sibling = prp;
	for(pprp = &parent->child; (prp = *pprp); pprp = &prp->sibling) {
		if(prp == child) {
			*pprp = parent;
			prp = parent;
		} else {
			prp->parent = child;
		}
	}
	*pprp = child->child;
	child->child = parent->child;
	parent->child = 0;
	child->num_processes = parent->num_processes; parent->num_processes = 0;
}


struct kerargs_process_swap {
	PROCESS	*parent;
	PROCESS	*child;
};

static void
kerext_process_swap(void *data) {
	struct kerargs_process_swap		*kap = data;
	PROCESS							*parent = kap->parent;
	PROCESS							*child = kap->child;
	CONNECT							*proc_cop;
	pid_t							pid;
	int								i;
	DEBUG							*swap;

	lock_kernel();
	
	/* Swap process containers */
	process_vector.vector[PINDEX(parent->pid)] = child;
	process_vector.vector[PINDEX(child->pid)] = parent;

	if(child->parent != parent) crash();
	process_swap(child);

	pid = parent->pid;
	parent->pid = child->pid;
	child->pid = pid;

	if(parent->ocb_list || child->ocb_list) {
		void 		*ocb_list;

		ocb_list = parent->ocb_list;
		parent->ocb_list = child->ocb_list;
		child->ocb_list = ocb_list;
	}

	if(parent->alarm) {
		TIMER	*alarm = parent->alarm;

		parent->alarm = 0;
		alarm->thread = child->valid_thp;
		child->alarm = alarm;
	}

	/* Flags to inherit */
#define FLAGS	(_NTO_PF_WAITINFO | \
				_NTO_PF_WAITDONE | \
				_NTO_PF_SLEADER | \
				_NTO_PF_ORPHAN_PGRP | \
				_NTO_PF_NOCLDSTOP | \
				_NTO_PF_PTRACED | \
				_NTO_PF_NOZOMBIE)
	i = parent->flags;
    parent->flags = (parent->flags & ~FLAGS) | (child->flags & FLAGS);
    child->flags = (child->flags & ~FLAGS) | (i & FLAGS);
#undef FLAGS

	// Find the child's connection to proc (It's always the first chancon entry)
	proc_cop = vector_lookup(&child->chancons, SYSMGR_COID & ~_NTO_SIDE_CHANNEL);

#ifndef NDEBUG
	// There should be two links, One from the Send() and one from the ConnectAttach()
	if(!proc_cop || !proc_cop->channel || proc_cop->links != 2) crash();
	if(proc_cop->un.lcl.pid != SYSMGR_PID || proc_cop->un.lcl.chid != SYSMGR_CHID) crash();
	if(parent->chancons.nentries != parent->chancons.nfree) crash();
	if(child->chancons.nentries != child->chancons.nfree + 1) crash();
	if(child->fdcons.nentries != child->fdcons.nfree) crash();
#endif

	for(i = 0 ; i < parent->fdcons.nentries ; ++i) {
		CONNECT							*cop;

		if((cop = vector_rem(&parent->fdcons, i))) {
			if(vector_add(&child->fdcons, cop, i) != i) {
				crash();
			}

			parent->nfds--;
			child->nfds++;

			cop->process = child;

			// Check if this connects to proc's channel, if so, share it...
			if(proc_cop && cop->channel == proc_cop->channel) {
				vector_rem(&child->chancons, SYSMGR_COID & ~_NTO_SIDE_CHANNEL);
				vector_add(&child->chancons, cop, SYSMGR_COID & ~_NTO_SIDE_CHANNEL);
				cop->links++;
				proc_cop->links--;
				proc_cop = 0;
			}
		}
	}

	swap = child->debugger;
	if((swap != NULL) && (swap->process == child)) {
		swap->process = parent;
	}
	swap = parent->debugger;
	if((swap != NULL) && (swap->process == parent)) {
		swap->process = child;
	}
	parent->debugger = child->debugger;
	child->debugger = swap;

	SETKSTATUS(actives[KERNCPU], 0);
}
	
int
ProcessSwap(PROCESS *parent, PROCESS *child) {
	struct kerargs_process_swap	data;

	data.parent = parent;
	data.child = child;
	return(__Ring0(kerext_process_swap, &data));
}


void rdecl
set_safe_aspace(int cpu) {
	//The address space that aspaces_prp[cpu] is pointing at is going away.
	//Point things at someplace safe.
	if(actives[cpu]->aspace_prp == NULL) {
		memmgr.aspace(sysmgr_prp, &aspaces_prp[cpu]);
	}
}



struct kerargs_process_shutdown {
	PROCESS	*prp;
	int		exec;
	int		tlb_safe_cpu;
};

static void
kerext_process_shutdown(void *data) {
	struct kerargs_process_shutdown	*args = data;
	PROCESS			*prp = args->prp;
	THREAD			*act = actives[KERNCPU];
	SIGTABLE		*stp;
	int				tid;
	int				i;
	int				try_again;

	lock_kernel();
	if(prp == NULL) {
		prp = act->process;
	}

	if(prp->memory) {
		// Make sure no threads reference the aspace before it's removed
		for(tid = 0; tid < prp->threads.nentries; tid++) {
			THREAD				*thp;

			if(VECP(thp, &prp->threads, tid)) {
				thp->aspace_prp = 0;
			}
		}

		try_again = 0;
		for(i = 0; i < NUM_PROCESSORS; ++i) {
			PROCESS *prp0;

			// The code between interrupt disable and enable should be short,
			// so that other CPUs will not have chance to get a interrupt and
			// switch aspace during the check
			InterruptDisable();
			if(get_inkernel() & INKERNEL_INTRMASK) {
				try_again = 1;
			}
			prp0 = *((PROCESS* volatile *)&aspaces_prp[i]);
			if(get_inkernel() & INKERNEL_INTRMASK) {
				try_again = 1;
			}
			InterruptEnable();

			if(prp == prp0) {
				// Switch current address space
				if(i == KERNCPU) {
					set_safe_aspace(i);
				} else {
					args->tlb_safe_cpu = i;
					SENDIPI(i, IPI_TLB_SAFE);
					try_again = 1;
				}
			}
		}
		if(try_again) {
			//Another CPU is pointing at this address space. We've sent
			//an IPI to get it to point at a safe entry (the process manager's),
			//but we have to wait until it's been processed before we can
			//destroy this address space.
			kererr(act, EAGAIN);
			return;
		}

		if(prp->flags & _NTO_PF_VFORKED) {
			// We're shutting down a vfork'd process. Don't free the
			// aspace, the parent process is still using it.
			prp->memory = NULL; 
		} else {
			memmgr.mdestroy(prp);
		}

		if(prp->memory || aspaces_prp[KERNCPU] == prp) {
			crash();
		}
		SETKSTATUS(act, 0);
		return;
	}

	// Remove all signal tables
	while((stp = prp->sig_table)) {
		prp->sig_table = stp->next;
		object_free(NULL, &sigtable_souls, stp);
	}

	// Check for guardian
	if(prp->guardian && (prp->guardian->flags & (_NTO_PF_LOADING | _NTO_PF_TERMING | _NTO_PF_ZOMBIE)) == 0) {
		process_swap(prp->guardian);
		prp->guardian = 0;
	}		

	if(prp == act->process) {
		if(!args->exec) {
			// if we need to, send a sigchld to the parent
			if (!(prp->flags & _NTO_PF_NOZOMBIE)) {
				signal_kill_process(prp->parent, SIGCHLD, prp->siginfo.si_code,
					prp->siginfo.si_value.sival_int, prp->siginfo.si_pid, 0 );
			}
			prp->siginfo.si_signo = SIGCHLD;
		}
		actives_prp[KERNCPU] = sysmgr_prp;
		thread_destroyall(act);
	} else {
		SETKSTATUS(act, prp->siginfo.si_status);
	}
}

int
ProcessShutdown(PROCESS *prp, int exec) {
	struct kerargs_process_shutdown	data;
	int								r;

	data.prp = prp;
	data.exec = exec;
	data.tlb_safe_cpu = -1;
	r = __Ring0(kerext_process_shutdown, &data);
#if defined(VARIANT_smp)
	//RUSH3: We should loop if r==-1 && errno == EAGAIN. Right now
	//RUSH3: the caller has to do that.
	if((r == -1) && (data.tlb_safe_cpu >= 0)) {
#ifndef NDEBUG
		unsigned	loop;
		#define 	DBG_LOOP_SET()	(loop = 0)
		#define		DBG_LOOP_CHK()	if(++loop == 0) crash()
#else
		#define 	DBG_LOOP_SET()	
		#define		DBG_LOOP_CHK()	
#endif
		/*
		 * The memory barriers below will translate into bus synchronizing opcodes
		 * to give the other processor a chance to acquire the cacheline and
		 * make progress.
		 *
		 * Do not delete: this will cause the PPC SMP kernel to livelock.
		 */
		// Pause outside the kernel until the TLB safe operation has
		// been done (acknowledged, anyway).
		DBG_LOOP_SET();
		do {
			DBG_LOOP_CHK();
			MEM_BARRIER_RW();
		} while(ipicmds[data.tlb_safe_cpu] & IPI_TLB_SAFE);

		DBG_LOOP_SET();
		do {
			DBG_LOOP_CHK();
			MEM_BARRIER_RW(); /* RUSH3: do we really need 3 membarriers here? */
			MEM_BARRIER_RW();
			MEM_BARRIER_RW();
		} while(actives[data.tlb_safe_cpu]->flags & _NTO_ITF_MSG_DELIVERY);
	}
#endif
	return(r);
}


struct kerargs_thread_tls {
	struct _thread_local_storage	*tls;
	uintptr_t						stackaddr;
	unsigned						stacksize;
	unsigned						guardsize;
	uintptr_t						esp;
};

void
kerext_thread_tls(void *data) {
	struct kerargs_thread_tls *kap = data;
	THREAD						*act = actives[KERNCPU];

	if(kap->esp) {
		struct _thread_local_storage		*tsp;

		tsp= (struct _thread_local_storage *)((kap->esp & ~(STACK_ALIGNMENT-1)) - sizeof *act->un.lcl.tls);
		// Make sure this occurs in prolog so a page fault can occur
		*tsp = *act->un.lcl.tls;
		lock_kernel();
		kap->tls = tsp;
		if(act->un.lcl.tls->__errptr == &act->un.lcl.tls->__errval) {
			kap->tls->__errptr = &kap->tls->__errval;
		}
	} 
	lock_kernel();
	if(kap->tls) {
		act->un.lcl.tls = kap->tls;
	}

	act->un.lcl.stackaddr = (void *)kap->stackaddr;
	act->un.lcl.tls->__stackaddr = (uint8_t *)act->un.lcl.stackaddr + kap->guardsize;
	act->un.lcl.stacksize = kap->stacksize;

	SETKSTATUS(act, kap->tls);
}

struct _thread_local_storage *
ThreadTLS(struct _thread_local_storage *tls, uintptr_t stackaddr,
			unsigned stacksize, unsigned guardsize, uintptr_t esp) {
	struct kerargs_thread_tls	data;

	data.tls = tls;
	data.stackaddr = stackaddr;
	data.stacksize = stacksize;
	data.guardsize = guardsize;
	data.esp = esp;
	return((struct _thread_local_storage *)__Ring0(kerext_thread_tls, &data));
}


uintptr_t
GetSp(THREAD *thp) {
	return KSP(thp);
}

__SRCVERSION("kerext_process.c $Rev: 199378 $");
