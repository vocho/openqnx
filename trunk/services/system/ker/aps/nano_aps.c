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

/* nano_aps.c
 *
 * Adaptive Partitioning scheduling
 *
 * Is the eqivalent of ker/nano_sched:
 *    - function pointer to overbind the basic scheduling functions of nano_sched:
 *      ready, block, block_and_ready, adjust_priority etc.
 *    - initializeation code
 *        - inits aps data
 *        - binds our replacements for read, block etc...
 *
 */



#include "externs.h"
#include "aps_alg.h"
#include "aps_time.h"
#include "proto_aps.h"
#include "aps.h"
#include <kernel/schedpart.h>
#include "mt_kertrace.h"
/*
 * Debug/sanity checks
 */
#define chk_cond(cond)
#define chk_lock()
#define chk_thread(a)

#ifdef DEBUG_APS
#define chk_dispatch(chk)	_chk_dispatch(chk, __LINE__)
static void _chk_dispatch(int chkstate, int line);
#else
#define chk_dispatch(chk)
#endif

__SRCVERSION("nano_aps.c $Rev: 212233 $");


/* security */
EXT	uint32_t	aps_security;	/* set of SCHED_APS_SEC_* flags from sys/sched_aps.h. located in ker_aps.c */



DISPATCH *lookup_dpp(int id) {
	if( id <0 || id >= num_ppg) return NULL;

	return (DISPATCH *)actives_ppg[id];

}
/*
 * mark_running() function for purpose group scheduler
 * Includes microbilling.
 */

void rdecl
mark_running_ppg(THREAD *new) {
	THREAD	*old = NULL;

if(new->runmask & (1 << RUNCPU)) {
	// Keep check for a couple of regress runs
	crash();
}
	old = actives[KERNCPU];
#ifdef _mt_LTT_TRACES_	/* PDB */
    mt_TRACE_DEBUG("PDB APS1");
    //mt_trace_var_debug(old->process->pid, old->tid, old);
	mt_trace_task_suspend(old->process->pid, old->tid);
#endif

	new->state = STATE_RUNNING;
	SB_PARTITION_IS_RUNNING(new->dpp->id);
	new->runcpu = KERNCPU;

	trace_emit_th_state(new, STATE_RUNNING);

//	VALIDATE_NEEDTORUN(new)
	need_to_run = 0;

	chk_cond(old != NULL);

	while(ticker_preamble || old->ticker_using) {
		// Wait for the clock handler to finish with this thread
		__cpu_membarrier();
	}

	/* microbill() before setting the new active. This prevents a tick interrupt from
	 * microbilling the last last tick to the new active.
	 */
	microbill(old);
	actives[KERNCPU] = new;
#ifdef _mt_LTT_TRACES_	/* PDB */
    mt_TRACE_DEBUG("PDB APS2");
    //mt_trace_var_debug(new->process->pid, new->tid, new);
	mt_trace_task_resume(new->process->pid, new->tid);
#endif
	if(new->priority == 0 && old->state == STATE_READY) crash();
}



/*
 * See if someone is a better candidate than current active
 */
//@@@
THREAD *find_better_candidate(THREAD *act) {
	DISPATCH	*dpp;
	THREAD		*thp;
	thp = choose_thread_to_schedule(act,NULL);
	if (!thp) return NULL;
	if (thp == act ) return NULL;
	dpp = thp->dpp;
	LINK3_REM(DISPATCH_LST(dpp, (thp)->priority), (thp), THREAD);

	/* If that list is now empty, clear the readybit.	*/
	if (DISPATCH_THP(dpp, (thp)->priority) == NULL) DISPATCH_CLR(dpp, (thp));
	return thp;
}
/* select_thread_ppg(), select_chosen()
 *
 * Select the next eligible thread that should now be run.
 * act, if passed in, is part of the selection process. thp
 * will be non-NULL only if a thread equal or better than act can be found.
 *
 * cpu will be used on SMP to match runmask, while prio may or
 * may not be enforced depending on budget of partitions.
 *
 * Note that is another thread of equal prio is available, it will be returned
 *
 * Use the variant, select_chosen() when the result of choose_thread..() is already known.
 *
 */
