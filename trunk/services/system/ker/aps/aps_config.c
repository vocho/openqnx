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
 * aps_config.c 
 * 
 * Adaptive Partitioning scheduling 
 *
 * Implemmtation of aps_config.h, function to configure aps, in particular implement SchedCtl() 
 * Mostly used by ker_aps.c 
 *
 */
#include "externs.h"
#include "aps_data.h"
#include "aps_alg.h"
#include "proto_aps.h"
#include <string.h>


/* ------------------------
 * configure_bmp_support() 
 *
 * sets various budget and control globals for current choice of BMP options. 
 * Returns EINTR if reinit_window_parms is interrupted by 3 clock ticks (very unlikely)
 */ 
int configure_bmp_support() { 
	/*first set bmp_support_factor variable used to set budgets on smp machines */ 
	set_bmp_support_factor(); 
	/*to meet budgets properly with bmp_support, we need to be in ratio mode */ 
	if (scheduling_policy_flags & SCHED_APS_SCHEDPOL_BMP_SAFETY) scheduling_policy_flags |= SCHED_APS_SCHEDPOL_FREETIME_BY_RATIO;
	/* since this is effectively a window size change we need to reinit the windowing parms */
	return reinit_window_parms(windowsize_in_ms); 
}

/* -----------------------
 * update_zerobudget_ap_set() 
 *
 * updates the bitmap of zero partition. Call when you've changed a budget
 */

void update_zerobudget_ap_set(void){ 
	int i;
	uint32_t zeroap_set; 
	/* first update a local copy, so we can set all bits in the global version atomically */ 
	zeroap_set = 0; 
	for (i=0;i<num_ppg;i++) zeroap_set |= (uint32_t)(0==actives_ppg[i]->budget_in_percent) << i;  
	zerobudget_ap_set = zeroap_set; 
} 

/*  ---------------------
 *  change_percent_budget()
 *  
 *  adjust percentage budget by returning excess to parent or taking from parent.
 *  returns -1 if parent doesnt have enough to satisfy request
 *  should not be used to change system partition
 *
 *  ***! note: call set_factors() after changing any paritions budget. need be done only once per kernel call !***
 *  
 */

int change_percent_budget(PPG *ppg, int new_budget_percent) { 

	if (ppg->parent) { 
		int			percent_delta;
		percent_delta = new_budget_percent - ppg->budget_in_percent; 
		if (percent_delta > ppg->parent->budget_in_percent) return -1;
		ppg->parent->budget_in_percent -= percent_delta;
		ppg->parent->bmp_budget_in_cycles = (bmp_support_factor * windowsize_in_cycles * ppg->parent->budget_in_percent) / 100; 
		ppg->parent->std_budget_in_cycles = (NUM_PROCESSORS * windowsize_in_cycles * ppg->parent->budget_in_percent) / 100; 
		// no need to change ppg->parent->used_cycles;
	} 		
	ppg->budget_in_percent = new_budget_percent; 
	ppg->bmp_budget_in_cycles = (bmp_support_factor * windowsize_in_cycles * ppg->budget_in_percent) / 100; 
	ppg->std_budget_in_cycles = (NUM_PROCESSORS * windowsize_in_cycles * ppg->budget_in_percent) / 100; 
	// no need to change ppg->used_cycles
	update_zerobudget_ap_set(); 
	return 0;
}


/* assign_partition_name 
 * creates a partition name in the range "Pa" to "Pz" for when the user has not set a name. 
 */
#define ASSIGNED_PNAME_LENGTH 2
void assign_partition_name(char *buffer) { 
	//buffer must be >=  ASSIGNED_PNAME_LENGTH+1 long 
	//this code assumes there are less than 27 partition. a safe bet. 
	int i; 
	buffer[0]='P'; 
	for(i=0;i<APS_MAX_PARTITIONS;i++) { 
		buffer[1]=i+'a'; 
		buffer[2]='\0'; 
		if (-1==aps_name_to_id(buffer) ) return; 		
	}
	crash(); 
}
	


/* ------------
 * create_ppg() 
 * 
 * create a new partition
 * returns partition id created or a -errno 
 * Parameters:
 *  name - may be NULL, caller must check if unique
 *  budget in % points of CPU
 *  parent_id - partition will be created from parent_id or the calling thread if
 * 				parent_id == -1 or the APS_CREATE_FLAGS_USE_PARENT_ID flag is not set
 */

