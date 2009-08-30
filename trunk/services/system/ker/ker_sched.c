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

int kdecl
ker_sched_get(THREAD *act, struct kerargs_sched_get *kap) {
	PROCESS	*prp;
	THREAD	*thp;


	// Verify the target process exists.
	if((prp = (kap->pid ? lookup_pid(kap->pid) : act->process)) == NULL) {
		return ESRCH;
	}

	// Verify the target thread exists.
	if((thp = (kap->tid ? vector_lookup(&prp->threads, kap->tid-1) : act)) == NULL) {
		return ESRCH;
	}

	// Verify we have the right to examine the target process
	if(!kerisusr(act, prp)) {
		return ENOERROR;
	}

	if(kap->param) {
		WR_VERIFY_PTR(act, kap->param, sizeof(*kap->param));
		WR_PROBE_OPT(act, kap->param, sizeof(*kap->param) / sizeof(int));
		kap->param->sched_curpriority = thp->priority;
		kap->param->sched_priority = thp->real_priority;

		lock_kernel();
		if(thp->policy == SCHED_SPORADIC) {
			if (thp->schedinfo.ss_info->org_priority) {
				// Running at low priority - return the original sched_priority
				kap->param->sched_priority = thp->schedinfo.ss_info->org_priority;
			}
			kap->param->sched_ss_low_priority = thp->schedinfo.ss_info->low_priority;
			kap->param->sched_ss_max_repl = thp->schedinfo.ss_info->max_repl;
			nsec2timespec(&kap->param->sched_ss_init_budget, thp->schedinfo.ss_info->init_budget);
			nsec2timespec(&kap->param->sched_ss_repl_period, thp->schedinfo.ss_info->repl_period);
		}
		unlock_kernel();
	}


	lock_kernel();
	SETKSTATUS(act,thp->policy);

	return ENOERROR;
}


int kdecl
ker_sched_set(THREAD *act, struct kerargs_sched_set *kap) {
	PROCESS	*prp;
	THREAD	*thp;
	int		 status;

	// Verify the target process exists.
	if((prp = (((kap->tid > 0) && kap->pid)  ? lookup_pid(kap->pid) : act->process)) == NULL) {
		return ESRCH;
	}

	// Verify the target thread exists, or find one for verification.
	if(kap->tid != -1) {
		if((thp = (kap->tid ? vector_lookup(&prp->threads, kap->tid-1) : act)) == NULL) {
			return ESRCH;
		}
	} else {
		if(kap->policy != SCHED_NOCHANGE) {
			return EINVAL;
		}
		thp = NULL;
	}

	// Verify we have the right to change the target process
	if(!kerisusr(act, prp)) {
		return ENOERROR;
	}

	// Verify we have good data pointers
	switch(kap->policy) {
	case SCHED_ADJTOHEAD:
	case SCHED_ADJTOTAIL:
		break;
	default:
		RD_VERIFY_PTR(act, &kap->param->sched_priority, sizeof(kap->param->sched_priority));
		RD_PROBE_INT(act, &kap->param->sched_priority, 1);
	}
	lock_kernel();

	/*
	 Either make a process wide priority adjustment or apply the parameter
	 change to only one thread as targetted.
	*/
	if(thp == NULL) {
		int delta;

		//NYI: no range checking being done....
		delta = kap->param->sched_priority - prp->process_priority;
		//lint --e(734) Loss of precision
		prp->process_priority += delta;

		/*@@@ Run the entire thread list adjusting by the delta amount?
		  PROBLEM: sched_thread does more range checking than
		  	adjust_priority does.
		for(tid = 0 ; tid < prp->threads.nentries ; ++tid) {
			if(thp = VECP(thp, &prp->threads, tid)) {
				adjust_priority(thp, thp->priority + delta, thp->dpp);
				thp->real_priority += delta;
			}
		}
		*/
	} else if((status = sched_thread(thp, kap->policy, kap->param)) != EOK) {
		return status;
	}

	return EOK;
}

int kdecl
ker_sched_info(THREAD *act, struct kerargs_sched_info *kap) {
	PROCESS					*prp;
	struct _sched_info		*sip = kap->info;

	// Verify the target process exists.
	if((prp = (kap->pid ? lookup_pid(kap->pid) : act->process)) == NULL) {
		return ESRCH;
	}

	WR_VERIFY_PTR(act, sip, sizeof(*sip));
	WR_PROBE_OPT(act, sip, sizeof(*sip) / sizeof(int));

	// Set min to 1 and max to number of priorities
	memset(sip, 0x00, sizeof *sip);
	if(act->process->cred->info.euid == 0) {
		sip->priority_max = NUM_PRI - 1;
	} else {
		sip->priority_max = priv_prio - 1;
	}
	sip->priority_min = 1;	//Only idle allowed at prio 0
	sip->priority_priv = priv_prio - 1;

	// Verify the policy
	switch(kap->policy) {
	case SCHED_RR:
	case SCHED_OTHER:
		sip->interval = qtimeptr->nsec_inc * _NTO_RR_INTERVAL_MUL;
		/* Fall Through */
	case SCHED_SPORADIC:
	case SCHED_FIFO:
		break;
	default:
		return EINVAL;
	}

	return EOK;
}


int kdecl
ker_sched_yield(THREAD *act, struct kerargs_null *kap) {

	lock_kernel();
	yield();
	return EOK;
}

__SRCVERSION("ker_sched.c $Rev: 153052 $");
