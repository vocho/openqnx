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



/* aps_alg.c 
 *
 * Adaptive Partitioning scheduling
 *
 * Globals and functions that implement the APS scheduling algorithm
 */

#include "aps_data.h"
#include "aps_alg.h"
#include "aps_time.h"
#include "proto_aps.h" 


uint32_t min_cycles_to_be_critscheduleable; /* small fraction of a tick to correct for variations in
	accuracy of ClockCyles() in a ms. PRxxxx */ 

uint32_t scheduling_policy_flags = SCHED_APS_SCHEDPOL_DEFAULT; 
int cur_hist_index; /* increments modulo TS_BUCKETS. index into usage_hist[] critical_usage_hist[] */
volatile uint32_t sched_window_rotations; /* increments same time as cur_hist_index, but is volatie,
	non-modulo and is for use as a retry flag for choose_thread_to_schedule.*/




/* starvation backstop. 
 *
 * This is, well ... a kludge, to prevent certain hard-to-fix scenarios from starving partitions. 
 *
 * The idea is that when to give preference to parititions that have been locked to our cpu and have been
 * ready continuously for the last windowsize but have not run. This situtation can happen with certain
 * combinations of priorities and runmasks. Can happen * even when user doesn not set runmasks on mips: PR28700.
 * All this should be replaced with something general after
 * the Trinity release. All the symbols are prefixed by sb_ or SB_ to make them easy to remove. 
 *
 * this gives starving partitions at least 1tick/window -- a hope they can get out of runmask config causing the starvation
 */ 

uint32_t sb_last_run_winrot[APS_MAX_PARTITIONS][PROCESSORS_MAX]; 
int      sb_partition_is_ready[APS_MAX_PARTITIONS][PROCESSORS_MAX]; 

/* call SB_PARTITION_IS_RUNNING whenever starting a thread, and also when you know a thread is running. Note that
 * FIFO threads can run longer than a windowsize, so call SB_PARTITION_IS_RUNNIG on an act FIFO thread before 
 * calling choose_thread_to_schedule
 */
/* these are defined in aps_alg.h 
    SB_PARTITION_IS_RUNNING(ap) 
    SB_PARTITION_IS_READY(ap)  
    SB_PARTITION_IS_NOT_READY(ap)   
*/
#define SB_STARVATION_METRIC(ap,top_thp) (\
	(top_thp->runmask == ~(1<<RUNCPU)) &&\
	sb_partition_is_ready[ap][KERNCPU] &&\
	sched_window_rotations > sb_last_run_winrot[ap][KERNCPU]+(uint32_t)(ts_buckets)&&\
	actives_ppg[ap]->std_budget_in_cycles\
	? sched_window_rotations - sb_last_run_winrot[ap][KERNCPU] : 0\
) 

/* /starvation backstop */ 



/* calculating relative fraction used
 *
 * see http://os.ott.qnx.com/wiki/index.php/Adaptive_partitioning:Algorithms#Computing_Relative_Fraction_Used
 */  
 
#define BITS_IN_TIME_USED (sizeof(uint32_t)*8)   
#define TIME_USED_SIGNIFICANT_BITS 16
#define TIME_USED_SHIFT (BITS_IN_TIME_USED-TIME_USED_SIGNIFICANT_BITS) 
#define RELATIVE_FRACTION_USED(ppg) (((ppg)->used_cycles>>TIME_USED_SHIFT) * (ppg)->relative_fraction_used_factor \
	+ (ppg)->relative_fraction_used_offset) 


//max_budget_error is stored as an inverse so we can use an integer. 200 means 0.5 percent.
#define MAX_BUDGET_ERROR_INVERSE 200

/*
 * the allowable range of MAX_BUDGET_ERROR is 0.5% to 0.125%, 
 * so MAX_BUDGET_ERROR_INVERSE may be at most 800 
 * SET_FACTORS() depens on this to not overflow 64 bit arithmetic. 
 * Also, multiplying run time by RELATIVE_FRACTION_USED_FACTOR depends on
 * this for not overflowing 16x16->32 bit arithmetic. 
 */

/* 
 * set_factors: should be called whenever the budget of any partititon changes. 
 * This recomputes the ppg->relative_fraction_used_factor used for 
 * ordering partitions of equal highest priority. 
 *
 * The ordering is done at runtime by computing RELATIVE_FRACTION_USED (aka: rfu). 
*/

