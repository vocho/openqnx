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
 * aps_data.c 
 * 
 * Adaptive Partitioning scheduling 
 * 
 * global data declarations and init functions. 
 *
 * See aps_data.h for comments about types/globals. 
 */
#include "aps_data.h"
#include "aps_time.h"
#include "aps_alg.h"
#include "proto_aps.h"


uint32_t	windowsize_in_ms; 
uint64_t	windowsize_in_cycles;
int		bmp_support_factor =1; 

int		ts_buckets;				
uint64_t	cycles_per_ms;   
uint64_t	cycles_per_bucket;

PPG		*actives_ppg[APS_MAX_PARTITIONS];

PPG		*system_ppg;

int		num_ppg;	    	


int		idle_hist[MAX_TS_BUCKETS];
uint32_t	idle_cycles;
uint32_t	idle_cycles_lasttick;  

int		hist_index_w2; 	/* cyclical index into _w2 tables */
int		hist_index_w3; 	/* cyclical index into _w3 tables */

long_reporting_window *actives_lrw[APS_MAX_PARTITIONS]; 


uint32_t	idle_hist_w2[MAX_W2_BUCKETS];	/* time spend idle during window2, nominally last second */ 
uint64_t	idle_cycles_w2;	/* total time spent idle in w2 window */ 
uint64_t	idle_hist_w3[MAX_W3_BUCKETS]; /* time spent idle during window3, nominally last 10 seconds */ 
uint64_t	idle_cycles_w3;	/* total time spent idle in w3 window */ 

uint32_t	zerobudget_ap_set; /* a bitmap of which partitions have budgets ==0 */ 


/* overall controls */ 
uint32_t	aps_security;	/* set of SCHED_APS_SEC_* flags from sys/sched_aps.h */ 


void zero_idle_cycles() { 
	idle_cycles =0; 
	memset(idle_hist, 0, sizeof(idle_hist)); 
	idle_cycles_w2=0;
	memset(idle_hist_w2, 0, sizeof(idle_hist_w2)); 
	idle_cycles_w3=0;
	memset(idle_hist_w3, 0, sizeof(idle_hist_w3)); 
}


/* must zero the lrws whever the scheduling window changes size */
void zero_lrw(int ap) { 
	long_reporting_window *lrw = actives_lrw[ap]; 
	memset(lrw, 0, sizeof(long_reporting_window)); 
	
}



/* reinitializes the windowing data structures. also adapts to a new windowsize. Returns EINTR if interrupted
 * thrice by clock interrupts. Returns EINVAL if new_windowsize_ms is out of range or if it would create
 * windows of the wrong size. (Some combinations of ClockPeriod and windowise can cause out-of-range
 * windowsizes.) 
 */ 
int 
reinit_window_parms(int new_windowsize_ms)
{
	int		i,ap; 
	int		curr_window_rotations ;
	int		retry_counter;
	int		new_ts_buckets; 

	if (new_windowsize_ms < MIN_WINDOWSIZE_MS || new_windowsize_ms > MAX_WINDOWSIZE_MS) return EINVAL;
	
	/* Calculate cycles per bucket. Read the current ticksize, in case user has changed it.*/
	{ uint64_t	tick_nsec;
		tick_nsec = SYSPAGE_ENTRY(qtime)->nsec_inc; 
		if (!tick_nsec) { 
			//ticker not yet initialized. choose default value 
			tick_nsec = SYSPAGE_ENTRY(cpuinfo)->speed <= _NTO_SLOW_MHZ ?
				_NTO_TICKSIZE_SLOW : _NTO_TICKSIZE_FAST;
		} 
		cycles_per_bucket = CONDITION_CC(SYSPAGE_ENTRY(qtime)->cycles_per_sec) /  ((uint64_t)1000000000 / tick_nsec );
	}
		
	cycles_per_ms = CONDITION_CC(SYSPAGE_ENTRY(qtime)->cycles_per_sec) / ((uint64_t)1000);
	new_ts_buckets = (cycles_per_ms * ((uint64_t)new_windowsize_ms)) / cycles_per_bucket; 	
	if (new_ts_buckets > MAX_TS_BUCKETS || new_ts_buckets < MIN_TS_BUCKETS ) return EINVAL;

	
	/* ts_buckets is within range, so we can now update some globals */
	
	windowsize_in_ms = new_windowsize_ms;
	windowsize_in_cycles = (uint64_t)new_ts_buckets * cycles_per_bucket; /* adjust for integer divide. make windowsize a multiple of a bucket */
	ts_buckets = new_ts_buckets; 
	
	max_microbill_cycles = 2* cycles_per_bucket;
	max_bankruptcy_grace = 2*ts_buckets;

	min_cycles_to_be_critscheduleable = cycles_per_bucket/32; 
	/* n ticks of time is a bit less than n*cycles_per_ms because syspage->cycles_per_second is not
	 * perfectly accurate. So we consider a partition to be bankrupt if it gets within a small fraction
	 * (1/32) of its critical budget to compensate for clockcycles() innacuracy */ 



	/* the window size shouldnt be changed after sub-partitions of the system partitions are made. But just to be paranoid,
	 * wipe the existing histories since they no longer make sense. First reset bankruptcy grace just in case some ap
	 * is near bankruptcy while were mucking with the sliding windows.*/ 
	

	/* the rest of this init may be corrupted by ppg_tick_hook(), so restart if it it's hit by a clock
	 * tick
	 */
	for(retry_counter=0; retry_counter<3; retry_counter++) { 
		curr_window_rotations = sched_window_rotations; 
		
		
		remaining_bankruptcy_grace = max_bankruptcy_grace;  
		
		cur_hist_index =0; 
		for (ap=0;ap<num_ppg;ap++) { 
			PPG *ppg;
			ppg = actives_ppg[ap]; 
			ppg->std_budget_in_cycles = (NUM_PROCESSORS * windowsize_in_cycles * ppg->budget_in_percent) / 100; 
			ppg->bmp_budget_in_cycles = (bmp_support_factor * windowsize_in_cycles * ppg->budget_in_percent) / 100; 
			ppg->used_cycles = 0;
			ppg->critical_cycles_used = 0; 
			for (i=0;i<ts_buckets;i++) {
				ppg->usage_hist[i] = 0; 
				ppg->critical_usage_hist[i] = 0; 
			}
			zero_lrw(ap); 
		}
		//keep the system partitions critical budget equal to the windowsize, so it can never become bankrupt
		if (system_ppg) system_ppg->critical_budget_in_cycles = NUM_PROCESSORS * windowsize_in_cycles; 
		
		zero_idle_cycles(); 
   		 hist_index_w2 = hist_index_w3 = 0; //PR27890 
		/* no need to call set_factors() since budgets in percent have not changed */ 
    		if (curr_window_rotations == sched_window_rotations) return EOK; 
	} 
	//hmm. perhaps the user has set the tick size to be very short. In anycase, we have not properly reset the
	//window. 
	if (ker_verbose >=3) kprintf("APS:3 clock intrs resetting window\n");
	return EINTR;

} 