THREAD * rdecl select_chosen(THREAD *act, int cpu, int prio, THREAD *thp, int run_as_critical) {
	//thp is chosen thread as determined by choose_thread_to_schedule()
	DISPATCH	*dpp;
	uint32_t	runmask = 1 << cpu;

	if(act && thp == act) {
		// Choose_thread only looks for better thread, not equal prio in
		// same partition.
		dpp = act->dpp;
		if(run_as_critical || (thp = DISPATCH_THP(dpp, act->priority)) == NULL) {
			/* If act is still best, return NULL. Note that we dont round-robin timeslice critical threads. */
			return NULL;
		}


		// Continue - we have a better candidate.
		// But make sure the runmask matches, else keep looking.
		while ( thp->runmask & runmask) {
			thp=thp->next.thread;
			if (NULL==thp) {
				return NULL;
			}
		}


		// Make sure thp is not billed critical.
		UPD_BILL_AS_CRIT(thp, run_as_critical);
	}

	/* Check if idle is suitable */
	if (!thp && prio <= 0) { //nothing better than idler
		thp = DISPATCH_THP( (DISPATCH*)system_ppg, 0);
		while(thp) {
			if((thp->runmask & runmask) == 0) break;
			thp = thp->next.thread;
		}
	}
	if(thp == NULL) crash();
	dpp = thp->dpp;
   	LINK3_REM(DISPATCH_LST(dpp, (thp)->priority), (thp), THREAD);

	/* If that list is now empty, clear the readybit.	*/
   	if (DISPATCH_THP(dpp, (thp)->priority) == NULL)  DISPATCH_CLR(dpp, (thp));

	return thp;
}

THREAD * rdecl select_thread_ppg(THREAD *act, int cpu, int prio) {
	THREAD		*thp;
	int		run_as_critical = 0;

 	thp = choose_thread_to_schedule(act, &run_as_critical );
	return select_chosen(act,cpu,prio,thp, run_as_critical);
}

/*
 * The select_cpu is used with SMP
 *
 * The algorithm for selecting a CPU is based on the following:
 *
 * 1. Scan through the cpus and see if runmask allows running on it
 * 2. If a CPU matches, select if either we are higher prio, or the
 *    thread running on that CPU is running with "free time"
 * 3. In case of tie above, we select the CPU which was running a thread on free time
 */

int rdecl
select_cpu_ppg(THREAD *thp) {
	THREAD		*act;
	int		i, j, cpu;
	unsigned	minprio;
	unsigned	alternates[PROCESSORS_MAX];

	for(i = NUM_PROCESSORS - 1 , j = 0, cpu = -1, minprio = thp->priority ; i >= 0 ; --i) {
		if((thp->runmask & (1 << i)) == 0) {
			act = actives[i];
			if(act->priority < minprio) {
				j = 0;
				cpu = i;
				minprio = act->priority;
			} else if(act->priority == minprio) {
				alternates[j++] = i;
			}
		}
	}

	// If we found a cpu make sure it is the best one in the case of
	// multiple choices!
	if(cpu != -1  &&  thp->runcpu != cpu  &&  j) {
		while(j) {
			int		new_cpu;

			new_cpu = alternates[--j];
			if(new_cpu == thp->runcpu) {
				cpu = new_cpu;
			}
		}
	}

	return cpu;
}

/*
 * yield is called from the SchedYield kernel call. We have a generic
 * AP version here which uses select_thread to select a better runnable
 * thread in the same (or other) partition.
 */

static void rdecl
yield_ppg(void) {
	THREAD		*act = actives[KERNCPU];
	THREAD		*thp;
	uint8_t		prio = act->priority;

	// Run down this priority queue looking for a thread which will
	// run on this processor.
	chk_lock();

	/*
	 * This should work for UP, SMP and APS. The requirement is that
	 * select_thread be smart enough to return a candidate equal or better
	 * to act.
	 */

	thp = select_thread(act, KERNCPU, prio);
#ifndef NDEBUG
// A prio lower than act is OK for APS, but we should never get idle.
if((thp != NULL) && (thp->priority == 0)) crash();
#endif

	if(thp && (thp != act)) {
		// select_thread has returned a better candidate and
		// thp has already been removed from dpp
		DISPATCH	*dpp = act->dpp;

		LINK3_END(DISPATCH_LST(dpp, prio), act, THREAD);
		DISPATCH_SET(dpp, act);

		RR_RESET_TICK(act);
		act->state = STATE_READY;
		trace_emit_th_state(act, STATE_READY);

		thp->runcpu = KERNCPU;
		mark_running(thp);

		//If we give up the CPU, it is like blocking
		if(thp != act) {
			SS_STOP_RUNNING(act, 0);
			SS_MARK_ACTIVATION(act);
		}
	}
	chk_dispatch(1);
}



