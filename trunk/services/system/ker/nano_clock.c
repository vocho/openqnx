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
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA


    Authors: Jerome Stadelmann (JSN), Pietro Descombes (PDB), Daniel Rossier (DRE)
    Emails: <firstname.lastname@heig-vd.ch>
    Copyright (c) 2009 Reconfigurable Embedded Digital Systems (REDS) Institute from HEIG-VD, Switzerland
*/

#include "externs.h"
#include "mt_kertrace.h"

#define RESCHED_TIMESLICE 0x1	/* Timeslice is exhausted */
#define RESCHED_SCHEDULE  0x2	/* Scheduling parameters changed globally */
#define RESCHED_THREAD    0x4	/* This thread needs to be re-scheduled */

// See clock_start() for details on when we can get rid of these temp
// variables
static volatile uint64_t	stable;
volatile uint64_t			*nssptr = &stable;

const struct sigevent *
clock_handler(void *dummy, int id) {
	QTIME		*qtp = qtimeptr;
	int			i;
	int			(*timer_reload)(struct syspage_entry *, QTIME *);
	int 		my_cpu;
	int			my_inkernel;
	int         hi_pri;
	unsigned	reschedl;
	unsigned	preempt_prio;
#ifdef _mt_LTT_TRACES_	/* PDB */
//	mt_TRACE_DEBUG("clk_hand");
//	mt_trace_hw_timer();
#endif
	// If the hardware doesn't keep a free-running timer or automatically
	// re-arm the interrupt, the startup code will set the timer_reload
	// callout address to non-NULL. Silly way to design the system, since
	// the routine will have to deal with clock skew, but there you go.
	// The kernel debugger also grabs this and points it to code within
	// itself when it's present - it's used for check for async stop
	// requests.
	timer_reload = calloutptr->timer_reload;
	if(timer_reload != NULL) {
		if(!timer_reload(_syspage_ptr, qtp)) return(NULL);
	}

    #if defined(COUNT_CYCLES)
		// Update cycle counter so we can emulate rdtsc on non-pentium machines.
		COUNT_CYCLES(qtp->timer_load, &cycles);
	#endif


	reschedl = 0;
	/*
	 This check is here to see if we have any new scheduling to perform.
	 If we have a replenishment due, then we need to tell the threads to
	 go into the reschedule code to sort out the replenishments.
	*/
	if(ss_replenish_list && ss_replenish_time <= (qtp->nsec + qtp->nsec_inc)) {
		reschedl |= RESCHED_SCHEDULE;
	}

	// Have to use RUNCPU since KERNCPU is not
	// set in an interrupt handler.
	my_cpu = RUNCPU;
	my_inkernel = get_inkernel();

	/* @@@ This is a quick way to hook this in, but we will likely change */
	if(scheduler_tick_hook) {
		if(scheduler_tick_hook()) {
			//aps scheduler wants us to timeslice this thread because it's partition is out of budget.
			reschedl |=RESCHED_SCHEDULE;
		}
		hi_pri = NUM_PRI;
	} else {
		hi_pri = DISPATCH_HIGHEST_PRI(actives[0]->dpp);
	}

	preempt_prio = 0;
	for(i = 0; i < NUM_PROCESSORS; ++i) {
		THREAD *act;

		// first we announce our intent to mess with this thread
		ticker_preamble = 1;
		act = actives[i];
		act->ticker_using = 1;
		MEM_BARRIER_WR();
		ticker_preamble = 0;

		// now we check to see if actives[i] got changed.
        if((act != (volatile THREAD *)actives[i]) || (act->state != STATE_RUNNING)) {
			// hmm, this thread is being mucked with by someone else so don't
			// touch him.
			act->ticker_using = 0;
			continue;
		}

		act->running_time += qtp->nsec_inc;
		act->process->running_time += qtp->nsec_inc;

		if ( kerop_clock_handler_hook != NULL ) {
			kerop_clock_handler_hook(act);
		}

		if((my_inkernel & INKERNEL_NOW) && (i == KERNCPU)
		 &&((act->process->pid != SYSMGR_PID)
		 ||(act->tid >= NUM_PROCESSORS))) {
			act->process->system_time += qtp->nsec_inc;
		}

		RR_ADD_FULLTICK(act);

		reschedl &= ~(RESCHED_THREAD|RESCHED_TIMESLICE);

		/* Remove the execution amount off this thread if appropriate */
		if(act->policy == SCHED_SPORADIC && act->schedinfo.ss_info->org_priority == 0) {
			act->schedinfo.ss_info->consumed += qtp->nsec_inc;
			//Force a scheduling change if our budget exhausted
			if(act->schedinfo.ss_info->curr_budget <= qtp->nsec_inc) {
				reschedl |= RESCHED_SCHEDULE;
				act->schedinfo.ss_info->curr_budget = 0;
			} else {
				act->schedinfo.ss_info->curr_budget -= qtp->nsec_inc;
			}
		}

		/*
		 This logic is kind of twisted.  We only guarantee on SMP systems that
		 the highest priority thread will run, secondary threads running on
		 secondary CPU's will run whenever they want.
		 We will tell a thread to re-schedule itself under the following
		 conditions:
			- The process has exhausted its timeslice and there is a
			  higher priority process ready to run and the process is
			  a RR or an OTHER policy (smp case)
			- A global re-schedule has been asked for
			- The running time of the process is up

		 old code was:
			if((hi_pri >= act->priority && act->policy != SCHED_FIFO) ||
			   (act->process->running_time >= act->process->max_cpu_time) ||
			   act->policy == SCHED_SPORADIC || resched > 1) {
		*/

		if(!nopreempt && (IS_SCHED_RR(act) && (act->schedinfo.rr_ticks >= RR_MAXTICKS)) ) {
			reschedl |= RESCHED_TIMESLICE;
		}
		if(reschedl) {
			if((reschedl & RESCHED_TIMESLICE) && hi_pri >= act->priority) {
				switch(act->policy) {
				case SCHED_RR:
				case SCHED_OTHER:
					reschedl |= RESCHED_THREAD;
					break;
				default:
					break;
				}
			}

			if((reschedl & (RESCHED_SCHEDULE | RESCHED_THREAD)) ||
			   (act->process->running_time >= act->process->max_cpu_time)) {

				if (clock_handler_hook_for_ts_preemption) {
					clock_handler_hook_for_ts_preemption(act, reschedl);
				}

				if((my_inkernel & INKERNEL_NOW) && (preempt_prio <= act->priority)) {
					preempt_prio = act->priority;
				}
				if(i == my_cpu)  {
					atomic_set((unsigned *)&act->async_flags, _NTO_ATF_TIMESLICE);
				} else {
					SENDIPI(i, IPI_TIMESLICE);
				}
			}
		}
        // indicate that it is safe for the kernel to touch this fellow
		act->ticker_using = 0;
	}

	// Increment the time.
	qtp->nsec += qtp->nsec_inc;

	// Make sure the increment is seen by all the CPU's
	MEM_BARRIER_WR();

	if(qtp->adjust.tick_count) {
		// Adjust the clock
		qtp->nsec_tod_adjust += qtp->adjust.tick_nsec_inc;
		// If an adjustment to the clock is over we restore orig nsec_inc.
		if(--qtp->adjust.tick_count == 0) {
			qtp->adjust.tick_nsec_inc = 0;
		}
		MEM_BARRIER_WR();
	}

	// Let people looking at the system page know that the time field
	// adjustments are done.
	*nssptr = qtp->nsec;

	// Check for timers expiring.
	timer_expiry(qtp);

	if(preempt_prio > 0) {
		// Somebody's in the kernel - make sure that they know to preempt
		// since we've got some timeslicing to do
		intrevent_preemption(preempt_prio + 1);
	}
	return NULL;
}


