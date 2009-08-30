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
 * aps_time.h 
 * 
 * Adaptive Partitioning scheduling 
 *
 * low_level timing function, microbill, and our hook into the clock interrupt  handler, ppg_tick_hook()
 *
 * implemented in aps_time.c 
 * 
 */



/* all timing calculations are done in ClockCyles() divided by a constant factor to control the number of signifcant
 * bits used. All variables with '_cycles' in their names nave been conditioned. Variables with _raw in their names
 * are not conditioned */
#define CONDITION_CC(clock_cycles) ((clock_cycles) >> CLOCKCYCLES_INCR_BIT)

/* a threshold for detecting an error in microbill() */ 
extern uint64_t		max_microbill_cycles; 

extern uint32_t		cycles_left_in_tick; /* only valid if microbill() has been recently called */ 

__SRCVERSION("aps_time.h $Rev: 153052 $"); 

