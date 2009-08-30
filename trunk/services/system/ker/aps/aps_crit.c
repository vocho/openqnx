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
 * aps_crit.c 
 * 
 * Adaptive Partitioning scheduling 
 *
 * globals an implementation of functions for critical time and bankruptcy
 * 
 */
#include "aps_data.h"
#include "proto_aps.h"
#include <sys/siginfo.h>


int	max_bankruptcy_grace;		/* in ticks, is 2 windowsizes */ 
int	remaining_bankruptcy_grace; 	/* in ticks, for delaying the delcaration of critical time overrun,
					i.e. bankruptcy, after the change of budget parms 
					*/ 

uint32_t	bankruptcy_policy = SCHED_APS_BNKR_BASIC;


bankruptcy_info last_bankrupter;
bankruptcy_info last_logged_bankrupter; 
int		last_bankrupting_ap;

/* handle_bankruptcy executes the user specified policy in response to a partition going bankrupt. The 
 * bankruptcy detection is in aps_alg.c: pgg_tick_hook() 
 */
void handle_bankruptcy( THREAD *act, PPG *ppg) { 
	trace_emit_sys_aps_bankruptcy(ppg->dpp.id, act->process->pid, act->tid);
	ppg->state_flags |= PPG_STATE_WAS_BANKRUPT; 
	//Send notification, if requested
	if (ppg->bankruptcy_notifier.sigev_notify!=SIGEV_NONE) {
		PROCESS *prp; 
		THREAD *notify_thp = NULL;
		prp=lookup_pid(ppg->notify_pid);
		if (prp) { 
			if (ppg->notify_tid==-1) {
				//for future use when someday user may bind explict pid/tid for SCHED_APS_ATTACH_EVENTS:
				notify_thp = prp->valid_thp;
			} else { 
				notify_thp = vector_lookup(&(prp->threads), ppg->notify_tid);  
			}
			if (notify_thp) {
				intrevent_add(&ppg->bankruptcy_notifier, notify_thp, clock_isr); 
			}
		}
		//make the user re-register for next notitication -- so notifications are throttled 
		SIGEV_NONE_INIT(&ppg->bankruptcy_notifier);
	}
		

	//also do user-settable bankruptcy actions
	if (bankruptcy_policy & SCHED_APS_BNKR_LOG) { 
		//throttle logs to prevent tight logging loop. by logging only when the bankrupting process changes.
		if (last_logged_bankrupter.last_suspect_pid != act->process->pid) {
#if 0
			//@@@ APS replace this with a pulse to a logging process 
			kprintf("APsched: partition %d bankrupt running pid %d: %s\n", act->dpp->id,act->process->pid, act->process->debug_name);
#endif
			SET_BANKRUPTCY_INFO(&last_logged_bankrupter,act);
		}
	}
	if (bankruptcy_policy & SCHED_APS_BNKR_CANCEL_BUDGET) { 
		ppg->critical_budget_in_cycles = 0;
		trace_emit_sys_aps_budgets(ppg->dpp.id, ppg->budget_in_percent, 0); 

		remaining_bankruptcy_grace = max_bankruptcy_grace; 

	}
	if (bankruptcy_policy & SCHED_APS_BNKR_REBOOT) { 
		aps_bankruptcy_crash(act->dpp->id, act->process->pid, act->process->debug_name);
		//never returns 
	}

			
	SET_BANKRUPTCY_INFO(&last_bankrupter,act);
	SET_BANKRUPTCY_INFO(&ppg->bnkr_info,act);
	last_bankrupting_ap =act->dpp->id;
}


__SRCVERSION("aps_crit.c $Rev: 153052 $"); 