int create_ppg(sched_aps_create_parms *parms, int max_prio) {
	PPG	*parent, *new;
	long_reporting_window *lrw;
	char *name = parms->name;
	uint8_t aps_create_flags = parms->aps_create_flags;
	int8_t parent_id = parms->parent_id;
	unsigned budget = parms->budget_percent;
	unsigned critical_budget_ms = parms->critical_budget_ms==-1 ? 0 : parms->critical_budget_ms;

	/* Check if we still have purpose group slots */
	if(num_ppg >= APS_MAX_PARTITIONS) return -ENOSPC;
	
	if (!(aps_create_flags & APS_CREATE_FLAGS_USE_PARENT_ID) || (parent_id < 0)) {
		parent = (PPG *)actives[KERNCPU]->orig_dpp;
	} else {
		parent = actives_ppg[parent_id];
	}

	/* Check if we have budget we can give */
	if(parent->budget_in_percent < budget) return -EDQUOT;

	/* check if this create will result in the parent becomming zero budget */
	if (aps_security & SCHED_APS_SEC_NONZERO_BUDGETS && budget==parent->budget_in_percent) return -EACCES;

	
	new = _scalloc(sizeof (PPG));
	if (new) { 
		if (!(lrw = _scalloc(sizeof(long_reporting_window)))) { 
			free(new);
			return -ENOMEM;
		} else { 
			//both scallocs ok.
		}
	} else { 
		return -ENOMEM;
	}
	
	remaining_bankruptcy_grace = max_bankruptcy_grace; 
	
	/* Initialize partition parameters */
	new->dpp.id = num_ppg;
	actives_ppg[num_ppg] = new;
	actives_lrw[new->dpp.id] = lrw;
	zero_lrw(new->dpp.id);
	new->parent = parent; 
	if (!name || 0==strcmp(name,"")) { 
		CRASHCHECK(sizeof(new->name) < (ASSIGNED_PNAME_LENGTH+1));
		assign_partition_name(new->name);
	} else {
		STRLCPY(new->name, name, sizeof(new->name)); 
	}
	new->high_prio = max_prio;

	//give partition budget by taking from parent
	new->budget_in_percent=0; new->used_cycles =0; 
	(void) change_percent_budget(new, budget); 
				
	new->critical_cycles_used = 0; 
	new->critical_budget_in_cycles = ((uint64_t)critical_budget_ms) * cycles_per_ms; 
	if (new->critical_budget_in_cycles > NUM_PROCESSORS * windowsize_in_cycles) new->critical_budget_in_cycles = NUM_PROCESSORS * windowsize_in_cycles;
	SIGEV_NONE_INIT(&new->bankruptcy_notifier);
	SIGEV_NONE_INIT(&new->overload_notifier);
	new->notify_pid = new->notify_tid = -1;
	new->state_flags =0;
	INIT_BANKRUPTCY_INFO(&new->bnkr_info); 
	
	num_ppg++;
	set_factors(); 
	
	trace_emit_sys_aps_name(new->dpp.id, new->name);
	trace_emit_sys_aps_budgets(new->dpp.id, new->budget_in_percent, new->critical_budget_in_cycles/cycles_per_ms); 
	return new->dpp.id;
}


/* ----------------
 * aps_name_to_id()
 * 
 * Linear search of partition for matching name. Returns partition number or -1
 */
int aps_name_to_id(char *name) {
	
	int		ap; 
	/* null names are never found */ 
	if (name && *name) { 
		for (ap=0;ap <num_ppg;ap++) if (0==strcmp(actives_ppg[ap]->name, name) ) return ap; 
	}
	return -1;
		
}   

/*  ---------------------------
 *  implements SCHED_APS_QUERY_PARMS of SchedCtl 
 */
void aps_get_info(sched_aps_info *sched_info) {  
	sched_info->num_partitions = num_ppg;
	sched_info->max_partitions = APS_MAX_PARTITIONS;
	sched_info->windowsize_cycles = windowsize_in_cycles; 
	sched_info->windowsize2_cycles = windowsize_in_cycles * MAX_W2_BUCKETS; 
	sched_info->windowsize3_cycles = windowsize_in_cycles * MAX_W2_BUCKETS * MAX_W3_BUCKETS; 
	sched_info->cycles_per_ms = cycles_per_ms; 
	sched_info->bankruptcy_policy = bankruptcy_policy; 
	sched_info->scheduling_policy_flags = scheduling_policy_flags;
}