void rdecl
snap_time_wakeup(void) {
	QTIME		*qtp = qtimeptr;
	INTERRUPT	*itp;

	if((itp = interrupt_level[HOOK_TO_LEVEL(_NTO_HOOK_IDLE)].queue)) {
		THREAD	*thp;

		thp = itp->thread;
		if(thp->aspace_prp != NULL && thp->aspace_prp != aspaces_prp[KERNCPU]) {
			memmgr.aspace(thp->aspace_prp, &aspaces_prp[KERNCPU]);
		}
		hook_idle(NULL, qtp, itp);
	}
}

//
// Get the current time.
//
// If the current nsec == -1 then power management has shut down the
// clock interrupt. In this case we invoke the power management callout
// to load the current time from somewhere (rtc on a PC) and re-enable
// the clock interrupt.
//
void rdecl
snap_time(uint64_t *tsp, int incl_tod) {
	QTIME		*qtp = qtimeptr;
	int64_t		adjust;

	do {
		*tsp = qtp->nsec;
		if(*tsp == (uint64_t)-1) {
			snap_time_wakeup();
		}
		adjust = qtp->nsec_tod_adjust;
		//Try again if the time has changed under our feet.
	} while(*tsp != *nssptr);
	if(incl_tod) {
		*tsp += adjust;
	}
}

#if defined(__X86__) && defined(__WATCOMC__)

extern unsigned muldiv( unsigned long val, unsigned mul, unsigned div );
#pragma aux muldiv = 	\
	"mul	ecx"			\
	"div	ebx"			\
	parm [eax] [ecx] [ebx] modify exact [eax edx] value [eax]

#elif defined(__X86__) && defined(__GNUC__)
#define muldiv(val,mul,div) ({ register unsigned long quo, rem; __asm__( \
       "mull %3\n\t" \
       "divl %4" \
       : "=&a" (quo), "=&d" (rem) : "0" ((unsigned long)val), \
       "rm" ((unsigned)mul), "rm" ((unsigned)div)); quo; })

#elif defined(__MIPS__) \
   || defined(__PPC__) \
   || defined(__ARM__) \
   || defined(__SH__)

