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
 * aps_time.c 
 * 
 * Adaptive Partitioning scheduling 
 *
 * low_level timing function, microbill, and our hook into the clock interrupt  handler, ppg_tick_hook()
 *
 * 
 */


#include "aps_data.h" 
#include "aps_time.h"
#include "aps_alg.h"
#include "proto_aps.h"

/*
 * Microbilling: accounting for thread execution to the resolution of ClockCycles()  
 *
 * Well. to the resolution of CONDITION_CC(ClockCycles()) which downshifts clockcyles to trade a some resolution for
 * range.
 * 
 */


/* to lock access to global timing variables */
struct intrspin aps_lock;

/* all timing calculations are done in ClockCyles() divided by a constant factor to control the number of signifcant
 * bits used. All variables with '_cycles' in their names nave been conditioned. Variables with _raw in their names
 * are not conditioned. Conditioning is done with CONDIDITION_CC() in aps_time.h */

uint64_t	max_microbill_cycles; /* a threshold for detecting an error in microbill() */ 

uint32_t	cycles_left_in_tick; /* only valid if microbill() has been recently called */ 


/* raw times are in unconditioned (i.e. unshifted) ClockCycles, but they are adjusted by clock cycles offest 
 * which means they can be compared across processors.
 */
#define READ_TIME_RAW(cpu) (  ClockCycles()-clockcycles_offset[cpu])

uint64_t 	last_time_raw;       /* last raw time for any processor */ 

/* last raw times read per processor, used to calculate how long a thread has been running on a particular cpu*/
uint64_t	cur_time_raw[PROCESSORS_MAX];



/* ------------------------------------------------------------------
 * init_microbill() 
 * 
 * call once.
 */ 


void init_microbill() {
	int		i;
	for(i = 0; i < NUM_PROCESSORS; i++) {
		// Assume clocks on CPUs are pretty synchronized
		cur_time_raw[i] = READ_TIME_RAW(i);
	}
	last_time_raw  = cur_time_raw[NUM_PROCESSORS]; 
	/* approximate cycles_left_in_tick  untill our first clock interrupt */
	cycles_left_in_tick = cycles_per_bucket; 
}



/* ------------------------------------------------------------------
 * microbill_one_cpu 
 * 
 * microbills one thread on one cpu.  a local function to microbill() and microbill_all_from_tick. 
 *
 * new_time_raw should be ClockCycles() - clockcycles_offset[cpu] 
 * 
 */ 
void 
#ifdef __GNUC__ 
inline 
#endif
microbill_one_cpu(THREAD *thp, int cpu, uint64_t new_time_raw) {
	uint64_t	delta;
	PPG		*ppg = (PPG *)thp->dpp;
	
	
	
	delta = CONDITION_CC(new_time_raw - cur_time_raw[cpu]);
	//Note the above doesnot require a wraparound check as long as new_time_raw and cur_time_raw are the same type.
	//ClockCycles() is internally shifted, by different amounts on different platforms, to guarantee this. 

	if (delta > max_microbill_cycles) { 
		/* this can occur on some machines that miss tick interrupts. Since anomalously large deltas can cause
		 * arithimetic overflows in the windowing algorithm which can cause a persitant error in scheduling,
		 * we clamp the value of delta. This clamping causes a temporary scheduling accuracy error.
		 */ 
		delta = max_microbill_cycles;
	} 

	
	/* Idle threads don't get accounted */
	if(thp->priority != 0) {
		ppg->usage_hist[cur_hist_index] += delta;
		ppg->used_cycles += delta;
		if (thp->sched_flags&AP_SCHED_BILL_AS_CRIT) { 
			ppg->critical_usage_hist[cur_hist_index] += delta; 
			ppg->critical_cycles_used += delta;
		}
	} else { 
		idle_hist[cur_hist_index] += delta;
		idle_cycles += delta;
	}
	cur_time_raw[cpu] = new_time_raw;
}


/* -------------------------------------------------------------------------------
 * microbill()
 * microbills time on one thread to current cpu. Call from kernel only
 * also updates the number of cycles left in the current tick. 
 */ 