/*
 * Resched is called in a couple of places:
 * 1. On exit from the clock tick to yield the CPU
 * 2. For SMP, anytime a reschedule IPI comes in
 */

static void rdecl
resched_ppg(void) {
	THREAD		*act = actives[KERNCPU];
	THREAD		*thp;
	DISPATCH	*dpp;
	uint8_t		prio;
	uint8_t		adjusted;



	// Note: only the CPU getting clock ticks will get called by ppg_tick_hook
	// and will microbill his current active plus every other cpu's active (i.e. ours).
	// So we are already microbilled at this point.

	if(NUM_PROCESSORS > 1) {
			need_to_run = NULL; //turn off need to run as it's not compatible with aps.
	}

	chk_lock();


	//If we are coming in from an ISR where the expiry of the execution
	//budget of a SS thread was detected, then look at rescheduling and
	//potentially drop the priority of the active thread.
	SS_CHECK_EXPIRY(act);
	adjusted = (act != actives[KERNCPU]);

	// If there is a replenishment pending, then do the adjustment
	adjusted = (ss_replenish_list && sched_ss_adjust()!=0 ) || adjusted;

	if ( adjusted ){
		//if the sporadic scheduler swapped the active, it did so with adjust_priority_ppg(), so AP rules
		//have been follwed and we can exit
		//Otherwise keep going
		if ( MAY_SCHEDULE((PPG*)(actives[KERNCPU]->dpp))) return;
		//it's possible approximations in adjustpritory, called by SS, picked an out-of-budget
		//partition. In that case, continue to invoke the full scheduler
	}


    	// SS may have changed the active.
	act=actives[KERNCPU];
	dpp = act->dpp;
	prio = act->priority;



	// check if this process exceeded its max cpu usage
	if (act->process->running_time > act->process->max_cpu_time &&
		!(act->flags & _NTO_TF_KILLSELF)) {

		if (signal_kill_process(act->process, SIGXCPU, 0, 0, act->process->pid,0) == SIGSTAT_IGNORED) {
			signal_kill_process(act->process, SIGKILL, 0, 0, act->process->pid,0);
		}
		return;
	}

	/*
	 * Look to see if we still are the best runnable candidate
	 * We look for 2 conditions:
	 * 1. no group has higher priority threads
	 *    with budget (which could happen since we may have
	 *    replenished budget at last clock tick).
	 * 2. If we are highest, we are the group with greatest fraction of
	 *    budget left.
	 *		@@@@ Not checking for condition #2 right now - revisit
	 */
	//@@@
	thp = find_better_candidate(act);
//	thp = choose_thread_to_schedule(act,0);
	if(thp && (thp->runmask & (1 << RUNCPU))) crash();

	if(thp == NULL) {
		/* Check for a round-robin expiry */

		if(IS_SCHED_RR(act) && (act->schedinfo.rr_ticks >= RR_MAXTICKS)) {
			/*look for other thread of this prio, with acceptable runmask*/
			thp = DISPATCH_THP(dpp, prio);
			while ( thp && (thp->runmask & (1<<KERNCPU)) ) {
				thp = thp->next.thread;
			}
			if (thp) {
				LINK3_REM(DISPATCH_LST(dpp, prio), thp, THREAD);
				/* no need to clear readybit as we're going to put act on this queue */
				LINK3_END(DISPATCH_LST(act->dpp, prio), act, THREAD);
				if(DISPATCH_THP(dpp, prio) == NULL) crash();
				/* This thread has exausted its timeslice */
				RR_RESET_TICK(act);
			}
		}
	} else {
		LINK3_BEG(DISPATCH_LST(act->dpp, prio), act, THREAD);
		DISPATCH_SET(act->dpp, act);
	}
	if(thp) {
		/* Someone else above has been selected to run */

		act->state = STATE_READY;
		trace_emit_th_state(act, STATE_READY);

		mark_running_ppg(thp);

        //If we give up the CPU, it is like blocking
		if(thp != act) {
			SS_STOP_RUNNING(act, 0);
			SS_MARK_ACTIVATION(thp);
		}
	} else {
		trace_emit_th_state(act, STATE_RUNNING);
		//VALIDATE_NEEDTORUN(0);
		need_to_run = 0;
	}
	chk_dispatch(1);
}

/*
 * Read a thread associated with a particular purpose group.
 * This may cause this thread to become active.
 */