void 
set_bmp_support_factor() { 
	/* This only applies to SMP, and is important when users are using runmasks. 
	 *
	 * the SCHED_APS_SCHEDPOL_BMP_SAFETY is a euphemisn for avoiding threads using runmask from monopolising
	 * a cpu. (PR28706) For example, a thread locked to cpu1 in a 51% partition can completely prevent a 49% partition
	 * from every running on cpu1. The SCHED_APS_SCHEDPOL_BMP_SAFETY avoids that monopolization by forcing 
	 * partitions to run out budget at budget/num_processors. 
	 *
	 * std_budget_in_cycles is arranged to always represent the precentage budget of a partition.  
	 *
	 * Normally, BMP_SAFETY is not set. Then bmp_budget_in_cycles is set to NUM_PROCESSORS * budget_percent * windowsize
	 * So on a 4 headed machine a 51% partition would have to accumulate 204ms (51ms for each processor) to be 
	 * over budget. (That works fine provided no thread is runmasked locked to a single cpu.) 	  
	 * 
	 * When BMP_SAFETY is set, bmp_budget_in_cycles is set to budget_percent * windowsize. So on a 4 headed machine, 
	 * a 51% partition is out of budget after running for a total of 51ms. That makes sure the scheduler always
	 * looks around for something else to run before a thread bound to one cpu monopolizes its partition. 
	 *
	 * On non-SMP, bmp_budget_in_cycles is equal to std_budget_in_cycles. So at all time, the scheduler use the critiion
	 * that used ccycles < bmp_budget_in_cycles means you are under budget and may be freely schedule by priority. 
	 * 
	 * And no, this isnt a nice way to do it. 
	 * 
	 */

	if (scheduling_policy_flags & SCHED_APS_SCHEDPOL_BMP_SAFETY) { 
		bmp_support_factor = 1; 
	} else { 
		bmp_support_factor = NUM_PROCESSORS; 
	} 
}



DISPATCH 
*create_default_dispatch(void) {
	
	/* Allocate and initialize the system partition */
	system_ppg = _scalloc(sizeof (PPG));
	system_ppg->dpp.id = 0;
	STRLCPY(system_ppg->name, APS_SYSTEM_PARTITION_NAME, sizeof(system_ppg->name)); 
	system_ppg->max_prio = NUM_PRI;
	system_ppg->high_prio = 0;
	/* system partition starts with 100% of the time */ 
	system_ppg->budget_in_percent = 100;
	system_ppg->bmp_budget_in_cycles = bmp_support_factor * windowsize_in_cycles;
	system_ppg->std_budget_in_cycles = NUM_PROCESSORS * windowsize_in_cycles; 
	system_ppg->used_cycles = 0;
	system_ppg->critical_budget_in_cycles = NUM_PROCESSORS * windowsize_in_cycles; //default to infinity.  
	system_ppg->critical_cycles_used = 0; 
 	system_ppg->parent = NULL;
	SIGEV_NONE_INIT(&system_ppg->bankruptcy_notifier);
	SIGEV_NONE_INIT(&system_ppg->overload_notifier);
	system_ppg->notify_pid = system_ppg->notify_tid = -1;
	system_ppg->state_flags = 0;
	INIT_BANKRUPTCY_INFO(&system_ppg->bnkr_info);	
	actives_ppg[APS_SYSTEM_PARTITION_ID] = system_ppg;


	actives_lrw[APS_SYSTEM_PARTITION_ID] = _scalloc(sizeof(long_reporting_window));
	if (actives_lrw[APS_SYSTEM_PARTITION_ID] == NULL) crash();
	zero_lrw(APS_SYSTEM_PARTITION_ID); 
	
	num_ppg = 1;

	set_factors(); 
	
	remaining_bankruptcy_grace = max_bankruptcy_grace; 

	trace_emit_sys_aps_name(APS_SYSTEM_PARTITION_ID, system_ppg->name);
	trace_emit_sys_aps_budgets(APS_SYSTEM_PARTITION_ID, system_ppg->budget_in_percent, system_ppg->critical_budget_in_cycles/cycles_per_ms); 
	/* return system partition - this is the default partition at boot time */
	return (DISPATCH *)system_ppg;
}



__SRCVERSION("aps_data.c $Rev: 153052 $"); 