static unsigned muldiv( unsigned long val, unsigned mul, unsigned divl ) {

	return ((uint64_t)val * mul) / divl;
}

#elif defined(KLUDGE_IMPLEMENTATION)
	/*
		Kludge implementation until we can get libgcc.a built for
		these machines.
	*/
static unsigned muldiv( unsigned long val, unsigned mul, unsigned div ) {

	return ((uint32_t)val * mul) / div;
}

#else
    #error calc_timer_load not configured for system
#endif

#define NANO_SCALE	(-9)

static unsigned long
scale_factor(int factor) {
	unsigned long	scale = 1;

	while(factor != 0) {
		scale *= 10;
		--factor;
	}
	return(scale);
}

void rdecl
clock_resolution(unsigned long nsec) {
	QTIME			*qtp = qtimeptr;
	unsigned long	scale;
	unsigned long	timer_load;

	if((qtp->flags & QTIME_FLAG_TIMER_ON_CPU0) && (RUNCPU != 0)) {
		// Force the load on CPU 0. Ends up invoking 'clock_load()' down
		// below.
		qtp->nsec_inc = nsec;
		SENDIPI(0, IPI_CLOCK_LOAD);
	} else {
		nsec += 1;	// Correct for rounding error on an exact specification.

		if(qtp->timer_scale < NANO_SCALE) {
			scale = scale_factor(NANO_SCALE - qtp->timer_scale);
			timer_load = muldiv(nsec, scale, qtp->timer_rate);
		} else {
			scale = scale_factor(qtp->timer_scale - NANO_SCALE);
			timer_load = nsec / (qtp->timer_rate * scale);
			if(timer_load == 0) timer_load = 1;
		}
		qtp->timer_load = timer_load;
		calloutptr->timer_load(_syspage_ptr, qtp);

		/* The timer_load routine may adjust qtp->timer_load */
		if(qtp->timer_scale < NANO_SCALE) {
			nsec = muldiv(qtp->timer_load, qtp->timer_rate, scale);
		} else {
			nsec = qtp->timer_load * (qtp->timer_rate * scale);
		}

		// Disable any adjustment in effect.
		INTR_LOCK(&clock_slock);
		qtp->adjust.tick_nsec_inc = qtp->adjust.tick_count = 0;

		INTR_UNLOCK(&clock_slock);

		// Load the new resolution
		qtp->nsec_inc = nsec;

		// If the calculated tick interval has been rounded down to less than
		// what the user would be allowed to set in ClockPeriod(), lower the
		// allowed minimum so that if the user does a ClockPeriod(), saves the
		// clock tick, sets a new one and then restores to the old value,
		// it all works.
		if(nsec < tick_minimum) tick_minimum = nsec;
	}
}

//
// Used when the timer hardware is CPU specific and we have to force
// a load on CPU0 with an IPI_CLOCK_LOAD. The 'nsec_inc' value was already
// set by a previous clock_resolution() call.
//
void rdecl
clock_load() {
	clock_resolution(qtimeptr->nsec_inc);
}

static void
kerext_clock_start(void *data) {
	int		level;
	int		id;

	level = get_interrupt_level(NULL, qtimeptr->intr);
	if(level < 0) {
		kprintf("Illegal clock intr\n");
		crash();
	}
	lock_kernel();
	id = interrupt_attach(level, &clock_handler, 0, _NTO_INTR_FLAGS_PROCESS);
	if(id >= 0) {
		clock_isr = vector_lookup(&interrupt_vector, id);
		SETKSTATUS(actives[KERNCPU], 0);
		clock_resolution(*(unsigned long *)data);
		timer_period();
	}
}


void
clock_start(unsigned long nsec) {
	// Tell outside users that they should be looking at the nsec_stable field
	if(_syspage_ptr->qtime.entry_size >= (offsetof(struct qtime_entry, nsec_stable) + sizeof(uint64_t))) {
		qtimeptr->flags |= QTIME_FLAG_CHECK_STABLE;
		nssptr = &qtimeptr->nsec_stable;
	} else {
		// After a suitable period of time, we can assume (or crash if not)
		// new startups that have allocated the bigger struct qtime_entry.
		// After that, the nssptr variable can be removed and uses of
		// it can be replaced with qtimeptr->nsec_stable. 2008/08/18
	}
	if(qtimeptr->flags & QTIME_FLAG_TIMER_ON_CPU0) {
		// Make sure we do the operation on CPU0
		(void)ThreadCtl(_NTO_TCTL_RUNMASK, (void *)1);
	}
	if(__Ring0(kerext_clock_start, &nsec) < 0) {
		kprintf("clock intr failure (%d)\n", errno);
		crash();
	}
	if(qtimeptr->flags & QTIME_FLAG_TIMER_ON_CPU0) {
		// We can run on any CPU again
		(void)ThreadCtl(_NTO_TCTL_RUNMASK, (void *)LEGAL_CPU_BITMASK);
	}
}

__SRCVERSION("nano_clock.c $Rev: 212631 $");
