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
 * Prototypes that are private to the adaptive partition scheduler
 */



int kdecl ker_sched_ctl(THREAD *act, struct kerargs_sched_ctl *kap);





/* --------------------------------------------------------------------------------------------
 * from aps_data.c 
 * --------------------------------------------------------------------------------------------
 */ 

/* reinit_window_parms recalculates all dynamic scheduling parameters. It also clears all scheduling
 * history. returns EINTR if interrupted by 3 clock ticks. (very unlikely) */ 
int reinit_window_parms(int new_windowsize_ms); 


/* must zero the lrws whever the scheduling window changes size */
void zero_lrw(int ap); 


/* bmp_support_factor configures data for BMP. Note call reinit_window_parms() after use */ 
void set_bmp_support_factor(); 


/* create_default_dispatch creates the system parition. */ 
DISPATCH *create_default_dispatch(void);





/* --------------------------------------------------------------------------------------------
 * from aps_config.c 
 * --------------------------------------------------------------------------------------------
 *
 * functions to implment the SchedCtl() api. The primary user of this is ker_aps.c 
 *
 * implemented in aps_config.c
 * 
 */

/*  ---------------------
 *  change_percentage_budget()
 *  
 *  adjust percentage budget by returning excess to parent or taking from parent.
 *  returns -1 if parent doesnt have enough to satisfy request
 *  should not be used to change system partition
 *
 *  ***! note: call set_factors() after changing any paritions budget. need be done only once per kernel call !***
 *  
 */
int change_percent_budget(PPG *ppg, int new_budget_percent); 


/* -------
 * create_ppg() 
 * 
 * create a new partition
 * returns partition id crated or a -errno 
 * Parameters:
 *  name - may be NULL, caller must check if unique
 *  budget in % points of CPU
 */
int create_ppg(sched_aps_create_parms *parms, int max_prio);



/* -----------
 * aps_name_to_id() 
 * 
 * Linear search of partition for matching name. Returns partition number or -1
 */
int aps_name_to_id(char *name);



/*  ---------------------------
 *  implements SCHED_APS_QUERY_PARMS of SchedCtl 
 */
void aps_get_info(sched_aps_info *sched_info);   


/*  -------------
 *  implements SCHED_APS_SET_PARMS option of SchedCtl
 */
int aps_set_parms(sched_aps_parms *parms); 


/*   --------------------
 *   implements SCHED_APS_QUERY_PARTITION option of SchedCtl
 */
int aps_get_partition_info( sched_aps_partition_info *part_info);   

/*   --------------------
 *   finds paritiion id parent of given partition, or -1 if the id is invalid
 *   Returns system_partition if asking for parent of the system partition
 */
int aps_get_parent(int id);



/*  -------------------
 *  implements SCHED_APS_MODIFY_PARTITION
 *
 *  assumes caller has checked that percentages are between 0 and 100; 
 */
int aps_modify_partition(sched_aps_modify_parms *parms);


/* -----------------------
	implements SCHED_APS_PARTITION_STATS option of SchedCtl
 */
int aps_get_partition_stats(sched_aps_partition_stats *part_stats, int n_aps_to_read);  


/*  ---------------------
 *  implements SCHED_APS_OVERALL_STATS option of SchedCtl
 */ 
int aps_get_overall_stats(sched_aps_overall_stats *parms);   



/* ----------------------
 * implements SCHED_APS_QUERY_THREAD optionof SchedCtl
 */
int aps_get_thread_info(THREAD *thp, sched_aps_query_thread_parms *parms);  


/*	-----------------
 *	implements SCHED_APS_ATTACH_EVENTS option of SchedCtl 
 */ 
int aps_attach_events(sched_aps_events_parm *parms);
	

/* ---------------------
 * implements SCHED_APS_JOIN_PARTITION, for moving a thread to a partition  
 */

int join_ppg(THREAD *thp, int id);

/* ---------------------
 * Implements SCHED_APS_JOIN_PARTITION, when only the partition of the process is being changed. 
 * Note: joint a process to a partition does not change the partition of any thread. It only determines
 * in which partition pulses will be handled. 
 */