static void rdecl
ready_ppg(THREAD *thp) {
	THREAD		*act;
	DISPATCH	*dpp;
	uint8_t		prio = thp->priority;
	int			was_sendvnc = 0;

	chk_lock();
	chk_dispatch(1);
	chk_thread(thp);

	timeout_stop(thp);

	/* QNX implements a number of the standard system calls as messages.  In this
	 * case, the non cancellable version of send is used, MsgSendvnc().
	 *
	 * For these calls we don't want to forfeit the rest of our timeslice, so we
	 * save this information away to help us decide whether to go to the end
	 * of the ready queue, or to stay at the front.
	 */
	if ( thp->state == STATE_REPLY && _TRACE_GETSYSCALL(thp->syscall) == __KER_MSG_SENDVNC ) {
		was_sendvnc = 1;
	}

	// Make sure the thread does not have a pending stop
	if((thp->flags & _NTO_TF_TO_BE_STOPPED) && !(thp->flags & _NTO_TF_KILLSELF)) {
		thp->state = STATE_STOPPED;
		trace_emit_th_state(thp, STATE_STOPPED);
		chk_dispatch(1);
		return;
	}

	thp->next.thread = NULL;
	thp->prev.thread = NULL;

	//Before we do anything with the actives, check to see if this is an
	//expired SS thread and if it is, then give a chance to schedule a
	//replenishment and drop the priority.  This code should only "do something"
	//when ready() is called after servicing requests generated by ISR's
	SS_CHECK_EXPIRY(actives[KERNCPU]);

	// If new thread is higher priority we make it active
	act = actives[KERNCPU];
	dpp = act->dpp;


	// Set the activation time, clear the consumed on ready or running
	SS_MARK_ACTIVATION(thp);

	/* If priority is higher and it has budget, run right away */
	// an approximation. AP schedulingg would allow us to preempt with a lower prio thread, but too much depends
	// only higher prio threads preempting
	// @@@ add inheritance of critical thread state
	/*
	 * If we're higher prio that the currently running thread, we have
	 * 4 cases to consider:
	 *  1. we're in a partition with budget so we should run
	 *  2. we're in a partition with no budget
	 *    2.1 if the current partition also has no budget, run (even
	 *           if current partition is running critical, we out-rank with prio)
	 *    2.2 if the current partition has budget, don't run
	 *  3. we are critical which means we should run
	 *  4. idle is currently running, therefore we should run
	 *    Note we microbill before checking budgets to make sure our view is current
	 *    Note that if the runmask does not match, we go down to the next path
	 */
	if((prio > act->priority) &&
			((thp->runmask & (1 << KERNCPU)) == 0) &&
			(MAY_SCHEDULE((PPG*)thp->dpp) ||
			(!(microbill(act), MAY_SCHEDULE((PPG*)act->dpp))) ||
			THREAD_IS_RUNNING_CRITICAL(thp) ||
			act->priority == 0)) {
		mark_running(thp);
		thp = act;

		//If new thread is higher prio, the old active is put at head
		//of it's ready queue (think fifo scheduling).
		act->state = STATE_READY;
		RR_ADD_PREEMPT_TICK(act);
		LINK3_BEG(DISPATCH_LST(dpp, act->priority), act, THREAD);
		SS_STOP_RUNNING(act, 1);
	} else {
		if(NUM_PROCESSORS > 1) {
			int		cpu;

			if(STATE_LAZY_RESCHED(thp) && (prio <= act->priority) && (thp->runmask & (1 << KERNCPU)) == 0) {
				thp->state = STATE_READY;
				dpp = thp->dpp;
				if ( was_sendvnc) {
					RR_ADD_PREEMPT_TICK(thp);
					LINK3_BEG(DISPATCH_LST(dpp, prio), thp, THREAD);
				} else {
					RR_RESET_TICK(thp);
					LINK3_END(DISPATCH_LST(dpp, prio), thp, THREAD);
				}
				trace_emit_th_state(thp, STATE_READY);
				if(thp->dpp != dpp) crash();
				DISPATCH_SET(dpp, thp);
				return;
			}
			if((cpu = select_cpu_ppg(thp)) != KERNCPU) {
				// Do we need to assign another cpu this thread.
				if(cpu != KERNCPU) {
					dpp = thp->dpp;
					thp->state = STATE_READY;
					trace_emit_th_state(thp, STATE_READY);
					if ( was_sendvnc ) {
						RR_ADD_PREEMPT_TICK(thp);
						LINK3_BEG(DISPATCH_LST(dpp, prio), thp, THREAD);
					} else  {
						RR_RESET_TICK(thp);
						LINK3_END(DISPATCH_LST(dpp, prio), thp, THREAD);
					}
					DISPATCH_SET(dpp, thp);

					//"need_to_run" stuff makes no sense for aps, (and little sense at any time). turn it off.
#if 0
					if (need_to_run == NULL && (thp->flags & _NTO_TF_SPECRET_MASK) == 0) {
						need_to_run_cpu = cpu;
						need_to_run = thp;
					}

#endif
					SENDIPI(cpu, IPI_RESCHED);
					return;
				}

				// The thread can replace the thread on this machine.
				mark_running(thp);

				thp = act;
				act->state = STATE_READY;
				if(thp->dpp != dpp) crash();
				RR_ADD_PREEMPT_TICK(act);
				LINK3_BEG(DISPATCH_LST(dpp, act->priority), act, THREAD);

				SS_STOP_RUNNING(act, 1);
			} else {
				dpp = thp->dpp;
				thp->state = STATE_READY;
				//If new thread is lower or equal prio, it goes at end of it's
				//ready queue, unless it was blocked due to a MsgSendvnc call (see
				// the above description of this issue.
				if ( was_sendvnc ) {
					RR_ADD_PREEMPT_TICK(thp);
					LINK3_BEG(DISPATCH_LST(dpp, prio), thp, THREAD);
				} else  {
					RR_RESET_TICK(thp);
					LINK3_END(DISPATCH_LST(dpp, prio), thp, THREAD);
				}
			}
		} else {
			thp->state = STATE_READY;
			//If new thread is lower or equal prio, it goes at end of it's
			//ready queue, unless it was blocked due to a MsgSendvnc call (see
			// the above description of this issue.
			dpp = thp->dpp;
			if ( was_sendvnc ) {
				RR_ADD_PREEMPT_TICK(thp);
				LINK3_BEG(DISPATCH_LST(dpp, prio), thp, THREAD);
			} else  {
				RR_RESET_TICK(thp);
				LINK3_END(DISPATCH_LST(dpp, prio), thp, THREAD);
			}
		}
	}

	trace_emit_th_state(thp, STATE_READY);
	if(thp->dpp != dpp) crash();
	DISPATCH_SET(dpp, thp);
	chk_dispatch(1);
}



