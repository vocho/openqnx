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
 * aps_crit.h 
 * 
 * Adaptive Partitioning scheduling 
 *  
 * definitions for the notions of critical threads, critical time, and bankruptcy 
 * 
 */
#include "aps_data.h"



extern	int	max_bankruptcy_grace;		/* in ticks, is 2 windowsizes */ 
extern	int	remaining_bankruptcy_grace; 	/* in ticks, for delaying the delcaration of critical time overrun,
										i.e. bankruptcy, after the change of budget parms 
										*/ 
extern	uint32_t	bankruptcy_policy; // default = SCHED_APS_BNKR_BASIC;


typedef struct {
	pid_t	last_suspect_pid;
	int	last_suspect_tid;
} bankruptcy_info;

#define INIT_BANKRUPTCY_INFO(bip) (bip)->last_suspect_pid=-1; (bip)->last_suspect_tid=-1;
#define SET_BANKRUPTCY_INFO(bip,thp) (bip)->last_suspect_pid = (thp)->process->pid; (bip)->last_suspect_tid=(thp)->tid; 

#define IS_BANKRUPT(ppg)  ( !MAY_SCHEDULE_CRITICAL(ppg) && (0!=(ppg)->critical_budget_in_cycles) && !remaining_bankruptcy_grace && (num_ppg>1)) 
/* in IS_BANKRUPT we apear to checking critical_budget twice. Actually the second check is to handle the case of 
 * the critical budget being rest to zero when there are still critical cycles used in the history window*/

#define THREAD_IS_RUNNING_CRITICAL(thp) ( ((thp)->sched_flags & AP_SCHED_RUNNING_CRIT)\
	&& MAY_SCHEDULE_CRITICAL((PPG*)(thp)->dpp))

#define UPD_BILL_AS_CRIT(thp,bill_as_crit) (thp)->sched_flags = (bill_as_crit) ?\
	(thp)->sched_flags|AP_SCHED_BILL_AS_CRIT : (thp)->sched_flags&~AP_SCHED_BILL_AS_CRIT;


extern bankruptcy_info last_bankrupter;
extern bankruptcy_info last_logged_bankrupter; 


extern int	last_bankrupting_ap;



__SRCVERSION("aps_crit.h $Rev: 153052 $"); 