int join_process_ppg(PROCESS *prp, int id);

/* ---------------------
 * Implements the schedpart_select_dpp()
 * see aps.h 
 */
DISPATCH *aps_select_ppg(PROCESS *prp, int id);




/* --------------------------------------------------------------------------------------------
 * from aps_alg.c
 * --------------------------------------------------------------------------------------------
 */ 

/* 
 * set_factors: should be called whenever the budget of any partititon changes. 
 * This recomputes the ppg->relative_fraction_used_factor used for 
 * ordering partitions of equal highest priority. 
 *
 * The ordering is done at runtime by computing RELATIVE_FRACTION_USED (aka: rfu). 
*/
void set_factors (); 




/*-------------------------------
 * choose_thread_to_schedule()
 * 
 * Picks a thread to schedule based on adaptive partitioning scheduling rules 
 * (http://os.ott.qnx.com/wiki/index.php/Adaptive_partitioning:Algorithms)
 *
 * Note this function updates the AP_SCHED_BILL_AS_CRIT flag of the chosen thread. Occaisonally code calls
 * this function for information, and doesnt make ready the chosen thread. In those cases the BILL_AS_CRIT
 * bit is set, but is harmless. It as effect only when the thread actually runs. It should wind-up being
 * choos_thread()-ed or select_thread()-ed (which resets the bit) before being mared ready.
 *
 * 
 * This function returns NULL if there is nothing better to run but idle. It never returns idle.
 * 
 * Parms:
 * 
 * pcur_candiate_thread:
 *     if pcur_candidate_thread is provided, this function will look for a better thread to run. 
 *     If there is no better thread, then it returns pcur_candidate_thread. Note that this will not return
 *     other threads of equal prio and in the same partition as pcur_candidate thread, which is what you'd want
 *     if its pcur_candidate_thread's time to round robin. 
 *
 * prun_as_critical: 
 *    a boolean which is set on return to indicate that the chosen thread's cpu time should be counted as critical
 *    when it next runs. If you call this function, but make ready some other thread the chosen partition, use
 *    this bool to set the AP_SCHED_BILL_AS_CRIT flag in the thread's sched_flags. 
 *     
 */

THREAD  *choose_thread_to_schedule(THREAD *pcur_candidate_thread, int *prun_as_crit);   



/* choose_between() applies an aproximation of the scheduling alorithnm to pick between two threads. */ 
THREAD *choose_between(THREAD *a, THREAD *b);   




/* ---------------------------------------------------------------------------------------------
 * from aps_time.c 
 * --------------------------------------------------------------------------------------------
 */

/* initalize microbill */ 
void init_microbill();


/* record time spend in the current thread since the last call to microbill(). Call when the state
 * of a running thread changes or a thread jumps partitions 
 */ 
void microbill(THREAD *thp); 


/*
 * This hooks into the scheduler clock tick, which means that al of ppg_tick_hook() runs from the
 * clock-tick interrupt handler in nano_clock.c 
 *
 * The return value signals the clock handler, in nano_clock.c, whether or not to call resched_ppg();
 *
 */


int rdecl ppg_tick_hook(void);






/* ---------------------------------------------------------------------------------------------
 * from aps_crit.c 
 * --------------------------------------------------------------------------------------------
 */ 


/* handle bankruptcy exceutes the user specified bankrupty policy on a partition already determined to be
 * bankrupt
 */
void handle_bankruptcy(THREAD *act, PPG *ppg); 

/* ----------------------------------------------------------------------------------------------
 * from aps_trace.c 
 * 
 * aps_trace.c is mostly just a placeholder for now. Eventualy aps trace definitions that are currently in
 * nano_trace.c will be moved there. 
 * 
 */

/* trace out intial budgets and names */ 
void sched_trace_initial_parms_ppg();


/* ----------------------------------------------------------------------------------------------
 * from aps_application_error.c 
 *
 * aps_bankruptcy_crash, just calls crash (at user's request when a partition goes bankrupt)
 */
void aps_bankruptcy_crash(int ap, int pid, char *debug_name); 


__SRCVERSION("proto_aps.h $Rev: 168521 $");