//
//	Block the active thread and ready the specified thread.
//	If the new thread is of higher priority than active, then
//	it can be made active immediately. If not, we must scan
//	the dispatch looking for the highest priority ready
//	thread.
//

static void rdecl
block_and_ready_ppg(THREAD *thp) {
	THREAD		*act = actives[KERNCPU];
	DISPATCH	*dpp;
	uint8_t		prio;

	chk_lock();
	chk_dispatch(0);
	chk_thread(thp);

	// Check for timeout timers
	if(act->timeout_flags & _NTO_TIMEOUT_MASK) {
		timeout_start(act);
	}

	timeout_stop(thp);

	thp->next.thread = NULL;
	thp->prev.thread = NULL;

	thp->restart = NULL;

	// If thp has a pending stop then degrade block_and_ready() to just block()
	if((thp->flags & _NTO_TF_TO_BE_STOPPED) && !(thp->flags & _NTO_TF_KILLSELF)) {
		thp->state = STATE_STOPPED;
		trace_emit_th_state(thp, STATE_STOPPED);
		block();
		chk_dispatch(1);
		return;
	}
	dpp = thp->dpp;
	prio = thp->priority;

	//We are blocking the active thread (externally) so stop the running
	//Mark the activation time for this new thread
	SS_STOP_RUNNING(act, ((act->state == STATE_READY) || (act->state == STATE_RUNNING)));
	SS_MARK_ACTIVATION(thp);

	// Will thp be the highest priority thread ready to run?
	// If thp is higher prio than act and has budget, run immediately
	// @@@ add inheritance of critial thread state.
	/*
	 * If we're higher prio that the currently running thread, we can
	 * take a shortcut in a few cases (avoid running the whole sched algorithm)
	 *  1. we're in the same partition which means we should run
	 *  2. we're in another partition with budget so we should run
	 *  3. we are critical which means we should run
	 */
	if((prio > act->priority) &&
			((thp->runmask & (1 << KERNCPU)) == 0) &&
			(MAY_SCHEDULE((PPG*)thp->dpp) ||
			THREAD_IS_RUNNING_CRITICAL(thp))) {
		mark_running(thp);
		chk_dispatch(1);
		return;
	}

	// Add thp to ready queue.
	if(thp->policy == SCHED_FIFO) {
		// FIFO threads must be queued at the end of the list to ensure
		// we don't cause this thread to run before any other FIFO threads
		// that are runnable at this priority
		LINK3_END(DISPATCH_LST(dpp, prio), thp, THREAD);
	} else {
		LINK3_BEG(DISPATCH_LST(dpp, prio), thp, THREAD);
	}
	DISPATCH_SET(dpp, thp);
	thp->state = STATE_READY;
	trace_emit_th_state(thp, STATE_READY);


	act = select_thread(NULL, KERNCPU, -1);// FIND_HIGHEST(act);
	if(act->runmask & (1 << KERNCPU)) crash();

	mark_running(act);

	chk_dispatch(1);
}
/*
 * Adjust the priority of a thread to the new value of prio
 * This may involve changing the active thread.
 *
 * The priority_inherit parameter is used to indicate whether this adjustment is the
 * result of a priority inheritance protocol.  If so, then when dropping priority we
 * consider it a preemptation and place the thread at the begining of the ready
 * queue.  Otherwise it goes on the end of the queue and gets it's round robin
 * information reset.
 */

