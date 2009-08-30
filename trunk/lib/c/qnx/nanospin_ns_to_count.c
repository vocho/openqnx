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




#include <time.h>
#include <inttypes.h>
#include <sys/neutrino.h>
#include <sys/syspage.h>
#include <errno.h>
#include "cpucfg.h"

static unsigned	overhead;
static unsigned hundred_loop_time;

int
nanospin_calibrate(int disable) {
	unsigned	start;
	unsigned	cycle_ticks;
	unsigned	cps;
	unsigned	ticks1;
	unsigned	ticks2;
	unsigned	ns1;
	unsigned	ns2;
	unsigned	t100;
	unsigned	count1;
	unsigned	count2;
	int			ratio;
	int			over;
	unsigned	tries;
	unsigned	runmask;

	//
	// Lock us to one CPU while calibrating
	//
	if(_syspage_ptr->num_cpu > 1) {
		runmask = 1;
		if(ThreadCtl(_NTO_TCTL_RUNMASK_GET_AND_SET, &runmask) == -1) {
			return errno;
		}
	}

	tries = 10;
	do {
		if(--tries == 0) {
			return EINTR;
		}
		start = CONDITION_CYCLES(ClockCycles());
		cycle_ticks = CONDITION_CYCLES(ClockCycles()) - start;

		// Load up cache
		nanospin_count(1);
		
		count1 = 10;
		do {
			count1 *= 10;
			if(disable){
				InterruptDisable();
			}
			start = CONDITION_CYCLES(ClockCycles());
			nanospin_count(count1);
			ticks1 = (CONDITION_CYCLES(ClockCycles()) - start) - cycle_ticks;
			if(disable) {
				InterruptEnable();
			}
		} while(ticks1 < 1000);
		count2 = count1 * 10;
	
		if(disable){
			InterruptDisable();
		}
		start    = CONDITION_CYCLES(ClockCycles());
		nanospin_count(count2);
		ticks2 = (CONDITION_CYCLES(ClockCycles()) - start) - cycle_ticks;
		if(disable){
			InterruptEnable();
		}

		//
		// If the ratio between the first and second loop is too far out
		// of whack, assume we got interrupted and try again.
		//
		ratio = ticks1 * 10;
		if(ratio >= ticks2)
			ratio -= ticks2;
		else
			ratio = ticks2 - ratio;
	} while(ratio > ticks1);

	//
	// Can run on all CPU's that we previously could now.
	//
	if(_syspage_ptr->num_cpu > 1) {
		if(ThreadCtl(_NTO_TCTL_RUNMASK_GET_AND_SET, &runmask) == -1) {
			return errno;
		}
	}

	cps = CONDITION_CYCLES(SYSPAGE_ENTRY(qtime)->cycles_per_sec);
    ns1 = (ticks1 * (uint64_t)1000000000) / cps;
    ns2 = (ticks2 * (uint64_t)1000000000) / cps;

    /*
     * Time for 100 loop counts = ((ns2 - ns1) * 100) / (count2 - count1)
	 * The multiply is by '90' to give some slop in the calculation, so
	 * we don't undershoot.
     */
    t100 = ((ns2 - ns1) * (100-10)) / (count2 - count1);

    hundred_loop_time = t100;

    /*
     * Overhead: the time it takes to get into the count loop
	 *	overhead should not be too big; otherwise, would make small delay time not available.
     */
	over = (ns1 * 10 - ns2) / (10 - 1) ;
    if( (over > 0) && (over < 1000) ) {
        overhead = over;
    }

	return EOK;
}

unsigned long
nanospin_ns_to_count(unsigned long nsec) {
	unsigned long	loops;
	int				err;

	if(hundred_loop_time == 0) {
		err = nanospin_calibrate(0);
		if(err != EOK) {
			errno = err;
			return -1UL;
		}
	}
	if(nsec <= overhead) return 1;
	loops = ((nsec - overhead) * 100) / hundred_loop_time;
	if(loops == 0) loops = 1;
	return loops;
}

__SRCVERSION("nanospin_ns_to_count.c $Rev: 200568 $");