/*  -------------
 *  implements SCHED_APS_SET_PARMS option of SchedCtl
 */
int aps_set_parms(sched_aps_parms *parms) { 
	int rc=EOK; 
	if (parms->windowsize_ms !=-1) { 
		rc = reinit_window_parms(parms->windowsize_ms); 
	}
	if (EOK!=rc) return rc; 
	
	if (parms -> bankruptcy_policyp) { 
		bankruptcy_policy = *(parms->bankruptcy_policyp);
		//if SCHED_APS_BNKR_LOG, reset last bankrupting process so we get a new log 
		if (bankruptcy_policy & SCHED_APS_BNKR_LOG)  INIT_BANKRUPTCY_INFO(&last_logged_bankrupter);  
		rc = EOK;
	}
	
	if (parms -> scheduling_policy_flagsp) { 
		uint32_t old_spf = scheduling_policy_flags; 
		scheduling_policy_flags = *(parms->scheduling_policy_flagsp); 
		if ((SCHED_APS_SCHEDPOL_BMP_SAFETY & old_spf) != (SCHED_APS_SCHEDPOL_BMP_SAFETY & scheduling_policy_flags)) { 
			rc = configure_bmp_support();
		}; 
		
	} 
	return rc;
}

/*   --------------------
 *   implements SCHED_APS_QUERY_PARTITION option of SchedCtl
 */
int aps_get_partition_info( sched_aps_partition_info *part_info) { 
	PPG *ppg; 

	if (part_info->id < 0 || part_info->id >=num_ppg) return EINVAL;
	ppg = actives_ppg[part_info->id]; 
	STRLCPY(part_info->name, ppg->name, sizeof(part_info->name)); 
	part_info->parent_id = ppg->parent ? ppg->parent->dpp.id : 0;  
	part_info->budget_percent = ppg->budget_in_percent; 
	part_info->budget_cycles = ppg->std_budget_in_cycles / (int)NUM_PROCESSORS;
	part_info->critical_budget_cycles = ppg->critical_budget_in_cycles; 
	part_info->notify_tid = ppg->notify_tid==-1 ? -1 : ppg->notify_tid+1; //convert to external tid 
	part_info->notify_pid = ppg->notify_pid;
	part_info->pinfo_flags = 0; 
	if (ppg->bankruptcy_notifier.sigev_notify!=SIGEV_NONE) part_info->pinfo_flags |= SCHED_APS_PINFO_BANKRUPTCY_NOTIFY_ARMED;
	if (ppg->overload_notifier.sigev_notify!=SIGEV_NONE) part_info->pinfo_flags |= SCHED_APS_PINFO_OVERLOAD_NOTIFY_ARMED;
	part_info->pid_at_last_bankruptcy = ppg->bnkr_info.last_suspect_pid;
	//convert tid to external range
	part_info->tid_at_last_bankruptcy = ppg->bnkr_info.last_suspect_tid ==-1? -1 : ppg->bnkr_info.last_suspect_tid+1; 
	return EOK; 
}

/*   --------------------
 *   finds paritiion id parent of given partition, or -1 if the id is invalid
 *   Returns system_partition if asking for parent of the system partition
 */
int	aps_get_parent(int id) { 
	DISPATCH *dpp; 
	if (id >=0 && id<num_ppg) { 
		dpp = (DISPATCH*)(actives_ppg)[id]->parent;
		if (dpp) {
			return dpp->id;
		} else {
			return APS_SYSTEM_PARTITION_ID;
		}
	} else { 
		return -1;
	}
}

/*  -------------------
 *  implements SCHED_APS_MODIFY_PARTITION
 *
 *  assumes caller has checked that percentages are between 0 and 100; 
 */
