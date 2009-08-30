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
#include <sys/image.h>
#include "apm.h"
#include "aps.h"


static void
init_soul(SOUL *sp) {
	(void)object_grow(sp);
	mdriver_check();
}


void
init_objects() {
	PROCESS		*prp;
	THREAD		*thp;
	int			i;
	unsigned	ker_stack_size;
	uintptr_t	ker_sp;
	DISPATCH	*dpp;

	// Link all vectors off the query vector.
	vector_add(&query_vector, &mempart_vector, _QUERY_MEMORY_PARTITION);
	vector_add(&query_vector, &schedpart_vector, _QUERY_SCHEDULER_PARTITION);
	vector_add(&query_vector, &soul_vector, 0);
	vector_add(&query_vector, &process_vector, 0);
	vector_add(&query_vector, &interrupt_vector, 0);
	vector_add(&query_vector, &vthread_vector, 0);
	vector_add(&query_vector, &chgbl_vector, 0);

	// Create all the objects.
	mdriver_check();
	init_soul(&process_souls);		// Allocate the min number of processes.
	init_soul(&thread_souls);		// Allocate the min number of threads.
	init_soul(&channel_souls);		// Allocate the min number of channels.
	init_soul(&chasync_souls);		// Allocate the min number of async channels.
	init_soul(&chgbl_souls);		// Allocate the min number of gbl channels.
	init_soul(&connect_souls);		// Allocate the min number of connects.
	init_soul(&pulse_souls);		// Allocate the min number of pulses.
	init_soul(&sync_souls);			// Allocate the min number of syncs.
	init_soul(&syncevent_souls);	// Allocate the min number of syncevents.
	init_soul(&interrupt_souls);	// Allocate the min number of interrupts.
	init_soul(&sigtable_souls);		// Allocate the min number of signal tables.
	init_soul(&timer_souls);		// Allocate the min number of timers.
	init_soul(&credential_souls);	// Allocate the min number of credentials.
	init_soul(&ssinfo_souls);		// Allocate the min number of ss schedinfos.
	init_soul(&threadname_souls);	// Allocate the min number of thread name souls

	// Create all hash tables
	sync_hash.table = _scalloc((sync_hash.mask + 1) * sizeof(SYNC *));

	// Allocate the process table mapping vector. Reserves first slot.
	vector_add(&process_vector, 0, process_souls.min);	// Pregrow vector

	// Create a process container for all the kernel threads.
	prp = object_alloc(NULL, &process_souls);
	vector_add(&process_vector, prp, 0);	// Stuff slot 0 so any attempt to use faults
	procnto_prp = prp;
	prp->pgrp = prp->pid = vector_add(&process_vector, prp, 1);

	/*
	 * before we go any further, associate procnto_prp with the system memory
	 * partition. This will allow all other allocations to be able to locate a
	 * partition of type sysram.
	 * Scheduling partition initialization is done below after init_scheduler()
	 * is called
	*/
	(void)MEMPART_ASSOCIATE(prp, mempart_getid(NULL, sys_memclass_id), part_flags_NONE);

	prp->cred = credential_list = object_alloc(NULL, &credential_souls);
	prp->cred->next = NULL;
	prp->cred->links = 1;
	prp->seq = 1;

	// init the getrlimit/setrlimit stuff
	for(i=0; i < RLIM_NLIMITS; i++) {
		prp->rlimit_vals_soft[i] = prp->rlimit_vals_hard[i] = RLIM_INFINITY;
	}

	// override the max file descriptor entry 
	prp->rlimit_vals_soft[RLIMIT_NOFILE] = prp->rlimit_vals_hard[RLIMIT_NOFILE] = max_fds;

	// make sure we have a big limit for cpu time
	prp->max_cpu_time = RLIM64_INFINITY;
	
	prp->start_time = qtimeptr->nsec_tod_adjust;
	if(limits_max[0]) {
		prp->limits = object_alloc(0, &limits_souls);
		prp->limits->links = 1;
		memcpy(prp->limits->max, limits_max, sizeof(prp->limits->max));
	}
	prp->flags = _NTO_PF_RING0 | _NTO_PF_SLEADER | _NTO_PF_NOCLDSTOP;
	prp->num_active_threads = NUM_PROCESSORS;
	prp->boundry_addr = VM_KERN_SPACE_BOUNDRY;
	prp->sig_ignore.__bits[0] |= SIGMASK_BIT(SIGKILL) | SIGMASK_BIT(SIGSTOP) |
			SIGMASK_BIT(SIGTSTP) | SIGMASK_BIT(SIGTTIN) | SIGMASK_BIT(SIGTTOU);
	SIGMASK_SPECIAL(&prp->sig_queue);

	mdriver_check();
	//Alloc space for all the kernel stacks;
	ker_stack_size = KER_STACK_NBYTES * NUM_PROCESSORS;
	run_ker_stack_bot = (uintptr_t)_scalloc(ker_stack_size);
	run_ker_stack_top = run_ker_stack_bot + ker_stack_size;
	ker_sp = STACK_INIT(run_ker_stack_bot, ker_stack_size);

	// Initialize scheduler and default dispatch queue
	dpp = init_scheduler();
	prp->default_dpp = dpp;
	(void)SCHEDPART_ASSOCIATE(prp, schedpart_getid(NULL), part_flags_NONE);

	// Create the idle thread(s) and put it/them in the ready queue
	i = 0;
	do {
		uintptr_t	stack_top;
		void		*dummy;

		thp = object_alloc(NULL, &thread_souls);
		memset(thp, 0, sizeof(*thp));
		thp->tid = vector_add(&prp->threads, thp, 0);
		thp->state = STATE_RUNNING;
		thp->process = prp;
		thp->runcpu = i;
		// We can leave default_runmask to zero for inheritance
		thp->runmask = ~(1 << i);
		thp->last_chid = -1;
		thp->policy = SCHED_RR;
		thp->sched_flags = 0; 
		thp->dpp = thp->orig_dpp = dpp;
		SIGMASK_ONES(&thp->sig_blocked);
		thp->type = TYPE_THREAD;
		thp->flags = _NTO_TF_NOMULTISIG | _NTO_TF_WAAA;
		thp->real_priority = thp->priority = NUM_PRI - 1;
		thp->start_time = prp->start_time;
		thp->un.lcl.stackaddr = (void *)_smalloc(sizeof(startup_stack));
		thp->un.lcl.stacksize = sizeof(startup_stack);
		stack_top = STACK_INIT((uintptr_t)thp->un.lcl.stackaddr, thp->un.lcl.stacksize);
		STACK_ALLOC(thp->un.lcl.tls, stack_top, stack_top, sizeof(struct _thread_local_storage));
		STACK_ALLOC(dummy, stack_top, stack_top, STACK_INITIAL_CALL_CONVENTION_USAGE);
		SETKSP(thp, stack_top);
		SETKIP(thp, &idle);
		cpu_thread_init(NULL, thp, align_fault);

		actives[i] = thp;
		actives_prp[i] = prp;

		STACK_ALLOC(dummy, ker_stack[i], ker_sp, STACK_INITIAL_CALL_CONVENTION_USAGE);
		STACK_ALLOC(dummy, ker_sp, ker_sp, KER_STACK_NBYTES);
	} while(++i < NUM_PROCESSORS);
	prp->valid_thp = thp;
	mdriver_check();
}

__SRCVERSION("init_objects.c $Rev: 174816 $");