void microbill(THREAD *thp) {
	uint64_t	new_time_raw; 
	/*must be called from kernel only */
	
	/* The Disable/Enable is necessary for now to keep the counts synced */
	INTR_LOCK(&aps_lock);
	
	new_time_raw = READ_TIME_RAW(KERNCPU);
	
	microbill_one_cpu(thp, KERNCPU, new_time_raw); 
	
	cycles_left_in_tick -= CONDITION_CC(new_time_raw - last_time_raw); 
	//check for unsigned int underflow, meaning we are about to exhast the current tick. When this
	//happens, were are very close to the tick interrupt, so approximate by saying were at the end of
	//this tick.
	if (cycles_left_in_tick>cycles_per_bucket) cycles_left_in_tick=0; 
	last_time_raw = new_time_raw; 
	INTR_UNLOCK(&aps_lock);

}
/* --------------------------------------------------------------------------------
 * microbills all cpus at a tick interrupt. Call only fromt clock interrupt 
 * also resets the number of cycles left in the current tick. 
 */
void microbill_all_at_tick(void) {
	uint64_t	new_time_raw; 
	int		cpu; 
	
	/* The Disable/Enable is necessary for now to keep the counts synced */
	INTR_LOCK(&aps_lock);

        //compesate for ClockCyles() being different on each processor. 	
	new_time_raw = READ_TIME_RAW(RUNCPU); //RUNCPU since were at int level 

	
	for(cpu=0;cpu<NUM_PROCESSORS; cpu++){
		 microbill_one_cpu(actives[cpu], cpu, new_time_raw);	
	}	
	cycles_left_in_tick = cycles_per_bucket; 
	last_time_raw = new_time_raw; 
	
	INTR_UNLOCK(&aps_lock);

}

/* 
 * This hooks into the scheduler clock tick, which means that al of ppg_tick_hook() runs from the
 * clock-tick interrupt handler in nano_clock.c 
 *
 * Mostly this rotates the scheduling averaging window and the longer reporting windows. Note that
 * rotating the scheduling window ppg->usage_hist[] appears to use a different algorithm than rotating
 * the lrw->usage_w2[] and lrw->usage_w3[] windows. It's actually the same algorithm but since but the 
 * implemetations choose a different time when consider the current bucket to be full, the code is intentionally
 * slightly different. The total for a window should only be interrogated when we consider the current bucket 
 * to be full. 
 * 
 */