static void rdecl
adjust_priority_ppg(THREAD *thp, int prio, DISPATCH *newdpp, int priority_inherit) {
	THREAD		*act = actives[KERNCPU];
	DISPATCH	*dpp;

	chk_lock();
	chk_thread(thp);

	dpp = thp->dpp;

	if(thp->priority == prio && newdpp==dpp) {
		if(thp == act) {
			yield();
		}
		return;
	}

	switch(thp->state) {
	case STATE_READY:
		chk_cond(thp != act);
		chk_thread(thp);

		// Unlink from the ready queue
		LINK3_REM(DISPATCH_LST(dpp, thp->priority), thp, THREAD);
		if(!DISPATCH_THP(dpp, thp->priority)) {
			DISPATCH_CLR(dpp, thp);
		}

		thp->priority = prio;
		thp->dpp = dpp = newdpp;
		/*
		 * If we're higher prio that the currently running thread, we have
		 * 4 cases to consider:
		 *  1. we're in a partition with budget so we should run
		 *  2. we're in a partition with no budget
		 *    2.1 if the current partition also has no budget (free time), run
		 *    2.2 if the current partition has budget, don't run
		 *  3. we are critical which means we should run
		 *  4. idle is currently running, therefore we should run
		 */
		if(NUM_PROCESSORS > 1) {
			// Let ready() do the hard work.
			ready(thp);
		} else if((thp->priority > act->priority) &&
				(MAY_SCHEDULE((PPG*)thp->dpp) ||
				(!(microbill(act),MAY_SCHEDULE((PPG*)act->dpp))) ||
				THREAD_IS_RUNNING_CRITICAL(thp))) {
			act->state = STATE_READY;
			trace_emit_th_state(act, STATE_READY);
			RR_ADD_PREEMPT_TICK(act);
			mark_running(thp);
			thp = act;
			dpp = thp->dpp;
			//This guy should go to the head of the priority list, not the tail
			LINK3_BEG(DISPATCH_LST(dpp, thp->priority), thp, THREAD);
			DISPATCH_SET(dpp, thp);

			SS_STOP_RUNNING(act, 1);
		} else {
			// Put lower of (act, thp) to the end of the ready queue for its priority.
			RR_RESET_TICK(thp);
			LINK3_END(DISPATCH_LST(dpp, thp->priority), thp, THREAD);
			DISPATCH_SET(dpp, thp);
		}

		chk_dispatch(1);
		break;

	case STATE_RUNNING:
		chk_thread(thp);

		/* Check for highest prio thread -- if we are raising prio,
		 * then no problem as we were already highest with budget.
		 */
		if (act != thp) {
			// Must be running on another CPU - SMP
			if(thp->runcpu == KERNCPU) crash();
			thp->priority = prio;
			SENDIPI(thp->runcpu, IPI_RESCHED);
			break;
		} else {
			int	run_as_critical;
			THREAD	*chosen;
			act->priority = prio;
			act->dpp = newdpp;
			chosen = choose_thread_to_schedule(act,&run_as_critical);
			if(chosen && (chosen != act)) {
				THREAD *oldact = act;
				// Put active into ready queue
				dpp = newdpp;
				act->state = STATE_READY;
				trace_emit_th_state(act, STATE_READY);
				if( priority_inherit ) {
					RR_ADD_PREEMPT_TICK(act);
					LINK3_BEG(DISPATCH_LST(dpp, act->priority), act, THREAD);
				} else {
					RR_RESET_TICK(act);
					LINK3_END(DISPATCH_LST(dpp, act->priority), act, THREAD);
				}

				DISPATCH_SET(dpp, act);
				//We've already microbilled the old act in choose_thread..().

				act = select_chosen(0,0,0, chosen, run_as_critical);
				if(act == NULL) crash();


				mark_running(act);

				SS_STOP_RUNNING(oldact, 1);
			} else {
				act->priority = prio;
				trace_emit_th_state(act, act->state);
			}
  		}
		chk_dispatch(1);
		break;

	case STATE_NET_SEND:
	case STATE_SEND: {
		CONNECT *cop = thp->blocked_on;

		if(cop != NULL) {
			// Reposition the item on the send queue to match new priority
			pril_rem(&cop->channel->send_queue, thp);
			thp->priority = prio;
			pril_add(&cop->channel->send_queue, thp);
		} else {
			thp->priority = prio;
		}
		break;
	}

	case STATE_CONDVAR:
	case STATE_SEM: {
		SYNC		*syp = thp->blocked_on;

		if(syp != NULL) {
			// Reposition the item on the waiting list to match new priority
			pril_rem(&syp->waiting, thp);
			thp->priority = prio;
			pril_add(&syp->waiting, thp);
		} else {
			thp->priority = prio;
		}
		break;
	}

	case STATE_MUTEX: {
		SYNC		*syp = thp->blocked_on;
		THREAD		*first;
		PROCESS		*prp;
		int			owner;
		unsigned	id;

		if(syp != NULL) {
			first = pril_first(&syp->waiting);

			owner = first->args.mu.owner;
			if(owner != 0) {
				first->args.mu.owner = 0;
				mutex_holdlist_rem(syp);
			}

			// Reposition the item on the waiting list to match new priority
			pril_rem(&syp->waiting, thp);
			thp->priority = prio;
			pril_add(&syp->waiting, thp);

			// Put back on mutex hold list in proper place
			if(owner != 0) {
				id = SYNC_PINDEX(owner);
				if(id && id < process_vector.nentries && VECP(prp, &process_vector, id)) {
					thp = vector_lookup(&prp->threads, SYNC_TID(owner));
					if(thp != NULL) {
						mutex_holdlist_add(thp, syp);
						first = pril_first(&syp->waiting);
						first->args.mu.owner = owner;
					}
				}
			}
		} else {
			thp->priority = prio;
		}
		break;
	}

	default:
		thp->priority = prio;
	}
	chk_dispatch(1);
}

