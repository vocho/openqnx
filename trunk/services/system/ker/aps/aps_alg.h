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
 * aps_alg.h 
 * 
 * Adaptive Partitioning scheduling 
 *
 * globals and prototypes for the aps scheduling algorithm 
 * 
 */
#include "aps_data.h" 

#define aps_debugging 0 
#if aps_debugging
#include "addebug.h"
#else
#define disp(x)
#define disp_int(x)
#endif 


extern	uint32_t min_cycles_to_be_critscheduleable; /* small fraction of a tick to correct for variations in accuracy of
											   ClockCyles() in a ms. PRxxxx */ 
extern	uint32_t	scheduling_policy_flags; 
extern	int cur_hist_index; /* increments modulo TS_BUCKETS. index into usage_hist[] critical_usage_hist[] */
extern	volatile uint32_t	sched_window_rotations; /* increments same time as cur_hist_index, but is volatie, non-modulo and is
											   for use as a retry flag for choose_thread_to_schedule.*/


/* MAY_SCHEDULE_* macros: test if a partition "has budget left". 
 * 
 * a partition is considered to have budget if it has enough to pay for the time left in the current tick.
 * On SMP machines, that would be the rest of the tick times the number of processors running or wanting to
 * run that partition. The "rest of the tick" value is computed by MIN_CYCLES_TO_SCHEDULE, which includes a 
 * small term 1/256 of tick to compensate for roundoff errors in calculating cycles_left_in_tick.
 */

#define MIN_CYCLES_TO_SCHEDULE(num_ready_processors) ((num_ready_processors)*((uint32_t)cycles_left_in_tick+(cycles_per_bucket>>8)) )


/* MAY_SCHEDULE is used by nano_aps.c for approximately checking a single partition */
//@@@ revisit for SMP: rather than NUM_PROCESSORS we should consider the number of processors currently
//running the partition we are checking
#define MAY_SCHEDULE(ppg) ((ppg)->used_cycles + MIN_CYCLES_TO_SCHEDULE(NUM_PROCESSORS) <= (ppg)->bmp_budget_in_cycles) 

/* use MAY_SCHEDULE_MIN instead of MAY_SCHEDULE where you are checking many partitions and dont want to keep
 * recalculating MIN_CYCLES_TO_SCHEDULE()
 */
#define MAY_SCHEDULE_MIN(ppg, min_cycles) ((ppg)->used_cycles +(min_cycles) <= (ppg)->bmp_budget_in_cycles) 

/* MAY_SCHEDULE_CRITICAL checks if a partition has time left in it's critical budget. */		
#define MAY_SCHEDULE_CRITICAL(ppg) ((ppg)->critical_cycles_used + min_cycles_to_be_critscheduleable <= (ppg)->critical_budget_in_cycles) 


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

extern uint32_t sb_last_run_winrot[APS_MAX_PARTITIONS][PROCESSORS_MAX]; 
extern int      sb_partition_is_ready[APS_MAX_PARTITIONS][PROCESSORS_MAX]; 

/* call SB_PARTITION_IS_RUNNING whenever starting a thread, and also when you know a thread is running. Note that
 * FIFO threads can run longer than a windowsize, so call SB_PARTITION_IS_RUNNIG on an act FIFO thread before 
 * calling choose_thread_to_schedule
 */
#define SB_PARTITION_IS_RUNNING(ap) (sb_last_run_winrot[ap][KERNCPU] = sched_window_rotations)
#define SB_PARTITION_IS_READY(ap) (sb_partition_is_ready[ap][KERNCPU] = 1) 
#define SB_PARTITION_IS_NOT_READY(ap) (sb_partition_is_ready[ap][KERNCPU] = 0)  


__SRCVERSION("aps_alg.h $Rev: 153052 $"); 