void 
set_factors () { 
	int		ap; 
	uint64_t	total_product =1;
	uint64_t	t64;
	uint64_t	min_t64; 
	uint64_t	max_factor; 
	   
	//This assumes all budgets sum to 100% 
	//all this would be a lot easier with floats, but we cant do floating point in the kernel. 
 
	// must do calculations in percentages, not milliseconds    
	for(ap=0; ap<num_ppg; ap++) {
			if(actives_ppg[ap]->budget_in_percent) total_product *= actives_ppg[ap]->budget_in_percent;    
	}
	//43 bits is enough to store any possible product of 16 partitions' budgets.
	if (!total_product) crash(); 
 
	min_t64 = ~0; 
	for(ap=0; ap<num_ppg; ap++) {
		if (actives_ppg[ap]->budget_in_percent) { 
			t64 = total_product/(uint64_t)actives_ppg[ap]->budget_in_percent; 
			if (t64<min_t64) min_t64 = t64; 
		}
	}; 
	//for 16 aps, the largest possible value for min_t64 is 40 bits.
    


	/* first set factors for non-zero partitions, remember the largest factor */
	max_factor = 0; 
	//Note: order of calculation chosen so intermediate values don't exceed 64 bits. 
	for(ap=0; ap<num_ppg; ap++) {   
		if (actives_ppg[ap]->budget_in_percent) { 
			//do this calcuation to 64 bits. ordered to minimize loss of accuracy     
			t64 = ((total_product*(uint64_t)MAX_BUDGET_ERROR_INVERSE)  //assumes MAX_BUDGET_ERROR_INVERSE has less than 21 bits. a safe bet.
				/(uint64_t)actives_ppg[ap]->budget_in_percent)/min_t64;
			//at this point, t64 is at most 14 bits, for 16 partitions. Now convert to 16 bits.
			actives_ppg[ap]->relative_fraction_used_factor = t64;        
			if (t64>max_factor) max_factor = t64; 
			actives_ppg[ap]->relative_fraction_used_offset = 0; 
		} 
	}		

	/* rfu factors for zero-budget partitions 
	 * (or how to avoid dividing by zero) 
	 * 
	 * Arrange zero-budget partitions to have a computed rfu to always be greater than any possible rfu for
	 * a non-zero-budget partition. 
	 * 
	 * Even when a zero-budget partition has used no time, we want it's rfu to be bigger than the rfus of
	 * non-zero-budget partition, even if the non-zero partition has consumed the entire window.  
     *
	 * This is accomplished my making sure rfus for zero-budget partitions have a minimum value: the 
	 * relative_fraction_used_offset. The relative_fraction_used_offset is 1 plus the largest possible rfu
	 * of the non-zero partitions. The largest possible rfu value occurs when the partition with the largest
	 * relative_fraction_used_factor has used the whole windowsize.
	 * 
	 * Then all that remains it to set the relative_fraction_used_factor for all zero-budget partitions to be equal.
	 * The value is arbitrary. 
	 */
	{ 
		uint32_t 	max_rfu_plus_1 = max_factor * (windowsize_in_cycles>>TIME_USED_SHIFT)  +1;
		
		for(ap=0; ap<num_ppg; ap++) {   
			if (!actives_ppg[ap]->budget_in_percent) { 
		 		actives_ppg[ap]->relative_fraction_used_factor = 1;
				actives_ppg[ap]->relative_fraction_used_offset = max_rfu_plus_1; 
		 	}
    		}
	}
	
}


    



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