static int rdecl
may_thread_run_ppg(THREAD *thp) {
	return MAY_SCHEDULE((PPG*)thp->dpp);
}

static void rdecl
debug_moduleinfo_ppg(PROCESS *prp, THREAD *thp, void *dbg) {

	if(thp == NULL) {		// PROCESS-level information
	struct extsched_aps_dbg_process	*aps = dbg;

		aps->id = prp->default_dpp->id;
		memset(aps->reserved, 0, sizeof(aps->reserved));
	} else {				// THREAD-level information
	struct extsched_aps_dbg_thread	*aps = dbg;

		aps->id = thp->dpp->id;
		memset(aps->reserved, 0, sizeof(aps->reserved));
	}
}

static DISPATCH *init_optional_scheduler() {
	DISPATCH 	*dpp;


	INIT_BANKRUPTCY_INFO(&last_bankrupter);
	INIT_BANKRUPTCY_INFO(&last_logged_bankrupter);
	last_bankrupting_ap =-1;
	set_bmp_support_factor();
	if (EOK != reinit_window_parms(DEFAULT_WINDOWSIZE_MS)) crash();

	dpp = create_default_dispatch();

	init_microbill();

	resched = resched_ppg;
	yield = yield_ppg;
	mark_running = mark_running_ppg;
	ready = ready_ppg;
	adjust_priority = adjust_priority_ppg;
	block_and_ready = block_and_ready_ppg;
	select_thread = select_thread_ppg;
	may_thread_run = may_thread_run_ppg;
	sched_trace_initial_parms = sched_trace_initial_parms_ppg;
	debug_moduleinfo = debug_moduleinfo_ppg;
	scheduler_tick_hook = ppg_tick_hook;


	(void)kercall_attach(__KER_SCHED_CTL, (int kdecl (*)())ker_sched_ctl);
	//declare this to be the scheduler
	scheduler_type = SCHEDULER_TYPE_APS;

	{
		sched_aps_partition_info  info;
		int id = aps_name_to_id(APS_SYSTEM_PARTITION_NAME);
		CRASHCHECK(id < 0);	// if someone changes the init order, this will go off
		info.id = id;
		if (aps_get_partition_info(&info) != EOK) crash();
		CRASHCHECK(id != info.id);
		SCHEDPART_INIT(id, &info);
	}

	return dpp;
}