int aps_modify_partition(sched_aps_modify_parms *parms) {
	PPG *ppg; 
	uint32_t new_crit_cycles; 

	if (parms->id < 0 || parms->id >=num_ppg) return EINVAL;
	ppg = actives_ppg[parms->id]; 
    // may not modify the system ppg 
	if (!ppg->parent) return EINVAL; 

	if (aps_security & SCHED_APS_SEC_NONZERO_BUDGETS && 
		ppg->parent->budget_in_percent == parms->new_budget_percent - ppg->budget_in_percent) { 
			//would reduce parent's budget to zero. 
			return EACCES;
	}


	//adjust perecentage budget by returning excess to parent or taking from parent 
	if (parms->new_budget_percent !=-1) { 
		if (-1==change_percent_budget(ppg, parms->new_budget_percent)) return EINVAL;
		set_factors();
	}
	
	if (parms->new_critical_budget_ms!=-1){ 
		new_crit_cycles = (uint32_t)parms->new_critical_budget_ms * cycles_per_ms;
		ppg->critical_budget_in_cycles = (new_crit_cycles <= NUM_PROCESSORS * windowsize_in_cycles) ?
			new_crit_cycles : NUM_PROCESSORS * windowsize_in_cycles;
		remaining_bankruptcy_grace = max_bankruptcy_grace; 
		//@@@ APS should also delay any notification of that partition being overloaded.
	}
	trace_emit_sys_aps_budgets(ppg->dpp.id, ppg->budget_in_percent, ppg->critical_budget_in_cycles/cycles_per_ms); 
	trace_emit_sys_aps_budgets(ppg->parent->dpp.id, ppg->parent->budget_in_percent, 
					ppg->parent->critical_budget_in_cycles/cycles_per_ms); 

	return EOK;
}

/* -----------------------
	implements SCHED_APS_PARTITION_STATS option of SchedCtl
 */

int aps_get_partition_stats(sched_aps_partition_stats *part_stats, int n_aps_to_read) {
	PPG *ppg; 
	long_reporting_window *lrw;
	int i, retry_counter; 
	int start_ap, last_ap_plus1; 
	uint32_t start_window_rotations; 
	
	if (part_stats->id < 0 || part_stats->id >=num_ppg) return EINVAL;
	start_ap = part_stats[0].id; 	
	last_ap_plus1 = start_ap + n_aps_to_read;
	if (last_ap_plus1 > num_ppg) last_ap_plus1 = num_ppg;

	for(retry_counter=0; retry_counter<=1; retry_counter++){ 
		start_window_rotations = sched_window_rotations; 
		for(i=start_ap; i < last_ap_plus1; i++) { 
			sched_aps_partition_stats *p_stats;
			ppg = actives_ppg[i];
			lrw = actives_lrw[i];
			p_stats = &part_stats[i - start_ap]; 
			p_stats->run_time_cycles = ppg->used_cycles_lasttick / (unsigned int)NUM_PROCESSORS; 
			p_stats->critical_time_cycles = ppg->critical_cycles_used;
			p_stats->run_time_cycles_w2 = lrw->usage_w2/(unsigned int)NUM_PROCESSORS;
			p_stats->critical_time_cycles_w2 = lrw->critical_usage_w2;
			p_stats->run_time_cycles_w3 = lrw->usage_w3/(unsigned int)NUM_PROCESSORS;
			p_stats->critical_time_cycles_w3 = lrw->critical_usage_w3;
			p_stats->stats_flags =0; 
			if (IS_BANKRUPT(ppg)) p_stats->stats_flags |= SCHED_APS_PSTATS_IS_BANKRUPT_NOW; //this doent yet check if notification sent 
			if (ppg->state_flags & PPG_STATE_WAS_BANKRUPT) p_stats->stats_flags |= SCHED_APS_PSTATS_WAS_BANKRUPT; 
			p_stats->id = i; 
		}
		//If were were hit by a tick, and the windows rotated, while we were loading *stats, start again

		if (start_window_rotations != sched_window_rotations) continue;
	
		//mark unused stats entires. 
		for(i=last_ap_plus1-start_ap; i< n_aps_to_read; i++) {
				part_stats[i].id=-1;
		};
				
		return EOK;
	}
	//More than 1 retry? Shouldnt be pssible, unless the tick size is very short and there is a lot of io.  
	if (ker_verbose >=3) kprintf("APS:2 clock intrs getting stats\n");
	//return anyway with our probably corrupted stats.
	return EINTR; 
}