int rdecl ppg_tick_hook(void) {
	THREAD	*act = actives[RUNCPU]; //use RUNCPU rather than KERNCPU since we're in an interrupt handler
	int i;
	PPG *ppg;
	int bankruptcy_detected =0; 
	
	/* Bill the last tick for each processor */ 
	microbill_all_at_tick(); 

	if (remaining_bankruptcy_grace) remaining_bankruptcy_grace--; 
	
	/* if we will wrap the sched window on this tick, then rotate window2. We will rotate the sched window later 
	 * inside a spinlock. 
	 *
	 * Note that rotating window2 and window3 allows for errors caused by microbill being called
	 * on other processors. This effect is small and should be eliminated when we fix smp-microbill. For now it's not
	 * worth making the spin-lock section large enough to include the window2 and window3 rotations. 
	 */
	if(cur_hist_index+1 >= ts_buckets) { 
			long_reporting_window *lrw; 
			
			/* wrapping cur_hist_index is all we need to do for the scheduling window here. The rest of this THEN clause
			 * is for the long reporting windows which we only process when the scheduling window wraps*/
		
			/* rotate w2 reporting window */ 
			for (i=0; i< num_ppg; i++) { 
				uint64_t	t64;
				ppg = actives_ppg[i];
				lrw = actives_lrw[i]; 
				
				lrw->usage_w2 -= lrw->usage_hist_w2[hist_index_w2];
				t64 = ppg->used_cycles;
				lrw->usage_hist_w2[hist_index_w2] = t64;
				lrw->usage_w2 += t64;
				//current bucket is full here. 

				lrw->critical_usage_w2 -= lrw->critical_hist_w2[hist_index_w2];
				lrw->critical_hist_w2[hist_index_w2] = ppg->critical_cycles_used;
				lrw->critical_usage_w2 += ppg->critical_cycles_used;
				//current bucket is full here. 
							
			}
			idle_cycles_w2 -= idle_hist_w2[hist_index_w2];
			idle_hist_w2[hist_index_w2] = idle_cycles;
			idle_cycles_w2 += idle_cycles;
			//current bucket is full here
			
			hist_index_w2++;
			if (hist_index_w2 >= MAX_W2_BUCKETS) {
					
				hist_index_w2 =0; 
			
			
				/*then we wrapped w2, so fill one bucket of w3 with the totals from w2*/
				for (i=0; i< num_ppg; i++) { 
					ppg = actives_ppg[i];
					lrw = actives_lrw[i]; 
					
					lrw->usage_w3 -= lrw->usage_hist_w3[hist_index_w3];
					lrw->usage_hist_w3[hist_index_w3] = lrw->usage_w2;
					lrw->usage_w3 += lrw->usage_w2;
					//current bucket is full here

					lrw->critical_usage_w3 -= lrw->critical_hist_w3[hist_index_w3];
					lrw->critical_hist_w3[hist_index_w3] = lrw->critical_usage_w2;
					lrw->critical_usage_w3 += lrw->critical_usage_w2;
					//current bucket is full here

				}
				idle_cycles_w3 -= idle_hist_w3[hist_index_w3];
				idle_hist_w3[hist_index_w3] = idle_cycles_w2;
				idle_cycles_w3 += idle_cycles_w2;
				//current bucket is full here
						
				hist_index_w3++;
				if (hist_index_w3 >= MAX_W3_BUCKETS) hist_index_w3 = 0; 
			}
	}// endif curr_hist_index > ts_buckets 
	
    /* meanwhile,back at the scheduling window, time to rotate it by one bucket by removing times of the last slot from
	 * the running totals, and then making the last slot the current slot 
	 */
	INTR_LOCK(&aps_lock);
	//rotate the scheduling window and check for wrap
	if(++cur_hist_index >= ts_buckets ) cur_hist_index =0; 
	for(i = 0; i < num_ppg; i++) {
	    ppg = actives_ppg[i]; 		
       

		//current bucket is full here
		ppg->critical_cycles_used -= ppg->critical_usage_hist[cur_hist_index];
		ppg->critical_usage_hist[cur_hist_index]=0;
		

		//current bucket is full here
		//so save the total so we can report a constant total usage for the whole window over the duration of the next
		//tick where we will empty the current bucket and slolwy (with repeated microbill()s) fill itup. 
		ppg->used_cycles_lasttick = ppg->used_cycles; 
		ppg->used_cycles -= ppg->usage_hist[cur_hist_index];
		ppg->usage_hist[cur_hist_index] = 0;
	}
	//current bucket is full here
	//save total so we can report it during the next tick when the current bucket will be empty or filling.
	idle_cycles_lasttick = idle_cycles; 
	idle_cycles -= idle_hist[cur_hist_index];
	idle_hist[cur_hist_index]=0; 
	sched_window_rotations++;
	INTR_UNLOCK(&aps_lock);

	/* Bankrupcy handling */ 
	
	//to minumize bankruptcy checking overhead, only check when current thread is billed critical
	if (act->sched_flags & AP_SCHED_BILL_AS_CRIT) {
		//Only bother checking the partitions of running threads 
		ppg = (PPG*)(act->dpp);
		if(IS_BANKRUPT(ppg)) {
			bankruptcy_detected = 1; 
			handle_bankruptcy(act,ppg);
		}
	}
	

	//We should now check if there is a better candidate to run. That can happen if the current thread has just
	//caused its AP to run out of budget. In that case we should terminate the current thread's timeslice. 
	//
	//If we don't, APs with small budgets, ex <4% with a 100ms window, get more time than they should.
	//
	//However, that would require us to run the full (slow) scheduling algorithim. Instead, here is a quick approximation:
	//Tell nano_clock.c to terminate the current timeslice if the current thread is not critical and if it's AP is out
	//of budget.
	if(NUM_PROCESSORS > 1) {
		return 1;
	} else {
		return (!MAY_SCHEDULE((PPG*)(act->dpp)) && !THREAD_IS_RUNNING_CRITICAL(act) ) || bankruptcy_detected ;
	}
}


__SRCVERSION("aps_time.c $Rev: 163913 $"); 