/* This intialization only gets called if the [modules=aps] option has been used on the procnto line of
 * your buildfile. This function make us the scheduler by overbinding nano_sched.c's function pointers for
 * ready(), block() etc.
 *
 * The name "initialize_aps" must match the declaration in init_aps.S
 *
 * NOTE - this function is called several times, each with a different pass number
 *
 * You should take special care to only do the appropriate setup for each pass
 */
void
initialize_aps(unsigned version, unsigned pass) {
	if(version != LIBMOD_VERSION_CHECK) {
		kprintf("Version mismatch between procnto (%d) and libmod_aps (%d)\n", version, LIBMOD_VERSION_CHECK);
		crash();
	}
	switch(pass) {
	case 0:
	{
		extern void _schedpart_init(part_id_t, sched_aps_partition_info *);
		init_scheduler = init_optional_scheduler;
		schedpart_init = _schedpart_init;
		schedpart_select_dpp = aps_select_ppg;
		break;
	}
	default:
		break;
	}
}

#ifdef DEBUG_APS
/*
 * For now, we run the scheduler check at various places in the APS scheduler
 */


static void
_chk_dispatch(int chkstate, int line) {
	int i;
	THREAD *thp;
	THREAD **owner;
	DISPATCH *dpp;
	THREAD	*act;
	int	prio;

	if(ker_verbose < 3) {
		return;
	}

	act = actives[KERNCPU];
	if(act->process == NULL) crash();
	dpp = act->dpp;
	if(dpp == NULL) crash();
	if(chkstate  &&  act->state != STATE_RUNNING) {
		kprintf("State not running? line %d\n", line);
		kprintf("ret %x", __builtin_return_address(0));
		kprintf(" %x", __builtin_return_address(1));
		kprintf(" %x", __builtin_return_address(2));
		kprintf(" %x\n", __builtin_return_address(3));
		//crash();
	}
	if(!DISPATCH_ISSET(dpp, 0) || (DISPATCH_THP(dpp, 0) == NULL)) {
#if 0
		i = 0;
		for( ;; ) {
			if(i >= NUM_PROCESSORS) {
				// If all the idles are initialized (have set prio==0),
				// and the active thread isn't an idle, we should have
				// something on the prio==0 ready list.
				if(act->priority == 0) break;
				crash();
			}
			thp = procnto_prp->threads.vector[i];
			if(thp->priority != 0) break;
			++i;
		}
#endif
	}
	for(i = 0 ; i < NUM_PRI ; ++i) {
		if(DISPATCH_ISSET(dpp, i)) {
			owner = (THREAD **)&DISPATCH_LST(dpp, i).head;
			if(*owner == NULL) {
				crash();
			}

			for( ;; ) {
				thp = *owner;
				if(thp == NULL) break;
				if(!thp->process) {
					crash();
				}
				if(thp->priority != i) {
					crash();
				}
				if(thp->state != STATE_READY && (chkstate || thp != act)) {
					crash();
				}
				if(TYPE_MASK(thp->type) != TYPE_THREAD) {
					crash();
				}
				if(thp->prev != owner) {
					crash();
				}
				owner = &thp->next;
			}
		} else {
			if(DISPATCH_THP(dpp, i) != NULL) {
				crash();
			}
		}
	}
#if 0
	//*** this does not compile ***/
	thp = choose_thread_to_schedule(NULL,NULL);
	if(thp != NULL) {
		prio = thp->priority;
		if(prio > act->priority) {
			// This should happen only if the ready thread
			// is in a partition with zero budget left
			if(((PPG *)thp->dpp)->remaining_cycles > 0) {
				kprintf("inversion? prio %d %d cyc %d line %d\n",
				prio, act->priority,
				((PPG *)thp->dpp)->remaining_cycles, line);
				kprintf("ret %x", __builtin_return_address(0));
				kprintf(" %x", __builtin_return_address(1));
				kprintf(" %x", __builtin_return_address(2));
				kprintf(" %x\n", __builtin_return_address(3));
			}
		}
	}
#endif
}

#endif