THREAD  *choose_thread_to_schedule(THREAD *pcur_candidate_thread, int *prun_as_crit) { 
	/* vars for finding the best thread  */
	THREAD			*pbest_thread;
	int			sb_best_starvation_metric; 
	int			best_runable; //-1,0 or 1
	uint32_t		least_relative_fraction_used; 
	int			ap;
	int			runable; //boolean
	THREAD			*act;
	uint32_t		start_window_rotations; 
	int			retry_count;
	uint32_t		runable_ap_set; /* runable on this cpu: both ready and have budget */ 
	uint32_t		runable_any_cpu_set; /* runable on some cpu. != runable_ap_set when runmas
							are being used */
	uint32_t		competing_ap_set; /* aps demanding cpu time on this cpu, for bankrupcy 
						     - critial time purposes */ 
	uint32_t		sleeping_ap_set; /* non-zero aps which are not ready to run */
	uint32_t		cpu = KERNCPU;
	THREAD			*ptrds_per_ap[APS_MAX_PARTITIONS]; 
	int 			free_time, underload, prio_enable;	/*booleans*/ 
#if aps_debugging
	int			all_at_limit_bmp; 	/* boolean */
	int			all_at_limit; 		/* boolean */ 
#endif

	/* A local macro. Used to determine if a partition should be competing for determining if critical time should be
	 * billed to the partition that eventually wins in the choose_thread algorithm. For single-processors, this is
	 * equivalent to MAY_SCHEDULE(). 
	 */ 
#define IS_COMPETING(ppg, min_cycles_to_schedule) (((ppg)->used_cycles + (min_cycles_to_schedule)) <= ((ppg)->std_budget_in_cycles)) 
	//@@@ opti: NUM_PROCESSORS is a appox. should be ready procesors.
	
	act = actives[cpu];
	microbill(act);  //make sure act's partition is updated. also updates cycles_left_in_tick
	
	for(retry_count=0;retry_count<=1;retry_count++) { 
		uint32_t	min_to_schedule;
		start_window_rotations = sched_window_rotations; 	
		min_to_schedule = MIN_CYCLES_TO_SCHEDULE(NUM_PROCESSORS);
		//@@@ revisit for SMP. should consider number of ready processors, not all
		
		competing_ap_set = runable_ap_set =runable_any_cpu_set = sleeping_ap_set = 0;	
		//runable_ap_set = if not null (MAY_SCHEDULE((PPG*)(act->dpp)) || THREAD_IS_RUNNING_CRITICAL(act) ) <<(act->dpp->id); 

		/* init data values for the current best guess as to what should run */ 
    		if (pcur_candidate_thread) {
			/* find something better than pcurr_dandiate, including what may be a new higher prio thread in the same
			 * partition as pcur_canadiante */
			PPG	*pap; 
			unsigned	prio;
				
			pap = (PPG *)(pcur_candidate_thread->dpp); 
			prio = DISPATCH_HIGHEST_PRI((DISPATCH*)pap);
			pbest_thread = (prio > pcur_candidate_thread->priority) ? DISPATCH_THP((DISPATCH*)pap,prio) : pcur_candidate_thread; 
			if(pbest_thread->runmask & (1 << KERNCPU)) {
				// We've found someone higher, but he cannot run on this cpu.
				// Revert to original candidate, algorithm below should
				// pick up anyone else above our prio
				pbest_thread = pcur_candidate_thread;
			}
				//make sure sb_best_starvation_metric is zero. Since pbest_thread is running, its not starving
			SB_PARTITION_IS_RUNNING(pbest_thread->dpp->id); 
			if (pbest_thread->priority) { 
				best_runable = MAY_SCHEDULE_MIN(pap,min_to_schedule) || THREAD_IS_RUNNING_CRITICAL(pbest_thread);  
				least_relative_fraction_used =  RELATIVE_FRACTION_USED(pap); 
			} else { 
				//candidate is idle, dont consider it
				pbest_thread = NULL; 
				best_runable = -1; 
				least_relative_fraction_used = ~0; 
			} 
  		 } else { 
			pbest_thread = NULL; 
			best_runable = -1; 
			least_relative_fraction_used = ~0; 
   		}
		sb_best_starvation_metric = 0; 
		

		/* scan all partitions to see which are over/under budget to deterimine which of these 3 modes we are in:
		 * 1. some partition has budget, i.e. underload. 
		 * 2. free time mode. i.e. at least one partition is sleeping and the others are using up the free time
		 * 3. all partitions are at their budget limits, i.e. overload 
		 *
		 * In the process of scaning, cache the top threads for each partition
		 */ 
	
		
		for (ap=0; ap<num_ppg; ap++) { 
			PPG		*pap;
			THREAD		*ptrd;
			unsigned	prio;
			
			pap = actives_ppg[ap]; 
			prio = DISPATCH_HIGHEST_PRI((DISPATCH*)pap);
			if (!prio) { 
				sleeping_ap_set |= 1<<ap; 
				SB_PARTITION_IS_NOT_READY(ap); 
				ptrds_per_ap[ap]=NULL;
				continue;
			}
			ptrd = DISPATCH_THP((DISPATCH*)pap, prio); 
			
			//Can this thread run on some cpu?
			runable =  MAY_SCHEDULE_MIN(pap,min_to_schedule); 
			runable_any_cpu_set |= (uint32_t)( runable || THREAD_IS_RUNNING_CRITICAL(ptrd)) << ap;
			//@@@ opti: runable_any_cpu_set need not be computed on UP. 
			
			if(NUM_PROCESSORS > 1) {
				// Determine if runmask allows thread to run
				while((ptrd->runmask & (1 << cpu)) != 0) {
					ptrd = ptrd->next.thread;
					if(ptrd == NULL) {
						// No-one at this prio can run, go looking down next queue
						while((--prio > 0) && (ptrd=DISPATCH_THP((DISPATCH*)pap, prio)) == NULL) {
							//nothing to do
						}	
						if(!prio) break;
						if(ptrd == NULL) crash();
					}
				}
				if(!prio) {
					sleeping_ap_set |= 1<<ap; 
					//Note with BMP different processors can have a different number of sleeping APs. The sleeping
					//AP count is for our processor. 
					SB_PARTITION_IS_NOT_READY(ap); 
					ptrds_per_ap[ap]=NULL;
					continue;
				}
			}
			SB_PARTITION_IS_READY(ap); 
			ptrds_per_ap[ap]=ptrd; 

			CRASHCHECK(ptrd == NULL);
			
			//Can this partition run on this cpu?
			runable_ap_set |= (uint32_t)(runable || THREAD_IS_RUNNING_CRITICAL(ptrd)) <<ap;
			
			competing_ap_set |= (uint32_t)(IS_COMPETING(pap,min_to_schedule)||THREAD_IS_RUNNING_CRITICAL(ptrd))<<ap; 
		}

	
		//@@@ a small optimization: dont bother computing sleeping_ap_set if !aps_debugging or  
		//runable_any_cpu_set!=0 
		//
		/* any parition that's currently runningis not sleeping */
		{ int		i; 
		  THREAD	*thp;
		  for(i=0;i<NUM_PROCESSORS;i++) {
			thp = actives[i]; 
			if (thp->priority) sleeping_ap_set &= ~(1<<thp->dpp->id); 
		  } 
		  /* a sleeping partition with a zero budget should not trigger free-time mode */ 
		  sleeping_ap_set &= ~zerobudget_ap_set;
		} 
		
		
		underload = (runable_any_cpu_set!=0);  
#if aps_debugging
		all_at_limit = (0==competing_ap_set) && (0==sleeping_ap_set) ;
		all_at_limit_bmp = (0==runable_any_cpu_set) && (0==sleeping_ap_set) ;
#endif 
		free_time = (0==runable_any_cpu_set) && (0 != sleeping_ap_set);
		
		/* these are the merit functions by which we pick the best ap: 
		 * underload	         f=(runable, prio, rfu) 
		 * all_at_limit          f=(rfu) 
		 * all_at_limit_bmp      f=(rfu), only in SCHEDPOL_BMP_SAFETY mode  
		 * free_time, 
		 *     default:          f=(runable, prio, rfu) 
		 *     SCHEDPOL_RATIO    f=(runable, rfu) 
		 *
		 * These can be collapsed into one function f=(runable, prio_enable ? prio : 0, rfu) 
		 * where prio_enable is: 
		 */ 
		prio_enable = 
			underload 
			|| ((free_time && !(scheduling_policy_flags & SCHED_APS_SCHEDPOL_FREETIME_BY_RATIO)) ); 
	

		/*Therefore the all_at_limit and all_at_limit_bmp states need not be calculated */
#if aps_debugging
		{ static int counter;
		 	if (++counter>15000) { 
				counter=0; 
				disp("  rsm:");disp_int(runable_ap_set);
				disp("  rsa:");disp_int(runable_any_cpu_set); 
				disp(" cs:");disp_int(competing_ap_set);
				disp(" sl:");disp_int(sleeping_ap_set); 
				disp(" u:");disp_int(underload); 
				disp(" a:");disp_int(all_at_limit_bmp);disp("-");disp_int(all_at_limit); 
				disp(" f:");disp_int(free_time); 
				disp(" pe:");disp_int(prio_enable); 
			}
		}
#endif
		
		for (ap=0; ap<num_ppg; ap++) { 
			uint32_t	rfu;
			THREAD		*ptrd;
			uint32_t	sb_starvation_metric;

			ptrd = ptrds_per_ap[ap];
			if (!ptrd) continue; 
			runable = 0 != (runable_ap_set&(1<<ap)); 
			
			//this if-tree finds the best ap by comparing the tuple (runable, priority, relative_fraction)
			//As soon as it knows an ap has lost, it avoids computing the remaining elements
			//As soon as it knows an ap has won, it avoids testing the remaining elements, but still has to compute 
			//them to save as the best tuple. 
			sb_starvation_metric = SB_STARVATION_METRIC(ap, ptrd); 
			if (sb_starvation_metric < sb_best_starvation_metric) { 
					//loose because someone else is starving 
					continue;
			} else { 
				if (sb_starvation_metric == sb_best_starvation_metric) { 
						
		 			if (runable<best_runable) {						 
						//loose because you're over budget, or in BMP_SAFETY mode, over budget/NUM_PROCESSORS 
						continue;                                                       
					} else {                                                            
						if (runable==best_runable) {                                     
							CRASHCHECK(NULL == pbest_thread);
							if (prio_enable && ptrd->priority < pbest_thread->priority) {         
								// loose on priority 
								continue;
							} else { 
								rfu = RELATIVE_FRACTION_USED(actives_ppg[ap]); 
								if (!prio_enable || ptrd->priority == pbest_thread->priority) {                      
									if (rfu >= least_relative_fraction_used){
										//loose on rfu
										continue;  
									} else { 
										// win on relative fraction used 
									}// endif rfu >= 
								} else {
									// else win on priority
								}// endif prio == 
							} // endif prio < 
						} else {
							// win on runable 
							rfu = RELATIVE_FRACTION_USED(actives_ppg[ap]); 		
						}// endif runable ==
					}//endif runable < 
				} else { 
					// win because we are starving
					rfu = RELATIVE_FRACTION_USED(actives_ppg[ap]); 		
				} // endif starvation == 
			} //endif starvation < 
			
			//best ap so far. 
			sb_best_starvation_metric = sb_starvation_metric; 
			best_runable = runable;                                              
			pbest_thread = ptrd;            
			least_relative_fraction_used = rfu;
   	 	}// endfor ap=0
		//@@@ a speed optimization might be 3 different loops optomized for each case of free_time, underload, 
		//and all_at_load	
		
		
		//Bill selected thread for critical time only if it was critical, out of budget and 
		//had competition. For SMP, we only consider competition on the same processor as us.. 
	  	if  (pbest_thread && THREAD_IS_RUNNING_CRITICAL(pbest_thread) 
				&& !IS_COMPETING((PPG*)(pbest_thread->dpp), min_to_schedule) ) {
			//the active thread's partition is also competing, if it had budget  
			competing_ap_set |= (uint32_t)IS_COMPETING((PPG*)(act->dpp),min_to_schedule)<<(act->dpp->id);
			
			//Partitions of threads running on other cpus are not considered to be competing on this cpu.
			
			//the thread we're choosing is not competing against itself. 
			competing_ap_set &= ~(1<<pbest_thread->dpp->id);

		} else {
			competing_ap_set = 0;
		}
		//now, competing_ap_set !=0 means run as critical. 
		//
		if (start_window_rotations == sched_window_rotations) {
			//we havent been unterrupted by a clock tick, so our choice is valid
			if (pbest_thread) UPD_BILL_AS_CRIT(pbest_thread, competing_ap_set!=0) 
			if(prun_as_crit) *prun_as_crit = (competing_ap_set!=0);

			return pbest_thread;
		}
		// the scheduling windows rotated while we were choosing, so our choice is bogus. Try one more time.
		// 
	} //endfor retires=0
	
	//More than 1 retry? Shouldnt be pssible. Means we were hit with TWO clock interrupts while we were choosing. 
	if (ker_verbose >=3) kprintf("APS:2 clock intrs while choosing\n");
	return pbest_thread;

} // end choose_thread
#undef IS_COMPETING
#undef IS_SLEEPING

/* choose_between 
   Compares two threads and returns the one more likely to be next scheduled. 

   This does not implement the full scheduling algorithim which would require compareing to all other partitionis.
   In particular this does not consider the cases: 

   1. both a and b have budget, but some thread c in a different partition also has budget and higher prio 
   2. neither a nor b have budget, but some thread c in a different partition has budget and possibly a lower prio 
 */
THREAD *choose_between(THREAD *a, THREAD *b) { 
	int ta,tb;
	if (0==a->priority) return b;
	if (0==b->priority) return a; 
	ta = MAY_SCHEDULE((PPG*)(a->dpp)) || THREAD_IS_RUNNING_CRITICAL(a);
	tb = MAY_SCHEDULE((PPG*)(b->dpp)) || THREAD_IS_RUNNING_CRITICAL(b);
	if (ta>tb) return a;
	if (tb>ta) return b; 
	if (a->priority > b->priority) return a;
	if (b->priority > a->priority) return b;
	if (RELATIVE_FRACTION_USED((PPG*)(a->dpp)) < RELATIVE_FRACTION_USED((PPG*)(b->dpp)) ) return a; 
	return b;
}



__SRCVERSION("aps_alg.c $Rev: 199396 $");