/*  ---------------------
 *  implements SCHED_APS_OVERALL_STATS option of SchedCtl
 */ 
int aps_get_overall_stats(sched_aps_overall_stats *parms) { 
	parms->idle_cycles = (unsigned int)idle_cycles_lasttick / (unsigned int) NUM_PROCESSORS;
	parms->idle_cycles_w2 = idle_cycles_w2 / (unsigned int)NUM_PROCESSORS;
	parms->idle_cycles_w3 = idle_cycles_w3 / (unsigned int)NUM_PROCESSORS;
	parms->pid_at_last_bankruptcy = last_bankrupter.last_suspect_pid; 
	parms->tid_at_last_bankruptcy = last_bankrupter.last_suspect_tid ==-1? -1:last_bankrupter.last_suspect_tid+1; 
	parms ->id_at_last_bankruptcy = last_bankrupting_ap;
	return EOK;
}

/* ----------------------
 * implements SCHED_APS_QUERY_THREAD optionof SchedCtl
 */
int aps_get_thread_info(THREAD *thp, sched_aps_query_thread_parms *parms) {
	parms ->id = thp->orig_dpp->id; 
	parms ->inherited_id = thp->dpp->id; 
	//yeah yeah. The bits AP_SCHED_* happen to line up with APS_QUERY_CRIT_*, but let's not depend on that.
	parms->crit_state_flags =  (thp->sched_flags & AP_SCHED_PERM_CRIT) ? APS_QCRIT_PERM_CRITICAL : 0;
	parms->crit_state_flags |= (thp->sched_flags & AP_SCHED_RUNNING_CRIT) ? APS_QCRIT_RUNNING_CRITICAL : 0; 
	parms->crit_state_flags |= (thp->sched_flags & AP_SCHED_BILL_AS_CRIT) ? APS_QCRIT_BILL_AS_CRITICAL : 0; 
	return EOK;
} 


/*	-----------------
 *	implements SCHED_APS_ATTACH_EVENTS option of SchedCtl 
 */ 
int aps_attach_events(sched_aps_events_parm *parms) {
	PPG *ppg; 
	THREAD *act = actives[KERNCPU];
	if (parms->id < 0 || parms->id >=num_ppg) return EINVAL;
	ppg = actives_ppg[parms->id]; 

	parms->id = ((DISPATCH*)ppg)->id;
	if (parms->bankruptcy_notification) {
			memcpy(&(ppg->bankruptcy_notifier), parms->bankruptcy_notification, sizeof(ppg->bankruptcy_notifier));
	}
	if (parms->overload_notification) { 
		memcpy(&(ppg->overload_notifier), parms->overload_notification, sizeof(ppg->overload_notifier));
	};
	ppg->notify_tid = act->tid;
	ppg->notify_pid = act->process->pid; 
	return EOK;
}
	

/*
 * Associate the thread with this purpose group
 */

int join_ppg(THREAD *thp, int id) {
	DISPATCH 	*new; 
	/* Check ID is valid */
	/* @@@ do additional validation/security */

	if(id <0 ||id >= num_ppg) return EINVAL;
	//idle threads may not be moved out of System partition
	if (0==thp->priority) return EINVAL;
	new = (DISPATCH *)actives_ppg[id];
	if (thp->state == STATE_READY){
		//remove thread from current dispatch
		unready(thp, 0);
		thp->dpp = new; 
		ready(thp); 

	}
	thp->orig_dpp = thp->dpp = new; //only now is it safe to clobber thp->dpp 
	AP_CLEAR_CRIT(thp); 
	return EOK;
}

/*
 * Associate the process with this purpose group
 */

int join_process_ppg(PROCESS *prp, int id) {
	DISPATCH 	*new; 
	/* Check ID is valid */
	/* @@@ do additional validation/security */

	if (id < 0 || id >= num_ppg) return EINVAL;
	new = (DISPATCH *)actives_ppg[id];
	prp->default_dpp = new;

	return EOK;
}

DISPATCH *aps_select_ppg(PROCESS *prp, int id) {
	if (id < 0 || id >= num_ppg) return NULL;
	return (DISPATCH *)actives_ppg[id];
}


__SRCVERSION("aps_config.c $Rev: 168521 $"); 

